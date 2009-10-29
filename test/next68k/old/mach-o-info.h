/*

    INTERPRETER (Darwin-PPC -> Darwin-X86)
    
    mach-o-info.h
    
    Last change: 11/02/2004, Axel Auweter
        
    Portions Copyright (c) 1999-2003 Apple Computer, Inc. All Rights Reserved.

    This file contains Original Code and/or Modifications of Original Code as
    defined in and that are subject to the Apple Public Source License
    Version 2.0 (the 'License'). You may not use this file except in
    compliance with the License. Please obtain a copy of the License at
    http://www.opensource.apple.com/apsl/ and read it before using this file.

    The Original Code and all software distributed under the License are
    distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
    EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
    INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY, FITNESS
    FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT. Please see
    the License for the specific language governing rights and limitations
    under the License.
    
*/

/*******************************************************************************************************
 *                                                                                                     *
 *   Note: A reference of the structures and types declared in this file can be found at:              *
 *                                                                                                     *
 *   http://developer.apple.com/documentation/DeveloperTools/Conceptual/MachORuntime/MachORuntime.pdf  *
 *                                                                                                     *
 *******************************************************************************************************/
 

#ifndef machoinfo_h
#define machoinfo_h

// Custom types
#ifndef __APPLE__
typedef long cpu_type_t;
typedef long cpu_subtype_t;
#endif

// Define Mach-O-Header
typedef struct {
	unsigned long	  magic;		
  cpu_type_t      cputype;	
	cpu_subtype_t   cpusubtype;	
	unsigned long	  filetype;	
	unsigned long	  ncmds;		
	unsigned long	  sizeofcmds;	
	unsigned long	  flags;		
} mach_header;

// Define FAT-Header
typedef struct {
  unsigned long magic;
  unsigned long nfat_arch;
} fat_header;

// Define FAT-Arch
typedef struct {
  cpu_type_t    cputype;
  cpu_subtype_t cpusubtype;
  unsigned long offset;
  unsigned long size;
  unsigned long align;
} fat_arch;

// Define Load command structure
typedef struct {
  unsigned long cmd;
  unsigned long cmdsize;
} load_command;

typedef union lc_str {
  unsigned long offset;
  char* ptr;
} lc_str;

 
/*
 * Dynamicly linked shared libraries are identified by two things.  The
 * pathname (the name of the library as found for execution), and the
 * compatibility version number.  The pathname must match and the compatibility
 * number in the user of the library must be greater than or equal to the
 * library being used.  The time stamp is used to record the time a library was
 * built and copied into user so it can be use to determined if the library used
 * at runtime is exactly the same as used to built the program.
 */
typedef struct dylib {
    union lc_str  name;                 /* library's path name */
    unsigned long timestamp;            /* library's build time stamp */
    unsigned long current_version;      /* library's current version number */
    unsigned long compatibility_version;/* library's compatibility vers number*/
} dylib;

/*
 * A dynamically linked shared library (filetype == MH_DYLIB in the mach header)
 * contains a dylib_command (cmd == LC_ID_DYLIB) to identify the library.
 * An object that uses a dynamically linked shared library also contains a
 * dylib_command (cmd == LC_LOAD_DYLIB or cmd == LC_LOAD_WEAK_DYLIB) for each
 * library it uses.
 */
typedef struct dylib_command {
        unsigned long   cmd;            /* LC_ID_DYLIB, LC_LOAD_{,WEAK_}DYLIB */
        unsigned long   cmdsize;        /* includes pathname string */
        struct dylib    dylib;          /* the library identification */
} dylib_command;

/*
 * A program that uses a dynamic linker contains a dylinker_command to identify
 * the name of the dynamic linker (LC_LOAD_DYLINKER).  And a dynamic linker
 * contains a dylinker_command to identify the dynamic linker (LC_ID_DYLINKER).
 * A file can have at most one of these.
 */
typedef struct dylinker_command {
        unsigned long   cmd;            /* LC_ID_DYLINKER or LC_LOAD_DYLINKER */
        unsigned long   cmdsize;        /* includes pathname string */
        union lc_str    name;           /* dynamic linker's path name */
} dylinker_command;

