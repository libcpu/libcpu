#include "libcpu.h"
#include "m88k_internal.h"
#include "libcpu_m88k.h"
#include "frontend.h"
#include "arch_types.h"

#define ptr_PSR		ptr_xr[0]
#define ptr_TRAPNO	ptr_xr[1]
#define ptr_C		(cpu->feptr)

static void
arch_m88k_init(cpu_t *cpu, cpu_archinfo_t *info, cpu_archrf_t *rf)
{
	m88k_grf_t *reg;
	m88k_xrf_t *fp_reg;

	// Basic Information
	info->name = "m88k";
	info->full_name = "Motorola 88110";

	// This architecture is biendian, accept whatever the
	// client wants, override other flags.
	info->common_flags &= CPU_FLAG_ENDIAN_MASK;
	// Both r0 and x0 are hardwired to zero.
	info->common_flags |= CPU_FLAG_HARDWIRE_GPR0;
	info->common_flags |= CPU_FLAG_HARDWIRE_FPR0;
	// This architecture supports delay slots (w/o annihilation)
	// with 1 instruction.
	info->common_flags |= CPU_FLAG_DELAY_SLOT;
	info->delay_slots = 1;
	// The byte size is 8bits.
	// The word size is 32bits.
	// The float size is 80bits.
	// The address size is 32bits.
	info->byte_size = 8;
	info->word_size = 32;
	info->float_size = 80;
	info->address_size = 32;
	// Page size is just 4K.
	info->min_page_size = 4096;
	info->max_page_size = 4096;
	info->default_page_size = 4096;
	// There are 32 32-bit GPRs and 32 80-bit FPRs.
	info->register_count[CPU_REG_GPR] = 32;
	info->register_size[CPU_REG_GPR] = info->word_size;
	info->register_count[CPU_REG_FPR] = 32;
	info->register_size[CPU_REG_FPR] = info->float_size;
	// There are also 2 extra registers to handle
	// PSR and TRAPNO.
	info->register_count[CPU_REG_XR] = 2;
	info->register_size[CPU_REG_XR] = 32;

	// Setup the register files
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

	// Architecture Register File
	rf->pc = &reg->sxip;
	rf->grf = reg;
	rf->frf = fp_reg;
	rf->vrf = NULL;

	log("Motorola 88110 initialized.\n");
}

static void
arch_m88k_done(cpu_t *cpu)
{
	free(cpu->rf.frf);
	free(cpu->rf.grf);
}

static addr_t
arch_m88k_get_pc(cpu_t *, void *reg)
{
	return ((m88k_grf_t *)reg)->sxip;
}

#define C_SHIFT 28

static Value *
arch_m88k_flags_encode(cpu_t *cpu, BasicBlock *bb)
{
	Value *flags = ConstantInt::get(getIntegerType(32), 0);

	flags = arch_encode_bit(flags, (Value *)ptr_C, C_SHIFT, 32, bb);

	return flags;
}

static void
arch_m88k_flags_decode(cpu_t *cpu, Value *flags, BasicBlock *bb)
{
	arch_decode_bit(flags, (Value *)ptr_C, C_SHIFT, 32, bb);
}

static void
arch_m88k_emit_decode_reg(cpu_t *cpu, BasicBlock *bb)
{
	// declare flags
	ptr_C = new AllocaInst(getIntegerType(1), "C", bb);

	// decode PSR
	Value *flags = new LoadInst(cpu->ptr_PSR, "", false, bb);
	arch_m88k_flags_decode(cpu, flags, bb);
}

static void
arch_m88k_spill_reg_state(cpu_t *cpu, BasicBlock *bb)
{
	Value *flags = arch_m88k_flags_encode(cpu, bb);
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
	arch_m88k_done,
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
