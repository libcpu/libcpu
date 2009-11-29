/*
 * libcpu: basicblock.cpp
 *
 * Basic block handling (create, lookup)
 */
#include "libcpu.h"
#include "basicblock.h"
#include "tag.h"

bool
is_start_of_basicblock(cpu_t *cpu, addr_t a)
{
	return !!(get_tag(cpu, a) &
		(TAG_BRANCH_TARGET |	/* someone jumps/branches here */
		 TAG_SUBROUTINE |		/* someone calls this */
		 TAG_AFTER_CALL |		/* instruction after a call */
		 TAG_AFTER_COND |		/* instruction after a branch */
		 TAG_AFTER_TRAP |		/* instruction after a trap */
		 TAG_ENTRY));			/* client wants to enter guest code here */
}

bool
needs_dispatch_entry(cpu_t *cpu, addr_t a)
{
	return !!(get_tag(cpu, a) &
		(TAG_ENTRY |			/* client wants to enter guest code here */
		 TAG_AFTER_CALL |		/* instruction after a call */
		 TAG_AFTER_TRAP));		/* instruction after a call */
}

void
emit_store_pc(cpu_t *cpu, BasicBlock *bb_branch, addr_t new_pc)
{
	Value *v_pc = ConstantInt::get(getIntegerType(cpu->info.address_size), new_pc);
	new StoreInst(v_pc, cpu->ptr_PC, bb_branch);
}

void
emit_store_pc_return(cpu_t *cpu, BasicBlock *bb_branch, addr_t new_pc, BasicBlock *bb_ret)
{
	emit_store_pc(cpu, bb_branch, new_pc);
	BranchInst::Create(bb_ret, bb_branch);
}

BasicBlock *
create_basicblock(cpu_t *cpu, addr_t addr, Function *f, uint8_t bb_type) {
	char label[17];
	snprintf(label, sizeof(label), "%c%08llx", bb_type, (unsigned long long)addr);
log("creating basic block %s\n", label);
	return BasicBlock::Create(_CTX(), label, f, 0);
}

const BasicBlock *
lookup_basicblock(cpu_t *cpu, Function* f, addr_t pc, BasicBlock *bb_ret, uint8_t bb_type) {
	Function::const_iterator it;
	for (it = f->getBasicBlockList().begin(); it != f->getBasicBlockList().end(); it++) {
		const char *cstr = (*it).getNameStr().c_str();
		if (cstr[0] == bb_type) {
			addr_t pc2 = strtol(cstr + 1, (char **)NULL, 16);
			if (pc == pc2)
				return it;
		}
	}

	log("basic block %c%08llx not found - creating return basic block!\n", bb_type, pc);
	BasicBlock *new_bb = create_basicblock(cpu, pc, cpu->cur_func, BB_TYPE_EXTERNAL);
	emit_store_pc_return(cpu, new_bb, pc, bb_ret);
	return new_bb;
}
