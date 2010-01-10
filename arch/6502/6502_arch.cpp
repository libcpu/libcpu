#include <assert.h>

#include "libcpu.h"
#include "6502_isa.h"
#include "frontend.h"
#include "libcpu_6502.h"

static flags_layout_t arch_6502_flags_layout[] = {
	{ N_SHIFT, 'N', "N" },	/* negative */
	{ V_SHIFT, 'V', "V" },	/* overflow */
	{ X_SHIFT, 0,   "X" },	/* unassigned */
	{ B_SHIFT, 0,   "B" },	/* break */
	{ D_SHIFT, 0,   "D" },	/* decimal mode */
	{ I_SHIFT, 0,   "I" },	/* interrupt disable */
	{ Z_SHIFT, 'Z', "Z" },	/* zero */
	{ C_SHIFT, 'C', "C" },	/* carry */
	{ -1, 0, NULL }
};

static void
arch_6502_init(cpu_t *cpu, cpu_archinfo_t *info, cpu_archrf_t *rf)
{
	assert(offsetof(reg_6502_t, pc) == 5);

	// Basic Information
	info->name = "6502";
	info->full_name = "MOS 6502";

	// This architecture is little endian, override any user flag.
	info->common_flags = CPU_FLAG_ENDIAN_LITTLE;
	// The byte and word size are both 8bits.
	// The address size is 16bits.
	info->byte_size = 8;
	info->word_size = 8;
	info->address_size = 16;
	// There are 4 8-bit GPRs
	info->register_count[CPU_REG_GPR] = 4;
	info->register_size[CPU_REG_GPR] = info->word_size;
	// There is also 1 extra register to handle PSR.
	info->register_count[CPU_REG_XR] = 1;
	info->register_size[CPU_REG_XR] = 8;

	info->flags_size = 8;
	info->flags_layout = arch_6502_flags_layout;

	reg_6502_t *reg;
	reg = (reg_6502_t*)malloc(sizeof(reg_6502_t));
	reg->pc = 0;
	reg->a = 0;
	reg->x = 0;
	reg->y = 0;
	reg->s = 0xFF;
	reg->p = 0;

	rf->pc = &reg->pc;
	rf->grf = reg;
}

static void
arch_6502_done(cpu_t *cpu)
{
	free(cpu->rf.grf);
}

static addr_t
arch_6502_get_pc(cpu_t *, void *reg)
{
	return ((reg_6502_t*)reg)->pc;
}

static uint64_t
arch_6502_get_psr(cpu_t *, void *reg)
{
	return ((reg_6502_t*)reg)->p;
}

static int
arch_6502_get_reg(cpu_t *cpu, void *reg, unsigned reg_no, uint64_t *value)
{
	switch (reg_no) {
		case 0: *value = ((reg_6502_t *)reg)->a; break;
		case 1: *value = ((reg_6502_t *)reg)->x; break;
		case 2: *value = ((reg_6502_t *)reg)->y; break;
		case 3: *value = ((reg_6502_t *)reg)->s; break;
		default: return (-1);
	}
	return (0);
}

arch_func_t arch_func_6502 = {
	arch_6502_init,
	arch_6502_done,
	arch_6502_get_pc,
	NULL, //emit_decode_reg
	NULL, //spill_reg_state
	arch_6502_tag_instr,
	arch_6502_disasm_instr,
	arch_6502_translate_cond,
	arch_6502_translate_instr,
	// idbg support
	arch_6502_get_psr,
	arch_6502_get_reg,
	NULL
};