/*
 * A program (filetype == MH_EXECUTE) that is
 * prebound to its dynamic libraries has one of these for each library that
 * the static linker used in prebinding.  It contains a bit vector for the
 * modules in the library.  The bits indicate which modules are bound (1) and
 * which are not (0) from the library.  The bit for module 0 is the low bit
 * of the first byte.  So the bit for the Nth module is:
 * (linked_modules[N/8] >> N%8) & 1
 */
typedef struct prebound_dylib_command {
        unsigned long   cmd;            /* LC_PREBOUND_DYLIB */
        unsigned long   cmdsize;        /* includes strings */
        union lc_str    name;           /* library's path name */
        unsigned long   nmodules;       /* number of modules in library */
        union lc_str    linked_modules; /* bit vector of linked modules */
} prebound_dylib_command;

/*
 * The twolevel_hints_command contains the offset and number of hints in the
 * two-level namespace lookup hints table.
 */
typedef struct twolevel_hints_command {
    unsigned long cmd;          /* LC_TWOLEVEL_HINTS */
    unsigned long cmdsize;      /* sizeof(struct twolevel_hints_command) */
    unsigned long offset;       /* offset to the hint table */
    unsigned long nhints;       /* number of hints in the hint table */
} twolevel_hints_command;

/*
 * The entries in the two-level namespace lookup hints table are twolevel_hint
 * structs.  These provide hints to the dynamic link editor where to start
 * looking for an undefined symbol in a two-level namespace image.  The
 * isub_image field is an index into the sub-images (sub-frameworks and
 * sub-umbrellas list) that made up the two-level image that the undefined
 * symbol was found in when it was built by the static link editor.  If
 * isub-image is 0 the the symbol is expected to be defined in library and not
 * in the sub-images.  If isub-image is non-zero it is an index into the array
 * of sub-images for the umbrella with the first index in the sub-images being
 * 1. The array of sub-images is the ordered list of sub-images of the umbrella
 * that would be searched for a symbol that has the umbrella recorded as its
 * primary library.  The table of contents index is an index into the
 * library's table of contents.  This is used as the starting point of the
 * binary search or a directed linear search.
 */
struct twolevel_hint {
    unsigned long 
        isub_image:8,   /* index into the sub images */
        itoc:24;        /* index into the table of contents */
};

#ifndef __APPLE__
typedef int             vm_prot_t;
#endif

/* 
 * The segment load command indicates that a part of this file is to be
 * mapped into the task's address space.  The size of this segment in memory,
 * vmsize, maybe equal to or larger than the amount to map from this file,
 * filesize.  The file is mapped starting at fileoff to the beginning of
 * the segment in memory, vmaddr.  The rest of the memory of the segment,
 * if any, is allocated zero fill on demand.  The segment's maximum virtual
 * memory protection and initial virtual memory protection are specified
 * by the maxprot and initprot fields.  If the segment has sections then the
 * section structures directly follow the segment command and their size is
 * reflected in cmdsize.
 */
typedef struct segment_command {
        unsigned long   cmd;            /* LC_SEGMENT */
        unsigned long   cmdsize;        /* includes sizeof section structs */
        char            segname[16];    /* segment name */
        unsigned long   vmaddr;         /* memory address of this segment */
        unsigned long   vmsize;         /* memory size of this segment */
        unsigned long   fileoff;        /* file offset of this segment */
        unsigned long   filesize;       /* amount to map from the file */
        vm_prot_t       maxprot;        /* maximum VM protection */
        vm_prot_t       initprot;       /* initial VM protection */
        unsigned long   nsects;         /* number of sections in segment */
        unsigned long   flags;          /* flags */
} segment_command;

/*
 * The symtab_command contains the offsets and sizes of the link-edit 4.3BSD
 * "stab" style symbol table information as described in the header files
 * <nlist.h> and <stab.h>.
 */
typedef struct symtab_command {
        unsigned long   cmd;            /* LC_SYMTAB */
        unsigned long   cmdsize;        /* sizeof(struct symtab_command) */
        unsigned long   symoff;         /* symbol table offset */
        unsigned long   nsyms;          /* number of symbol table entries */
        unsigned long   stroff;         /* string table offset */
        unsigned long   strsize;        /* string table size in bytes */
} symtab_command;

