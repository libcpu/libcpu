/*
 * libcpu: translate_singlestep_bb.cpp
 *
 * This translates all code until we reach a control flow instruction.
 * Tight loops will not exit, but loop inside the translation.
 */
#include "libcpu.h"
#include "basicblock.h"
#include "disasm.h"
#include "tag.h"
#include "translate.h"
#include "translate_singlestep.h"

BasicBlock *
cpu_translate_singlestep_bb(cpu_t *cpu, BasicBlock *bb_ret, BasicBlock *bb_trap)
{
	addr_t entry = cpu->f.get_pc(cpu, cpu->rf.grf);
	addr_t pc = entry;

	BasicBlock *cur_bb = create_basicblock(cpu, pc, cpu->func_jitmain, BB_TYPE_NORMAL);

	tag_t tag;
	BasicBlock *bb_target = NULL, *bb_next = NULL, *bb_delay = NULL, *bb_cont = NULL;
	do {
//printf("%s:%d\n", __func__, __LINE__);
		addr_t new_pc, next_pc;

		if (LOGGING)
			disasm_instr(cpu, pc);

		cpu->f.tag_instr(cpu, pc, &tag, &new_pc, &next_pc);

		/* get target basic block */
		if (tag & TAG_RET)
			bb_target = bb_ret;
		if (tag & (TAG_CALL|TAG_BRANCH)) {
			if (new_pc == NEW_PC_NONE) { /* translate_instr() will set PC */
				bb_target = bb_ret;
			} else {
				if (new_pc == entry)	/* tight loop */
					bb_target = cur_bb;
				else
					bb_target = create_singlestep_return_basicblock(cpu, new_pc, bb_ret);
			}
		}
		/* get not-taken basic block */
		if (tag & TAG_CONDITIONAL)
			bb_next = create_singlestep_return_basicblock(cpu, next_pc, bb_ret);

		bb_cont = translate_instr(cpu, pc, tag, bb_target, bb_trap, bb_next, cur_bb);

		pc = next_pc;
		
	} while (is_inside_code_area(cpu, pc) && /* end of code section */
			bb_cont); /* last intruction jumped away */

	return cur_bb;
}
