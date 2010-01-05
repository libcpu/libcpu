#include "libcpu.h"
#include "x86_isa.h"
#include "x86_decode.h"

static unsigned long decode_table[256] = {
	/*[0x0]*/	0,
	/*[0x1]*/	0,
	/*[0x2]*/	0,
	/*[0x3]*/	0,
	/*[0x4]*/	0,
	/*[0x5]*/	0,
	/*[0x6]*/	0,
	/*[0x7]*/	0,
	/*[0x8]*/	0,
	/*[0x9]*/	0,
	/*[0xA]*/	0,
	/*[0xB]*/	0,
	/*[0xC]*/	0,
	/*[0xD]*/	0,
	/*[0xE]*/	0,
	/*[0xF]*/	0,
	/*[0x10]*/	0,
	/*[0x11]*/	0,
	/*[0x12]*/	0,
	/*[0x13]*/	0,
	/*[0x14]*/	0,
	/*[0x15]*/	0,
	/*[0x16]*/	0,
	/*[0x17]*/	0,
	/*[0x18]*/	0,
	/*[0x19]*/	0,
	/*[0x1A]*/	0,
	/*[0x1B]*/	0,
	/*[0x1C]*/	0,
	/*[0x1D]*/	0,
	/*[0x1E]*/	0,
	/*[0x1F]*/	0,
	/*[0x20]*/	0,
	/*[0x21]*/	0,
	/*[0x22]*/	0,
	/*[0x23]*/	0,
	/*[0x24]*/	0,
	/*[0x25]*/	0,
	/*[0x26]*/	0,
	/*[0x27]*/	INSTR_DAA | ADDMODE_IMPLIED,
	/*[0x28]*/	0,
	/*[0x29]*/	0,
	/*[0x2A]*/	0,
	/*[0x2B]*/	0,
	/*[0x2C]*/	0,
	/*[0x2D]*/	0,
	/*[0x2E]*/	0,
	/*[0x2F]*/	INSTR_DAS | ADDMODE_IMPLIED,
	/*[0x30]*/	0,
	/*[0x31]*/	0,
	/*[0x32]*/	0,
	/*[0x33]*/	0,
	/*[0x34]*/	0,
	/*[0x35]*/	0,
	/*[0x36]*/	0,
	/*[0x37]*/	INSTR_AAA | ADDMODE_IMPLIED,
	/*[0x38]*/	0,
	/*[0x39]*/	0,
	/*[0x3A]*/	0,
	/*[0x3B]*/	0,
	/*[0x3C]*/	0,
	/*[0x3D]*/	0,
	/*[0x3E]*/	0,
	/*[0x3F]*/	INSTR_AAS | ADDMODE_IMPLIED,
	/*[0x40]*/	0,
	/*[0x41]*/	0,
	/*[0x42]*/	0,
	/*[0x43]*/	0,
	/*[0x44]*/	0,
	/*[0x45]*/	0,
	/*[0x46]*/	0,
	/*[0x47]*/	0,
	/*[0x48]*/	0,
	/*[0x49]*/	0,
	/*[0x4A]*/	0,
	/*[0x4B]*/	0,
	/*[0x4C]*/	0,
	/*[0x4D]*/	0,
	/*[0x4E]*/	0,
	/*[0x4F]*/	0,
	/*[0x50]*/	INSTR_PUSH | ADDMODE_REG | WIDTH_FULL,
	/*[0x51]*/	INSTR_PUSH | ADDMODE_REG | WIDTH_FULL,
	/*[0x52]*/	INSTR_PUSH | ADDMODE_REG | WIDTH_FULL,
	/*[0x53]*/	INSTR_PUSH | ADDMODE_REG | WIDTH_FULL,
	/*[0x54]*/	INSTR_PUSH | ADDMODE_REG | WIDTH_FULL,
	/*[0x55]*/	INSTR_PUSH | ADDMODE_REG | WIDTH_FULL,
	/*[0x56]*/	INSTR_PUSH | ADDMODE_REG | WIDTH_FULL,
	/*[0x57]*/	INSTR_PUSH | ADDMODE_REG | WIDTH_FULL,
	/*[0x58]*/	INSTR_POP | ADDMODE_REG | WIDTH_FULL,
	/*[0x59]*/	INSTR_POP | ADDMODE_REG | WIDTH_FULL,
	/*[0x5A]*/	INSTR_POP | ADDMODE_REG | WIDTH_FULL,
	/*[0x5B]*/	INSTR_POP | ADDMODE_REG | WIDTH_FULL,
	/*[0x5C]*/	INSTR_POP | ADDMODE_REG | WIDTH_FULL,
	/*[0x5D]*/	INSTR_POP | ADDMODE_REG | WIDTH_FULL,
	/*[0x5E]*/	INSTR_POP | ADDMODE_REG | WIDTH_FULL,
	/*[0x5F]*/	INSTR_POP | ADDMODE_REG | WIDTH_FULL,
	/*[0x60]*/	INSTR_PUSHA | ADDMODE_IMPLIED,
	/*[0x61]*/	INSTR_POPA | ADDMODE_IMPLIED,
	/*[0x62]*/	0,
	/*[0x63]*/	0,
	/*[0x64]*/	0,
	/*[0x65]*/	0,
	/*[0x66]*/	0,
	/*[0x67]*/	0,
	/*[0x68]*/	0,
	/*[0x69]*/	0,
	/*[0x6A]*/	0,
	/*[0x6B]*/	0,
	/*[0x6C]*/	0,
	/*[0x6D]*/	0,
	/*[0x6E]*/	0,
	/*[0x6F]*/	0,
	/*[0x70]*/	0,
	/*[0x71]*/	0,
	/*[0x72]*/	0,
	/*[0x73]*/	0,
	/*[0x74]*/	0,
	/*[0x75]*/	0,
	/*[0x76]*/	0,
	/*[0x77]*/	0,
	/*[0x78]*/	0,
	/*[0x79]*/	0,
	/*[0x7A]*/	0,
	/*[0x7B]*/	0,
	/*[0x7C]*/	0,
	/*[0x7D]*/	0,
	/*[0x7E]*/	0,
	/*[0x7F]*/	0,
	/*[0x80]*/	0,
	/*[0x81]*/	0,
	/*[0x82]*/	0,
	/*[0x83]*/	0,
	/*[0x84]*/	0,
	/*[0x85]*/	0,
	/*[0x86]*/	0,
	/*[0x87]*/	0,
	/*[0x88]*/	INSTR_MOV | ADDMODE_REG_RM | WIDTH_BYTE,
	/*[0x89]*/	INSTR_MOV | ADDMODE_REG_RM | WIDTH_FULL,
	/*[0x8A]*/	0,
	/*[0x8B]*/	0,
	/*[0x8C]*/	0,
	/*[0x8D]*/	0,
	/*[0x8E]*/	0,
	/*[0x8F]*/	0,
	/*[0x90]*/	INSTR_NOP | ADDMODE_IMPLIED,
	/*[0x91]*/	0,
	/*[0x92]*/	0,
	/*[0x93]*/	0,
	/*[0x94]*/	0,
	/*[0x95]*/	0,
	/*[0x96]*/	0,
	/*[0x97]*/	0,
	/*[0x98]*/	INSTR_CBW | ADDMODE_IMPLIED,
	/*[0x99]*/	INSTR_CWD | ADDMODE_IMPLIED,
	/*[0x9A]*/	0,
	/*[0x9B]*/	0,
	/*[0x9C]*/	INSTR_PUSHF | ADDMODE_IMPLIED,
	/*[0x9D]*/	INSTR_POPF | ADDMODE_IMPLIED,
	/*[0x9E]*/	INSTR_SAHF | ADDMODE_IMPLIED,
	/*[0x9F]*/	INSTR_LAHF | ADDMODE_IMPLIED,
	/*[0xA0]*/	0,
	/*[0xA1]*/	0,
	/*[0xA2]*/	0,
	/*[0xA3]*/	0,
	/*[0xA4]*/	0,
	/*[0xA5]*/	0,
	/*[0xA6]*/	0,
	/*[0xA7]*/	0,
	/*[0xA8]*/	0,
	/*[0xA9]*/	0,
	/*[0xAA]*/	0,
	/*[0xAB]*/	0,
	/*[0xAC]*/	0,
	/*[0xAD]*/	0,
	/*[0xAE]*/	0,
	/*[0xAF]*/	0,
	/*[0xB0]*/	0,
	/*[0xB1]*/	0,
	/*[0xB2]*/	0,
	/*[0xB3]*/	0,
	/*[0xB4]*/	0,
	/*[0xB5]*/	0,
	/*[0xB6]*/	0,
	/*[0xB7]*/	0,
	/*[0xB8]*/	INSTR_MOV | ADDMODE_IMM_REG | WIDTH_FULL,
	/*[0xB9]*/	INSTR_MOV | ADDMODE_IMM_REG | WIDTH_FULL,
	/*[0xBA]*/	INSTR_MOV | ADDMODE_IMM_REG | WIDTH_FULL,
	/*[0xBB]*/	INSTR_MOV | ADDMODE_IMM_REG | WIDTH_FULL,
	/*[0xBC]*/	INSTR_MOV | ADDMODE_IMM_REG | WIDTH_FULL,
	/*[0xBD]*/	INSTR_MOV | ADDMODE_IMM_REG | WIDTH_FULL,
	/*[0xBE]*/	INSTR_MOV | ADDMODE_IMM_REG | WIDTH_FULL,
	/*[0xBF]*/	INSTR_MOV | ADDMODE_IMM_REG | WIDTH_FULL,
	/*[0xC0]*/	0,
	/*[0xC1]*/	0,
	/*[0xC2]*/	0,
	/*[0xC3]*/	INSTR_RET | ADDMODE_IMPLIED,
	/*[0xC4]*/	0,
	/*[0xC5]*/	0,
	/*[0xC6]*/	0,
	/*[0xC7]*/	0,
	/*[0xC8]*/	0,
	/*[0xC9]*/	0,
	/*[0xCA]*/	0,
	/*[0xCB]*/	INSTR_RETF | ADDMODE_IMPLIED,
	/*[0xCC]*/	0,
	/*[0xCD]*/	0,
	/*[0xCE]*/	INSTR_INTO | ADDMODE_IMPLIED,
	/*[0xCF]*/	INSTR_IRET | ADDMODE_IMPLIED,
	/*[0xD0]*/	0,
	/*[0xD1]*/	0,
	/*[0xD2]*/	0,
	/*[0xD3]*/	0,
	/*[0xD4]*/	0,
	/*[0xD5]*/	0,
	/*[0xD6]*/	0,
	/*[0xD7]*/	0,
	/*[0xD8]*/	0,
	/*[0xD9]*/	0,
	/*[0xDA]*/	0,
	/*[0xDB]*/	0,
	/*[0xDC]*/	0,
	/*[0xDD]*/	0,
	/*[0xDE]*/	0,
	/*[0xDF]*/	0,
	/*[0xE0]*/	0,
	/*[0xE1]*/	0,
	/*[0xE2]*/	0,
	/*[0xE3]*/	0,
	/*[0xE4]*/	0,
	/*[0xE5]*/	0,
	/*[0xE6]*/	0,
	/*[0xE7]*/	0,
	/*[0xE8]*/	0,
	/*[0xE9]*/	0,
	/*[0xEA]*/	0,
	/*[0xEB]*/	0,
	/*[0xEC]*/	0,
	/*[0xED]*/	0,
	/*[0xEE]*/	0,
	/*[0xEF]*/	0,
	/*[0xF0]*/	0,
	/*[0xF1]*/	0,
	/*[0xF2]*/	0,
	/*[0xF3]*/	0,
	/*[0xF4]*/	INSTR_HLT | ADDMODE_IMPLIED,
	/*[0xF5]*/	INSTR_CMC | ADDMODE_IMPLIED,
	/*[0xF6]*/	0,
	/*[0xF7]*/	0,
	/*[0xF8]*/	INSTR_CLC | ADDMODE_IMPLIED,
	/*[0xF9]*/	INSTR_STC | ADDMODE_IMPLIED,
	/*[0xFA]*/	INSTR_CLI | ADDMODE_IMPLIED,
	/*[0xFB]*/	INSTR_STI | ADDMODE_IMPLIED,
	/*[0xFC]*/	INSTR_CLD | ADDMODE_IMPLIED,
	/*[0xFD]*/	INSTR_STD | ADDMODE_IMPLIED,
	/*[0xFE]*/	0,
	/*[0xFF]*/	0,
};

