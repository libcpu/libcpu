/*
 * libcpu: x86_disasm.cpp
 *
 * disassembler
 */

#include <stdlib.h>
#include <stdio.h>

#include "libcpu.h"
#include "x86_isa.h"
#include "x86_decode.h"

static const char* mnemo[] = {
	/* [INSTR_AAA] */	 "aaa",
	/* [INSTR_AAD] */	 "aad",
	/* [INSTR_AAM] */	 "aam",
	/* [INSTR_AAS] */	 "aas",
	/* [INSTR_ADC] */	 "adc",
	/* [INSTR_ADD] */	 "add",
	/* [INSTR_AND] */	 "and",
	/* [INSTR_CALL] */	 "call",
	/* [INSTR_CBW] */	 "cbw",
	/* [INSTR_CLC] */	 "clc",
	/* [INSTR_CLD] */	 "cld",
	/* [INSTR_CLI] */	 "cli",
	/* [INSTR_CMC] */	 "cmc",
	/* [INSTR_CMPSB] */	 "cmpsb",
	/* [INSTR_CMPSW] */	 "cmpsw",
	/* [INSTR_CMP] */	 "cmp",
	/* [INSTR_CWD] */	 "cwd",
	/* [INSTR_DAA] */	 "daa",
	/* [INSTR_DAS] */	 "das",
	/* [INSTR_DEC] */	 "dec",
	/* [INSTR_DIV] */	 "div",
	/* [INSTR_HLT] */	 "hlt",
	/* [INSTR_IDIV] */	 "idiv",
	/* [INSTR_IMUL] */	 "imul",
	/* [INSTR_INC] */	 "inc",
	/* [INSTR_INTO] */	 "into",
	/* [INSTR_INT] */	 "int",
	/* [INSTR_IN] */	 "in",
	/* [INSTR_IRET] */	 "iret",
	/* [INSTR_JAE] */	 "jae",
	/* [INSTR_JA] */	 "ja",
	/* [INSTR_JBE] */	 "jbe",
	/* [INSTR_JB] */	 "jb",
	/* [INSTR_JCXZ] */	 "jcxz",
	/* [INSTR_JC] */	 "jc",
	/* [INSTR_JE] */	 "je",
	/* [INSTR_JGE] */	 "jge",
	/* [INSTR_JG] */	 "jg",
	/* [INSTR_JLE] */	 "jle",
	/* [INSTR_JL] */	 "jl",
	/* [INSTR_JMP] */	 "jmp",
	/* [INSTR_JNAE] */	 "jnae",
	/* [INSTR_JNA] */	 "jna",
	/* [INSTR_JNBE] */	 "jnbe",
	/* [INSTR_JNB] */	 "jnb",
	/* [INSTR_JNC] */	 "jnc",
	/* [INSTR_JNE] */	 "jne",
	/* [INSTR_JNGE] */	 "jnge",
	/* [INSTR_JNG] */	 "jng",
	/* [INSTR_JNLE] */	 "jnle",
	/* [INSTR_JNL] */	 "jnl",
	/* [INSTR_JNO] */	 "jno",
	/* [INSTR_JNP] */	 "jnp",
	/* [INSTR_JNS] */	 "jns",
	/* [INSTR_JNZ] */	 "jnz",
	/* [INSTR_JO] */	 "jo",
	/* [INSTR_JPE] */	 "jpe",
	/* [INSTR_JPO] */	 "jpo",
	/* [INSTR_JP] */	 "jp",
	/* [INSTR_JS] */	 "js",
	/* [INSTR_JZ] */	 "jz",
	/* [INSTR_LAHF] */	 "lahf",
	/* [INSTR_LDS] */	 "lds",
	/* [INSTR_LEA] */	 "lea",
	/* [INSTR_LES] */	 "les",
	/* [INSTR_LODSB] */	 "lodsb",
	/* [INSTR_LODSW] */	 "lodsw",
	/* [INSTR_LOOPE] */	 "loope",
	/* [INSTR_LOOPNE] */	 "loopne",
	/* [INSTR_LOOPNZ] */	 "loopnz",
	/* [INSTR_LOOPZ] */	 "loopz",
	/* [INSTR_LOOP] */	 "loop",
	/* [INSTR_MOVSB] */	 "movsb",
	/* [INSTR_MOVSW] */	 "movsw",
	/* [INSTR_MOV] */	 "mov",
	/* [INSTR_MUL] */	 "mul",
	/* [INSTR_NEG] */	 "neg",
	/* [INSTR_NOP] */	 "nop",
	/* [INSTR_NOT] */	 "not",
	/* [INSTR_OR] */	 "or",
	/* [INSTR_OUT] */	 "out",
	/* [INSTR_POPA] */	 "popa",
	/* [INSTR_POPF] */	 "popf",
	/* [INSTR_POP] */	 "pop",
	/* [INSTR_PUSHA] */	 "pusha",
	/* [INSTR_PUSHF] */	 "pushf",
	/* [INSTR_PUSH] */	 "push",
	/* [INSTR_RCL] */	 "rcl",
	/* [INSTR_RCR] */	 "rcr",
	/* [INSTR_REPE] */	 "repe",
	/* [INSTR_REPNE] */	 "repne",
	/* [INSTR_REPNZ] */	 "repnz",
	/* [INSTR_REPZ] */	 "repz",
	/* [INSTR_REP] */	 "rep",
	/* [INSTR_RETF] */	 "retf",
	/* [INSTR_RET] */	 "ret",
	/* [INSTR_ROL] */	 "rol",
	/* [INSTR_ROR] */	 "ror",
	/* [INSTR_SAHF] */	 "sahf",
	/* [INSTR_SAL] */	 "sal",
	/* [INSTR_SAR] */	 "sar",
	/* [INSTR_SBB] */	 "sbb",
	/* [INSTR_SCASB] */	 "scasb",
	/* [INSTR_SCASW] */	 "scasw",
	/* [INSTR_SHIFT_GRP2] */ "<grp2>",
	/* [INSTR_SHL] */	 "shl",
	/* [INSTR_SHR] */	 "shr",
	/* [INSTR_STC] */	 "stc",
	/* [INSTR_STD] */	 "std",
	/* [INSTR_STI] */	 "sti",
	/* [INSTR_STOSB] */	 "stosb",
	/* [INSTR_STOSW] */	 "stosw",
	/* [INSTR_SUB] */	 "sub",
	/* [INSTR_TEST] */	 "test",
	/* [INSTR_XCHG] */	 "xchg",
	/* [INSTR_XLATB] */	 "xlatb",
	/* [INSTR_XOR] */	 "xor",
};

