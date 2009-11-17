#include "libcpu.h"
#include "m88k_internal.h"
#include "libcpu_m88k.h"
#include "cpu_generic.h"
#include "arch_types.h"

Value *m88k_ptr_C;

static void
arch_m88k_init(cpu_t *cpu)
{
	m88k_regfile_t *reg;

	cpu->is_little_endian = !!(cpu->flags_arch & CPU_M88K_IS_LE);
	cpu->reg_size = 32;
	cpu->has_special_r0 = true;

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

	cpu->fp_reg = NULL;
	cpu->fp_reg_size = 80;
	cpu->has_special_fr0 = true;
	cpu->count_regs_f32 = 0;
	cpu->count_regs_f64 = 0;
	cpu->count_regs_f80 = 0;
	cpu->count_regs_f128 = 0;

	printf("Motorola 88100 initialized.\n");
}

static addr_t
arch_m88k_get_pc(cpu_t *, void *reg)
{
	return ((m88k_regfile_t*)reg)->sxip;
}

#define C_SHIFT 28

static Value *
arch_m88k_flags_encode(BasicBlock *bb)
{
	Value *flags = ConstantInt::get(getIntegerType(32), 0);

	flags = arch_encode_bit(flags, m88k_ptr_C, C_SHIFT, 32, bb);

	return flags;
}

static void
arch_m88k_flags_decode(Value *flags, BasicBlock *bb)
{
	arch_decode_bit(flags, m88k_ptr_C, C_SHIFT, 32, bb);
}

static void
arch_m88k_emit_decode_reg(cpu_t *cpu, BasicBlock *bb)
{
	// declare flags
	m88k_ptr_C = new AllocaInst(getIntegerType(1), "C", bb);

#if not_yet
	// decode PSR
	Value *flags = new LoadInst(ptr_PSR, "", false, bb);
	arch_m88k_flags_decode(flags, bb);
#endif
}

static void
arch_m88k_spill_reg_state(cpu_t *cpu, BasicBlock *bb)
{
#if not_yet
	Value *flags = arch_m88k_flags_encode(bb);
	new StoreInst(flags, ptr_PSR, false, bb);
#endif
}

arch_func_t arch_func_m88k = {
	arch_m88k_init,
	arch_m88k_get_pc,
	arch_m88k_emit_decode_reg,
	arch_m88k_spill_reg_state,
	arch_m88k_tag_instr,
	arch_m88k_disasm_instr,
	arch_m88k_recompile_instr
};
