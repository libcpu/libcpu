#include "libcpu.h"
#include "m68k_internal.h"
#include "frontend.h"

static void
arch_m68k_init(cpu_t *cpu, cpu_archinfo_t *info, cpu_archrf_t *rf)
{
	// Basic Information
	info->name = "m68k";
	info->full_name = "Motorola 68000";

	// This architecture is big endian, override the user
  // flags.
	info->common_flags = CPU_FLAG_ENDIAN_BIG;
	// The byte size is 8bits.
	// The word size is 32bits.
	// The float size is 80bits.
	// The address size is 32bits.
	info->byte_size = 8;
	info->word_size = 32;
	info->float_size = 80;
	info->address_size = 32;
	// Page size is 4K or 8K, default is 8K.
	info->min_page_size = 4096;
	info->max_page_size = 8192;
	info->default_page_size = 8192;
	// There are 16 32-bit GPRs 
	info->register_count[CPU_REG_GPR] = 16;
	info->register_size[CPU_REG_GPR] = info->word_size;

	reg_m68k_t *reg;
	reg = (reg_m68k_t*)malloc(sizeof(reg_m68k_t));
	for (int i=0; i<16; i++) 
		reg->r[i] = 0;

	reg->pc = 0;

  rf->pc = &reg->pc;
  rf->grf = reg;
}

static addr_t
arch_m68k_get_pc(cpu_t *, void *reg)
{
	return ((reg_m68k_t*)reg)->pc;
}

static uint64_t
arch_m68k_get_psr(cpu_t *, void *reg)
{
	return ((reg_m68k_t*)reg)->psr;
}

static int
arch_m68k_get_reg(cpu_t *cpu, void *reg, unsigned reg_no, uint64_t *value)
{
	if (reg_no > 15)
		return (-1);

	*value = ((reg_m68k_t *)reg)->r[reg_no];
	return (0);
}

arch_func_t arch_func_m68k = {
	arch_m68k_init,
	NULL,
	arch_m68k_get_pc,
	NULL, /* emit_decode_reg */
	NULL, /* spill_reg_state */
	arch_m68k_tag_instr,
	arch_m68k_disasm_instr,
	arch_m68k_translate_cond,
	arch_m68k_translate_instr,
	// idbg support
	arch_m68k_get_psr,
	arch_m68k_get_reg,
	NULL
};
