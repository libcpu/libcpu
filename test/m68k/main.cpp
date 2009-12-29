#include "libcpu.h"
#include "netlong.h"

#include "arch/m68k/libcpu_m68k.h"

#include <stdio.h>
#include <stdlib.h>
#include "types.h"

#define LOAD_ADDRESS 0x10000 /* let's load the app at 4 KB */
uint32_t
get32(FILE *f) {
	return fgetc(f)<<24 | fgetc(f)<<16 | fgetc(f)<<8 | fgetc(f);
}

uint32_t
load32(uint8_t * RAM, addr_t a) {
	return ntohl(*(uint32_t*)&RAM[a]);
}

void
store32(uint8_t * RAM, addr_t a, uint32_t b) {
	*(uint32_t*)&RAM[a] = htonl(b);
}

/*
 * The AmigaOS/Tripos Hunk format consists of any number of code/data hunks,
 * and each hunk comes with relocation information directly after it. This is
 * a list of offsets and hunknumbers: The offset in this hunk has to be
 * patched with the address of one specific hunk. Since relocation information
 * is not stored at the end of the file after all hunks, if the file is read
 * sequentially, the load addresses of the other hunks are not known yet,
 * so it is necesary to do two passes over the file.
 */
int
load_code(char *filename, uint8_t *RAM, int ramsize, addr_t *s, addr_t *e, addr_t *entry) {
	FILE *f;
	uint32_t hunk_type, n, tablesize, offset, firsthunk, lasthunk, hunk, curhunk, i;
	addr_t hunk_address[256]; /* max 256 hunks */
	addr_t cur_address;
	int pass;

	for (pass=0; pass<2; pass++) {
		if (!(f = fopen(filename, "rb")))
			return 0;

		hunk_type = get32(f);
		if (hunk_type != 0x03F3) {
			printf("hunk_header expected!\n");
			exit(1);
		}
		while ((n = get32(f))) { /* read hunk names */
			for (i=0; i<n; i++)
				get32(f);
		}
		tablesize = get32(f);
		firsthunk = get32(f);
		lasthunk = get32(f);
			
		for (i=0; i<tablesize; i++) /* hunk sizes */
			get32(f);

		curhunk = firsthunk-1;
		cur_address = LOAD_ADDRESS;

		while (!feof(f)) {
//			printf("hunk_type: %x\n", hunk_type);

			hunk_type = get32(f);
			if (hunk_type == 0xFFFFFFFF)
				break;
			switch (hunk_type) {
				case 0x03E9: /* hunk_code */
				case 0x03EA: /* hunk_data */
					curhunk++;
					hunk_address[curhunk] = cur_address;
					n = get32(f);
					if (!fread(&RAM[cur_address], 1, n*4, f))
					  return 0;
					cur_address += n*4;
					break;
				case 0x03EC: /* hunk_reloc */
					while ((n = get32(f))) {
						hunk = get32(f);/* hunk number */
						for (i=0; i<n; i++) {
							offset = get32(f);
//							printf("patch offset %x with start address of hunk %d\n", offset, hunk);
							if (pass)
								store32(RAM, hunk_address[curhunk]+offset, load32(RAM, hunk_address[curhunk]+offset)+hunk_address[hunk]);
						}
					}
					break;
				case 0x03F2: /* hunk_end */
					break;
				case 0x03F0: /* hunk_symbol */
					while ((n = get32(f))) {
						for (i=0; i<n+1; i++)
							get32(f);
					}
					break;
				default:
					printf("hunk_type: %x\n", hunk_type);
					exit(1);
			}
		}

		fclose(f);
	}

	*s = LOAD_ADDRESS;
	*e = cur_address;
	*entry = *s;
// hack
//	RAM[LOAD_ADDRESS+0x20] = 0x4E;
//	RAM[LOAD_ADDRESS+0x21] = 0x75;
// hack
	return 1;
}

//////////////////////////////////////////////////////////////////////
// command line parsing helpers
//////////////////////////////////////////////////////////////////////
void tag_extra(cpu_t *cpu, char *entries) {
	addr_t entry;
	char* old_entries;

	while (entries && *entries) {
	/* just one for now */
		if (entries[0] == ',')
			entries++;
		if (!entries[0])
			break;
		old_entries = entries;
		entry = (addr_t)strtol(entries, &entries, 0);
		if (entries == old_entries) {
			printf("Error parsing entries!\n");
			exit(3);
		}
		cpu_tag(cpu, entry);
	}
}

