#define START 0
#define ENTRY 0

#include "timings.h"

#include <libcpu.h>
#include "arch/mips/libcpu_mips.h"
#include "arch/arm/libcpu_arm.h"
#include "arch/m88k/libcpu_m88k.h"
#include "arch/m88k/m88k_isa.h"
#include "arch/x86/libcpu_x86.h"

#define RET_MAGIC 0x4D495354

static void
debug_function(cpu_t *cpu)
{
	fprintf(stderr, "%s:%u\n", __FILE__, __LINE__);
}

//////////////////////////////////////////////////////////////////////
#define SINGLESTEP_NONE	0
#define SINGLESTEP_STEP	1
#define SINGLESTEP_BB	2
//////////////////////////////////////////////////////////////////////

int
main(int argc, char **argv)
{
	char *executable;
	cpu_arch_t arch;
	cpu_t *cpu;
	uint8_t *RAM;
	FILE *f;
	int ramsize;

	int singlestep = SINGLESTEP_NONE;
	int log = 1;
	int print_ir = 0;

	/* parameter parsing */
	if (argc < 2) {
		printf("Usage: %s [--print-ir] <binary>\n", argv[0]);
		return 0;
	}
	if (!strcmp(argv[1], "--print-ir")) {
		print_ir = 1;
		argv++;
	}
	executable = argv[1];
	arch = CPU_ARCH_8086;

	ramsize = 1*1024*1024;
	RAM = (uint8_t*)malloc(ramsize);

	cpu = cpu_new(arch, 0, 0);

	cpu_set_flags_codegen(cpu, CPU_CODEGEN_OPTIMIZE);
	cpu_set_flags_debug(cpu, 0
		| (print_ir? CPU_DEBUG_PRINT_IR : 0)
		| (print_ir? CPU_DEBUG_PRINT_IR_OPTIMIZED : 0)
		| (log? CPU_DEBUG_LOG :0)
		| (singlestep == SINGLESTEP_STEP? CPU_DEBUG_SINGLESTEP    : 0)
		| (singlestep == SINGLESTEP_BB?   CPU_DEBUG_SINGLESTEP_BB : 0)
		);

	cpu_set_ram(cpu, RAM);
	
	/* load code */
	if (!(f = fopen(executable, "rb"))) {
		printf("Could not open %s!\n", executable);
		return 2;
	}
	cpu->code_start = START;
	cpu->code_end = cpu->code_start + fread(&RAM[cpu->code_start], 1, ramsize-cpu->code_start, f);
	fclose(f);
	cpu->code_entry = cpu->code_start + ENTRY;

	cpu_tag(cpu, cpu->code_entry);

	cpu_translate(cpu); /* force translation now */

	printf("\n*** Executing...\n");

	printf("GUEST run..."); fflush(stdout);

	cpu_run(cpu, debug_function);

	printf("done!\n");

	cpu_free(cpu);

	return 0;
}
