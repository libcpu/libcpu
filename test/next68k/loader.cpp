/* loader.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mach-o/loader.h>
#include <mach-o/fat.h>
#include <mach-o/nlist.h>

#define DEBUG 1
#define debug_printf printf

#include "endian.h"
#include "loader.h"

/* Globals */
LdrLLFunctions *LdrLibFunctions;

void *my_malloc(size_t size) {
	void *p;
	// fprintf(stderr, "allocing size: %d\n", size);
	p = malloc(size);
	if (!p) {
		fprintf(stderr, "Malloc failed!\n");
		exit(1);
	}
	return p;
}

/* LdrLoadMachO loads a MachO binary into memory including the dynamic shared libraries */
LdrResult_t *LdrLoadMachO (char *filename, char *strentrypoint, char *RAM) {

	FILE *machfile = fopen(filename, "rb");
	if(!machfile)
		return 0;

	/* Validate file to be MachO */
	LdrMagic_t magic = LdrValidateFile(machfile);

	if(!magic)
		return 0;

	/* Initialize the current file pointer to where the PPC definition is */
	unsigned long fp;
	if(magic == MH_MAGIC || magic == MH_CIGAM) {
		/* We are loading a thin file, mach header is at position 0 */
		fp = 0;
		
		/* Check if the thin file is for PPC */
		// if(!LdrCheckHdrCpuType(CPU_TYPE_POWERPC, 0, machfile))
		if(!LdrCheckHdrCpuType(CPU_TYPE_MC680x0, 0, machfile))
			return 0;
		}
	else {
		/* We are loading a fat file, let's find out where the mach header is */
		// fp = LdrGetArchOffset(CPU_TYPE_POWERPC, machfile);
		fp = LdrGetArchOffset(CPU_TYPE_MC680x0, machfile);
		
		/* Exit if LdrGetArchOffset couldn't find the specified architecture definition */
		if (!fp)
			return 0;
		}


	/* Get the mach header at offset fp */
	struct mach_header mach_header;
	if(!LdrGetMachHeader(&mach_header, fp, machfile))
		return 0;
		
	/* Now that the header stuff is done, start loading the file into memory */
	/* buf holds the base address of all allocated memory, buflen holds the length of the buffer buf points to  */
	void *buf=RAM;
	unsigned long buflen=0;
	
	/* This is where we store the address of the requested entrypoint in */
	unsigned long reqentrypoint = 0;
	unsigned long textentrypoint = 0;
	
	/* Load the load commands into memory (requires less disk operations) */
	struct load_command *load_commands_buf = (struct load_command*) my_malloc(mach_header.sizeofcmds);
	// debug_printf("Load-commands my_malloced at %#lx\n", load_commands_buf);
	struct load_command *current_cmd = load_commands_buf;
	if(!load_commands_buf)
		return 0;
	
	if(fseek(machfile, fp+sizeof(mach_header), SEEK_SET))
		return 0;
		
	if(!fread(load_commands_buf, mach_header.sizeofcmds, 1, machfile))
		return 0;
		
	/* At the moment, loading a file is done in two steps:
	   1. Load all data according to LC_SEGMENT and LC_LOAD_DYLIB commands
	   2. Patch calls to libraries in a second pass */
	unsigned int i;
	struct nlist *symtab = NULL;
	char *strtab = NULL;
	uint32_t *idrsymtab= NULL;
	for(i=0;i<mach_header.ncmds;i++) {
		switch(BE32_toHost(current_cmd->cmd)) {
			case LC_SEGMENT:
				if(!LdrLoadSegment((struct segment_command*)current_cmd, &buf, &buflen, machfile, fp, 0))
					return 0;
				break;
			case LC_LOAD_DYLIB:
				if(!LdrLoadDylib((struct dylib_command*)current_cmd, &buf, &buflen))
					return 0;
				break;
			case LC_SYMTAB:
				symtab = LdrLoadSymtab((struct symtab_command*)current_cmd, fp, machfile);
#if 0
				if(!symtab)
					return 0;
#endif
				strtab = LdrLoadStrtab((struct symtab_command*)current_cmd, fp, machfile);
#if 0
				if(!strtab)
					return 0;
#endif
				break;
			case LC_DYSYMTAB:
				idrsymtab = LdrLoadIdrSymtab((struct dysymtab_command*)current_cmd, fp, machfile);
				if(!idrsymtab)
					return 0;
				break;
			}

		/* Set current to the next command, casting done because cmdsize is in bytes */
		current_cmd = (struct load_command*)((unsigned char*)current_cmd + BE32_toHost(current_cmd->cmdsize));
	}
	
	/* Now that everything has been loaded, append a __SoftPear Segment */
	/* This contains instructions recognized by the interpreter to call functions provided by the local environment */
	if(!LdrLoadNativeLibJumps(&buf, &buflen))
		return 0;

	/* Show which library functions have been loaded */
	/* TODO: Remove LdrLLLibFunctionsPrint() completely, when loader works */
	LdrLLLibFunctionsPrint();
	
	/* Prepare for the second pass */
	current_cmd = load_commands_buf;
	for(i=0;i<mach_header.ncmds;i++) {
		switch(BE32_toHost(current_cmd->cmd)) {
			case LC_SEGMENT:
				LdrFakeDyld((struct segment_command*)current_cmd, buf, symtab, strtab, idrsymtab);
				if(!textentrypoint)
					textentrypoint = LdrGetTextText((struct segment_command*)current_cmd, buf);
				if(!LdrBindSegment((struct segment_command*)current_cmd, symtab, strtab, idrsymtab, buf, 0))
					return 0;
				break;
			case LC_SYMTAB:
				/* Second pass of faking dyld */
				LdrFakeDyld2((struct symtab_command*)current_cmd, buf, fp, machfile);
				/* Find the requested entrypoint */
				if(strentrypoint)
					reqentrypoint = LdrGetEntrypoint((struct symtab_command*)current_cmd, strentrypoint, fp, machfile);
				break;
			}

		/* Set current to the next command, casting done because cmdsize is in bytes */
		current_cmd = (struct load_command*)((unsigned char*)current_cmd + BE32_toHost(current_cmd->cmdsize));
	}

	if(!reqentrypoint && !textentrypoint)
		return 0;				

	free(load_commands_buf);
	
	fclose(machfile);
	
	/* Free symtab and strtab */
	// free(symtab);
	// free(strtab);
	// free(idrsymtab);
	
	/* Clear linked list of library functions */
	LdrLLLibFunctionsClearList();
	
	LdrResult_t *result = (LdrResult_t*) my_malloc(sizeof(LdrResult_t));
	// debug_printf("LdrResult my_malloced at %#lx\n", result);

	result->base = buf;
	result->entrypoint = reqentrypoint?reqentrypoint:textentrypoint;
	return (LdrResult_t*)result->entrypoint;
}

