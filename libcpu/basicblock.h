enum {
	BB_TYPE_NORMAL   = 'L', /* basic block for instructions */
	BB_TYPE_COND     = 'C', /* basic block for "taken" case of cond. execution */
	BB_TYPE_DELAY    = 'D', /* basic block for delay slot in non-taken case of cond. exec. */
	BB_TYPE_EXTERNAL = 'E'  /* basic block for unknown addresses; just traps */
};

bool is_start_of_basicblock(cpu_t *cpu, addr_t a);
bool needs_dispatch_entry(cpu_t *cpu, addr_t a);
BasicBlock *create_basicblock(cpu_t *cpu, addr_t addr, Function *f, uint8_t bb_type);
const BasicBlock *lookup_basicblock(cpu_t *cpu, Function* f, addr_t pc, BasicBlock *bb_ret, uint8_t bb_type);
void emit_store_pc(cpu_t *cpu, BasicBlock *bb_branch, addr_t new_pc);
void emit_store_pc_return(cpu_t *cpu, BasicBlock *bb_branch, addr_t new_pc, BasicBlock *bb_ret);
