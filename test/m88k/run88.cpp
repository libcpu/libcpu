#include <sys/types.h>
#include <sys/mman.h>
#include <libcpu.h>
#include "arch/m88k/libcpu_m88k.h"
#include "arch/m88k/m88k_isa.h"

#include "obsd41/openbsd41.h"
#include "xec-us-syscall-if.h"
#include "xec-us-syscall.h"
#include "xec-byte-order.h"
#include "xec-debug.h"
#include "nix.h"
#include "loader.h"

#define DEBUGGER

#define RAM_SIZE (16 * 1024 * 1024)
#define STACK_TOP ((long long)(RAM+RAM_SIZE-4))

#define PC (((m88k_grf_t*)cpu->reg)->sxip)
#define TRAPNO (((m88k_grf_t*)cpu->reg)->trapno)
#define PSR (((m88k_grf_t*)cpu->reg)->psr)
#define R (((m88k_grf_t*)cpu->reg)->r)

static size_t host_page_size;
static uint8_t *RAM;

/* XEC Mem If */
static xec_haddr_t
run88_mem_gtoh(xec_mem_if_t *self, xec_gaddr_t addr, xec_mem_flg_t *mf)
{
	*mf = 0;
#if 0
	if (addr >= RAM_SIZE) {
		*mf = XEC_MEM_VADDR | XEC_MEM_INVALID | XEC_MEM_NOT_PRESENT;
		return 0;
	}
#endif
	return (xec_haddr_t)((uintptr_t)RAM + addr);
}

static xec_gaddr_t
run88_mem_htog(xec_mem_if_t *self, xec_haddr_t addr, xec_mem_flg_t *mf)
{
	*mf = 0;
	return (xec_gaddr_t)((uintptr_t)addr - (uintptr_t)RAM);
}

static xec_gaddr_t
run88_mem_gmap(xec_mem_if_t *self, xec_haddr_t addr, size_t len, unsigned flags)
{
	uintptr_t dist;

	if (len >= 0x1000000) {
		assert(0 && "the guest passed insane value!");
		return (xec_gaddr_t)(-1);
	}

	dist = addr - (xec_haddr_t)RAM;
	fprintf(stderr, "GMAP: %llx || %p -> %llx\n",
		(unsigned long long)addr, RAM, (unsigned long long)dist);

	if (sizeof(dist) == sizeof(uint64_t)) {
		if (dist >= RAM_SIZE && dist < (uintptr_t)(4ULL * 1024 * 1024 * 1024))
			return (dist);
		else
			assert(0 && "The address isn't in the low 4G!");
	}

	return (xec_gaddr_t)(-1);
}

static xec_mem_if_vtbl_t const run88_mem_if_vtbl = {
	run88_mem_gmap,

	run88_mem_htog,
	run88_mem_gtoh,

	NULL,
	NULL
};

static xec_mem_if_t *
run88_new_mem_if(void)
{
	xec_mem_if_t *mem_if;

	mem_if = (xec_mem_if_t *)malloc(sizeof(xec_mem_if_t));
	if (mem_if != NULL)
		mem_if->vtbl = &run88_mem_if_vtbl;

	return mem_if;
}

extern "C" void __xec_log_init(void); // XXX move this away
extern "C" void obsd41_init(void); // XXX move this away
extern "C" xec_us_syscall_if_t *obsd41_us_syscall_create(xec_mem_if_t *memif); // XXX move this away

#define FIX_BOGUS_HOME 1

/*
 * OpenBSD/m88k userspace frame layout construction
 */
typedef uint32_t m88k_uintptr_t; // XXX
void *g_uframe_log = NULL;

