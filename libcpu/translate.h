BasicBlock *translate_instr(cpu_t *cpu, addr_t pc, tag_t tag, BasicBlock *bb_target, BasicBlock *bb_next, BasicBlock *bb_trap, BasicBlock *cur_bb);
void emit_store_pc(cpu_t *cpu, BasicBlock *bb_branch, addr_t new_pc);
void emit_store_pc_return(cpu_t *cpu, BasicBlock *bb_branch, addr_t new_pc, BasicBlock *bb_ret);