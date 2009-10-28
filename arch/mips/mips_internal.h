#include "libcpu.h"
#include "libcpu_mips.h"

int arch_mips_tag_instr(uint8_t* RAM, addr_t pc, int *flow_type, addr_t *new_pc);
int arch_mips_disasm_instr(uint8_t* RAM, addr_t pc, char *line, unsigned int max_line);
int arch_mips_recompile_instr(uint8_t* RAM, addr_t pc, BasicBlock *bb_dispatch, BasicBlock *bb);

#define INSTR(a) RAM32(RAM, a)
