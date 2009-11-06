#include "libcpu.h"
#include "m88k_internal.h"
#include "libcpu_m88k.h"
#include "cpu_generic.h"
#include "arch_types.h"

Value *m88k_ptr_C;

void
arch_m88k_init(cpu_t *cpu)
{
	m88k_regfile_t *reg;

	reg_size = 32;
	is_little_endian = !!(cpu->flags_arch & CPU_M88K_IS_LE);
	has_special_r0 = true;

	reg = (m88k_regfile_t*)malloc(sizeof(m88k_regfile_t));
	for (int i=0; i<32; i++) {
		reg->gpr[i] = 0;
#if the_register_map_has_been_done
		reg->xfr[i].i.dbl = 0;
#endif
	}
	reg->sxip = 0;
	reg->snip = 0;
	reg->sfip = 0;
	cpu->reg = reg;
	cpu->pc_width = 32;
	cpu->count_regs_i8 = 0;
	cpu->count_regs_i16 = 0;
	cpu->count_regs_i32 = 32;
	cpu->count_regs_i64 = 0;
	//cpu->count_regs_f32 = 0;
	//cpu->count_regs_f64 = 32;
	//cpu->count_regs_v64 = 32;

	printf("Motorola 88100 initialized.\n");
}

addr_t
arch_m88k_get_pc(void *reg)
{
	return ((m88k_regfile_t*)reg)->sxip;
}

void
arch_m88k_emit_decode_reg(BasicBlock *bb)
{
	// declare flags
	m88k_ptr_C = new AllocaInst(IntegerType::get(1), "C", bb);

	// decode PSR
//	Value *flags = new LoadInst(ptr_PSR, "", false, bb);
}

arch_func_t arch_func_m88k = {
	arch_m88k_init,
	arch_m88k_get_pc,
	arch_m88k_emit_decode_reg,
	NULL, /* spill_reg_state */
	arch_m88k_tag_instr,
	arch_m88k_disasm_instr,
	arch_m88k_recompile_instr
};
