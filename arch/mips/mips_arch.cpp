#include "libcpu.h"
#include "mips_internal.h"
#include "cpu_generic.h"

void
arch_mips_init(cpu_t *cpu)
{
	cpu->reg_size = (cpu->flags_arch & CPU_MIPS_IS_64BIT)? 64:32;
	cpu->is_little_endian = !!(cpu->flags_arch & CPU_MIPS_IS_LE);
	cpu->has_special_r0 = true;
	cpu->fp_reg_size = 64;
	cpu->has_special_fr0 = true;

	if (cpu->reg_size == 64) {
		reg_mips64_t *reg;
		reg = (reg_mips64_t*)malloc(sizeof(reg_mips64_t));
		for (int i=0; i<32; i++) 
			reg->r[i] = 0;
		reg->pc = 0;
		cpu->reg = reg;
		cpu->pc_width = 64; //XXX actually it's 32!
		cpu->count_regs_i32 = 0;
		cpu->count_regs_i64 = 32;
	} else {
		reg_mips32_t *reg;
		reg = (reg_mips32_t*)malloc(sizeof(reg_mips32_t));
		for (int i=0; i<32; i++) 
			reg->r[i] = 0;
		reg->pc = 0;
		cpu->reg = reg;
		cpu->pc_width = 32;
		cpu->count_regs_i32 = 32;
		cpu->count_regs_i64 = 0;
	}

	cpu->count_regs_i8 = 0;
	cpu->count_regs_i16 = 0;

	cpu->fp_reg = NULL;
	cpu->count_regs_f32 = 0;
	cpu->count_regs_f64 = 0;
	cpu->count_regs_f80 = 0;
	cpu->count_regs_f128 = 0;

	log("%d bit MIPS initialized.\n", cpu->reg_size);
}

addr_t
arch_mips_get_pc(cpu_t *cpu, void *reg)
{
	if (cpu->reg_size == 64)
		return ((reg_mips64_t*)reg)->pc;
	else
		return ((reg_mips32_t*)reg)->pc;
}

arch_func_t arch_func_mips = {
	arch_mips_init,
	arch_mips_get_pc,
	NULL, /* emit_decode_reg */
	NULL, /* spill_reg_state */
	arch_mips_tag_instr,
	arch_mips_disasm_instr,
	arch_mips_translate_cond,
	arch_mips_translate_instr
};
