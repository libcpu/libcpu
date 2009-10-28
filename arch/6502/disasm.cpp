#include <stdlib.h>
#include <stdio.h>

#include "types.h"
#include "isa.h"

const char *addmode_template[] = {
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
arch_6502_disasm_instr(uint8_t* RAM, addr_t pc, char *line, unsigned int max_line) {
	uint8_t opcode = RAM[pc];
	char line2[8];

	if (instraddmode[opcode].addmode == ADDMODE_BRA) {
			snprintf(line2, sizeof(line2), "$%02llX", pc+2 + (int8_t)RAM[pc+1]);
	} else {
		switch (length[instraddmode[opcode].addmode]) {
			case 0:
				snprintf(line2, sizeof(line2), addmode_template[instraddmode[opcode].addmode], 0);
				break;
			case 1:
				snprintf(line2, sizeof(line2), addmode_template[instraddmode[opcode].addmode], RAM[pc+1]);
				break;
			case 2:
				snprintf(line2, sizeof(line2), addmode_template[instraddmode[opcode].addmode], RAM[pc+1] | RAM[pc+2]<<8);
				break;
			default:
				printf("Table error at %s:%d\n", __FILE__, __LINE__);
				exit(1);
		}
	}
	
	snprintf(line, max_line, "%s %s", mnemo[instraddmode[opcode].instr], line2);
	return arch_6502_instr_length(RAM, pc);
}