static void
decode_dst_operand(struct x86_instr *instr)
{
	struct x86_operand *operand = &instr->dst;

	switch (instr->flags & DST_MASK) {
	case DST_NONE:
		break;
	case DST_REG:
		operand->type	= OP_REG;

		if (instr->flags & MOD_RM)
			operand->reg	= instr->rm;
		else
			operand->reg	= instr->opcode & 0x07;
		break;
	case DST_MEM:
		operand->type	= OP_MEM;
		operand->reg	= instr->rm;
		break;
	case DST_MEM_DISP_BYTE:
	case DST_MEM_DISP_FULL:
		operand->type	= OP_MEM_DISP;
		operand->reg	= instr->rm;
		operand->disp	= instr->disp;
		break;
	}
}

static void
decode_src_operand(struct x86_instr *instr)
{
	struct x86_operand *operand = &instr->src;

	switch (instr->flags & SRC_MASK) {
	case SRC_NONE:
		break;
	case SRC_IMM:
		operand->type	= OP_IMM;
		operand->imm	= instr->imm_data;
		break;
	case SRC_REG:
		operand->type	= OP_REG;

		if (instr->flags & MOD_RM)
			operand->reg	= instr->reg_opc;
		else
			operand->reg	= instr->opcode & 0x07;
		break;
	}
}

