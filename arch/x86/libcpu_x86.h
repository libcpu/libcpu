//////////////////////////////////////////////////////////////////////
// 8086 specific
//////////////////////////////////////////////////////////////////////

extern int         arch_8086_tag_instr(cpu_t *cpu, addr_t pc, tag_t *tag, addr_t *new_pc, addr_t *next_pc);
extern int         arch_8086_disasm_instr(cpu_t *cpu, addr_t pc, char *line, unsigned int max_line);
extern Value      *arch_8086_translate_cond(cpu_t *cpu, addr_t pc, BasicBlock *bb);

extern arch_func_t arch_func_8086;
