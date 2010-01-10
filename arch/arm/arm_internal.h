/*
 * libcpu: arm_internal.h
 *
 * prototypes of functions exported to core
 */

int arch_arm_tag_instr(cpu_t *cpu, addr_t pc, tag_t *tag, addr_t *new_pc, addr_t *next_pc);
int arch_arm_disasm_instr(cpu_t *cpu, addr_t pc, char *line, unsigned int max_line);
int arch_arm_translate_instr(cpu_t *cpu, addr_t pc, BasicBlock *bb);
Value *arch_arm_translate_cond(cpu_t *cpu, addr_t pc, BasicBlock *bb);
void arch_arm_emit_decode_reg(cpu_t *cpu, BasicBlock *bb);
void arch_arm_spill_reg_state(cpu_t *cpu, BasicBlock *bb);
