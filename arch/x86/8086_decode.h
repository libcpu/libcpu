#ifndef I8086_DECODE_H
#define I8086_DECODE_H

#include "libcpu.h"

#include <stdint.h>

/* Decoded x86 instruction */
struct x86_instr {
	int		type;

	int		nr_prefixes;
	uint8_t		opcode;
	int		w;		/* word/byte */
	int		d;		/* direction (or s = sign extension) */
	int		reg;

	uint8_t		imm_lo;
	uint8_t		imm_hi;
};

int
arch_8086_decode_instr(struct x86_instr *instr, uint8_t* RAM, addr_t pc);

int
arch_8086_instr_length(struct x86_instr *instr);

#endif /* I8086_DECODE_H */
