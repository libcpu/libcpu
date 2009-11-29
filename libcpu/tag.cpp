/*
 * libcpu: tag.cpp
 *
 * Do a depth search of all reachable code and associate
 * every reachable instruction with flags that indicate
 * instruction type (branch,call,ret, ...), flags
 * (conditional, ...) and code flow information (branch
 * target, ...)
 */
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

	cpu->tags_dirty = true;
}

bool
is_inside_code_area(cpu_t *cpu, addr_t a)
{
	return a >= cpu->code_start && a < cpu->code_end;
}

void
or_tag(cpu_t *cpu, addr_t a, tag_t t)
{
	if (is_inside_code_area(cpu, a)) {
		cpu->tag[a - cpu->code_start] |= t;
		cpu->tags_dirty = true;
	}
}

static void
and_tag(cpu_t *cpu, addr_t a, tag_t t)
{
	if (is_inside_code_area(cpu, a)) {
		cpu->tag[a - cpu->code_start] &= t;
		cpu->tags_dirty = true;
	}
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

	for(;;) {
		if (!is_inside_code_area(cpu, pc))
			return;
		if (is_code(cpu, pc))	/* we have already been here, ignore */
			return;

		if (LOGGING) {
			log("%*s", level, "");
			disasm_instr(cpu, pc);
		}

		bytes = cpu->f.tag_instr(cpu, pc, &tag, &new_pc, &next_pc);
		or_tag(cpu, pc, tag | TAG_CODE);

		if (tag & TAG_CONDITIONAL)
			or_tag(cpu, next_pc, TAG_AFTER_COND);

		if (tag & TAG_TRAP)	{
			/* regular trap - no code after it */
			if (!(cpu->flags_hint & (CPU_HINT_TRAP_RETURNS | CPU_HINT_TRAP_RETURNS_TWICE)))
				return;
			/*
			 * client hints that a trap will likely return,
			 * so tag code after it (optimization for usermode
			 * code that makes syscalls)
			 */
			or_tag(cpu, next_pc, TAG_AFTER_TRAP);
			/*
			 * client hints that a trap will likely return
			 * - to the next instruction AND
			 * - to the instruction after that
			 * OpenBSD on M88K skips an instruction on a trap
			 * return if there was an error.
			 */
			if (cpu->flags_hint & CPU_HINT_TRAP_RETURNS_TWICE) {
				tag_t dummy1;
				addr_t next_pc2, dummy2;
				next_pc2 = next_pc + cpu->f.tag_instr(cpu, next_pc, &dummy1, &dummy2, &dummy2);
				or_tag(cpu, next_pc2, TAG_AFTER_TRAP);
				tag_recursive(cpu, next_pc2, level+1);
			}
		}

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
tag_start(cpu_t *cpu, addr_t pc)
{
	/* initialize data structure on demand */
	if (!cpu->tag)
		init_tagging(cpu);

	log("starting tagging at $%02llx\n", (unsigned long long)pc);

	or_tag(cpu, pc, TAG_ENTRY); /* client wants to enter the guest code here */
	and_tag(cpu, pc, ~TAG_TRANSLATED); /* allow new functions to see this, even if we have seen it before */
	tag_recursive(cpu, pc, 0);
}