static const char *to_mnemonic(struct x86_instr *instr)
{
	return mnemo[instr->type];
}

static const char *byte_reg_names[] = {
	"%al",
	"%cl",
	"%dl",
	"%bl",
	"%ah",
	"%ch",
	"%dh",
	"%bh",
};

static const char *word_reg_names[] = {
	"%ax",
	"%cx",
	"%dx",
	"%bx",
	"%sp",
	"%bp",
	"%si",
	"%di",
};

static const char *mem_byte_reg_names[] = {
	"%bx,%si",
	"%bx,%di",
	"%bp,%si",
	"%bp,%di",
	"%si",
	"%di",
	NULL,
	"%bx",
};

static const char *seg_override_names[] = {
	"",
	"%es:",
	"%cs:",
	"%ss:",
	"%ds:",
};

static const char *prefix_names[] = {
	"",
	"repnz ",
	"rep ",
};

static const char *sign_to_str(int n)
{
	if (n >= 0)
		return "";

	return "-";
}

static const char *to_reg_name(struct x86_instr *instr, int reg_num)
{
	if (instr->flags & WIDTH_BYTE)
		return byte_reg_names[reg_num];

	return word_reg_names[reg_num];
}

static int
print_operand(char *operands, size_t size, struct x86_instr *instr, struct x86_operand *operand)
{
	int ret = 0;

	switch (operand->type) {
	case OP_IMM:
		ret = snprintf(operands, size, "$0x%x", operand->imm);
		break;
	case OP_REG:
		ret = snprintf(operands, size, "%s", to_reg_name(instr, operand->reg));
		break;
	case OP_MEM:
		ret = snprintf(operands, size, "%s(%s)", seg_override_names[instr->seg_override], mem_byte_reg_names[operand->reg]);
		break;
	case OP_MEM_DISP:
		ret = snprintf(operands, size, "%s%s0x%x(%s)", seg_override_names[instr->seg_override], sign_to_str(operand->disp), abs(operand->disp), mem_byte_reg_names[operand->reg]);
		break;
	}
	return ret;
}

int
arch_8086_disasm_instr(cpu_t *cpu, addr_t pc, char *line, unsigned int max_line)
{
	struct x86_instr instr;
	char operands[32];
	int len = 0;

	if (arch_8086_decode_instr(&instr, cpu->RAM, pc) != 0) {
		fprintf(stderr, "error: unable to decode opcode %x\n", instr.opcode);
		exit(1);
	}

	operands[0] = '\0';

	/* AT&T syntax operands */
	if (!(instr.flags & SRC_NONE))
		len += print_operand(operands+len, sizeof(operands)-len, &instr, &instr.src);

	if (!(instr.flags & SRC_NONE) && !(instr.flags & DST_NONE))
		len += snprintf(operands+len, sizeof(operands)-len, ",");

	if (!(instr.flags & DST_NONE))
		len += print_operand(operands+len, sizeof(operands)-len, &instr, &instr.dst);

        snprintf(line, max_line, "%s%s\t%s", prefix_names[instr.rep_prefix], to_mnemonic(&instr), operands);

        return arch_8086_instr_length(&instr);
}
