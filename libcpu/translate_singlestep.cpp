/*
 * libcpu: translate_singlestep.cpp
 *
 * This translates a single instruction and hooks up all
 * basic blocks (branch target, taken, non-taken, ...)
 * so that execution will always exit after the instruction.
 */
#include "libcpu.h"
#include "disasm.h"
#include "tag.h"
#include "basicblock.h"
#include "translate.h"

//////////////////////////////////////////////////////////////////////
// single stepping
//////////////////////////////////////////////////////////////////////

BasicBlock *
create_singlestep_return_basicblock(cpu_t *cpu, addr_t new_pc, BasicBlock *bb_ret)
{
	BasicBlock *bb_branch = create_basicblock(cpu, new_pc, cpu->func_jitmain, BB_TYPE_NORMAL);
	emit_store_pc_return(cpu, bb_branch, new_pc, bb_ret);
	return bb_branch;
}

BasicBlock *
cpu_translate_singlestep(cpu_t *cpu, BasicBlock *bb_ret, BasicBlock *bb_trap)
{
	addr_t new_pc;
	tag_t tag;
	BasicBlock *cur_bb = NULL, *bb_target = NULL, *bb_next = NULL, *bb_cont = NULL;
	addr_t next_pc, pc = cpu->f.get_pc(cpu, cpu->rf.grf);

	cur_bb = BasicBlock::Create(_CTX(), "instruction", cpu->func_jitmain, 0);

	if (LOGGING)
		disasm_instr(cpu, pc);

	cpu->f.tag_instr(cpu, pc, &tag, &new_pc, &next_pc);

	/* get target basic block */
	if ((tag & TAG_RET) || (new_pc == NEW_PC_NONE)) /* translate_instr() will set PC */
		bb_target = bb_ret;
	else if (tag & (TAG_CALL|TAG_BRANCH))
		bb_target = create_singlestep_return_basicblock(cpu, new_pc, bb_ret);
	/* get not-taken & conditional basic block */
	if (tag & TAG_CONDITIONAL)
		bb_next = create_singlestep_return_basicblock(cpu, next_pc, bb_ret);

	bb_cont = translate_instr(cpu, pc, tag, bb_target, bb_trap, bb_next, cur_bb);

	/* If it's not a branch, append "store PC & return" to basic block */
	if (bb_cont)
		emit_store_pc_return(cpu, bb_cont, next_pc, bb_ret);

	return cur_bb;
}