static void
decode_imm(struct x86_instr *instr, uint8_t* RAM, addr_t *pc)
{
	addr_t new_pc = *pc;

	switch (instr->flags & WIDTH_MASK) {
	case WIDTH_FULL: {
		uint8_t imm_lo = RAM[new_pc++];
		uint8_t imm_hi = RAM[new_pc++];

		instr->imm_data	= (uint16_t)((imm_hi << 8) | imm_lo);
		instr->nr_bytes	+= 2;
		break;
	}
	case WIDTH_BYTE:
		instr->imm_data	= (uint8_t)RAM[new_pc++];
		instr->nr_bytes	+= 1;
		break;
	}
	*pc = new_pc;
}

static void
decode_disp(struct x86_instr *instr, uint8_t* RAM, addr_t *pc)
{
	addr_t new_pc = *pc;

	switch (instr->flags & DST_MASK) {
	case DST_MEM_DISP_FULL: {
		uint8_t disp_lo = RAM[new_pc++];
		uint8_t disp_hi = RAM[new_pc++];

		instr->disp	= (int16_t)((disp_hi << 8) | disp_lo);
		instr->nr_bytes	+= 2;
		break;
	}
	case DST_MEM_DISP_BYTE:
		instr->disp	= (int8_t)RAM[new_pc++];
		instr->nr_bytes	+= 1;
		break;
	}
	*pc = new_pc;
}

