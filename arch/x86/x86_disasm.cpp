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
#define DECLARE_INSTR(name,str) str,
#include "x86_instr.h"
#undef DECLARE_INSTR
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
print_operand(addr_t pc, char *operands, size_t size, struct x86_instr *instr, struct x86_operand *operand)
{
	int ret = 0;

	switch (operand->type) {
	case OP_IMM:
		ret = snprintf(operands, size, "$0x%x", operand->imm);
		break;
	case OP_REL:
		ret = snprintf(operands, size, "%x", (unsigned int)((long)pc + instr->nr_bytes + operand->rel));
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
		len += print_operand(pc, operands+len, sizeof(operands)-len, &instr, &instr.src);

	if (!(instr.flags & SRC_NONE) && !(instr.flags & DST_NONE))
		len += snprintf(operands+len, sizeof(operands)-len, ",");

	if (!(instr.flags & DST_NONE))
		len += print_operand(pc, operands+len, sizeof(operands)-len, &instr, &instr.dst);

        snprintf(line, max_line, "%s%s\t%s", prefix_names[instr.rep_prefix], to_mnemonic(&instr), operands);

        return arch_8086_instr_length(&instr);
}
