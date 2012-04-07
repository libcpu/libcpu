/*
 * libcpu: i386_decode.h
 *
 * instruction decoding
 */

#ifndef I386_DECODE_H
#define I386_DECODE_H

#include "libcpu.h"

#include <stdint.h>

enum i386_operand_type {
	OP_IMM,
	OP_MEM,
	OP_MEM_DISP,
	OP_REG,
	OP_REL,
};

enum i386_seg_override {
	NO_OVERRIDE,
	ES_OVERRIDE,
	CS_OVERRIDE,
	SS_OVERRIDE,
	DS_OVERRIDE,
};

enum i386_rep_prefix {
	NO_PREFIX,
	REPNZ_PREFIX,
	REPZ_PREFIX,
};

struct i386_operand {
	enum i386_operand_type	type;
	uint8_t			reg;
	int32_t			disp;		/* address displacement can be negative */
	union {
		uint32_t		imm;
		int32_t			rel;
	};
};

enum i386_instr_flags {
	MOD_RM			= (1U << 8),
	DIR_REVERSED		= (1U << 9),

	/* Operand sizes */
	WIDTH_BYTE		= (1U << 10),	/* 8 bits */
	WIDTH_FULL		= (1U << 11),	/* 16 bits or 32 bits */
	WIDTH_MASK		= WIDTH_BYTE|WIDTH_FULL,

	/* Source operand */
	SRC_NONE		= (1U << 12),

	SRC_IMM			= (1U << 13),
	SRC_IMM8		= (1U << 14),
	IMM_MASK		= SRC_IMM|SRC_IMM8,

	SRC_REL			= (1U << 15),
	REL_MASK		= SRC_REL,

	SRC_REG			= (1U << 16),
	SRC_ACC			= (1U << 17),
	SRC_MEM			= (1U << 18),
	SRC_MEM_DISP_BYTE	= (1U << 19),
	SRC_MEM_DISP_FULL	= (1U << 20),
	SRC_MASK		= SRC_NONE|IMM_MASK|REL_MASK|SRC_REG|SRC_ACC|SRC_MEM|SRC_MEM_DISP_BYTE|SRC_MEM_DISP_FULL,

	/* Destination operand */
	DST_NONE		= (1U << 21),
	DST_REG			= (1U << 22),
	DST_ACC			= (1U << 23),	/* AL/AX */
	DST_MEM			= (1U << 24),
	DST_MEM_DISP_BYTE	= (1U << 25),	/* 8 bits */
	DST_MEM_DISP_FULL	= (1U << 26),	/* 16 bits or 32 bits */
	DST_MASK		= DST_NONE|DST_REG|DST_ACC|DST_MEM|DST_MEM_DISP_BYTE|DST_MEM_DISP_FULL,

	MEM_DISP_MASK		= SRC_MEM_DISP_BYTE|SRC_MEM_DISP_FULL|DST_MEM_DISP_BYTE|DST_MEM_DISP_FULL,

	GROUP_2			= (1U << 27),

	GROUP_MASK		= GROUP_2,
};

/*
 *	Addressing modes.
 */
enum i386_addmode {
	ADDMODE_ACC_MEM		= SRC_ACC|DST_MEM|DIR_REVERSED,	/* AL/AX -> memory */
	ADDMODE_ACC_REG		= SRC_ACC|DST_REG,		/* AL/AX -> reg */
	ADDMODE_IMM		= SRC_IMM|DST_NONE,		/* immediate operand */
	ADDMODE_IMM8_RM		= SRC_IMM8|MOD_RM|DIR_REVERSED,	/* immediate -> register/memory */
	ADDMODE_IMM_ACC		= SRC_IMM|DST_ACC,		/* immediate -> AL/AX */
	ADDMODE_IMM_REG		= SRC_IMM|DST_REG,		/* immediate -> register */
	ADDMODE_IMPLIED		= SRC_NONE|DST_NONE,		/* no operands */
	ADDMODE_MEM_ACC		= SRC_ACC|DST_MEM,		/* memory -> AL/AX */
	ADDMODE_REG		= SRC_REG|DST_NONE,		/* register */
	ADDMODE_REG_RM		= SRC_REG|MOD_RM|DIR_REVERSED,	/* register -> register/memory */
	ADDMODE_REL		= SRC_REL|DST_NONE,		/* relative */
	ADDMODE_RM_REG		= DST_REG|MOD_RM,		/* register/memory -> register */
};

struct i386_instr {
	unsigned long		nr_bytes;

	uint8_t			opcode;		/* Opcode byte */
	uint8_t			width;
	uint8_t			mod;		/* Mod */
	uint8_t			rm;		/* R/M */
	uint8_t			reg_opc;	/* Reg/Opcode */
	uint32_t		disp;		/* Address displacement */
	union {
		uint32_t		imm_data;	/* Immediate data */
		int32_t			rel_data;	/* Relative address data */
	};

	unsigned long		type;		/* See enum i386_instr_types */
	unsigned long		flags;		/* See enum i386_instr_flags */
	enum i386_seg_override	seg_override;
	enum i386_rep_prefix	rep_prefix;
	struct i386_operand	src;
	struct i386_operand	dst;
};

int
arch_i386_decode_instr(struct i386_instr *instr, uint8_t* RAM, addr_t pc);

int
arch_i386_instr_length(struct i386_instr *instr);

#endif /* I386_DECODE_H */
