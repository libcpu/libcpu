#include <stdlib.h>
#include <stdio.h>

#include "libcpu.h"
#include "8086_isa.h"
#include "8086_decode.h"

static const char* mnemo[] = {
	/* [INSTR_AAA] */ 	 "aaa",
	/* [INSTR_AAD] */ 	 "aad",
	/* [INSTR_AAM] */ 	 "aam",
	/* [INSTR_AAS] */ 	 "aas",
	/* [INSTR_ADC] */ 	 "adc",
	/* [INSTR_ADD] */ 	 "add",
	/* [INSTR_AND] */ 	 "and",
	/* [INSTR_CALL] */ 	 "call",
	/* [INSTR_CBW] */ 	 "cbw",
	/* [INSTR_CLC] */ 	 "clc",
	/* [INSTR_CLD] */ 	 "cld",
	/* [INSTR_CLI] */ 	 "cli",
	/* [INSTR_CMC] */ 	 "cmc",
	/* [INSTR_CMP] */ 	 "cmp",
	/* [INSTR_CMPSB] */ 	 "cmpsb",
	/* [INSTR_CMPSW] */ 	 "cmpsw",
	/* [INSTR_CWD] */ 	 "cwd",
	/* [INSTR_DAA] */ 	 "daa",
	/* [INSTR_DAS] */ 	 "das",
	/* [INSTR_DEC] */ 	 "dec",
	/* [INSTR_DIV] */ 	 "div",
	/* [INSTR_HLT] */ 	 "hlt",
	/* [INSTR_IDIV] */ 	 "idiv",
	/* [INSTR_IMUL] */ 	 "imul",
	/* [INSTR_IN] */ 	 "in",
	/* [INSTR_INC] */ 	 "inc",
	/* [INSTR_INT] */ 	 "int",
	/* [INSTR_INTO] */ 	 "into",
	/* [INSTR_IRET] */ 	 "iret",
	/* [INSTR_JA] */ 	 "ja",
	/* [INSTR_JAE] */ 	 "jae",
	/* [INSTR_JB] */ 	 "jb",
	/* [INSTR_JBE] */ 	 "jbe",
	/* [INSTR_JC] */ 	 "jc",
	/* [INSTR_JCXZ] */ 	 "jcxz",
	/* [INSTR_JE] */ 	 "je",
	/* [INSTR_JG] */ 	 "jg",
	/* [INSTR_JGE] */ 	 "jge",
	/* [INSTR_JL] */ 	 "jl",
	/* [INSTR_JLE] */ 	 "jle",
	/* [INSTR_JMP] */ 	 "jmp",
	/* [INSTR_JNA] */ 	 "jna",
	/* [INSTR_JNAE] */ 	 "jnae",
	/* [INSTR_JNB] */ 	 "jnb",
	/* [INSTR_JNBE] */ 	 "jnbe",
	/* [INSTR_JNC] */ 	 "jnc",
	/* [INSTR_JNE] */ 	 "jne",
	/* [INSTR_JNG] */ 	 "jng",
	/* [INSTR_JNGE] */ 	 "jnge",
	/* [INSTR_JNL] */ 	 "jnl",
	/* [INSTR_JNLE] */ 	 "jnle",
	/* [INSTR_JNO] */ 	 "jno",
	/* [INSTR_JNP] */ 	 "jnp",
	/* [INSTR_JNS] */ 	 "jns",
	/* [INSTR_JNZ] */ 	 "jnz",
	/* [INSTR_JO] */ 	 "jo",
	/* [INSTR_JP] */ 	 "jp",
	/* [INSTR_JPE] */ 	 "jpe",
	/* [INSTR_JPO] */ 	 "jpo",
	/* [INSTR_JS] */ 	 "js",
	/* [INSTR_JZ] */ 	 "jz",
	/* [INSTR_LAHF] */ 	 "lahf",
	/* [INSTR_LDS] */ 	 "lds",
	/* [INSTR_LEA] */ 	 "lea",
	/* [INSTR_LES] */ 	 "les",
	/* [INSTR_LODSB] */ 	 "lodsb",
	/* [INSTR_LODSW] */ 	 "lodsw",
	/* [INSTR_LOOP] */ 	 "loop",
	/* [INSTR_LOOPE] */ 	 "loope",
	/* [INSTR_LOOPNE] */ 	 "loopne",
	/* [INSTR_LOOPNZ] */ 	 "loopnz",
	/* [INSTR_LOOPZ] */ 	 "loopz",
	/* [INSTR_MOV] */ 	 "mov",
	/* [INSTR_MOVSB] */ 	 "movsb",
	/* [INSTR_MOVSW] */ 	 "movsw",
	/* [INSTR_MUL] */ 	 "mul",
	/* [INSTR_NEG] */ 	 "neg",
	/* [INSTR_NOP] */ 	 "nop",
	/* [INSTR_NOT] */ 	 "not",
	/* [INSTR_OR] */ 	 "or",
	/* [INSTR_OUT] */ 	 "out",
	/* [INSTR_POP] */ 	 "pop",
	/* [INSTR_POPA] */ 	 "popa",
	/* [INSTR_POPF] */ 	 "popf",
	/* [INSTR_PUSH] */ 	 "push",
	/* [INSTR_PUSHA] */ 	 "pusha",
	/* [INSTR_PUSHF] */ 	 "pushf",
	/* [INSTR_RCL] */ 	 "rcl",
	/* [INSTR_RCR] */ 	 "rcr",
	/* [INSTR_REP] */ 	 "rep",
	/* [INSTR_REPE] */ 	 "repe",
	/* [INSTR_REPNE] */ 	 "repne",
	/* [INSTR_REPNZ] */ 	 "repnz",
	/* [INSTR_REPZ] */ 	 "repz",
	/* [INSTR_RET] */ 	 "ret",
	/* [INSTR_RETF] */ 	 "retf",
	/* [INSTR_ROL] */ 	 "rol",
	/* [INSTR_ROR] */ 	 "ror",
	/* [INSTR_SAHF] */ 	 "sahf",
	/* [INSTR_SAL] */ 	 "sal",
	/* [INSTR_SAR] */ 	 "sar",
	/* [INSTR_SBB] */ 	 "sbb",
	/* [INSTR_SCASB] */ 	 "scasb",
	/* [INSTR_SCASW] */ 	 "scasw",
	/* [INSTR_SHL] */ 	 "shl",
	/* [INSTR_SHR] */ 	 "shr",
	/* [INSTR_STC] */ 	 "stc",
	/* [INSTR_STD] */ 	 "std",
	/* [INSTR_STI] */ 	 "sti",
	/* [INSTR_STOSB] */ 	 "stosb",
	/* [INSTR_STOSW] */ 	 "stosw",
	/* [INSTR_SUB] */ 	 "sub",
	/* [INSTR_TEST] */ 	 "test",
	/* [INSTR_XCHG] */ 	 "xchg",
	/* [INSTR_XLATB] */ 	 "xlatb",
	/* [INSTR_XOR] */ 	 "xor",
};

