#include "libcpu.h"
#include "frontend.h"
#include "fapra_internal.h"

static inline uint32_t opc(uint32_t ins) {
  return ins >> 26;
}

static const char *disas_opc(uint32_t ins) {
	switch (opc(ins)) {
	default:
		fprintf(stderr, "Illegal instruction!\n");
		exit(EXIT_FAILURE);
	case RFE:
		return "rfe";
	case PERM:
		return "perm";
	case RDC8:
		return "rdc8";
	case TGE:
		return "tge";
	case TSE:
		return "tse";
	case LDW:
		return "ldw";
	case STW:
		return "stw";
	case LDB:
		return "ldb";
	case STB:
		return "stb";
	case LDIH:
		return "ldih";
	case LDIL:
		return "ldil";
	case ADDI:
		return "addi";
	case ADD:
		return "add";
	case SUB:
		return "sub";
	case AND:
		return "and";
	case OR:
		return "or";
	case NOT:
		return "not";
	case SARI:
		return "sari";
	case SAL:
		return "sal";
	case SAR:
		return "sar";
	case MUL:
		return "mul";
	case NOP:
		return "nop";
	case JMP:
		return "jmp";
	case BRA:
		return "bra";
	case BZ:
		return "bz";
	case BNZ:
		return "bnz";
	case CALL:
		return "call";
	case BL:
		return "bl";
	}
}

int
arch_fapra_disasm_instr(cpu_t *cpu, addr_t pc, char *line, unsigned int max_line) {
	uint32_t instr = INSTR(pc);

	snprintf(line, max_line, "%s\n", disas_opc(instr));

	return 4;
}
