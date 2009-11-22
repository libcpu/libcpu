//////////////////////////////////////////////////////////////////////
// 6502 specific
//////////////////////////////////////////////////////////////////////
#define CPU_6502_BRK_TRAP   (1<<0)
#define CPU_6502_XXX_TRAP   (1<<1)
#define CPU_6502_I_TRAP     (1<<2)
#define CPU_6502_D_TRAP     (1<<3)
#define CPU_6502_D_IGNORE   (1<<4)
#define CPU_6502_V_IGNORE   (1<<5)

extern int         arch_6502_tag_instr(cpu_t *cpu, addr_t pc, tag_t *tag, addr_t *new_pc);
extern int         arch_6502_disasm_instr(cpu_t *cpu, addr_t pc, char *line, unsigned int max_line);
extern Value      *arch_6502_recompile_cond(cpu_t *cpu, addr_t pc, BasicBlock *bb);

extern arch_func_t arch_func_6502;