typedef struct dysymtab_command {
    unsigned long cmd;          /* LC_DYSYMTAB */
    unsigned long cmdsize;      /* sizeof(struct dysymtab_command) */

    unsigned long ilocalsym;    /* index to local symbols */
    unsigned long nlocalsym;    /* number of local symbols */

    unsigned long iextdefsym;   /* index to externally defined symbols */
    unsigned long nextdefsym;   /* number of externally defined symbols */

    unsigned long iundefsym;    /* index to undefined symbols */
    unsigned long nundefsym;    /* number of undefined symbols */

    unsigned long tocoff;       /* file offset to table of contents */
    unsigned long ntoc;         /* number of entries in table of contents */

    unsigned long modtaboff;    /* file offset to module table */
    unsigned long nmodtab;      /* number of module table entries */

    unsigned long extrefsymoff;  /* offset to referenced symbol table */
    unsigned long nextrefsyms;   /* number of referenced symbol table entries */

    unsigned long indirectsymoff; /* file offset to the indirect symbol table */
    unsigned long nindirectsyms;  /* number of indirect symbol table entries */

    unsigned long extreloff;    /* offset to external relocation entries */
    unsigned long nextrel;      /* number of external relocation entries */

    unsigned long locreloff;    /* offset to local relocation entries */
    unsigned long nlocrel;      /* number of local relocation entries */

} dysymtab_command;      

typedef struct nlist {
	union {
		char *n_name;	/* for use when in-core */
		long  n_strx;	/* index into the string table */
	} n_un;
	unsigned char n_type;	/* type flag, see below */
	unsigned char n_sect;	/* section number or NO_SECT */
	short	      n_desc;	/* see <mach-o/stab.h> */
	unsigned long n_value;	/* value of this symbol (or stab offset) */
} nlist;

#define	NO_SECT		0	/* symbol is not in any section */
#define MAX_SECT	255	/* 1 thru 255 inclusive */

#define	N_STAB	0xe0  /* if any of these bits set, a symbolic debugging entry */
#define	N_PEXT	0x10  /* private external symbol bit */
#define	N_TYPE	0x0e  /* mask for the type bits */
#define	N_EXT	0x01  /* external symbol bit, set for external symbols */

#define	N_UNDF	0x0		/* undefined, n_sect == NO_SECT */
#define	N_ABS	0x2		/* absolute, n_sect == NO_SECT */
#define	N_SECT	0xe		/* defined in section number n_sect */
#define	N_PBUD	0xc		/* prebound undefined (defined in a dylib) */
#define N_INDR	0xa		/* indirect */

/*
 * Symbolic debugger symbols.  The comments give the conventional use for
 * 
 * 	.stabs "n_name", n_type, n_sect, n_desc, n_value
 *
 * where n_type is the defined constant and not listed in the comment.  Other
 * fields not listed are zero. n_sect is the section ordinal the entry is
 * refering to.
 */
#define	N_GSYM	0x20	/* global symbol: name,,NO_SECT,type,0 */
#define	N_FNAME	0x22	/* procedure name (f77 kludge): name,,NO_SECT,0,0 */
#define	N_FUN	0x24	/* procedure: name,,n_sect,linenumber,address */
#define	N_STSYM	0x26	/* static symbol: name,,n_sect,type,address */
#define	N_LCSYM	0x28	/* .lcomm symbol: name,,n_sect,type,address */
#define N_BNSYM 0x2e	/* begin nsect sym: 0,,n_sect,0,address */
#define	N_RSYM	0x40	/* register sym: name,,NO_SECT,type,register */
#define	N_SLINE	0x44	/* src line: 0,,n_sect,linenumber,address */
#define N_ENSYM 0x4e	/* end nsect sym: 0,,n_sect,0,address */
#define	N_SSYM	0x60	/* structure elt: name,,NO_SECT,type,struct_offset */
#define	N_SO	0x64	/* source file name: name,,n_sect,0,address */
#define	N_LSYM	0x80	/* local sym: name,,NO_SECT,type,offset */
#define N_BINCL	0x82	/* include file beginning: name,,NO_SECT,0,sum */
#define	N_SOL	0x84	/* #included file name: name,,n_sect,0,address */
#define	N_PSYM	0xa0	/* parameter: name,,NO_SECT,type,offset */
#define N_EINCL	0xa2	/* include file end: name,,NO_SECT,0,0 */
#define	N_ENTRY	0xa4	/* alternate entry: name,,n_sect,linenumber,address */
#define	N_LBRAC	0xc0	/* left bracket: 0,,NO_SECT,nesting level,address */
#define N_EXCL	0xc2	/* deleted include file: name,,NO_SECT,0,sum */
#define	N_RBRAC	0xe0	/* right bracket: 0,,NO_SECT,nesting level,address */
#define	N_BCOMM	0xe2	/* begin common: name,,NO_SECT,0,0 */
#define	N_ECOMM	0xe4	/* end common: name,,n_sect,0,0 */
#define	N_ECOML	0xe8	/* end common (local name): 0,,n_sect,0,address */
#define	N_LENG	0xfe	/* second stab entry with length information */




