#include "libcpu.h"
#include "libcpu_mips.h"

int arch_mips_tag_instr(cpu_t *cpu, addr_t pc, tag_t *tag, addr_t *new_pc);
int arch_mips_disasm_instr(cpu_t *cpu, addr_t pc, char *line, unsigned int max_line);
int arch_mips_recompile_instr(cpu_t *cpu, addr_t pc, BasicBlock *bb);
Value *arch_mips_recompile_cond(cpu_t *cpu, addr_t pc, BasicBlock *bb);

#define INSTR(a) RAM32(cpu->RAM, a)
