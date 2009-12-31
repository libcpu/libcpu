#include <stdlib.h>
#include <stdio.h>

#include "libcpu.h"
#include "8086_isa.h"

static const char* mnemo[] = {
	/* [INSTR_AAA] */ 	 "AAA",
	/* [INSTR_AAD] */ 	 "AAD",
	/* [INSTR_AAM] */ 	 "AAM",
	/* [INSTR_AAS] */ 	 "AAS",
	/* [INSTR_ADC] */ 	 "ADC",
	/* [INSTR_ADD] */ 	 "ADD",
	/* [INSTR_AND] */ 	 "AND",
	/* [INSTR_CALL] */ 	 "CALL",
	/* [INSTR_CBW] */ 	 "CBW",
	/* [INSTR_CLC] */ 	 "CLC",
	/* [INSTR_CLD] */ 	 "CLD",
	/* [INSTR_CLI] */ 	 "CLI",
	/* [INSTR_CMC] */ 	 "CMC",
	/* [INSTR_CMP] */ 	 "CMP",
	/* [INSTR_CMPSB] */ 	 "CMPSB",
	/* [INSTR_CMPSW] */ 	 "CMPSW",
	/* [INSTR_CWD] */ 	 "CWD",
	/* [INSTR_DAA] */ 	 "DAA",
	/* [INSTR_DAS] */ 	 "DAS",
	/* [INSTR_DEC] */ 	 "DEC",
	/* [INSTR_DIV] */ 	 "DIV",
	/* [INSTR_HLT] */ 	 "HLT",
	/* [INSTR_IDIV] */ 	 "IDIV",
	/* [INSTR_IMUL] */ 	 "IMUL",
	/* [INSTR_IN] */ 	 "IN",
	/* [INSTR_INC] */ 	 "INC",
	/* [INSTR_INT] */ 	 "INT",
	/* [INSTR_INTO] */ 	 "INTO",
	/* [INSTR_IRET] */ 	 "IRET",
	/* [INSTR_JA] */ 	 "JA",
	/* [INSTR_JAE] */ 	 "JAE",
	/* [INSTR_JB] */ 	 "JB",
	/* [INSTR_JBE] */ 	 "JBE",
	/* [INSTR_JC] */ 	 "JC",
	/* [INSTR_JCXZ] */ 	 "JCXZ",
	/* [INSTR_JE] */ 	 "JE",
	/* [INSTR_JG] */ 	 "JG",
	/* [INSTR_JGE] */ 	 "JGE",
	/* [INSTR_JL] */ 	 "JL",
	/* [INSTR_JLE] */ 	 "JLE",
	/* [INSTR_JMP] */ 	 "JMP",
	/* [INSTR_JNA] */ 	 "JNA",
	/* [INSTR_JNAE] */ 	 "JNAE",
	/* [INSTR_JNB] */ 	 "JNB",
	/* [INSTR_JNBE] */ 	 "JNBE",
	/* [INSTR_JNC] */ 	 "JNC",
	/* [INSTR_JNE] */ 	 "JNE",
	/* [INSTR_JNG] */ 	 "JNG",
	/* [INSTR_JNGE] */ 	 "JNGE",
	/* [INSTR_JNL] */ 	 "JNL",
	/* [INSTR_JNLE] */ 	 "JNLE",
	/* [INSTR_JNO] */ 	 "JNO",
	/* [INSTR_JNP] */ 	 "JNP",
	/* [INSTR_JNS] */ 	 "JNS",
	/* [INSTR_JNZ] */ 	 "JNZ",
	/* [INSTR_JO] */ 	 "JO",
	/* [INSTR_JP] */ 	 "JP",
	/* [INSTR_JPE] */ 	 "JPE",
	/* [INSTR_JPO] */ 	 "JPO",
	/* [INSTR_JS] */ 	 "JS",
	/* [INSTR_JZ] */ 	 "JZ",
	/* [INSTR_LAHF] */ 	 "LAHF",
	/* [INSTR_LDS] */ 	 "LDS",
	/* [INSTR_LEA] */ 	 "LEA",
	/* [INSTR_LES] */ 	 "LES",
	/* [INSTR_LODSB] */ 	 "LODSB",
	/* [INSTR_LODSW] */ 	 "LODSW",
	/* [INSTR_LOOP] */ 	 "LOOP",
	/* [INSTR_LOOPE] */ 	 "LOOPE",
	/* [INSTR_LOOPNE] */ 	 "LOOPNE",
	/* [INSTR_LOOPNZ] */ 	 "LOOPNZ",
	/* [INSTR_LOOPZ] */ 	 "LOOPZ",
	/* [INSTR_MOV] */ 	 "MOV",
	/* [INSTR_MOVSB] */ 	 "MOVSB",
	/* [INSTR_MOVSW] */ 	 "MOVSW",
	/* [INSTR_MUL] */ 	 "MUL",
	/* [INSTR_NEG] */ 	 "NEG",
	/* [INSTR_NOP] */ 	 "NOP",
	/* [INSTR_NOT] */ 	 "NOT",
	/* [INSTR_OR] */ 	 "OR",
	/* [INSTR_OUT] */ 	 "OUT",
	/* [INSTR_POP] */ 	 "POP",
	/* [INSTR_POPA] */ 	 "POPA",
	/* [INSTR_POPF] */ 	 "POPF",
	/* [INSTR_PUSH] */ 	 "PUSH",
	/* [INSTR_PUSHA] */ 	 "PUSHA",
	/* [INSTR_PUSHF] */ 	 "PUSHF",
	/* [INSTR_RCL] */ 	 "RCL",
	/* [INSTR_RCR] */ 	 "RCR",
	/* [INSTR_REP] */ 	 "REP",
	/* [INSTR_REPE] */ 	 "REPE",
	/* [INSTR_REPNE] */ 	 "REPNE",
	/* [INSTR_REPNZ] */ 	 "REPNZ",
	/* [INSTR_REPZ] */ 	 "REPZ",
	/* [INSTR_RET] */ 	 "RET",
	/* [INSTR_RETF] */ 	 "RETF",
	/* [INSTR_ROL] */ 	 "ROL",
	/* [INSTR_ROR] */ 	 "ROR",
	/* [INSTR_SAHF] */ 	 "SAHF",
	/* [INSTR_SAL] */ 	 "SAL",
	/* [INSTR_SAR] */ 	 "SAR",
	/* [INSTR_SBB] */ 	 "SBB",
	/* [INSTR_SCASB] */ 	 "SCASB",
	/* [INSTR_SCASW] */ 	 "SCASW",
	/* [INSTR_SHL] */ 	 "SHL",
	/* [INSTR_SHR] */ 	 "SHR",
	/* [INSTR_STC] */ 	 "STC",
	/* [INSTR_STD] */ 	 "STD",
	/* [INSTR_STI] */ 	 "STI",
	/* [INSTR_STOSB] */ 	 "STOSB",
	/* [INSTR_STOSW] */ 	 "STOSW",
	/* [INSTR_SUB] */ 	 "SUB",
	/* [INSTR_TEST] */ 	 "TEST",
	/* [INSTR_XCHG] */ 	 "XCHG",
	/* [INSTR_XLATB] */ 	 "XLATB",
	/* [INSTR_XOR] */ 	 "XOR",
};