/*
 * A segment is made up of zero or more sections.  Non-MH_OBJECT files have
 * all of their segments with the proper sections in each, and padded to the
 * specified segment alignment when produced by the link editor.  The first
 * segment of a MH_EXECUTE and MH_FVMLIB format file contains the mach_header
 * and load commands of the object file before its first section.  The zero
 * fill sections are always last in their segment (in all formats).  This
 * allows the zeroed segment padding to be mapped into memory where zero fill
 * sections might be.
 *
 * The MH_OBJECT format has all of its sections in one segment for
 * compactness.  There is no padding to a specified segment boundary and the
 * mach_header and load commands are not part of the segment.
 *
 * Sections with the same section name, sectname, going into the same segment,
 * segname, are combined by the link editor.  The resulting section is aligned
 * to the maximum alignment of the combined sections and is the new section's
 * alignment.  The combined sections are aligned to their original alignment in
 * the combined section.  Any padded bytes to get the specified alignment are
 * zeroed.
 *
 * The format of the relocation entries referenced by the reloff and nreloc
 * fields of the section structure for mach object files is described in the
 * header file <reloc.h>.
 */
typedef struct section {
        char            sectname[16];   /* name of this section */
        char            segname[16];    /* segment this section goes in */
        unsigned long   addr;           /* memory address of this section */
        unsigned long   size;           /* size in bytes of this section */
        unsigned long   offset;         /* file offset of this section */
        unsigned long   align;          /* section alignment (power of 2) */
        unsigned long   reloff;         /* file offset of relocation entries */
        unsigned long   nreloc;         /* number of relocation entries */
        unsigned long   flags;          /* flags (section type and attributes)*/
        unsigned long   reserved1;      /* reserved */
        unsigned long   reserved2;      /* reserved */
} section;

/*
 * Format of a relocation entry of a Mach-O file.  Modified from the 4.3BSD
 * format.  The modifications from the original format were changing the value
 * of the r_symbolnum field for "local" (r_extern == 0) relocation entries.
 * This modification is required to support symbols in an arbitrary number of
 * sections not just the three sections (text, data and bss) in a 4.3BSD file.
 * Also the last 4 bits have had the r_type tag added to them.
 */
typedef struct relocation_info {
   long         r_address;      /* offset in the section to what is being
                                   relocated */
   unsigned int r_symbolnum:24, /* symbol index if r_extern == 1 or section
                                   ordinal if r_extern == 0 */
                r_pcrel:1,      /* was relocated pc relative already */
                r_length:2,     /* 0=byte, 1=word, 2=long */
                r_extern:1,     /* does not include value of sym referenced */
                r_type:4;       /* if not 0, machine specific relocation type */
} relocation_info;
#define R_ABS   0               /* absolute relocation type for Mach-O files */

// Define keys for recognizing fat and mach headers
#define FAT_MAGIC 0xCAFEBABE
#define FAT_CIGAM 0xBEBAFECA
#define MH_MAGIC 0xFEEDFACE
#define MH_CIGAM 0xCEFAEDFE

// Define different CPU-Types
#define CPU_TYPE_ANY     ((cpu_type_t) -1)
#define CPU_TYPE_VAX     ((cpu_type_t) 1)
#define	CPU_TYPE_MC680x0 ((cpu_type_t) 6)
#define CPU_TYPE_I386    ((cpu_type_t) 7)
#define CPU_TYPE_MIPS    ((cpu_type_t) 8)
#define CPU_TYPE_MC98000 ((cpu_type_t) 10)
#define CPU_TYPE_HPPA    ((cpu_type_t) 11)
#define CPU_TYPE_MC88000 ((cpu_type_t) 13)
#define CPU_TYPE_SPARC   ((cpu_type_t) 14)
#define CPU_TYPE_I860    ((cpu_type_t) 15)
#define CPU_TYPE_POWERPC ((cpu_type_t) 18)