void
openbsd_m88k_setup_uframe(cpu_t           *cpu,
						  xec_mem_if_t    *mem,
						  int              argc,
						  char           **argv,
						  char           **envp,
						  m88k_uintptr_t  *stack_top)
{
	char          **p;
	char           *base;
	char           *ptr;
	char           *ap; /* arg */
	int             n;
	xec_mem_flg_t   mf;
	int             envc   = 0;
	size_t          arglen = 0;
	size_t          envlen = 0;
	size_t          totlen = 0;
	size_t          frmlen = 0;
	m88k_uintptr_t  stack_base = 0;
	struct uframe {
		int32_t argc;
		m88k_uintptr_t argv[1];
	} *uframe;

	for (n = 0; n < argc; n++) {
		frmlen += sizeof(m88k_uintptr_t);
		arglen += strlen(argv[n]) + 1; /* zero */
	}

	if (envp != NULL) {
		for (p = envp, envc = 0; *p != NULL; p++, envc++) {
			frmlen += sizeof(m88k_uintptr_t);
#ifdef FIX_BOGUS_HOME
			if (strncmp(*p, "HOME=", 5) == 0)
			  {
				if ((*p)[strlen(*p) - 1] != '/')
				  envlen++;
			  }
#endif
			envlen += strlen(*p) + 1; /* zero */
		}
	}

	frmlen += sizeof(m88k_uintptr_t);
	frmlen += sizeof(m88k_uintptr_t);

	frmlen += sizeof(*uframe);
	totlen = (frmlen + arglen + envlen + 4095) & -4096;

	XEC_LOG(g_uframe_log, XEC_LOG_DEBUG, 0,
			"Total Length = %u Frame Length = %u Environ Length = %u Args Length = %u",
			totlen, frmlen, envlen, arglen);

#if 0
	XEC_LOG(g_uframe_log, XEC_LOG_DEBUG, 0,
			"stack: top [guest: %08lx host: %p] base [guest: %08lx host: %p]",
			(unsigned long)g_stack_top, base,
			(unsigned long)STACK_BASE, stack_base);

#endif
#define STACK_GUEST(x) ((uintptr_t)(x) - (uintptr_t)RAM)

	/* Begin of uframe */
	uframe = (struct uframe *)((uintptr_t)STACK_TOP - totlen);
	XEC_LOG(g_uframe_log, XEC_LOG_DEBUG, 0, "uframe = guest:%08lx host:%p",
			(unsigned long)STACK_GUEST(uframe), uframe);
	/* Begin of strings */
	ap = (char *)uframe + frmlen;

	uframe->argc = xec_byte_swap_big_to_host32(argc);

	for (n = 0; n < argc; n++) {
		XEC_LOG(g_uframe_log, XEC_LOG_DEBUG, 0, "argv[%zu] = guest:%08lx host:%p",
				n, (unsigned long)STACK_GUEST(ap), ap);
		uframe->argv[n] = xec_byte_swap_big_to_host32(STACK_GUEST(ap));
		strcpy(ap, argv[n]);
		ap += strlen(argv[n]) + 1;
	}

	uframe->argv[argc] = 0;

	for (n = 0; n < envc; n++) {
#ifdef FIX_BOGUS_HOME
		int extra = 0;
#endif

		XEC_LOG(g_uframe_log, XEC_LOG_DEBUG, 0, "envp[%zu] = guest:%08lx host:%p",
				n, (unsigned long)STACK_GUEST (ap), ap);
		uframe->argv[n + argc + 1] = xec_byte_swap_big_to_host32(STACK_GUEST(ap));
		strcpy(ap, envp[n]);
#ifdef FIX_BOGUS_HOME
		if (strncmp(ap, "HOME=", 5) == 0) {
			if (ap[strlen(ap) - 1] != '/') {
				strcat(ap, "/");
				extra = 1;
			}
		}
#endif
		ap += strlen(envp[n]) + extra + 1;
	}

	uframe->argv[argc + envc + 1] = 0;

	*stack_top = STACK_GUEST(uframe);

	XEC_LOG(g_uframe_log, XEC_LOG_DEBUG, 0, "Guest Stack Pointer = %lx", (unsigned long)(*stack_top));
}

static void
debug_function(cpu_t *cpu)
{
	fprintf(stderr, "%s:%u [trap %u]\n", __FILE__, __LINE__, ((m88k_grf_t*)cpu->reg)->r[13]);
}

static void
dump_state(uint8_t *RAM, m88k_grf_t *reg)
{
	printf("%08llx:", (unsigned long long)reg->sxip);
	for (int i=0; i<32; i++) {
		if (!(i%4))
			printf("\n");
		printf("R%02d=%08x ", i, (unsigned int)reg->r[i]);
	}
	int base = reg->r[31];
	for (int i=0; i<256 && i+base<65536; i+=4) {
		if (!(i%16))
			printf("\nSTACK: ");
		printf("%08x ", *(unsigned int*)&RAM[(base+i)]);
	}
	printf("\n");
}

void
aspace_lock(void)
{
#if defined(__x86_64__)
	static size_t const four_gigs = 4ULL*1024*1024*1024;

	if (host_page_size == 0)
		host_page_size = getpagesize();

	/* Unmap the low 4G by the kernel, it may be mapped using a single 4G page! */
	munmap(NULL, four_gigs);

	/* Remap 4G immediatly */
	RAM = (uint8_t *)mmap(NULL, four_gigs, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE|MAP_FIXED, -1, 0);
	if (RAM == (uint8_t *)MAP_FAILED) {
		fprintf(stderr, "error: cannot allocate %u bytes\n", RAM_SIZE);
		exit(EXIT_FAILURE);
	}

	/* Protect page zero for null deref */
	mprotect(NULL, host_page_size, 0);
#else
#warning "Not tested on this arch."
#endif
}

void
aspace_unlock(void)
{
#ifdef __x86_64__
	/* Unmap as much as possible. */
	munmap((void*)RAM_SIZE, 0x100000000 - RAM_SIZE);
#endif
}