static const char *to_mnemonic(struct x86_instr *instr)
{
	return mnemo[instr->type];
}

static const char *reg_names[] = {
	"al",
	"cl",
	"dl",
	"bl",
	"ah",
	"ch",
	"dh",
	"bh",
};

static const char *reg_names_wide[] = {
	"ax",
	"cx",
	"dx",
	"bx",
	"sp",
	"bp",
	"si",
	"di",
};

static const char *to_reg_name(int reg_num, int w)
{
	if (w)
		return reg_names_wide[reg_num];

	return reg_names[reg_num];
}

int
arch_8086_disasm_instr(cpu_t *cpu, addr_t pc, char *line, unsigned int max_line)
{
	struct x86_instr instr;
	char operands[32];

	if (arch_8086_decode_instr(&instr, cpu->RAM, pc) != 0)
		return -1;

	/* AT&T syntax operands */
	if (instr.d)
		snprintf(operands, sizeof(operands), "$0x%02x%02x,%s", instr.imm_hi, instr.imm_lo, to_reg_name(instr.reg, instr.w));
	else
		snprintf(operands, sizeof(operands), "%s,$0x%02x%02x", to_reg_name(instr.reg, instr.w), instr.imm_hi, instr.imm_lo);

        snprintf(line, max_line, "%s\t%s", to_mnemonic(&instr), operands);

        return arch_8086_instr_length(&instr);
}