// Define different CPU-Subtypes
#define	CPU_SUBTYPE_MULTIPLE         ((cpu_subtype_t) -1)
#define CPU_SUBTYPE_LITTLE_ENDIAN    ((cpu_subtype_t) 0)
#define CPU_SUBTYPE_BIG_ENDIAN       ((cpu_subtype_t) 1)
#define	CPU_SUBTYPE_VAX_ALL          ((cpu_subtype_t) 0)
#define CPU_SUBTYPE_VAX780           ((cpu_subtype_t) 1)
#define CPU_SUBTYPE_VAX785           ((cpu_subtype_t) 2)
#define CPU_SUBTYPE_VAX750           ((cpu_subtype_t) 3)
#define CPU_SUBTYPE_VAX730           ((cpu_subtype_t) 4)
#define CPU_SUBTYPE_UVAXI            ((cpu_subtype_t) 5)
#define CPU_SUBTYPE_UVAXII           ((cpu_subtype_t) 6)
#define CPU_SUBTYPE_VAX8200          ((cpu_subtype_t) 7)
#define CPU_SUBTYPE_VAX8500          ((cpu_subtype_t) 8)
#define CPU_SUBTYPE_VAX8600          ((cpu_subtype_t) 9)
#define CPU_SUBTYPE_VAX8650          ((cpu_subtype_t) 10)
#define CPU_SUBTYPE_VAX8800          ((cpu_subtype_t) 11)
#define CPU_SUBTYPE_UVAXIII          ((cpu_subtype_t) 12)
#define	CPU_SUBTYPE_MC680x0_ALL      ((cpu_subtype_t) 1)
#define CPU_SUBTYPE_MC68030          ((cpu_subtype_t) 1)
#define CPU_SUBTYPE_MC68040          ((cpu_subtype_t) 2)
#define	CPU_SUBTYPE_MC68030_ONLY     ((cpu_subtype_t) 3)
#define	CPU_SUBTYPE_I386_ALL         ((cpu_subtype_t) 3)
#define CPU_SUBTYPE_386              ((cpu_subtype_t) 3)
#define CPU_SUBTYPE_486              ((cpu_subtype_t) 4)
#define CPU_SUBTYPE_486SX            ((cpu_subtype_t) 4 + 128)
#define CPU_SUBTYPE_586              ((cpu_subtype_t) 5)
#define CPU_SUBTYPE_INTEL(f, m)      ((cpu_subtype_t) (f) + ((m) << 4))
#define CPU_SUBTYPE_PENT             CPU_SUBTYPE_INTEL(5, 0)
#define CPU_SUBTYPE_PENTPRO          CPU_SUBTYPE_INTEL(6, 1)
#define CPU_SUBTYPE_PENTII_M3        CPU_SUBTYPE_INTEL(6, 3)
#define CPU_SUBTYPE_PENTII_M5        CPU_SUBTYPE_INTEL(6, 5)
//#define CPU_SUBTYPE_INTEL_FAMILY (x) ((x) & 15)
#define CPU_SUBTYPE_INTEL_FAMILY_MAX 15
//#define CPU_SUBTYPE_INTEL_MODEL (x)  ((x) >> 4)
#define CPU_SUBTYPE_INTEL_MODEL_ALL	 0
#define	CPU_SUBTYPE_MIPS_ALL         ((cpu_subtype_t) 0)
#define CPU_SUBTYPE_MIPS_R2300       ((cpu_subtype_t) 1)
#define CPU_SUBTYPE_MIPS_R2600       ((cpu_subtype_t) 2)
#define CPU_SUBTYPE_MIPS_R2800       ((cpu_subtype_t) 3)
#define CPU_SUBTYPE_MIPS_R2000a      ((cpu_subtype_t) 4)
#define CPU_SUBTYPE_MIPS_R2000       ((cpu_subtype_t) 5)
#define CPU_SUBTYPE_MIPS_R3000a      ((cpu_subtype_t) 6)
#define CPU_SUBTYPE_MIPS_R3000       ((cpu_subtype_t) 7)
#define	CPU_SUBTYPE_MC98000_ALL      ((cpu_subtype_t) 0)
#define CPU_SUBTYPE_MC98601          ((cpu_subtype_t) 1)
#define	CPU_SUBTYPE_HPPA_ALL         ((cpu_subtype_t) 0)
#define CPU_SUBTYPE_HPPA_7100        ((cpu_subtype_t) 0)
#define CPU_SUBTYPE_HPPA_7100LC      ((cpu_subtype_t) 1)
#define	CPU_SUBTYPE_MC88000_ALL      ((cpu_subtype_t) 0)
#define CPU_SUBTYPE_MC88100          ((cpu_subtype_t) 1)
#define CPU_SUBTYPE_MC88110          ((cpu_subtype_t) 2)
#define	CPU_SUBTYPE_SPARC_ALL        ((cpu_subtype_t) 0)
#define CPU_SUBTYPE_I860_ALL         ((cpu_subtype_t) 0)
#define CPU_SUBTYPE_I860_860         ((cpu_subtype_t) 1)
#define CPU_SUBTYPE_POWERPC_ALL      ((cpu_subtype_t) 0)
#define CPU_SUBTYPE_POWERPC_601      ((cpu_subtype_t) 1)
#define CPU_SUBTYPE_POWERPC_602      ((cpu_subtype_t) 2)
#define CPU_SUBTYPE_POWERPC_603      ((cpu_subtype_t) 3)
#define CPU_SUBTYPE_POWERPC_603e     ((cpu_subtype_t) 4)
#define CPU_SUBTYPE_POWERPC_603ev    ((cpu_subtype_t) 5)
#define CPU_SUBTYPE_POWERPC_604      ((cpu_subtype_t) 6)
#define CPU_SUBTYPE_POWERPC_604e     ((cpu_subtype_t) 7)
#define CPU_SUBTYPE_POWERPC_620      ((cpu_subtype_t) 8)
#define CPU_SUBTYPE_POWERPC_750      ((cpu_subtype_t) 9)
#define CPU_SUBTYPE_POWERPC_7400     ((cpu_subtype_t) 10)
#define CPU_SUBTYPE_POWERPC_7450     ((cpu_subtype_t) 11)
#define CPU_SUBTYPE_POWERPC_Max      ((cpu_subtype_t) 10)
#define CPU_SUBTYPE_POWERPC_SCVger   ((cpu_subtype_t) 11)
#define CPU_SUBTYPE_POWERPC_970      ((cpu_subtype_t) 100)

