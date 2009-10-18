//////////////////////////////////////////////////////////////////////
// 6502 specific
//////////////////////////////////////////////////////////////////////
#define CPU_6502_BRK_TRAP   (1<<0)
#define CPU_6502_XXX_TRAP   (1<<1)
#define CPU_6502_I_TRAP     (1<<2)
#define CPU_6502_D_TRAP     (1<<3)
#define CPU_6502_D_IGNORE   (1<<4)
#define CPU_6502_V_IGNORE   (1<<5)

extern void       *arch_6502_reg_init();
extern StructType *arch_6502_get_struct_reg(void);
extern void        arch_6502_emit_decode_reg(BasicBlock *bb);
extern void        arch_6502_spill_reg_state(BasicBlock *bb);
extern int         arch_6502_tag_instr(uint8_t* RAM, addr_t pc, int *flow_type, addr_t *new_pc);
extern int         arch_6502_disasm_instr(uint8_t* RAM, addr_t pc, char *line, unsigned int max_line);
extern int         arch_6502_recompile_instr(uint8_t* RAM, addr_t pc, BasicBlock *bb);

extern arch_func_t arch_func_6502;