/* FakeDyld - pretend that dyld has worked on the file */
void LdrFakeDyld(struct segment_command *segment_command, void *buf, struct nlist *symtab, char *strtab, uint32_t *idrsymtab) {
	/*
	For faking the dyld, we do two things at the moment:
		1) add a non-zero value to the first long in __DATA, __data (this is checked by the code in start.s)
		2) insert references into our linked list to dyld functions so that LdrBindSegment does not complain about them as a missing reference
	*/
	
	
	if(strcmp(segment_command->segname, "__DATA")==0) {
	/* Go through sections and search for __dyld */
		unsigned int i;
		struct section *sections = (struct section*)(segment_command+1);
		for (i=0; i<segment_command->nsects; i++) {
		
			/* A list of section structs follows right after the segment_command */
			struct section *current_sect = &sections[i];
			
			if(strcmp(current_sect->sectname, "__dyld")==0) {
				/* The first long in the dyld-section is the dyld_lazy_symbol_binding_entry_point which should be nonzero */
				*((uint32_t*)&(((unsigned char*)buf)[current_sect->addr])) = 0xCAFEAFFE;

				/* The second long in the dyld-section is the dyld_func_lookup_pointer */
				uint32_t *noopfunc = (uint32_t*)my_malloc(sizeof(uint32_t));   // size of blr is 32 bits
				*noopfunc = BE32_toHost(0x4e800020); // OpCode blr

				uint32_t *dyld_func_lookup = (uint32_t*)my_malloc(5*sizeof(uint32_t));
				dyld_func_lookup[0] = BE32_toHost(0x38600000);   // OpCode li r3, 0x0
				//XXX potential bit loss for 64 bit?
				dyld_func_lookup[1] = BE32_toHost(0x3c630000 | (((uintptr_t)noopfunc & 0xFFFF0000)>>16));   // OpCode addis r3, r3, hi16(noopfunc)
				dyld_func_lookup[2] = BE32_toHost(0x60630000 | ((uintptr_t)noopfunc & 0x0000FFFF));   // OpCode ori r3, r3, lo16(noopfunc)
				dyld_func_lookup[3] = BE32_toHost(0x90640000);   // OpCode stw r3, 0x0(r4)
				dyld_func_lookup[4] = BE32_toHost(0x4e800020);   // OpCode blr

				//XXX potential bit loss for 64 bit?
				*((uint32_t*)&(((unsigned char*)buf)[current_sect->addr+0x4])) = BE32_toHost((uintptr_t)dyld_func_lookup);
			}
			
			if(strcmp(current_sect->sectname, "__data")==0) {
			/* Set the value at __DATA, __data + 0x14 to zero (Note, this is the so called _pointer_to__darwin_preregister_frame_info!)
			   The problem is that some binaries don't list this entry in their symbol table but it seems to located always at this
			   position. If this causes trouble, we have to find a better way */
				*((uint32_t*)&(((unsigned char*)buf)[current_sect->addr+0x14])) = 0x0;
			/* Same thing like above but this time with _pointer_to_objcInit at __DATA, __data + 0x10 */
				*((uint32_t*)&(((unsigned char*)buf)[current_sect->addr+0x10])) = 0x0;
			}
			
#if 0
			uint32_t section_type = current_sect->flags & SECTION_TYPE;
			if(section_type == S_NON_LAZY_SYMBOL_POINTERS) {
				int n = current_sect->reserved1;
				unsigned int j;
				for(j=0; j<(current_sect->size/sizeof(uint32_t)); j++) {
					uint32_t patchaddr = current_sect->addr + j*sizeof(uint32_t);
					char *funcname = strtab + BE32_toHost(symtab[BE32_toHost(idrsymtab[j+n])].n_un.n_strx);
					
					/* This should fix refences to "stdin", "stdout", "stderr"... nativeSF is declared in nativelib.cc */
					if(strcmp(funcname, "___sF") == 0) {
						debug_printf("Patched ___sF to %#lx!\n", nativeSF);
						*((uint32_t*)&(((unsigned char*)buf)[patchaddr])) = BE32_toHost((uint32_t)nativeSF);
					}

					/* This should prevent us from calling mach_init_routine during start */
					if(strcmp(funcname, "_mach_init_routine")==0) {
						debug_printf("Patched _mach_init_routine to %#lx!\n", 0);
						*((uint32_t*)&(((unsigned char*)buf)[patchaddr])) = 0;
					}
					
					/* This should prevent us from calling __cthread_init_routine during start */
					if(strcmp(funcname, "__cthread_init_routine")==0) {
						debug_printf("Patched __cthread_init_routine to %#lx!\n", 0);
						*((uint32_t*)&(((unsigned char*)buf)[patchaddr])) = 0;
					}
					
					/* Sometimes, errno is referenced from the non-lazy-symbol pointers - don't ask me why
					TODO: Right now, we patch it to zero, it should be patched to something useful */
					if(strcmp(funcname, "_errno")==0) {
						debug_printf("Patched _errno to %#lx!\n", 0);
						*((uint32_t*)&(((unsigned char*)buf)[patchaddr])) = 0;
					}
					
					/* Same as above (TODO: Find a generic way to patch this stuff referenced via non-lazy symbol pointers */
					if(strcmp(funcname, "__CurrentRuneLocale")==0) {
						debug_printf("Patched __CurrentRuneLocale to %#lx!\n", &native_CurrentRuneLocale);
						*((uint32_t*)&(((unsigned char*)buf)[patchaddr])) = Host_to_BE32((uint32_t)&native_CurrentRuneLocale);
					}
					/*if(strcmp(funcname, "__DefaultRuneLocale")==0) {
						debug_printf("Patched __DefaultRuneLocale to %#lx!\n", &_DefaultRuneLocale);
						*((unsigned long*)&(((unsigned char*)buf)[patchaddr])) = (unsigned long)&_DefaultRuneLocale;
					}*/
					
					if(strcmp(funcname, "_optarg")==0) {
						debug_printf("Patched _optarg to %#lx!\n", &Darwin_optarg);
						*((uint32_t*)&(((unsigned char*)buf)[patchaddr])) = Host_to_BE32((uint32_t)&Darwin_optarg);
					}
					if(strcmp(funcname, "_optind")==0) {
						debug_printf("Patched _optind to %#lx!\n", &Darwin_optind);
						*((uint32_t*)&(((unsigned char*)buf)[patchaddr])) = Host_to_BE32((uint32_t)&Darwin_optind);
					}
					if(strcmp(funcname, "_opterr")==0) {
						debug_printf("Patched _opterr to %#lx!\n", &Darwin_opterr);
						*((uint32_t*)&(((unsigned char*)buf)[patchaddr])) = Host_to_BE32((uint32_t)&Darwin_opterr);
					}
					if(strcmp(funcname, "_optopt")==0) {
						debug_printf("Patched _optopt to %#lx!\n", &Darwin_optopt);
						*((uint32_t*)&(((unsigned char*)buf)[patchaddr])) = Host_to_BE32((uint32_t)&Darwin_optopt);
					}
				}
			}
#endif
		}
		
		LdrLLLibFunctionsAppend("__dyld_register_func_for_remove_image", 0);
		LdrLLLibFunctionsAppend("__dyld_register_func_for_add_image", 0);
		LdrLLLibFunctionsAppend("__init_keymgr", 0);
		LdrLLLibFunctionsAppend("__keymgr_set_and_unlock_processwide_ptr", 0);
		LdrLLLibFunctionsAppend("__keymgr_get_and_lock_processwide_ptr", 0);
		
		LdrLLLibFunctionsAppend("_mach_init_routine", 0);
		LdrLLLibFunctionsAppend("__cthread_init_routine", 0);
		LdrLLLibFunctionsAppend("___keymgr_global",0);

		/* Unfortunately, we cannot set the reference to ___keymgr_dwarf2_register_sections to zero,
		   because the initialization code jumps in there either way. Therefore, we set up a no-op
		   function and link the reference there */
		uint32_t *noopfunc = (uint32_t*)my_malloc(sizeof(uint32_t));   // size of blr is 32 bits
		*noopfunc = BE32_toHost(0x4e800020); // OpCode blr
		LdrLLLibFunctionsAppend("___keymgr_dwarf2_register_sections", (unsigned long)noopfunc);

	}
}

