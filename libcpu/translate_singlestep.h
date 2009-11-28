BasicBlock *create_singlestep_return_basicblock(cpu_t *cpu, addr_t new_pc, BasicBlock *bb_ret);
BasicBlock *cpu_translate_singlestep(cpu_t *cpu, BasicBlock *bb_ret, BasicBlock *bb_trap);
