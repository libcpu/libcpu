#include <libcpu.h>

#include "arch/arm/libcpu_arm.h"

#define START_NO 1000000000
//#define START_NO 10
#define RET_MAGIC 0x4D495354

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

	cpu = cpu_new(CPU_ARCH_ARM, CPU_FLAG_ENDIAN_LITTLE, 0);
	cpu_set_flags_optimize(cpu, CPU_OPTIMIZE_ALL);
//	cpu_set_flags_debug(cpu, CPU_DEBUG_NONE);
	cpu_set_flags_debug(cpu, CPU_DEBUG_PRINT_IR);
//	cpu_set_flags_debug(cpu, CPU_DEBUG_SINGLESTEP);
//	cpu_set_flags_debug(cpu, CPU_DEBUG_SINGLESTEP | CPU_DEBUG_PRINT_IR);
//	cpu_set_flags_arch(cpu, 
//		CPU_6502_BRK_TRAP |
//		CPU_6502_XXX_TRAP |
//		CPU_6502_V_IGNORE);
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

	cpu->code_start = 0;
	cpu->code_end = cpu->code_start + fread(&RAM[cpu->code_start], 1, ramsize-cpu->code_start, f);
	fclose(f);

	cpu->code_entry = cpu->code_start;

	cpu_tag(cpu, cpu->code_entry);

	if (entries && *entries == '@')
		tag_extra_filename(cpu, entries + 1);
	else
		tag_extra(cpu, entries); /* tag extra entry points from the command line */

#ifdef RET_OPTIMIZATION
	find_rets(RAM, cpu->code_start, cpu->code_end);
#endif

	printf("*** Executing...\n");

#define PC (((reg_arm_t*)cpu->rf.grf)->pc)
#define R (((reg_arm_t*)cpu->rf.grf)->r)

	PC = cpu->code_entry;
	R[0] = START_NO; // parameter
	R[14] = RET_MAGIC; // link register

	int step = 0;
	int i;

	for(;;) {
		breakpoint();
		int ret = cpu_run(cpu, debug_function);
		//printf("ret = %d\n", ret);

		if (PC == RET_MAGIC-4)
			break;

		switch (ret) {
			case JIT_RETURN_NOERR: /* JIT code wants us to end execution */
				break;
			case JIT_RETURN_SINGLESTEP:
				printf("%04X: ", PC);
				for (i=0; i<16; i++)
					printf("R%d=%x ", i, R[i]);
				printf("\n");
				debug_function(cpu);
				printf("::STEP:: %d\n", step++);
				cpu_flush(cpu);
				break;
			case JIT_RETURN_FUNCNOTFOUND:
				// bad :(
				printf("%s: error: $%04X not found!\n", __func__, PC);
				printf("PC: ");
				for (i=0; i<16; i++)
					printf("%02X ", RAM[PC+i]);
				printf("\n");
				exit(1);
			default:
				printf("unknown return code: %d\n", ret);
		}
	}

	printf("Result: %d\n", R[0]);

	printf("done.\n");

	return 0;
}
