#include "libcpu.h"
#include "tag_generic.h"
#include "tag.h"

/*
 * TODO: on architectures with constant instruction sizes,
 * this shouldn't waste extra tag data for every byte of
 * code memory, but have one tag per instruction location.
 */

static void
init_tagging(cpu_t *cpu)
{
	addr_t tagging_size, i;

	tagging_size = cpu->code_end - cpu->code_start;
	cpu->tagging_type = (tagging_type_t*)malloc(tagging_size * sizeof(tagging_type_t));
	for (i = 0; i < tagging_size; i++)
		cpu->tagging_type[i] = TAG_TYPE_UNKNOWN;
}

static bool
is_inside_code_area(cpu_t *cpu, addr_t a)
{
	return a >= cpu->code_start && a < cpu->code_end;
}

static void
or_tagging_type(cpu_t *cpu, addr_t a, tagging_type_t t)
{
	if (is_inside_code_area(cpu, a))
		cpu->tagging_type[a - cpu->code_start] |= t;
}

/* access functions */
tagging_type_t
get_tagging_type(cpu_t *cpu, addr_t a)
{
	if (is_inside_code_area(cpu, a))
		return cpu->tagging_type[a - cpu->code_start];
	else
		return TAG_TYPE_UNKNOWN;
}

bool
is_code(cpu_t *cpu, addr_t a)
{
	return !!(get_tagging_type(cpu, a) & TAG_TYPE_CODE);
}

extern void disasm_instr(cpu_t *cpu, addr_t pc);

static void
tag_recursive(cpu_t *cpu, addr_t pc, int level)
{
	int bytes;
	int flow_type;
	addr_t new_pc;

	for(;;) {
		if (!is_inside_code_area(cpu, pc))
			return;
		if (is_code(cpu, pc))	/* we have already been here, ignore */
			return;

#ifdef VERBOSE
		for (int i=0; i<level; i++) printf(" ");
		disasm_instr(cpu, pc);
#endif

		or_tagging_type(cpu, pc, TAG_TYPE_CODE);

		bytes = cpu->f.tag_instr(cpu, pc, &flow_type, &new_pc);
		
		switch (flow_type & ~FLOW_TYPE_CONDITIONAL) {
			case FLOW_TYPE_ERR:
			case FLOW_TYPE_RETURN:
				or_tagging_type(cpu, pc, TAG_TYPE_RET);
				/* execution ends here, the follwing location is not reached */
				return;
			case FLOW_TYPE_BRANCH:
				or_tagging_type(cpu, pc, TAG_TYPE_BRANCH);
				or_tagging_type(cpu, new_pc, TAG_TYPE_BRANCH_TARGET);
				tag_recursive(cpu, new_pc, level+1);
				if (!(flow_type & FLOW_TYPE_CONDITIONAL))
					return;
				or_tagging_type(cpu, pc+bytes, TAG_TYPE_AFTER_BRANCH);
				break;
			case FLOW_TYPE_CALL:
				/* tag subroutine, then continue with next instruction */
				or_tagging_type(cpu, pc, TAG_TYPE_CALL);
				or_tagging_type(cpu, new_pc, TAG_TYPE_SUBROUTINE);
				or_tagging_type(cpu, pc+bytes, TAG_TYPE_AFTER_CALL);
				tag_recursive(cpu, new_pc, level+1);
				break;
			case FLOW_TYPE_CONTINUE:
				break; /* continue with next instruction */
		}
		pc += bytes;
	}
}

void
cpu_tag(cpu_t *cpu, addr_t pc)
{
	/* for singlestep, we don't need this */
	if (cpu->flags_debug & CPU_DEBUG_SINGLESTEP)
		return;

	/* initialize data structure on demand */
	if (!cpu->tagging_type)
		init_tagging(cpu);

#if VERBOSE
	printf("starting tagging at $%02llx\n", (unsigned long long)pc);
#endif

	or_tagging_type(cpu, pc, TAG_TYPE_ENTRY); /* client wants to enter the guest code here */
	tag_recursive(cpu, pc, 0);
}

