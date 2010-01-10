#include "libcpu.h"

int arch_m68k_tag_instr(cpu_t *cpu, addr_t pc, tag_t *tag, addr_t *new_pc, addr_t *next_pc);
int arch_m68k_disasm_instr(cpu_t *cpu, addr_t pc, char *line, unsigned int max_line);
int arch_m68k_translate_instr(cpu_t *cpu, addr_t pc, BasicBlock *bb);
Value *arch_m68k_translate_cond(cpu_t *cpu, addr_t pc, BasicBlock *bb);

int arch_m68k_instr_length(cpu_t *cpu, addr_t pc);

#define INSTR(a) RAM32(RAM, a)