static const char *reg_names[] = {
	"AL",
	"CL",
	"DL",
	"BL",
	"AH",
	"CH",
	"DH",
	"BH",
};

static const char *reg_names_wide[] = {
	"AX",
	"CX",
	"DX",
	"BX",
	"SP",
	"BP",
	"SI",
	"DI",
};

static const char *to_reg_name(int reg_num, int w)
{
	if (w)
		return reg_names_wide[reg_num];

	return reg_names[reg_num];
}

struct x86_instr {
	int		nr_prefixes;
	uint8_t		opcode;
	int		w;		/* word/byte */
	int		d;		/* direction (or s = sign extension) */
	int		reg;
};

static int
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
		instr->w	= 0;
		instr->d	= 0;
		instr->reg	= opcode & 0x03;
		break;
	case 0xb8:	/* Move imm16 to reg16 */
		instr->w	= 1;
		instr->d	= 0;
		instr->reg	= opcode & 0x03;
		break;
	default:
		instr->w	= opcode & 0x01;
		instr->d	= opcode & 0x02;
		break;
	}

	return 0;
}

static int
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

int
arch_8086_disasm_instr(cpu_t *cpu, addr_t pc, char *line, unsigned int max_line)
{
	struct x86_instr instr;

	if (arch_8086_decode_instr(&instr, cpu->RAM, pc) != 0)
		return -1;

        snprintf(line, max_line, "%x %s", instr.opcode, to_reg_name(instr.reg, instr.w));

        return arch_8086_instr_length(&instr);
}
