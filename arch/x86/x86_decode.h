#ifndef X86_DECODE_H
#define X86_DECODE_H

#include "libcpu.h"

#include <stdint.h>

enum x86_operand_type {
	OP_REG,
	OP_IMM,
	OP_MEM,
	OP_MEM_DISP,
};

enum x86_seg_override {
	NO_OVERRIDE,
	ES_OVERRIDE,
	CS_OVERRIDE,
	SS_OVERRIDE,
	DS_OVERRIDE,
};

struct x86_operand {
	enum x86_operand_type	type;
	uint8_t			reg;
	int32_t			disp;		/* address displacement can be negative */
	uint32_t		imm;
};

enum x86_instr_flags {
	MOD_RM			= (1U << 8),

	/* Operand sizes */
	WIDTH_BYTE		= (1U << 9),	/* 8 bits */
	WIDTH_WIDE		= (1U << 10),	/* 16 bits or 32 bits */
	WIDTH_MASK		= WIDTH_BYTE|WIDTH_WIDE,

	/* Source operand */
	SRC_NONE		= (1U << 11),
	SRC_IMM			= (1U << 12),
	SRC_REG			= (1U << 13),
	SRC_MASK		= SRC_NONE|SRC_IMM|SRC_REG,

	/* Destination operand */
	DST_NONE		= (1U << 14),
	DST_REG			= (1U << 15),
	DST_MEM			= (1U << 16),
	DST_MEM_DISP_BYTE	= (1U << 17),	/* 8 bits */
	DST_MEM_DISP_WIDE	= (1U << 18),	/* 16 bits or 32 bits */
	DST_MASK		= DST_NONE|DST_REG|DST_MEM|DST_MEM_DISP_BYTE|DST_MEM_DISP_WIDE,

	MEM_DISP_MASK		= DST_MEM_DISP_BYTE|DST_MEM_DISP_WIDE,
};

/*
 *	Addressing modes.
 */
enum x86_addmode {
	ADDMODE_REG		= SRC_REG|DST_NONE,	/* register */
	ADDMODE_IMM_REG		= SRC_IMM|DST_REG,	/* immediate -> register */
	ADDMODE_IMPLIED		= SRC_NONE|DST_NONE,	/* no operands */
	ADDMODE_REG_RM		= SRC_REG|MOD_RM	/* register -> register/memory */
};

struct x86_instr {
	unsigned long		nr_bytes;

	uint8_t			opcode;		/* Opcode byte */
	uint8_t			width;
	uint8_t			mod;		/* Mod */
	uint8_t			rm;		/* R/M */
	uint8_t			reg_opc;	/* Reg/Opcode */
	uint32_t		disp;		/* Address displacement */
	uint32_t		imm_data;	/* Immediate data */

	unsigned long		flags;		/* See enum x86_instr_flags */
	enum x86_seg_override	seg_override;
	struct x86_operand	src;
	struct x86_operand	dst;
};

static uint8_t x86_instr_type(struct x86_instr *instr)
{
	return instr->flags & 0xff;
}

int
arch_8086_decode_instr(struct x86_instr *instr, uint8_t* RAM, addr_t pc);

int
arch_8086_instr_length(struct x86_instr *instr);

#endif /* X86_DECODE_H */
