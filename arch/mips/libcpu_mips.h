//////////////////////////////////////////////////////////////////////
// MIPS specific
//////////////////////////////////////////////////////////////////////
#define CPU_MIPS_IS_32BIT      0
#define CPU_MIPS_IS_64BIT  (1<<0)
#define CPU_MIPS_IS_BE       0
#define CPU_MIPS_IS_LE     (1<<1)

extern void       *arch_mips_reg_init();
extern void        arch_mips_emit_decode_reg(BasicBlock *bb);
extern void        arch_mips_spill_reg_state(BasicBlock *bb);
extern int         arch_mips_tag_instr(uint8_t* RAM, addr_t pc, int *flow_type, addr_t *new_pc);
extern int         arch_mips_disasm_instr(uint8_t* RAM, addr_t pc, char *line, unsigned int max_line);
extern int         arch_mips_recompile_instr(uint8_t* RAM, addr_t pc, BasicBlock *bb);

extern arch_func_t arch_func_mips;
