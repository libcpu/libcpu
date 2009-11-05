#include "libcpu.h"
#include "cpu_generic.h"
#include "arm_internal.h"
#include "tag_generic.h"

using namespace llvm;
extern Function* func_jitmain;

extern const BasicBlock *lookup_basicblock(Function* f, addr_t pc);

extern Value* ptr_PC;

#define BAD printf("%s:%d\n", __func__, __LINE__); exit(1);
#define LOG printf("%s:%d\n", __func__, __LINE__);

//////////////////////////////////////////////////////////////////////
// tagging
//////////////////////////////////////////////////////////////////////

int arch_arm_tag_instr(uint8_t* RAM, addr_t pc, int *flow_type, addr_t *new_pc) {
	*flow_type = FLOW_TYPE_CONTINUE;
	return 4;
}

int arch_arm_recompile_instr(uint8_t* RAM, addr_t pc, BasicBlock *bb_dispatch, BasicBlock *bb) {
	BAD;
}


//printf("%s:%d PC=$%04X\n", __func__, __LINE__, pc);
//printf("%s:%d\n", __func__, __LINE__);
