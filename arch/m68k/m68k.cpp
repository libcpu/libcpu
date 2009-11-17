#include "libcpu.h"
#include "m68k_internal.h"
#include "cpu_generic.h"

void
arch_m68k_init(cpu_t *cpu)
{
	cpu->reg_size = 32;
	cpu->is_little_endian = true;
	cpu->has_special_r0 = false;
	cpu->fp_reg_size = 80;
	cpu->has_special_fr0 = false;

	reg_m68k_t *reg;
	reg = (reg_m68k_t*)malloc(sizeof(reg_m68k_t));
	for (int i=0; i<16; i++) 
		reg->r[i] = 0;
	reg->pc = 0;
	cpu->reg = reg;
	cpu->pc_width = 32;
	cpu->count_regs_i8 = 0;
	cpu->count_regs_i16 = 0;
	cpu->count_regs_i32 = 16;
	cpu->count_regs_i64 = 0;

	cpu->count_regs_f32 = 0;
	cpu->count_regs_f64 = 0;
	cpu->count_regs_f80 = 0;
	cpu->count_regs_f128 = 0;
}

addr_t
arch_m68k_get_pc(cpu_t *, void *reg)
{
	return ((reg_m68k_t*)reg)->pc;
}

arch_func_t arch_func_m68k = {
	arch_m68k_init,
	arch_m68k_get_pc,
	NULL, /* emit_decode_reg */
	NULL, /* spill_reg_state */
	arch_m68k_tag_instr,
	arch_m68k_disasm_instr,
	arch_m68k_recompile_instr
};
