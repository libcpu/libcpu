/*
 * libcpu: i386_translate.cpp
 *
 * main translation code
 */

#include "libcpu.h"
#include "i386_isa.h"
#include "i386_cc.h"
#include "frontend.h"
#include "i386_internal.h"
#include "i386_types.h"


Value *
arch_i386_translate_cond(cpu_t *cpu, addr_t pc, BasicBlock *bb)
{
	return NULL;
}

static int
arch_i386_translate_instr(cpu_t *cpu, addr_t pc, BasicBlock *bb)
{
	return 0;
}

static void
arch_i386_init(cpu_t *cpu, cpu_archinfo_t *info, cpu_archrf_t *rf)
{
	info->name = "i386";
	info->full_name = "Intel i386";

	info->common_flags = CPU_FLAG_ENDIAN_LITTLE;

	info->byte_size		= 8;
	info->word_size		= 16;
	info->address_size	= 24;

	info->register_count[CPU_REG_GPR]	= 8;
	info->register_size[CPU_REG_GPR]	= info->word_size;

	info->register_count[CPU_REG_XR]	= 0;
	info->register_size[CPU_REG_XR]		= 0;

	reg_i386_t *reg;
	reg = (reg_i386_t*)malloc(sizeof(reg_i386_t));
	reg->ax		= 0;
	reg->bx		= 0;
	reg->cx		= 0;
	reg->dx		= 0;
	reg->si		= 0;
	reg->di		= 0;
	reg->bp		= 0;
	reg->sp		= 0;
	reg->ip		= 0;
	reg->flags	= 0;
	reg->cs		= 0;
	reg->ds		= 0;
	reg->ss		= 0;
	reg->es		= 0;

	rf->pc	= &reg->ip;
	rf->grf	= reg;
}

static void
arch_i386_done(cpu_t *cpu)
{
	free(cpu->feptr);
	free(cpu->rf.grf);
}

static void
arch_i386_emit_decode_reg(cpu_t *cpu, BasicBlock *bb)
{
}

static void
arch_i386_spill_reg_state(cpu_t *cpu, BasicBlock *bb)
{
}

static addr_t
arch_i386_get_pc(cpu_t *, void *reg)
{
	return ((reg_i386_t*)reg)->ip;
}

static uint64_t
arch_i386_get_psr(cpu_t *, void *reg)
{
	return 0xdeadbeefdeadbeefULL;
}

static int
arch_i386_get_reg(cpu_t *cpu, void *reg, unsigned reg_no, uint64_t *value)
{
	return -1;
}

arch_func_t arch_func_i386 = {
	arch_i386_init,
	arch_i386_done,
	arch_i386_get_pc,
	arch_i386_emit_decode_reg,
	arch_i386_spill_reg_state,
	arch_i386_tag_instr,
	arch_i386_disasm_instr,
	arch_i386_translate_cond,
	arch_i386_translate_instr,
	// idbg support
	arch_i386_get_psr,
	arch_i386_get_reg,
	NULL
};