// Define file types
#define	MH_OBJECT     0x1
#define	MH_EXECUTE    0x2
#define	MH_FVMLIB     0x3
#define	MH_CORE       0x4
#define	MH_PRELOAD    0x5
#define	MH_DYLIB      0x6
#define	MH_DYLINKER   0x7
#define	MH_BUNDLE     0x8
#define	MH_DYLIB_STUB 0x9

// Define mach-header-flags
#define	MH_NOUNDEFS        0x1
#define	MH_INCRLINK        0x2
#define MH_DYLDLINK        0x4
#define MH_BINDATLOAD      0x8
#define MH_PREBOUND        0x10	
#define MH_SPLIT_SEGS      0x20
#define MH_LAZY_INIT       0x40
#define MH_TWOLEVEL        0x80	
#define MH_FORCE_FLAT      0x100
#define MH_NOMULTIDEFS     0x200
#define MH_NOFIXPREBINDING 0x400


// Define load command types
#define	LC_SEGMENT         0x1
#define	LC_SYMTAB	         0x2
#define	LC_SYMSEG	         0x3
#define	LC_THREAD	         0x4
#define	LC_UNIXTHREAD	     0x5
#define	LC_LOADFVMLIB	     0x6
#define	LC_IDFVMLIB	       0x7	
#define	LC_IDENT           0x8	
#define LC_FVMFILE	       0x9	
#define LC_PREPAGE         0xa     
#define	LC_DYSYMTAB	       0xb	
#define	LC_LOAD_DYLIB	     0xc
#define	LC_ID_DYLIB	       0xd	
#define LC_LOAD_DYLINKER   0xe
#define LC_ID_DYLINKER	   0xf	
#define	LC_PREBOUND_DYLIB  0x10
#define	LC_ROUTINES	       0x11	
#define	LC_SUB_FRAMEWORK   0x12	
#define	LC_SUB_UMBRELLA    0x13
#define	LC_SUB_CLIENT	     0x14
#define	LC_SUB_LIBRARY     0x15	
#define	LC_TWOLEVEL_HINTS  0x16
#define	LC_PREBIND_CKSUM   0x17
#define	LC_LOAD_WEAK_DYLIB (0x18 | LC_REQ_DYLD)

#endif
