#include <stdlib.h>
#include <stdio.h>

#include "types.h"
#include "libcpu.h"
#include "6502_isa.h"

static const char *addmode_template[] = {
	/*[ADDMODE_ABS]*/	"$%04X",
	/*[ADDMODE_ABSX]*/	"$%04X,X",
	/*[ADDMODE_ABSY]*/	"$%04X,Y",
	/*[ADDMODE_ACC]*/	"A",
	/*[ADDMODE_BRA]*/	"$%02X",
	/*[ADDMODE_IMM]*/	"#$%02X",
	/*[ADDMODE_IMPL]*/	"",
	/*[ADDMODE_IND]*/	"($%04X)",
	/*[ADDMODE_INDX]*/	"($%02X,X)",
	/*[ADDMODE_INDY]*/	"($%02X),Y",
	/*[ADDMODE_ZP]*/	"$%02X",
	/*[ADDMODE_ZPX]*/	"$%02X,X",
	/*[ADDMODE_ZPY]*/	"$%02X,Y"
};

static const char* mnemo[] = {
	/*[INSTR_ADC]*/ "ADC",
	/*[INSTR_AND]*/ "AND",
	/*[INSTR_ASL]*/ "ASL",
	/*[INSTR_BCC]*/ "BCC",
	/*[INSTR_BCS]*/ "BCS",
	/*[INSTR_BEQ]*/ "BEQ",
	/*[INSTR_BIT]*/ "BIT",
	/*[INSTR_BMI]*/ "BMI",
	/*[INSTR_BNE]*/ "BNE",
	/*[INSTR_BPL]*/ "BPL",
	/*[INSTR_BRK]*/ "BRK",
	/*[INSTR_BVC]*/ "BVC",
	/*[INSTR_BVS]*/ "BVS",
	/*[INSTR_CLC]*/ "CLC",
	/*[INSTR_CLD]*/ "CLD",
	/*[INSTR_CLI]*/ "CLI",
	/*[INSTR_CLV]*/ "CLV",
	/*[INSTR_CMP]*/ "CMP",
	/*[INSTR_CPX]*/ "CPX",
	/*[INSTR_CPY]*/ "CPY",
	/*[INSTR_DEC]*/ "DEC",
	/*[INSTR_DEX]*/ "DEX",
	/*[INSTR_DEY]*/ "DEY",
	/*[INSTR_EOR]*/ "EOR",
	/*[INSTR_INC]*/ "INC",
	/*[INSTR_INX]*/ "INX",
	/*[INSTR_INY]*/ "INY",
	/*[INSTR_JMP]*/ "JMP",
	/*[INSTR_JSR]*/ "JSR",
	/*[INSTR_LDA]*/ "LDA",
	/*[INSTR_LDX]*/ "LDX",
	/*[INSTR_LDY]*/ "LDY",
	/*[INSTR_LSR]*/ "LSR",
	/*[INSTR_NOP]*/ "NOP",
	/*[INSTR_ORA]*/ "ORA",
	/*[INSTR_PHA]*/ "PHA",
	/*[INSTR_PHP]*/ "PHP",
	/*[INSTR_PLA]*/ "PLA",
	/*[INSTR_PLP]*/ "PLP",
	/*[INSTR_ROL]*/ "ROL",
	/*[INSTR_ROR]*/ "ROR",
	/*[INSTR_RTI]*/ "RTI",
	/*[INSTR_RTS]*/ "RTS",
	/*[INSTR_SBC]*/ "SBC",
	/*[INSTR_SEC]*/ "SEC",
	/*[INSTR_SED]*/ "SED",
	/*[INSTR_SEI]*/ "SEI",
	/*[INSTR_STA]*/ "STA",
	/*[INSTR_STX]*/ "STX",
	/*[INSTR_STY]*/ "STY",
	/*[INSTR_TAX]*/ "TAX",
	/*[INSTR_TAY]*/ "TAY",
	/*[INSTR_TSX]*/ "TSX",
	/*[INSTR_TXA]*/ "TXA",
	/*[INSTR_TXS]*/ "TXS",
	/*[INSTR_TYA]*/ "TYA",
	/*[INSTR_XXX]*/ "???"	
};

static int
arch_6502_instr_length(uint8_t* RAM, addr_t pc) {
	return length[instraddmode[RAM[pc]].addmode]+1;
}

/*
 * Write an ASCII disassembly of one instruction at "pc"
 * in "RAM" into "line" (max length "max_line"), return
 * number of bytes consumed.
 */
int
arch_6502_disasm_instr(cpu_t *cpu, addr_t pc, char *line, unsigned int max_line) {
	uint8_t opcode = cpu->RAM[pc];
	char line2[8];

	if (instraddmode[opcode].addmode == ADDMODE_BRA) {
			snprintf(line2, sizeof(line2), "$%02llX", pc+2 + (int8_t)cpu->RAM[pc+1]);
	} else {
		switch (length[instraddmode[opcode].addmode]) {
			case 0:
				snprintf(line2, sizeof(line2), addmode_template[instraddmode[opcode].addmode], 0);
				break;
			case 1:
				snprintf(line2, sizeof(line2), addmode_template[instraddmode[opcode].addmode], cpu->RAM[pc+1]);
				break;
			case 2:
				snprintf(line2, sizeof(line2), addmode_template[instraddmode[opcode].addmode], cpu->RAM[pc+1] | cpu->RAM[pc+2]<<8);
				break;
			default:
				printf("Table error at %s:%d\n", __FILE__, __LINE__);
				exit(1);
		}
	}
	
	snprintf(line, max_line, "%s %s", mnemo[instraddmode[opcode].instr], line2);
	return arch_6502_instr_length(cpu->RAM, pc);
}