void tag_extra_filename(cpu_t *cpu, char *filename) {
	FILE *fp;
	char *buf;
	size_t nbytes;

	fp = fopen(filename, "r");
	if (!fp) {
		perror("error opening tag file");
		exit(3);
	}
	fseek(fp, 0, SEEK_END);
	nbytes = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	buf = (char*)malloc(nbytes + 1);
	nbytes = fread(buf, 1, nbytes, fp);
	buf[nbytes] = '\0';
	fclose(fp);

	while (nbytes && buf[nbytes - 1] == '\n')
		buf[--nbytes] = '\0';

	tag_extra(cpu, buf);
	free(buf);
}

#ifdef __GNUC__
void __attribute__((noinline))
breakpoint() {
asm("nop");
}
#else
void breakpoint() {}
#endif

static void
debug_function(cpu_t *cpu) {
	//reg_m68k_t *reg = (reg_m68k_t*)cpu->rf.grf;
	fprintf(stderr, "%s:%u\n", __FILE__, __LINE__);
}

//////////////////////////////////////////////////////////////////////
int
main(int argc, char **argv) {
	char *executable;
	char *entries;
	cpu_t *cpu;
	uint8_t *RAM;

	int ramsize = 65536;
	RAM = (uint8_t*)malloc(ramsize);

	cpu = cpu_new(CPU_ARCH_M68K, 0, 0);
	cpu_set_flags_optimize(cpu, CPU_OPTIMIZE_ALL);
	cpu_set_flags_debug(cpu, CPU_DEBUG_NONE);
	cpu_set_ram(cpu, RAM);

/* parameter parsing */
	if (argc<2) {
		printf("Usage: %s executable [entries]\n", argv[0]);
		return 0;
	}

	executable = argv[1];
	if (argc>=3)
		entries = argv[2];
	else
		entries = 0;

/* load code */

	FILE *f;

	if (!(f = fopen(executable, "rb"))) {
		printf("Could not open %s!\n", executable);
		return 2;
	}

	cpu->code_start = 0xA000;
	cpu->code_end = cpu->code_start + fread(&RAM[cpu->code_start], 1, ramsize-cpu->code_start, f);
	fclose(f);

	cpu->code_entry = RAM[cpu->code_start] | RAM[cpu->code_start+1]<<8; /* start vector at beginning ($A000) */

	cpu_tag(cpu, cpu->code_entry);

	if (entries && *entries == '@')
		tag_extra_filename(cpu, entries + 1);
	else
		tag_extra(cpu, entries); /* tag extra entry points from the command line */

#ifdef RET_OPTIMIZATION
	find_rets(RAM, cpu->code_start, cpu->code_end);
#endif

	printf("*** Executing...\n");

#define PC (((reg_6502_t*)cpu->rf.grf)->pc)
#define A (((reg_6502_t*)cpu->rf.grf)->a)
#define X (((reg_6502_t*)cpu->rf.grf)->x)
#define Y (((reg_6502_t*)cpu->rf.grf)->y)
#define S (((reg_6502_t*)cpu->rf.grf)->s)
#define P (((reg_6502_t*)cpu->rf.grf)->p)

	PC = cpu->code_entry;
	S = 0xFF;

	for(;;) {
		breakpoint();
		int ret = cpu_run(cpu, debug_function);
		//printf("ret = %d\n", ret);
		switch (ret) {
			case JIT_RETURN_NOERR: /* JIT code wants us to end execution */
				break;
			case JIT_RETURN_FUNCNOTFOUND:
//				printf("LIB: $%04X: A=$%02X X=$%02X Y=$%02X S=$%02X P=$%02X\n", pc, a, x, y, s, p);

#if 0
				if (kernal_dispatch(RAM, &PC, &A, &X, &Y, &S, &P)) {
					// the runtime could handle it, so do an RTS
					PC = RAM[0x0100+(++(S))];
					PC |= (RAM[0x0100+(++(S))]<<8);
					PC++;
					continue;
				}
#endif
				
				// maybe it's a JMP in RAM: interpret it
				if (RAM[PC]==0x4C) {
					PC = RAM[PC+1] | RAM[PC+2]<<8;
					continue;
				}

				// bad :(
				printf("%s: error: $%04X not found!\n", __func__, PC);
				int i;
				printf("PC: ");
				for (i=0; i<16; i++)
					printf("%02X ", RAM[PC+i]);
				printf("\n");
				exit(1);
			default:
				printf("unknown return code: %d\n", ret);
		}
	}

	printf("done.\n");

	return 0;
}
