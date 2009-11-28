#define BENCHMARK_FIB
//#define SINGLESTEP

#ifdef BENCHMARK_FIB
# define START 0
# define ENTRY 0
#else
# define START 0
# define ENTRY 0
#endif

#include "timings.h"

#define START_NO 1000000000
#define TIMES 100

#include <libcpu.h>
#include "arch/m88k/libcpu_m88k.h"
#include "arch/m88k/m88k_isa.h"

//////////////////////////////////////////////////////////////////////
// command line parsing helpers
//////////////////////////////////////////////////////////////////////

void tag_extra(cpu_t *cpu, char *entries)
{
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

void tag_extra_filename(cpu_t *cpu, char *filename)
{
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

static double
ieee754_fp80_to_double(fp80_reg_t reg)
{
#if defined(__i386__) || defined(__x86_64__)
	uint32_t sign      = (reg.i.hi & 0x8000) != 0;
	uint32_t exp       = reg.i.hi & 0x7fff;
	uint64_t mantissa  = reg.i.lo;

	return (double)reg.f;
#else
	uint32_t sign      = (reg.i.hi & 0x8000) != 0;
	uint32_t exp       = reg.i.hi & 0x7fff;
	uint64_t mantissa  = reg.i.lo << 1;

	uint64_t v64 = ((uint64_t)sign << 63) | 
		((uint64_t)(exp & 0x7000) << 48) |
		((uint64_t)(exp & 0x3ff) << 52) |
		(mantissa >> 12);

	return *(double *)&v64;
#endif
}

static void
ieee754_fp80_set_d(fp80_reg_t *reg, double v)
{
#if defined(__i386__) || defined(__x86_64__)
	reg->f = v;
#else
	uint64_t v64       = *(uint64_t *)&v;
	uint32_t sign      = (v64 >> 63);
	uint32_t exp       = (v64 >> 52) & 0x7ff;
	uint64_t mantissa  = v64 & ((1ULL << 52) - 1);

	mantissa <<= 11;
	mantissa |= (1ULL << 63);

	exp = ((exp & 0x700) << 4) | ((-((exp >> 9) & 1)) & 0xf00) | (exp & 0x3ff);

	reg->i.hi = (sign << 15) | exp;
	reg->i.lo = mantissa;
#endif
}


static void
dump_state(uint8_t *RAM, m88k_grf_t *reg, m88k_xrf_t *xrf)
{
	printf("%08llx:", (unsigned long long)reg->sxip);
	for (int i=0; i<32; i++) {
		if (!(i%4))
			printf("\n");
		printf("R%02d=%08x ", i, (unsigned int)reg->r[i]);
	}
	for (int i=0; xrf != NULL && i<32; i++) {
		if (!(i%2))
			printf("\n");
		printf("X%02d=%04x%016llx (%.8f) ", i,
			xrf->x[i].i.hi, xrf->x[i].i.lo,
			ieee754_fp80_to_double(xrf->x[i]));
	}
	uint32_t base = reg->r[31];
	for (int i=0; i<256 && i+base<65536; i+=4) {
		if (!(i%16))
			printf("\nSTACK: ");
		printf("%08x ", *(unsigned int*)&RAM[(base+i)]);
	}
	printf("\n");
}

static void
debug_function(cpu_t *cpu) {
	fprintf(stderr, "%s:%u\n", __FILE__, __LINE__);
}

#if 0
int fib(int n)
{
	if (n==0 || n==1)
		return n;
	else
		return fib(n-1) + fib(n-2);
}
#else
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
#endif

//////////////////////////////////////////////////////////////////////

int
main(int argc, char **argv)
{
	char *executable;
	char *entries;
	cpu_t *cpu;
	uint8_t *RAM;
	FILE *f;
	int ramsize;
	char *stack;
	int i;
#ifdef BENCHMARK_FIB
	int r1, r2;
	uint64_t t1, t2, t3, t4;
	unsigned start_no = START_NO;
#endif
#ifdef SINGLESTEP
	int step = 0;
#endif
	ramsize = 5*1024*1024;
	RAM = (uint8_t*)malloc(ramsize);

	cpu = cpu_new(CPU_ARCH_M88K, CPU_FLAG_ENDIAN_BIG, 0);

#ifdef SINGLESTEP
	cpu_set_flags_optimize(cpu, CPU_OPTIMIZE_ALL);
	cpu_set_flags_debug(cpu, CPU_DEBUG_SINGLESTEP | CPU_DEBUG_PRINT_IR | CPU_DEBUG_PRINT_IR_OPTIMIZED);
#else
	cpu_set_flags_optimize(cpu, CPU_OPTIMIZE_ALL);
//	cpu_set_flags_optimize(cpu, CPU_OPTIMIZE_NONE);
//	cpu_set_flags_optimize(cpu, 0x3eff);
//	cpu_set_flags_optimize(cpu, 0x3eff);
	cpu_set_flags_debug(cpu, CPU_DEBUG_PRINT_IR | CPU_DEBUG_PRINT_IR_OPTIMIZED);
#endif

	cpu_set_ram(cpu, RAM);
	
	/* parameter parsing */
	if (argc < 2) {
#ifdef BENCHMARK_FIB
		printf("Usage: %s executable [itercount] [entries]\n", argv[0]);
#else
		printf("Usage: %s executable [entries]\n", argv[0]);
#endif
		return 0;
	}

	executable = argv[1];
#ifdef BENCHMARK_FIB
	if (argc >= 3)
		start_no = atoi(argv[2]);

	if (argc >= 4)
		entries = argv[3];
	else
		entries = 0;
#else
	if (argc >= 3)
		entries = argv[2];
	else
		entries = 0;
#endif

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

	if (entries && *entries == '@')
		tag_extra_filename(cpu, entries + 1);
	else
		tag_extra(cpu, entries); /* tag extra entry points from the command line */

#ifdef RET_OPTIMIZATION
	find_rets(RAM, cpu->code_start, cpu->code_end);
#endif

	printf("*** Executing...\n");

#define STACK_SIZE 65536

	stack = (char *)(ramsize - STACK_SIZE); // THIS IS *GUEST* ADDRESS!

#define STACK ((long long)(stack+STACK_SIZE-4))

#define PC (((m88k_grf_t*)cpu->rf.grf)->sxip)
#define PSR (((m88k_grf_t*)cpu->rf.grf)->psr)
#define R (((m88k_grf_t*)cpu->rf.grf)->r)
#define X (((m88k_xrf_t*)cpu->rf.fpr)->x)

	PC = cpu->code_entry;
#if 0
	for (i = 1; i < 32; i++)
		R[i] = 0xF0000000 + i;	// DEBUG
#endif
	R[31] = STACK; // STACK
	R[1] = -1; // return address

#ifdef BENCHMARK_FIB//fib
	R[2] = start_no; // parameter
#else
	R[2] = 0x3f800000; // 1.0
	ieee754_fp80_set_d(&X[2], 1.0);
#endif
	dump_state(RAM, (m88k_grf_t*)cpu->rf.grf, NULL);

#ifdef SINGLESTEP
	for(step = 0;;) {
		printf("::STEP:: %d\n", step++);

		cpu_run(cpu, debug_function);

		dump_state(RAM, (m88k_grf_t*)cpu->reg);
		printf ("NPC=%08x\n", PC);

		if (PC == -1)
			break;

		cpu_flush(cpu);
		printf("*** PRESS <ENTER> TO CONTINUE ***\n");
		getchar();
	}
#else
	for(;;) {
		int ret;
		breakpoint();
		ret = cpu_run(cpu, debug_function);
		printf("ret = %d\n", ret);
		switch (ret) {
			case JIT_RETURN_NOERR: /* JIT code wants us to end execution */
				break;
			case JIT_RETURN_FUNCNOTFOUND:
				dump_state(RAM, (m88k_grf_t*)cpu->rf.grf, (m88k_xrf_t*)cpu->rf.frf);

				if (PC == -1)
					goto double_break;

				// bad :(
				printf("%s: error: $%llX not found!\n", __func__, (unsigned long long)PC);
				printf("PC: ");
				for (i = 0; i < 16; i++)
					printf("%02X ", RAM[PC+i]);
				printf("\n");
				exit(1);
			default:
				printf("unknown return code: %d\n", ret);
		}
	}
double_break:
#ifdef BENCHMARK_FIB
	printf("start_no=%u\n", start_no);

	printf("RUN1..."); fflush(stdout);
	PC = cpu->code_entry;

	for (i = 1; i < 32; i++)
		R[i] = 0xF0000000 + i;	// DEBUG

	R[31] = STACK; // STACK
	R[1] = -1; // return address

	R[2] = start_no; // parameter
	breakpoint();

	t1 = abs_time();
//	for (int i=0; i<TIMES; i++)
		cpu_run(cpu, debug_function);
	r1 = R[2];
	t2 = abs_time();

	printf("done!\n");
	
	dump_state(RAM, (m88k_grf_t*)cpu->rf.grf, NULL);

	printf("RUN2..."); fflush(stdout);
	t3 = abs_time();
//	for (int i=0; i<TIMES; i++)
		r2 = fib(start_no);
	t4 = abs_time();
	printf("done!\n");

	printf("%d -- %d\n", r1, r2);
	printf("%lld -- %lld\n", t2-t1, t4-t3);
	printf("%f%%\n",  (float)(t2-t1)/(float)(t4-t3));
#endif
#endif

	printf("done.\n");

	int base = 0x2000;
	for (int i=0; i<256; i+=4) {
		if (!(i%16))
			printf("\nDATA: ");
		printf("%08x ", *(unsigned int*)&RAM[(base+i)]);
	}
	printf("\n");

	return 0;
}