/* This is a second pass of faking the dynamic linker which workd on an LC_SYMTAB command */
void LdrFakeDyld2(struct symtab_command *symtab_command, void *buf, unsigned long fp, FILE *machfile) {
	
	struct nlist *symtable = (struct nlist*) my_malloc(sizeof(struct nlist) * BE32_toHost(symtab_command->nsyms));
	// debug_printf("1 symtable my_malloced at %#lx\n", symtable);
	char *strtable = (char*) my_malloc(sizeof(char) * BE32_toHost(symtab_command->strsize));
	// debug_printf("1 strtable my_malloced at %#lx\n", strtable);
	
	fseek(machfile, fp+BE32_toHost(symtab_command->symoff), SEEK_SET);
	fread(symtable, sizeof(struct nlist), BE32_toHost(symtab_command->nsyms), machfile);
	
	fseek(machfile, fp+BE32_toHost(symtab_command->stroff), SEEK_SET);
	fread(strtable, BE32_toHost(symtab_command->strsize), 1, machfile);
/*
	unsigned int i;
	for(i=0; i<BE32_toHost(symtab_command->nsyms); i++) {
		// TODO: This is currently a no-op, since all faking is done in LdrFakeDyld
	}
*/
//	free(symtable);
//	free(strtable);
}

/* Find the address of __TEXT, __text as an alternative entrypoint instead of _main */
unsigned long LdrGetTextText(struct segment_command *segment_command, void *buf) {

	if(strcmp(segment_command->segname, "__TEXT")==0) {
	/* Go through sections and search for lazy symbol pointers */
		unsigned int i;
		struct section *sections = (struct section*)(segment_command+1);
		for (i=0; i<segment_command->nsects; i++) {
			/* A list of section structs follows right after the segment_command */
			struct section *current_sect = &sections[i];
			if(strcmp(current_sect->sectname, "__text")==0)
				return current_sect->addr;
		}
	}
	
	return 0;
}