int
main(int ac, char **av, char **ep)
{
	int rc;
	cpu_t *cpu;
	xec_guest_info_t guest_info;
	xec_mem_if_t *mem_if;
	xec_monitor_t *monitor;
	xec_us_syscall_if_t *us_syscall;
	m88k_uintptr_t stack_top;
	nix_env_t *env;
	bool debugging = false;

	aspace_lock();

	if (ac < 2) {
		fprintf(stderr, "usage: %s <executable> [args...]\n", *av);
		exit(EXIT_FAILURE);
	}

	/* Initialize xec, nix and loader. */
	xec_init();
	obsd41_init();
	loader_init();

	/* Create CPU */
	cpu = cpu_new(CPU_ARCH_M88K);
	if (cpu == NULL) {
		fprintf(stderr, "error: failed initializing M88K architecture.\n");
		exit(EXIT_FAILURE);
	}

	/* Create XEC bridge mem-if */
	mem_if = run88_new_mem_if();

	/* Create the XEC US Syscall */
	us_syscall = obsd41_us_syscall_create(mem_if);
	if (us_syscall == NULL) {
		fprintf(stderr, "error: failed creating xec userspace syscall.\n");
		exit(EXIT_FAILURE);
	}

	/* Create NIX env */
	env = nix_env_create(mem_if);
	if (env == NULL) {
		fprintf(stderr, "error: failed creating nix environment.\n");
		exit(EXIT_FAILURE);
	}

	/* Load the executable */
	rc = loader_load(mem_if, av[1]);
	if (rc != LOADER_SUCCESS) {
		fprintf(stderr, "error: cannot load executable '%s', error=%d.\n", av[1], rc);
		exit(EXIT_FAILURE);
	}

	/* Setup arguments */
	g_uframe_log = xec_log_register("uframe");
	openbsd_m88k_setup_uframe(cpu, mem_if, ac - 1, av + 1, NULL, &stack_top);

	/* Setup and initialize the CPU */
	cpu_set_flags_arch(cpu, CPU_M88K_IS_32BIT | CPU_M88K_IS_BE);
	cpu_set_flags_optimize(cpu, CPU_OPTIMIZE_NONE);
	cpu_set_flags_debug(cpu, 0);
	cpu_set_flags_hint(cpu, CPU_HINT_TRAP_RETURNS_TWICE);
	cpu_set_ram(cpu, RAM);

	cpu_init(cpu);

	/* Create XEC bridge monitor */
	guest_info.name = "m88k";
	guest_info.endian = XEC_ENDIAN_BIG;
	guest_info.byte_size = 8;
	guest_info.word_size = 32;
	guest_info.page_size = 4096;
	monitor = xec_monitor_create(&guest_info, mem_if, cpu->reg, NULL);
	if (monitor == NULL) {
		fprintf(stderr, "error: failed createc xec monitor.\n");
		exit(EXIT_FAILURE);
	}

	/* Setup registers for execution */
	PC = g_ahdr.entry;

	R[31] = stack_top; // Stack Pointer
	R[1]  = -1;        // Return Address

	cpu->code_start = g_ahdr.tstart;
	cpu->code_end   = g_ahdr.tstart + g_ahdr.tsize;
	cpu->code_entry = g_ahdr.entry;

	cpu_tag(cpu, cpu->code_entry);

	dump_state(RAM, (m88k_grf_t*)cpu->reg);

#ifdef DEBUGGER
	debugging = true;
#endif

	aspace_unlock();

	for (;;) {
		if (debugging) {
			rc = cpu_debugger(cpu, debug_function);
			if (rc < 0) {
				debugging = false;
				continue;
			}
		} else {
			rc = cpu_run(cpu, debug_function);
		}

		switch (rc) {
			case JIT_RETURN_NOERR: /* JIT code wants us to end execution */
				break;

			case JIT_RETURN_FUNCNOTFOUND:
				dump_state(RAM, (m88k_grf_t*)cpu->reg);

				if (PC == -1)
					goto double_break;

				// bad :(
				printf("%s: error: 0x%llX not found!\n", __func__, (unsigned long long)PC);
				printf("PC: ");
				for (size_t i = 0; i < 16; i++)
					printf("%02X ", RAM[PC+i]);
				printf("\n");
				exit(EXIT_FAILURE);
				break;

			case JIT_RETURN_TRAP:
				printf("TRAP %u / %u!\n", TRAPNO, R[13]);
				xec_us_syscall_dispatch(us_syscall, monitor);
				break;

			default:
				printf("unknown return code: %d\n", rc);
				break;
		}
	}

double_break:

	fprintf(stderr, "yatta!\n");

	exit(EXIT_SUCCESS);
}
