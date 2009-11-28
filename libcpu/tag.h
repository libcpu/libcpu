#include "libcpu/libcpu.h"

/* base types to be returned by tag_instr() */
#define TAG_CONTINUE	(1<<0)	/* instr does not deal with code flow */
#define TAG_CALL		(1<<1)	/* instr is a (conditional or unconditional) subroutine call */
#define TAG_RET			(1<<2)	/* instr is a return from a subroutine */
#define TAG_BRANCH		(1<<3)	/* instr is a (conditional or unconditional) jump/branch */
#define TAG_TRAP		(1<<4)	/* instr is a software trap, e.g. system calls, soft interrupts */

/* flags to be returned by tag_instr */
#define TAG_CONDITIONAL	(1<<5)	/* this is a conditional instr, e.g. conditional move */
#define TAG_DELAY_SLOT	(1<<6)	/* there is a delay slot following (only in comb. w/ CALL/RET/BRANCH */

#define TAG_COND_BRANCH (TAG_BRANCH|TAG_CONDITIONAL)

/* flags internal to libcpu */
#define TAG_CODE		(1<<7)	/* there is a reachable executable instruction here */
#define TAG_BRANCH_TARGET (1<<8)/* target of a (conditional or unconditional) jump/branch */
#define TAG_SUBROUTINE	(1<<9)	/* target of a (conditional or unconditional) subroutine call */
#define TAG_AFTER_CALL	(1<<10)	/* execution continues here after a subroutine call returns */
#define TAG_AFTER_COND	(1<<11)	/* execution continues here if a conditional instr is not taken */
#define TAG_ENTRY		(1<<12)	/* the client wants to be able to start execution at this instruction */
#define TAG_AFTER_TRAP	(1<<13)	/* execution continues here after a trap reenters translation unit */

#define TAG_UNKNOWN      0	/* unused (or not yet discovered) code or data */

tag_t get_tag(cpu_t *cpu, addr_t a);
bool is_inside_code_area(cpu_t *cpu, addr_t a);
bool is_code(cpu_t *cpu, addr_t a);
void tag_start(cpu_t *cpu, addr_t pc);

/*
 * NEW_PC_NONE states that the destination of a call is unknown.
 * If the bitness of the guest is less than that of the host, this
 * is an invalid address for the guest, otherwise it's the top of
 * the address space minus 1, which should be the most unlikely
 * address for code. The tagging code assumes that NEW_PC_NONE
 * is outside of code_start and code_end.
 */
#define NEW_PC_NONE (addr_t)-1