/* Bind a segment (fix calls to external libraries) */
int LdrBindSegment(struct segment_command *segment_command, struct nlist *symtab, char *strtab, uint32_t *idrsymtab, void *buf, unsigned long liboffset) {
	/* TODO: liboffset seems not to be needed */

	/* Note that the segment_command structure should already be in the host's endianess from LdrLoadSegment */

	/* Go through sections and search for lazy symbol pointers */
	unsigned int i;
	struct section *sections = (struct section*)(segment_command+1);
	for (i=0; i<segment_command->nsects; i++) {
		
		/* A list of section structs follows right after the segment_command */
		struct section *current_sect = &sections[i];
		
		uint32_t section_type = current_sect->flags & SECTION_TYPE;
		if(section_type == S_LAZY_SYMBOL_POINTERS) {
			/* This is the section to patch */
			int n = current_sect->reserved1;
			unsigned int j;
			for(j=0; j<(current_sect->size/sizeof(uint32_t)); j++) {
				uint32_t patchaddr = current_sect->addr + j*sizeof(uint32_t);
				char *funcname = strtab + BE32_toHost(symtab[BE32_toHost(idrsymtab[j+n])].n_un.n_strx);
				
				/* Go through linked list of libfunctions and find the one to patch */
				LdrLLFunctions *current = LdrLibFunctions;
				while(current) {
					if(strcmp(funcname, current->name)==0) {
						/* I'm sure this can be done in an easier way... But for now, it works */
						 *((uint32_t*)&(((unsigned char*)buf)[patchaddr])) = BE32_toHost(current->offset);
						 debug_printf("Linking %s...\n", funcname);
						 break;
					}
					current = current->next;
				}
				
				if(!current) {
					/* The file couldn't be linked completely */
					printf("Warning: Could not dylink %s!\n", funcname);
				}
			}

		}
	}

	return 1;
}

/* Get the indirect symbol table from a given file */
uint32_t* LdrLoadIdrSymtab(struct dysymtab_command *dysymtab_command, unsigned long fp, FILE *machfile) {

	uint32_t *idrsymtable = (uint32_t*) my_malloc(sizeof(uint32_t) * BE32_toHost(dysymtab_command->nindirectsyms));
	// debug_printf("idrsymtable my_malloced at %#lx\n", idrsymtable);
	if(fseek(machfile, fp + BE32_toHost(dysymtab_command->indirectsymoff), SEEK_SET))
		return NULL;
	if(!fread(idrsymtable, sizeof(uint32_t), BE32_toHost(dysymtab_command->nindirectsyms), machfile))
		return NULL;
	
	return idrsymtable;
}

/* Get the symbol table from a given file */
struct nlist *LdrLoadSymtab(struct symtab_command *symtab_command, unsigned long fp, FILE *machfile) {

