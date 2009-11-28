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
		//log("%04X: %d\n", pc, get_tag(cpu, pc));
		if (is_start_of_basicblock(cpu, pc)) {
			create_basicblock(cpu, pc, cpu->func_jitmain, BB_TYPE_NORMAL);
			bbs++;
		}
		pc++;
	}
	log("bbs: %d\n", bbs);

	// create dispatch basicblock
	BasicBlock* bb_dispatch = BasicBlock::Create(_CTX(), "dispatch", cpu->func_jitmain, 0);
	Value *v_pc = new LoadInst(cpu->ptr_PC, "", false, bb_dispatch);
	SwitchInst* sw = SwitchInst::Create(v_pc, bb_ret, bbs /*XXX upper bound, not accurate count!*/, bb_dispatch);

	for (pc = cpu->code_start; pc<cpu->code_end; pc++) {
		if (needs_dispatch_entry(cpu, pc)) {
			log("info: adding case: %llx\n", pc);
			ConstantInt* c = ConstantInt::get(getIntegerType(cpu->info.address_size), pc);
			BasicBlock *target = (BasicBlock*)lookup_basicblock(cpu, cpu->func_jitmain, pc, BB_TYPE_NORMAL);
			if (!target) {
				log("error: unknown rts target $%04llx!\n", (unsigned long long)pc);
				exit(1);
			} else {
				sw->addCase(c, target);
			}
		}
	}

// translate basic blocks
    Function::const_iterator it;
    for (it = cpu->func_jitmain->getBasicBlockList().begin(); it != cpu->func_jitmain->getBasicBlockList().end(); it++) {
		const BasicBlock *hack = it;
		BasicBlock *cur_bb = (BasicBlock*)hack;	/* cast away const */
		const char *cstr = (*it).getNameStr().c_str();
		if (cstr[0] != BB_TYPE_NORMAL)
			continue; // skip special blocks like entry, dispatch...
		pc = strtol(cstr+1, (char **)NULL, 16);
log("basicblock: L%08llx\n", (unsigned long long)pc);
		tag_t tag;
		BasicBlock *bb_target = NULL, *bb_next = NULL, *bb_cont = NULL;
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
				if (new_pc == NEW_PC_NONE) { /* translate_instr() will set PC */
					bb_target = bb_dispatch;
				} else {
					bb_target = (BasicBlock*)lookup_basicblock(cpu, cpu->func_jitmain, new_pc, BB_TYPE_NORMAL);
					if (!bb_target) { /* outside of code segment */
						bb_target = create_basicblock(cpu, new_pc, cpu->func_jitmain, BB_TYPE_EXTERNAL);
						emit_store_pc_return(cpu, bb_target, new_pc, bb_ret);
					}
				}
			}
			/* get not-taken basic block */
			if (tag & TAG_CONDITIONAL)
 				bb_next = (BasicBlock*)lookup_basicblock(cpu, cpu->func_jitmain, next_pc, BB_TYPE_NORMAL);

			bb_cont = translate_instr(cpu, pc, tag, bb_target, bb_trap, bb_next, cur_bb);

			pc = next_pc;
			
		} while (!(is_start_of_basicblock(cpu, pc)) && /* new basic block starts here */
				is_code(cpu, pc) && /* end of code section */ //XXX no: this is whether it's TAG_CODE
				bb_cont); /* last intruction jumped away */

		/* link with next basic block if there isn't a control flow instr. already */
		if (bb_cont) {
			BasicBlock *target = (BasicBlock*)lookup_basicblock(cpu, cpu->func_jitmain, pc, BB_TYPE_NORMAL);
			if (!target) {
				printf("error: unknown continue $%04llx!\n", (unsigned long long)pc);
				exit(1);
			}
			log("info: linking continue $%04llx!\n", (unsigned long long)pc);
			BranchInst::Create(target, bb_cont);
		}
    }

	return bb_dispatch;
}
