#include "libcpu.h"
#include "m88k_internal.h"
#include "libcpu_m88k.h"
#include "cpu_generic.h"
#include "arch_types.h"

#define ptr_PSR    ptr_r32[32]
#define ptr_TRAPNO ptr_r32[33]

Value *m88k_ptr_C;

static void
arch_m88k_init(cpu_t *cpu)
{
	m88k_grf_t *reg;
	m88k_xrf_t *fp_reg;

	cpu->is_little_endian = !!(cpu->flags_arch & CPU_M88K_IS_LE);
	cpu->reg_size = 32;
	cpu->has_special_r0 = true;

	reg = (m88k_grf_t *)malloc(sizeof(m88k_grf_t));
	fp_reg = (m88k_xrf_t *)malloc(sizeof(m88k_xrf_t));
	for (int i = 0; i < 32; i++) {
		reg->r[i] = 0;
		fp_reg->x[i].i.hi = 0;
		fp_reg->x[i].i.lo = 0;
	}
	reg->sxip = 0;
	reg->psr = 0;
	reg->trapno = 0;

	cpu->reg = reg;
	cpu->pc_width = 32;
	cpu->count_regs_i8 = 0;
	cpu->count_regs_i16 = 0;
	cpu->count_regs_i32 = 32 + 2;
	cpu->count_regs_i64 = 0;

	cpu->fp_reg = fp_reg;
	cpu->fp_reg_size = 80;
	cpu->has_special_fr0 = true;
	cpu->count_regs_f32 = 0;
	cpu->count_regs_f64 = 0;
	cpu->count_regs_f80 = 32;
	cpu->count_regs_f128 = 0;

	printf("Motorola 88100 initialized.\n");
}

static addr_t
arch_m88k_get_pc(cpu_t *, void *reg)
{
	return ((m88k_grf_t *)reg)->sxip;
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

	// decode PSR
	Value *flags = new LoadInst(cpu->ptr_PSR, "", false, bb);
	arch_m88k_flags_decode(flags, bb);
}

static void
arch_m88k_spill_reg_state(cpu_t *cpu, BasicBlock *bb)
{
	Value *flags = arch_m88k_flags_encode(bb);
	new StoreInst(flags, cpu->ptr_PSR, false, bb);
}

static uint64_t
arch_m88k_get_psr(cpu_t *, void *regs)
{
	return ((m88k_grf_t *)regs)->psr;
}

static int
arch_m88k_get_reg(cpu_t *cpu, void *regs, unsigned reg_no, uint64_t *value)
{
	if (reg_no > 31)
		return (-1);

	*value = ((m88k_grf_t *)regs)->r[reg_no];
	return (0);
}

static int
arch_m88k_get_fp_reg(cpu_t *cpu, void *regs, unsigned reg_no, void *value)
{
	if (reg_no > 31)
		return (-1);

	*(fp80_reg_t *)value = ((m88k_xrf_t *)regs)->x[reg_no];
	return (0);
}

arch_func_t arch_func_m88k = {
	arch_m88k_init,
	arch_m88k_get_pc,
	arch_m88k_emit_decode_reg,
	arch_m88k_spill_reg_state,
	arch_m88k_tag_instr,
	arch_m88k_disasm_instr,
	arch_m88k_translate_cond,
	arch_m88k_translate_instr,
	// idbg support
	arch_m88k_get_psr,
	arch_m88k_get_reg,
	arch_m88k_get_fp_reg
};