	struct nlist *symtable = (struct nlist*) my_malloc(sizeof(struct nlist) * BE32_toHost(symtab_command->nsyms));
	// debug_printf("2 symtable my_malloced at %#lx\n", symtable);
#if 0
	if(fseek(machfile, fp + BE32_toHost(symtab_command->symoff), SEEK_SET))
		return NULL;
	if(!fread(symtable, sizeof(struct nlist), BE32_toHost(symtab_command->nsyms), machfile))
		return NULL;
#endif
	
	return symtable;
}

/* Get the string table from a given file */
char *LdrLoadStrtab(struct symtab_command *symtab_command, unsigned long fp, FILE *machfile) {
	char *strtable = (char*) my_malloc(sizeof(char) * BE32_toHost(symtab_command->strsize));
	// debug_printf("4 strtable my_malloced at %#lx\n", strtable);
	if(fseek(machfile, fp + BE32_toHost(symtab_command->stroff), SEEK_SET))
		return NULL;
	if(!fread(strtable, BE32_toHost(symtab_command->strsize), 1, machfile))
		return NULL;
	
	return strtable;
}

/* Append list of a library's functions to the global list */
void LdrLibGetSymbolsSymtab(struct symtab_command *symtab_command, unsigned long fp, unsigned long liboffset, FILE *machfile) {
	
	struct nlist *symtable = (struct nlist*) my_malloc(sizeof(struct nlist) * BE32_toHost(symtab_command->nsyms));
	// debug_printf("3 symtable my_malloced at %#lx\n", symtable);
	char *strtable = (char*) my_malloc(sizeof(char) * BE32_toHost(symtab_command->strsize));
	// debug_printf("3 strtable my_malloced at %#lx\n", strtable);
	
	fseek(machfile, fp+BE32_toHost(symtab_command->symoff), SEEK_SET);
	fread(symtable, sizeof(struct nlist), BE32_toHost(symtab_command->nsyms), machfile);
	
	fseek(machfile, fp+BE32_toHost(symtab_command->stroff), SEEK_SET);
	fread(strtable, BE32_toHost(symtab_command->strsize), 1, machfile);
	
	unsigned int i;
	for(i=0; i<BE32_toHost(symtab_command->nsyms); i++) {
		if(symtable[i].n_type & 0x01) {
			LdrLLLibFunctionsAppend(&strtable[BE32_toHost(symtable[i].n_un.n_strx)], BE32_toHost(symtable[i].n_value)+liboffset);
		}
	}
	
//	free(symtable);
//	free(strtable);
}

/* Seek the symbol table for the requested entrypoint */
uint32_t LdrGetEntrypoint(struct symtab_command *symtab_command, char *strentrypoint, unsigned long fp, FILE *machfile) {
	
	struct nlist *symtable = (struct nlist*) my_malloc(sizeof(struct nlist) * BE32_toHost(symtab_command->nsyms));
	// debug_printf("4 symtable my_malloced at %#lx\n", symtable);
	char *strtable = (char*) my_malloc(sizeof(char) * BE32_toHost(symtab_command->strsize));
	// debug_printf("2 strtable my_malloced at %#lx\n", strtable);
	
	fseek(machfile, fp+BE32_toHost(symtab_command->symoff), SEEK_SET);
	fread(symtable, sizeof(struct nlist), BE32_toHost(symtab_command->nsyms), machfile);
	
	fseek(machfile, fp+BE32_toHost(symtab_command->stroff), SEEK_SET);
	fread(strtable, BE32_toHost(symtab_command->strsize), 1, machfile);
	
	uint32_t result = 0;

	unsigned int i;
	for(i=0; i<BE32_toHost(symtab_command->nsyms); i++)
		if(strcmp(strentrypoint, &strtable[BE32_toHost(symtable[i].n_un.n_strx)])==0)
			result = BE32_toHost(symtable[i].n_value);
	
//	free(symtable);
//	free(strtable);
	
	return result;
}

