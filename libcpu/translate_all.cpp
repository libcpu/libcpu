/*
 * libcpu: translate_all.cpp
 *
 * This translates all known code by creating basic blocks and
 * filling them with instructions.
 */
#include "libcpu.h"
#include "basicblock.h"
#include "disasm.h"
#include "tag.h"
#include "translate.h"


BasicBlock *
cpu_translate_all(cpu_t *cpu, BasicBlock *bb_ret, BasicBlock *bb_trap)
{
	// find all instructions that need labels and create basic blocks for them
	int bbs = 0;
	addr_t pc;
	pc = cpu->code_start;
	while (pc < cpu->code_end) {
		// Do not create the basic block if it is already present in some other function.
		if (is_start_of_basicblock(cpu, pc) && !(get_tag(cpu, pc) & TAG_TRANSLATED)) {
			create_basicblock(cpu, pc, cpu->cur_func, BB_TYPE_NORMAL);
			bbs++;
		}
		pc++;
	}
	log("bbs: %d\n", bbs);

	// create dispatch basicblock
	BasicBlock* bb_dispatch = BasicBlock::Create(_CTX(), "dispatch", cpu->cur_func, 0);
	Value *v_pc = new LoadInst(cpu->ptr_PC, "", false, bb_dispatch);
	SwitchInst* sw = SwitchInst::Create(v_pc, bb_ret, bbs, bb_dispatch);

	// translate basic blocks
	bbaddr_map &bb_addr = cpu->func_bb[cpu->cur_func];
	bbaddr_map::const_iterator it;
	for (it = bb_addr.begin(); it != bb_addr.end(); it++) {
		pc = it->first;
		BasicBlock *cur_bb = it->second;

		tag_t tag;
		BasicBlock *bb_target = NULL, *bb_next = NULL, *bb_cont = NULL;

		// Tag the function as translated.
		or_tag(cpu, pc, TAG_TRANSLATED);

		log("basicblock: L%08llx\n", (unsigned long long)pc);

		// Add dispatch switch case for basic block.
		ConstantInt* c = ConstantInt::get(getIntegerType(cpu->info.address_size), pc);
		sw->addCase(c, cur_bb);

		do {
			tag_t dummy1;

			if (LOGGING)
				disasm_instr(cpu, pc);

			tag = get_tag(cpu, pc);

			/* get address of the following instruction */
			addr_t new_pc, next_pc;
			cpu->f.tag_instr(cpu, pc, &dummy1, &new_pc, &next_pc);

			/* get target basic block */
			if (tag & TAG_RET)
				bb_target = bb_dispatch;
			if (tag & (TAG_CALL|TAG_BRANCH)) {
				if (new_pc == NEW_PC_NONE) /* translate_instr() will set PC */
					bb_target = bb_dispatch;
				else
					bb_target = (BasicBlock*)lookup_basicblock(cpu, cpu->cur_func, new_pc, bb_ret, BB_TYPE_NORMAL);
			}
			/* get not-taken basic block */
			if (tag & TAG_CONDITIONAL)
 				bb_next = (BasicBlock*)lookup_basicblock(cpu, cpu->cur_func, next_pc, bb_ret, BB_TYPE_NORMAL);

			bb_cont = translate_instr(cpu, pc, tag, bb_target, bb_trap, bb_next, cur_bb);

			pc = next_pc;
			
		} while (
					/* new basic block starts here (and we haven't translated it yet)*/
					(!is_start_of_basicblock(cpu, pc)) &&
					/* end of code section */ //XXX no: this is whether it's TAG_CODE
					is_code(cpu, pc) &&
					/* last intruction jumped away */
					bb_cont
				);

		/* link with next basic block if there isn't a control flow instr. already */
		if (bb_cont) {
			BasicBlock *target = (BasicBlock*)lookup_basicblock(cpu, cpu->cur_func, pc, bb_ret, BB_TYPE_NORMAL);
			log("info: linking continue $%04llx!\n", (unsigned long long)pc);
			BranchInst::Create(target, bb_cont);
		}
    }

	return bb_dispatch;
}
