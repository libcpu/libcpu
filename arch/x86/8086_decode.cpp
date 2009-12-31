#include "libcpu.h"
#include "8086_isa.h"
#include "8086_decode.h"

int
arch_8086_decode_instr(struct x86_instr *instr, uint8_t* RAM, addr_t pc)
{
	uint8_t opcode;

	/* Prefixes */
	instr->nr_prefixes = 0;
	for (;;) {
		switch (opcode = RAM[pc++]) {
		case 0x26:	/* ES override */
		case 0x2e:	/* CS override */
		case 0x36:	/* SS override */
		case 0x3e:	/* DS override */
		case 0xf2:	/* REPNE/REPNZ */
		case 0xf3:	/* REP/REPE/REPZ */
			instr->nr_prefixes++;
			break;
		default:
			goto done_prefixes;	
		}
	}

done_prefixes:
	instr->opcode	= opcode & ~0x03;

	switch (instr->opcode) {
	case 0xb0:	/* Move imm8 to reg8 */
		instr->type	= INSTR_MOV;
		instr->w	= 0;
		instr->d	= 1;
		instr->reg	= opcode & 0x03;
		break;
	case 0xb8:	/* Move imm16 to reg16 */
		instr->type	= INSTR_MOV;
		instr->w	= 1;
		instr->d	= 1;
		instr->reg	= opcode & 0x03;
		break;
	default:
		instr->w	= opcode & 0x01;
		instr->d	= opcode & 0x02;
		break;
	}
	instr->imm_lo	= RAM[pc++];	
	instr->imm_hi	= RAM[pc++];	

	return 0;
}

int
arch_8086_instr_length(struct x86_instr *instr)
{
	int len = instr->nr_prefixes;

	len += 1;		/* opcode byte */

	if (instr->w)
		len += 2;	/* data is word */
	else
		len += 1;	/* data is byte */

	return len;
}
