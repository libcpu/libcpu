#include "libcpu.h"
#include "libcpu_arm.h"

int arch_arm_tag_instr(uint8_t* RAM, addr_t pc, int *flow_type, addr_t *new_pc);
int arch_arm_disasm_instr(uint8_t* RAM, addr_t pc, char *line, unsigned int max_line);
int arch_arm_recompile_instr(uint8_t* RAM, addr_t pc, BasicBlock *bb_dispatch, BasicBlock *bb);
void arch_arm_emit_decode_reg(BasicBlock *bb);
void arch_arm_spill_reg_state(BasicBlock *bb);

#define INSTR(a) RAM32(RAM, a)
