#include "libcpu.h"
#include "arm_internal.h"
#include "cpu_generic.h"

static void
arch_arm_init(cpu_t *cpu)
{
	cpu->reg_size = 32;
	cpu->is_little_endian = !!(cpu->flags_arch & CPU_ARM_IS_LE);
	cpu->has_special_r0 = false;
	cpu->fp_reg_size = 64;
	cpu->has_special_fr0 = false;

	reg_arm_t *reg;
	reg = (reg_arm_t*)malloc(sizeof(reg_arm_t));
	for (int i=0; i<17; i++) /* this includes pc */
		reg->r[i] = 0;
	cpu->reg = reg;
	cpu->pc_width = 32;

	cpu->count_regs_i8 = 0;
	cpu->count_regs_i16 = 0;
	cpu->count_regs_i32 = 17;
	cpu->count_regs_i64 = 0;

	cpu->fp_reg = NULL;
	cpu->count_regs_f32 = 0;
	cpu->count_regs_f64 = 0;
	cpu->count_regs_f80 = 0;
	cpu->count_regs_f128 = 0;
}

static addr_t
arch_arm_get_pc(cpu_t *, void *reg)
{
	return ((reg_arm_t *)reg)->pc;
}

static uint64_t
arch_arm_get_psr(cpu_t *, void *reg)
{
	return ((reg_arm_t *)reg)->cpsr;
}

static int
arch_arm_get_reg(cpu_t *cpu, void *reg, unsigned reg_no, uint64_t *value)
{
	if (reg_no > 15)
		return (-1);

	*value = ((reg_arm_t *)reg)->r[reg_no];
	return (0);
}

arch_func_t arch_func_arm = {
	arch_arm_init,
	arch_arm_get_pc,
	arch_arm_emit_decode_reg,
	arch_arm_spill_reg_state,
	arch_arm_tag_instr,
	arch_arm_disasm_instr,
	arch_arm_translate_cond,
	arch_arm_translate_instr,
	// idbg support
	arch_arm_get_psr,
	arch_arm_get_reg,
	NULL
};