static void
decode_modrm_byte(struct x86_instr *instr, uint8_t modrm)
{
	instr->mod	= (modrm & 0xc0) >> 6;
	instr->reg_opc	= (modrm & 0x38) >> 3;
	instr->rm	= (modrm & 0x07);

	switch (instr->mod) {
	case 0x00:
		instr->flags	|= DST_MEM;
		break;
	case 0x01:
		instr->flags	|= DST_MEM_DISP_BYTE;
		break;
	case 0x02:
		instr->flags	|= DST_MEM_DISP_FULL;
		break;
	case 0x03:
		instr->flags	|= DST_REG;
		break;
	}
	instr->nr_bytes++;
}

int
arch_8086_decode_instr(struct x86_instr *instr, uint8_t* RAM, addr_t pc)
{
	uint8_t opcode;

	instr->nr_bytes = 1;

	/* Prefixes */
	instr->seg_override	= NO_OVERRIDE;
	for (;;) {
		switch (opcode = RAM[pc++]) {
		case 0x26:
			instr->seg_override	= ES_OVERRIDE;
			break;
		case 0x2e:
			instr->seg_override	= CS_OVERRIDE;
			break;
		case 0x36:
			instr->seg_override	= SS_OVERRIDE;
			break;
		case 0x3e:
			instr->seg_override	= DS_OVERRIDE;
			break;
		case 0xf2:	/* REPNE/REPNZ */
		case 0xf3:	/* REP/REPE/REPZ */
			break;
		default:
			goto done_prefixes;
		}
		instr->nr_bytes++;
	}

done_prefixes:

	/* Opcode byte */
	instr->opcode	= opcode;

	instr->flags	= decode_table[opcode];

	if (instr->flags == 0)	/* Unrecognized? */
		return -1;

	if (instr->flags & MOD_RM)
		decode_modrm_byte(instr, RAM[pc++]);

	if (instr->flags & MEM_DISP_MASK)
		decode_disp(instr, RAM, &pc);

	if (instr->flags & SRC_IMM)
		decode_imm(instr, RAM, &pc);

	decode_src_operand(instr);

	decode_dst_operand(instr);

	return 0;
}

int
arch_8086_instr_length(struct x86_instr *instr)
{
	return instr->nr_bytes;
}
