/*
 * libcpu: i386_internal.cpp
 *
 * prototypes of functions exported to core
 */

extern int         arch_i386_tag_instr(cpu_t *cpu, addr_t pc, tag_t *tag, addr_t *new_pc, addr_t *next_pc);
extern int         arch_i386_disasm_instr(cpu_t *cpu, addr_t pc, char *line, unsigned int max_line);
extern Value      *arch_i386_translate_cond(cpu_t *cpu, addr_t pc, BasicBlock *bb);
