#include "libcpu.h"
#include "tag.h"

/*
 * TODO: on architectures with constant instruction sizes,
 * this shouldn't waste extra tag data for every byte of
 * code memory, but have one tag per instruction location.
 */

static void
init_tagging(cpu_t *cpu)
{
	addr_t nitems, i;

	nitems = cpu->code_end - cpu->code_start;
	cpu->tag = (tag_t*)malloc(nitems * sizeof(tag_t));
	for (i = 0; i < nitems; i++)
		cpu->tag[i] = TAG_UNKNOWN;
}

static bool
is_inside_code_area(cpu_t *cpu, addr_t a)
{
	return a >= cpu->code_start && a < cpu->code_end;
}

static void
or_tag(cpu_t *cpu, addr_t a, tag_t t)
{
	if (is_inside_code_area(cpu, a))
		cpu->tag[a - cpu->code_start] |= t;
}

/* access functions */
tag_t
get_tag(cpu_t *cpu, addr_t a)
{
	if (is_inside_code_area(cpu, a))
		return cpu->tag[a - cpu->code_start];
	else
		return TAG_UNKNOWN;
}

bool
is_code(cpu_t *cpu, addr_t a)
{
	return !!(get_tag(cpu, a) & TAG_CODE);
}

extern void disasm_instr(cpu_t *cpu, addr_t pc);

static void
tag_recursive(cpu_t *cpu, addr_t pc, int level)
{
	int bytes;
	tag_t tag;
	addr_t new_pc, next_pc;
	tag_t dummy1;
	addr_t dummy2;

	for(;;) {
		if (!is_inside_code_area(cpu, pc))
			return;
		if (is_code(cpu, pc))	/* we have already been here, ignore */
			return;

#ifdef VERBOSE
		for (int i=0; i<level; i++) printf(" ");
		disasm_instr(cpu, pc);
#endif

		bytes = cpu->f.tag_instr(cpu, pc, &tag, &new_pc, &next_pc);
		or_tag(cpu, pc, tag | TAG_CODE);

		if (tag & TAG_CONDITIONAL)
			or_tag(cpu, next_pc, TAG_AFTER_COND);

		if (tag & TAG_TRAP)	/* code ends here and continues at the next instruction */
			or_tag(cpu, next_pc, TAG_AFTER_TRAP);

		if (tag & TAG_CALL) {
			/* tag subroutine, then continue with next instruction */
			or_tag(cpu, new_pc, TAG_SUBROUTINE);
			or_tag(cpu, next_pc, TAG_AFTER_CALL);
			tag_recursive(cpu, new_pc, level+1);
		}

		if (tag & TAG_BRANCH) {
			or_tag(cpu, new_pc, TAG_BRANCH_TARGET);
			tag_recursive(cpu, new_pc, level+1);
			if (!(tag & TAG_CONDITIONAL))
				return;
		}

		if (tag & TAG_RET)	/* execution ends here, the follwing location is not reached */
			return;

		pc = next_pc;
	}
}

void
cpu_tag(cpu_t *cpu, addr_t pc)
{
	/* for singlestep, we don't need this */
	if (cpu->flags_debug & CPU_DEBUG_SINGLESTEP)
		return;

	/* initialize data structure on demand */
	if (!cpu->tag)
		init_tagging(cpu);

#if VERBOSE
	printf("starting tagging at $%02llx\n", (unsigned long long)pc);
#endif

	or_tag(cpu, pc, TAG_ENTRY); /* client wants to enter the guest code here */
	tag_recursive(cpu, pc, 0);
}

