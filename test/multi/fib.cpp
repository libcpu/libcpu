#define START 0
#define ENTRY 0

#include "timings.h"

#define START_NO 1000000000

#include <libcpu.h>
#include "arch/arm/arm_types.h"
#include "arch/m88k/m88k_isa.h"

#define RET_MAGIC 0x4D495354

static void
debug_function(cpu_t *cpu) {
	fprintf(stderr, "%s:%u\n", __FILE__, __LINE__);
}

int fib(int n)
{
    int f2=0;
    int f1=1;
    int fib=0;
    int i;
 
    if (n==0 || n==1)
        return n;
    for(i=2;i<=n;i++){
        fib=f1+f2; /*sum*/
        f2=f1;
        f1=fib;
    }
    return fib;
}

//////////////////////////////////////////////////////////////////////
#define SINGLESTEP_NONE	0
#define SINGLESTEP_STEP	1
#define SINGLESTEP_BB	2
//////////////////////////////////////////////////////////////////////

int
main(int argc, char **argv)
{
	char *s_arch;
	char *executable;
	cpu_arch_t arch;
	cpu_t *cpu;
	uint8_t *RAM;
	FILE *f;
	int ramsize;
	int r1, r2;
	uint64_t t1, t2, t3, t4;
	unsigned start_no = START_NO;

	int singlestep = SINGLESTEP_NONE;
	int log = 1;
	int print_ir = 1;

	/* parameter parsing */
	if (argc < 3) {
		printf("Usage: %s executable [arch] [itercount] [entries]\n", argv[0]);
		return 0;
	}
	s_arch = argv[1];
	executable = argv[2];
	if (argc >= 4)
		start_no = atoi(argv[3]);
	if (!strcmp("mips", s_arch))
		arch = CPU_ARCH_MIPS;
	else if (!strcmp("m88k", s_arch))
		arch = CPU_ARCH_M88K;
	else if (!strcmp("arm", s_arch))
		arch = CPU_ARCH_ARM;
	else if (!strcmp("fapra", s_arch))
		arch = CPU_ARCH_FAPRA;
	else {
		printf("unknown architecture '%s'!\n", s_arch);
		return 0;
	}

	ramsize = 5*1024*1024;
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

	printf("number of iterations: %u\n", start_no);

	uint32_t *reg_pc, *reg_lr, *reg_param, *reg_result;
	switch (arch) {
		case CPU_ARCH_M88K:
			reg_pc = &((m88k_grf_t*)cpu->rf.grf)->sxip;
			reg_lr = &((m88k_grf_t*)cpu->rf.grf)->r[1];
			reg_param = &((m88k_grf_t*)cpu->rf.grf)->r[2];
			reg_result = &((m88k_grf_t*)cpu->rf.grf)->r[2];
			break;
		case CPU_ARCH_MIPS:
			reg_pc = &((reg_mips32_t*)cpu->rf.grf)->pc;
			reg_lr = &((reg_mips32_t*)cpu->rf.grf)->r[31];
			reg_param = &((reg_mips32_t*)cpu->rf.grf)->r[4];
			reg_result = &((reg_mips32_t*)cpu->rf.grf)->r[4];
			break;
		case CPU_ARCH_ARM:
			reg_pc = &((reg_arm_t*)cpu->rf.grf)->pc;
			reg_lr = &((reg_arm_t*)cpu->rf.grf)->r[14];
			reg_param = &((reg_arm_t*)cpu->rf.grf)->r[0];
			reg_result = &((reg_arm_t*)cpu->rf.grf)->r[0];
			break;
		case CPU_ARCH_FAPRA:
			reg_pc = &((reg_fapra32_t*)cpu->rf.grf)->pc;
			reg_lr = &((reg_fapra32_t*)cpu->rf.grf)->r[0];
			reg_param = &((reg_fapra32_t*)cpu->rf.grf)->r[3];
			reg_result = &((reg_fapra32_t*)cpu->rf.grf)->r[3];
			break;
		default:
			fprintf(stderr, "architecture %u not handled.\n", arch);
			exit(EXIT_FAILURE);
	}

	*reg_pc = cpu->code_entry;
	*reg_lr = RET_MAGIC;
	*reg_param = start_no;

	printf("GUEST run..."); fflush(stdout);

	t1 = abs_time();
	cpu_run(cpu, debug_function);
	t2 = abs_time();
	r1 = *reg_result;

	printf("done!\n");

	printf("HOST  run..."); fflush(stdout);
	t3 = abs_time();
	r2 = fib(start_no);
	t4 = abs_time();
	printf("done!\n");

  cpu_free(cpu);

	printf("Time GUEST: %lld\n", t2-t1);
	printf("Time HOST:  %lld\n", t4-t3);
	printf("Result HOST:  %d\n", r2);
	printf("Result GUEST: %d\n", r1);
	printf("GUEST required \033[1m%.2f%%\033[22m of HOST time.\n",  (float)(t2-t1)/(float)(t4-t3)*100);
	if (r1 == r2)
		printf("\033[1mSUCCESS!\033[22m\n\n");
	else
		printf("\033[1mFAILED!\033[22m\n\n");

	return 0;
}
//printf("%d\n", __LINE__);