/* Load a dynamic shared library. */
int LdrLoadDylib (struct dylib_command *dylib_command, void **buf, unsigned long *buflen) {

	char *dylib_filename = (char *)dylib_command+BE32_toHost(dylib_command->dylib.name.offset);

	/* Ignore libSystem.B.dylib */
	if(strcmp(dylib_filename, "/usr/lib/libSystem.B.dylib")==0)
		return 1;
	
	/* For testing: ignore /usr/lib/system/libmathCommon.A.dylib */
	if(strcmp(dylib_filename, "/usr/lib/system/libmathCommon.A.dylib")==0) {
		printf("Warning: libmathCommon.A.dylib was not loaded!\n");
		return 1;
	}
	
	/* For testing: ignore /usr/lib/libncurses.5.dylib */
	if(strcmp(dylib_filename, "/usr/lib/libncurses.5.dylib")==0) {
		printf("Warning: libncurses.5.dylib was not loaded!\n");
		return 1;
	}
		
	debug_printf("Loading library: %s\n", dylib_filename);

	/* TODO: Version check of library */
	
	/* Open the specified file */
	FILE *dylibfile = fopen(dylib_filename, "rb");
	if(!dylibfile)
		return 0;
	
	/* Validate file to be MachO */
	LdrMagic_t libmagic = LdrValidateFile(dylibfile);
	if(!libmagic)
		return 0;

	/* Initialize the current file pointer to where the PPC definition is */
	unsigned long libfp;
	if(libmagic == MH_MAGIC || libmagic == MH_CIGAM) {
		/* We are loading a thin file, mach header is at position 0 */
		libfp = 0;
		
		/* Check if the thin file is for PPC */
		if(!LdrCheckHdrCpuType(CPU_TYPE_POWERPC, 0, dylibfile) &&
		   !LdrCheckHdrCpuType(CPU_TYPE_MC680x0, 0, dylibfile))
			return 0;
		}
	else {
		/* We are loading a fat file, let's find out where the mach header is */
		// libfp = LdrGetArchOffset(CPU_TYPE_POWERPC, dylibfile);
		libfp = LdrGetArchOffset(CPU_TYPE_MC680x0, dylibfile);
		
		/* Exit if LdrGetArchOffset couldn't find the specified architecture definition */
		if (!libfp)
			return 0;
		}

	/* Get the mach header at offset fp */
	struct mach_header mach_header;
	if(!LdrGetMachHeader(&mach_header, libfp, dylibfile))
		return 0;
		
	/* Load the load commands into memory (requires less disk operations) */
	struct load_command *load_commands_buf = (struct load_command*) my_malloc(mach_header.sizeofcmds);
	struct load_command *current_cmd = (struct load_command*) load_commands_buf;
	if(!load_commands_buf)
		return 0;
	
	if(fseek(dylibfile, libfp+sizeof(mach_header), SEEK_SET))
		return 0;
		
	if(!fread(load_commands_buf, mach_header.sizeofcmds, 1, dylibfile))
		return 0;

	/* liboffset is the offset for the library in memory.
	   If you do not add an additional offset, your library
	   data will be loaded replacing the previously loaded data
	   of the application.
	   Alignment will be a multiple of 0x1000 */
	unsigned long liboffset = *buflen;
	while(liboffset%0x1000 != 0)
		liboffset++;

	unsigned int i;
	struct nlist *symtab = NULL;
	char *strtab = NULL;
	uint32_t *idrsymtab = NULL;

	for(i=0;i<mach_header.ncmds;i++) {
		/* We'll go thorugh the load commands and try to handle them */
		switch(BE32_toHost(current_cmd->cmd)) {
			case LC_SEGMENT:
				fprintf(stderr, "LC_SEGMENT: %p %ld\n", current_cmd, *buflen);
				if(!LdrLoadSegment((struct segment_command*)current_cmd, buf, buflen, dylibfile, libfp, liboffset))
					return 0;				
				break;
			case LC_LOAD_DYLIB:
				if(!LdrLoadDylib((struct dylib_command*)current_cmd, buf, buflen))
					return 0;
				break;
			case LC_ID_DYLIB:
				/* TODO: Check if the library we're trying to load is compatible to the one requested */
				break;
			case LC_SYMTAB:
				LdrLibGetSymbolsSymtab((struct symtab_command*)current_cmd, libfp, liboffset, dylibfile);

				symtab = LdrLoadSymtab((struct symtab_command*)current_cmd, libfp, dylibfile);
#if 0
				if(!symtab)
					return 0;
#endif
				strtab = LdrLoadStrtab((struct symtab_command*)current_cmd, libfp, dylibfile);
#if 0
				if(!strtab)
					return 0;
#endif

				break;
			case LC_DYSYMTAB:
				idrsymtab = LdrLoadIdrSymtab((struct dysymtab_command*)current_cmd, libfp, dylibfile);
				if(!idrsymtab)
					return 0;
				break;
			}

		/* Set current to the next command, casting done because cmdsize is in bytes */
		current_cmd = (struct load_command*)((unsigned char*)current_cmd + BE32_toHost(current_cmd->cmdsize));
	}
	
	/* Prepare for the second pass */
	current_cmd = load_commands_buf;
	for(i=0;i<mach_header.ncmds;i++) {
		switch(BE32_toHost(current_cmd->cmd)) {
			case LC_SEGMENT:
				if(!LdrBindSegment((struct segment_command*)current_cmd, symtab, strtab, idrsymtab, *buf, liboffset))
					return 0;
				break;
			}

		/* Set current to the next command, casting done because cmdsize is in bytes */
		current_cmd = (struct load_command*)((unsigned char*)current_cmd + BE32_toHost(current_cmd->cmdsize));
	}

	/* Free symtab and strtab */
//	free(symtab);
//	free(strtab);
//	free(idrsymtab);
	
//	free(load_commands_buf);
	
	fclose(dylibfile);

	return 1;
}

