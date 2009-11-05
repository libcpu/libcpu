#include "libcpu.h"
#include "arm_internal.h"
#include "cpu_generic.h"

void
arch_arm_init(cpu_t *cpu)
{
	reg_size = 32;
	is_little_endian = !!(cpu->flags_arch & CPU_ARM_IS_LE);
	has_special_r0 = false;

	reg_arm_t *reg;
	reg = (reg_arm_t*)malloc(sizeof(reg_arm_t));
	for (int i=0; i<16; i++) /* this includes pc */
		reg->r[i] = 0;
	cpu->reg = reg;
	cpu->pc_width = 32;

	cpu->count_regs_i8 = 0;
	cpu->count_regs_i16 = 0;
	cpu->count_regs_i32 = 16;
	cpu->count_regs_i64 = 0;
}

addr_t
arch_arm_get_pc(void *reg)
{
	return ((reg_arm_t*)reg)->pc;
}

arch_func_t arch_func_arm = {
	arch_arm_init,
	arch_arm_get_pc,
	NULL, /* emit_decode_reg */
	NULL, /* spill_reg_state */
	arch_arm_tag_instr,
	arch_arm_disasm_instr,
	arch_arm_recompile_instr
};
