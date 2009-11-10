#include <libcpu.h>

#include "arch/6502/libcpu_6502.h"

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

void __attribute__((noinline))
breakpoint() {
asm("nop");
}

static void
debug_function(uint8_t *RAM, void *r) {
	reg_6502_t *reg = (reg_6502_t*)r;
	printf("DEBUG: $%04X: A=$%02X X=$%02X Y=$%02X S=$%02X P=$%02X %02X/%02X\n", reg->pc, reg->a, reg->x, reg->y, reg->s, reg->p, RAM[0x33], RAM[0x34]);
	{ int i; for (i=0x01F0; i<0x0200; i++) printf("%02X ", RAM[i]); printf("\n"); }
}

extern int
kernal_dispatch(
	unsigned char *ram,
	unsigned short *pc,
	unsigned char *a,
	unsigned char *x,
	unsigned char *y,
	unsigned char *s,
	unsigned char *p); //XXX

//////////////////////////////////////////////////////////////////////
int
main(int argc, char **argv) {
	char *executable;
	char *entries;
	cpu_t *cpu;
	uint8_t *RAM;

	int ramsize = 65536;
	RAM = (uint8_t*)malloc(ramsize);

	cpu = cpu_new(CPU_ARCH_6502);
	cpu_set_flags_optimize(cpu, CPU_OPTIMIZE_ALL);
	cpu_set_flags_debug(cpu, CPU_DEBUG_NONE);
	cpu_set_flags_arch(cpu, 
		CPU_6502_BRK_TRAP |
		CPU_6502_XXX_TRAP |
		CPU_6502_V_IGNORE);
	cpu_set_ram(cpu, RAM);

	cpu_init(cpu);

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

#define PC (((reg_6502_t*)cpu->reg)->pc)
#define A (((reg_6502_t*)cpu->reg)->a)
#define X (((reg_6502_t*)cpu->reg)->x)
#define Y (((reg_6502_t*)cpu->reg)->y)
#define S (((reg_6502_t*)cpu->reg)->s)
#define P (((reg_6502_t*)cpu->reg)->p)

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

				if (kernal_dispatch(RAM, &PC, &A, &X, &Y, &S, &P)) {
					// the runtime could handle it, so do an RTS
					PC = RAM[0x0100+(++(S))];
					PC |= (RAM[0x0100+(++(S))]<<8);
					PC++;
					continue;
				}
				
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
