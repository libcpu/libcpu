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


BasicBlock *
create_basicblock(cpu_t *cpu, addr_t addr, Function *f, uint8_t bb_type) {
	char label[17];
	snprintf(label, sizeof(label), "%c%08llx", bb_type, (unsigned long long)addr);
log("creating basic block %s\n", label);
	return BasicBlock::Create(_CTX(), label, f, 0);
}

const BasicBlock *
lookup_basicblock(cpu_t *cpu, Function* f, addr_t pc, uint8_t bb_type) {
	Function::const_iterator it;
	for (it = f->getBasicBlockList().begin(); it != f->getBasicBlockList().end(); it++) {
		const char *cstr = (*it).getNameStr().c_str();
		if (cstr[0] == bb_type) {
			addr_t pc2 = strtol(cstr + 1, (char **)NULL, 16);
			if (pc == pc2)
				return it;
		}
	}
	log("error: basic block %c%08llx not found!\n", bb_type, pc);
	return NULL;
}
