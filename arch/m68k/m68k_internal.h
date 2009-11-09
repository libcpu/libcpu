#include "libcpu.h"
#include "libcpu_m68k.h"

int arch_m68k_tag_instr(uint8_t* RAM, addr_t pc, int *flow_type, addr_t *new_pc);
int arch_m68k_disasm_instr(uint8_t* RAM, addr_t pc, char *line, unsigned int max_line);
int arch_m68k_recompile_instr(uint8_t* RAM, addr_t pc, BasicBlock *bb_dispatch, BasicBlock *bb, BasicBlock *bb_target, BasicBlock *bb_cond, BasicBlock *bb_next);

int arch_m68k_instr_length(uint8_t* RAM, addr_t pc);

#define INSTR(a) RAM32(RAM, a)
