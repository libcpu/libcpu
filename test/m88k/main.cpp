#define BENCHMARK_FIB

#ifdef BENCHMARK_FIB
# define START 0
# define ENTRY 0
#else
# define START 0x400670
# define ENTRY 0x52c
#endif
#define SINGLESTEP

#include <mach/mach_time.h>
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

static void
dump_state(uint8_t *RAM, m88k_regfile_t *reg)
{
	printf("%08llx:", (unsigned long long)reg->sxip);
	for (int i=0; i<32; i++) {
		if (!(i%4))
			printf("\n");
		printf("R%02d=%08x ", i, (unsigned int)reg->gpr[i]);
	}
	int base = reg->gpr[31];
	for (int i=0; i<256 && i+base<65536; i+=4) {
		if (!(i%16))
			printf("\nSTACK: ");
		printf("%08x ", *(unsigned int*)&RAM[(base+i)]);
	}
	printf("\n");
}

static void
debug_function(uint8_t *RAM, void *r)
{
//	reg_6502_t *reg = (reg_6502_t*)r;
//	printf("DEBUG: $%04X: A=$%02X X=$%02X Y=$%02X S=$%02X P=$%02X %02X/%02X\n", reg->pc, reg->a, reg->x, reg->y, reg->s, reg->p, RAM[0x33], RAM[0x34]);
//	{ int i; for (i=0x01F0; i<0x0200; i++) printf("%02X ", RAM[i]); printf("\n"); }
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

	cpu = cpu_new(CPU_ARCH_M88K);


#ifdef SINGLESTEP
	cpu_set_flags_optimize(cpu, CPU_OPTIMIZE_ALL);
	cpu_set_flags_debug(cpu, CPU_DEBUG_SINGLESTEP | CPU_DEBUG_PRINT_IR | CPU_DEBUG_PRINT_IR_OPTIMIZED);
#else
//	cpu_set_flags_optimize(cpu, CPU_OPTIMIZE_NONE);
	cpu_set_flags_optimize(cpu, CPU_OPTIMIZE_ALL);
	cpu_set_flags_debug(cpu, CPU_DEBUG_PRINT_IR | CPU_DEBUG_PRINT_IR_OPTIMIZED);
#endif

	cpu_set_flags_arch(cpu, CPU_M88K_IS_32BIT | CPU_M88K_IS_BE);
	cpu_set_ram(cpu, RAM);
	
	cpu_init(cpu);

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

#define PC (((m88k_regfile_t*)cpu->reg)->sxip)
#define R (((m88k_regfile_t*)cpu->reg)->gpr)

	PC = cpu->code_entry;

	for (i = 1; i < 32; i++)
		R[i] = 0xF0000000 + i;	// DEBUG

	R[31] = STACK; // STACK
	R[1] = -1; // return address

#ifdef BENCHMARK_FIB//fib
	R[2] = start_no; // parameter
#else
define STRING "HelloHelloHelloHelloHelloHelloHelloHelloHelloHello\n"
	R[4] = 0x1000;
	R[5] = strlen(STRING);
	R[6] = 0x2000;
	strcpy((char*)&RAM[R[4]], STRING);
#endif
	dump_state(RAM, (m88k_regfile_t*)cpu->reg);

#ifdef SINGLESTEP
	for(step = 0;;) {
		printf("::STEP:: %d\n", step++);

		cpu_run(cpu, debug_function);

		dump_state(RAM, (m88k_regfile_t*)cpu->reg);
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
				dump_state(RAM, (m88k_regfile_t*)cpu->reg);

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

	t1 = mach_absolute_time();
//	for (int i=0; i<TIMES; i++)
		cpu_run(cpu, debug_function);
	r1 = R[2];
	t2 = mach_absolute_time();

	printf("done!\n");
	
	dump_state(RAM, (m88k_regfile_t*)cpu->reg);

	printf("RUN2..."); fflush(stdout);
	t3 = mach_absolute_time();
//	for (int i=0; i<TIMES; i++)
		r2 = fib(start_no);
	t4 = mach_absolute_time();
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
