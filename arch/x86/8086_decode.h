#ifndef I8086_DECODE_H
#define I8086_DECODE_H

#include "libcpu.h"

#include <stdint.h>

enum x86_operand_type {
	OP_REG,
	OP_IMM,
};

struct x86_operand {
	enum x86_operand_type	type;
	uint8_t			reg;
	uint8_t			imm;
};

enum x86_instr_flags {
	ModRM			= (1U << 8),
	ByteOp			= (1U << 9),	/* Byte Operands */

	SrcNone			= (1U << 10),
	SrcImm8			= (1U << 11),
	SrcImm16		= (1U << 12),
	SrcMask			= SrcNone|SrcImm8|SrcImm16,

	DstNone			= (1U << 13),
	DstReg			= (1U << 14),
	DstMask			= DstNone|DstReg,
};

struct x86_instr {
	uint8_t			opcode;		/* opcode byte */
	unsigned long		op_bytes;
	unsigned long		flags;		/* see enum x86_instr_flags */
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

#endif /* I8086_DECODE_H */