/* Load a segment into memory */
int LdrLoadSegment(struct segment_command *segment_command, void **buf, unsigned long *buflen, FILE *machfile, unsigned long mh_offset, unsigned long reloffset) {

	/* Convert structure to Host's endianess */
	segment_command->vmaddr = BE32_toHost(segment_command->vmaddr);
	segment_command->vmsize = BE32_toHost(segment_command->vmsize);
	segment_command->fileoff = BE32_toHost(segment_command->fileoff);
	segment_command->filesize = BE32_toHost(segment_command->filesize);
	segment_command->nsects = BE32_toHost(segment_command->nsects);
	segment_command->flags = BE32_toHost(segment_command->flags);

	/* Add reloffset to vmaddr */
	segment_command->vmaddr += reloffset;

	debug_printf("segname: %s\n", segment_command->segname);
	debug_printf("vmaddr: %#lx\n", (unsigned long)segment_command->vmaddr);
	debug_printf("vmsize: %#lx\n", (unsigned long)segment_command->vmsize);

	/* Check if vmaddr + vmsize exceed the space allocated by spMemInit() */
#if 0
	if((segment_command->vmaddr + segment_command->vmsize) > SOFTPEAR_CODEBASE)
		return 0;
#endif

	/* Change buflen if necessary */
	if(*buflen < segment_command->vmaddr + segment_command->vmsize)
			*buflen = segment_command->vmaddr + segment_command->vmsize;
	
	/* Catch __PAGEZERO */
	if(strcmp("__PAGEZERO", segment_command->segname)==0)
		return 1;

	/* Go through sections and load them into memory */
	unsigned int i;
	struct section *sections = (struct section*)(segment_command+1);
	for (i=0; i<segment_command->nsects; i++) {
		/* A list of section structs follows right after the segment_command */
		struct section *current_sect = &sections[i];
		
		/* Convert structure to Host's endianess */
		current_sect->addr = BE32_toHost(current_sect->addr);
		current_sect->size = BE32_toHost(current_sect->size);
		current_sect->offset = BE32_toHost(current_sect->offset);
		current_sect->align = BE32_toHost(current_sect->align);
		current_sect->reloff = BE32_toHost(current_sect->reloff);
		current_sect->nreloc = BE32_toHost(current_sect->nreloc);
		current_sect->flags = BE32_toHost(current_sect->flags);
		current_sect->reserved1 = BE32_toHost(current_sect->reserved1);
		current_sect->reserved2 = BE32_toHost(current_sect->reserved2);
		
		/* Continue if the size is 0 */
		if(current_sect->size == 0)
			continue;
			
		/* Add reloffset to load section behind previously loaded data if needed */
		current_sect->addr += reloffset;
		
		if(current_sect->offset != 0) {
			/* Seek the section in the file */
			if(fseek(machfile, current_sect->offset + mh_offset, SEEK_SET))
				return 0;

			void *target = &(((unsigned char*)*buf)[current_sect->addr]);
	
			/* And load it */
			fread(target, current_sect->size, 1, machfile); /* ignore it if we read nothing - otool says: "(past end of file)" */
			debug_printf("Loaded %s, %s to: %#lx\n", current_sect->segname, current_sect->sectname, (unsigned long)target);
		}
	}
		
	return 1;
}

/* Check if the offset specified contains a Mach header with the given CPU-Type */
int LdrCheckHdrCpuType(cpu_type_t cputype, unsigned long offset, FILE *machfile) {
	/* Seek to the offset and read the mach_header structure */
	if (fseek(machfile, offset, SEEK_SET))
		return 0;
	
	struct mach_header mach_header;
	if (!fread(&mach_header, sizeof(struct mach_header), 1, machfile))
		return 0;
		
	/* If we seek for a i386 architecture, the Mach header will be little endian, otherwise big endian */
	if(cputype == CPU_TYPE_I386)
		mach_header.cputype = BE32_toHost(mach_header.cputype);
	else
		mach_header.cputype = BE32_toHost(mach_header.cputype);
	
	return mach_header.cputype == cputype;
}

/* seek an opened fat file for a specified Mach header */
unsigned long LdrGetArchOffset(cpu_type_t cputype, FILE *machfile) {

	struct fat_header fat_header;

	/* Seek to the offset 0 to read the fat header */
	if(fseek(machfile, 0, SEEK_SET))
		return 0;

	/* Read the fat_header structure */
	if(!fread(&fat_header, sizeof(struct fat_header), 1, machfile))
		return 0;
	
	uint32_t i, nfat_arch = BE32_toHost(fat_header.nfat_arch);
	
	/* Search the fat_arch structures for the specified cputype */
	for(i=0;i<nfat_arch;i++) {
		
		struct fat_arch fat_arch;
		
		if(fseek(machfile, sizeof(struct fat_header) + i*sizeof(fat_arch), SEEK_SET))
			return 0;
		
		if(!fread(&fat_arch, sizeof(struct fat_arch), 1, machfile))
			return 0;
			
 		if((uint32_t)cputype == BE32_toHost(fat_arch.cputype))
 			return BE32_toHost(fat_arch.offset);
	}
	
	/* In case the specified cputype couldn't be found, return 0 */
	return 0;
}
	
