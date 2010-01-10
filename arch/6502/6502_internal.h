/*
 * libcpu: 6502_internal.h
 *
 * prototypes of functions exported to core
 */

extern int         arch_6502_tag_instr(cpu_t *cpu, addr_t pc, tag_t *tag, addr_t *new_pc, addr_t *next_pc);
extern int         arch_6502_disasm_instr(cpu_t *cpu, addr_t pc, char *line, unsigned int max_line);
extern Value      *arch_6502_translate_cond(cpu_t *cpu, addr_t pc, BasicBlock *bb);
extern int         arch_6502_translate_instr(cpu_t *cpu, addr_t pc, BasicBlock *bb);
