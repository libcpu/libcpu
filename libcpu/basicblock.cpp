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
	tag_t tag = get_tag(cpu, a);
	return (tag &
		(TAG_BRANCH_TARGET |	/* someone jumps/branches here */
		 TAG_SUBROUTINE |		/* someone calls this */
		 TAG_AFTER_CALL |		/* instruction after a call */
		 TAG_AFTER_COND |		/* instruction after a branch */
		 TAG_AFTER_TRAP |		/* instruction after a trap */
		 TAG_ENTRY))			/* client wants to enter guest code here */
		&& (tag & TAG_CODE);	/* only if we actually tagged it */
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
	BasicBlock *bb = BasicBlock::Create(_CTX(), label, f, 0);

	// if it's a label, cache the new basic block.
	if (bb_type == BB_TYPE_NORMAL)
		cpu->func_bb[f][addr] = bb;

	return bb;
}

const BasicBlock *
lookup_basicblock(cpu_t *cpu, Function* f, addr_t pc, BasicBlock *bb_ret, uint8_t bb_type) {
	// lookup for the basicblock associated to pc in specified function 'f'
	bbaddr_map &bb_addr = cpu->func_bb[f];
	bbaddr_map::const_iterator i = bb_addr.find(pc);
	if (i != bb_addr.end())
		return i->second;

	log("basic block %c%08llx not found in function %p - creating return basic block!\n", bb_type, pc, f);
	BasicBlock *new_bb = create_basicblock(cpu, pc, cpu->cur_func, BB_TYPE_EXTERNAL);
	emit_store_pc_return(cpu, new_bb, pc, bb_ret);

	return new_bb;
}