/* Check for the existance of a valid MachO MAGIC */
LdrMagic_t LdrValidateFile(FILE *machfile) {
	/* seek to the beginning */
	if(fseek(machfile, 0, SEEK_SET))
		return 0;
	
	/* check for FAT MAGIC */
	uint32_t magic;
	if(!fread(&magic, sizeof(uint32_t), 1, machfile))
		return 0;
		
	if (magic == FAT_MAGIC || magic == FAT_CIGAM || magic == MH_MAGIC || magic == MH_CIGAM) {
		fprintf(stderr, "magic = %08X\n", magic);
		return magic;
	} else {
		fprintf(stderr, "No magic here\n");
		return 0;
	}
}

/* Load a mach_header structure from file and convert endianess to host */
int LdrGetMachHeader(struct mach_header *mach_header, unsigned long offset, FILE *machfile) {

	if(fseek(machfile, offset, SEEK_SET))
		return 0;
	
	if(!fread(mach_header, sizeof(struct mach_header), 1, machfile))
		return 0;
	
	if(mach_header->magic == MH_CIGAM) {
		mach_header->magic = BE32_toHost(mach_header->magic);
		mach_header->cputype = BE32_toHost(mach_header->cputype);
		mach_header->cpusubtype = BE32_toHost(mach_header->cpusubtype);
		mach_header->filetype = BE32_toHost(mach_header->filetype);
		mach_header->ncmds = BE32_toHost(mach_header->ncmds);
		mach_header->sizeofcmds = BE32_toHost(mach_header->sizeofcmds);
		mach_header->flags = BE32_toHost(mach_header->flags);
		return 1;
	}
	else if(mach_header->magic == MH_MAGIC) {
		return 1;
	}
	else {
		return 0;
	}
}

/* Append a function symbol to the List */
void LdrLLLibFunctionsAppend(const char *name, unsigned long offset) {

	LdrLLFunctions *newelem = (LdrLLFunctions*) my_malloc(sizeof(LdrLLFunctions));
	// debug_printf("newelem my_malloced at %#lx\n", newelem);
	newelem->name = (char*) my_malloc(strlen(name)*sizeof(char)+1);
	// debug_printf("newelem->name my_malloced at %#lx\n", newelem->name);
	strcpy(newelem->name, name);
	newelem->offset = offset;
	newelem->next = NULL;
	
	if(LdrLibFunctions == NULL) {
		LdrLibFunctions = newelem;
		}
	else {
		LdrLLFunctions *current = LdrLibFunctions;
		while(current->next)
			current = current->next;
		current->next = newelem;
		}
}

/* Clear the complete linked list of LdrLLFunctions */
void LdrLLLibFunctionsClearList () {

	LdrLLFunctions *current = LdrLibFunctions;
	while(current) {
		/* Free the name string */
		free(current->name);
		
		/* Free the node itself */
		LdrLLFunctions *tmp = current;
		current = current->next;
		free(tmp);
	}
	
	/* Set the beginning to NULL */
	LdrLibFunctions = NULL;
}

/* Load the Native Library Call segment */
/* This consists of instructions that are recognized by the interpreter and
   should lead to the call of a native library function
   Opcode is 000010, bits 16 to 31 are an index into the NativeLibs-Table */
int LdrLoadNativeLibJumps(void **buf, unsigned long *buflen) {

	/* Don't do anything if kNativeLibCount is zero */
#if 0
	if(kNativeLibCount < 1)
		return 0;

	/* Align memory to the next 0x1000 bytes */
	while((*buflen)%0x1000 != 0)
		(*buflen)++;
		
	uint32_t target = *buflen;
	
	debug_printf("Building the __SoftPear-Segment at %#lx (Size: %#lx)\n", *buflen, sizeof(unsigned long) * kNativeLibCount);
	
	*buflen += sizeof(unsigned long) * kNativeLibCount;
	
	/* Make space for the segment */
	// *buf = realloc(*buf, *buflen);
	
#if 0
	int i;
	for(i=0; i<kNativeLibCount; i++, target+=sizeof(uint32_t)) {
		/* Insert the instruction */
		*((uint32_t*)(&((unsigned char*)*buf)[target])) = BE32_toHost(0x08000000 | i);
		/* And add an entry into the loader symbol table */
		LdrLLLibFunctionsAppend(NativeLibs[i].name, target);
		debug_printf("Added %s-Fake at %#lx\n", NativeLibs[i].name, target);
	}
#endif
	
	return 1;
#endif
	return 0;
}

/* Debug printout of LLFunctions */
void LdrLLLibFunctionsPrint() {
	debug_printf("Library functions currently loaded:\n");
	LdrLLFunctions *current = LdrLibFunctions;
	while(current) {
		debug_printf("%s: %#lx\n", current->name, current->offset);
		current = current->next;
	}
}

