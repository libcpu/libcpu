#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "isa.h"
#include "tag_generic.h"

int
arch_6502_tag_instr(uint8_t* RAM, addr_t pc, int *flow_type, addr_t *new_pc) {
	uint8_t opcode = RAM[pc];

	switch (instraddmode[opcode].instr) {
		case INSTR_BRK:
#ifdef WARNINGS
			printf("Warning: BRK at $%04X\n", pc);
#endif
			*flow_type = FLOW_TYPE_ERR;
			break;
		case INSTR_RTS:
			*flow_type = FLOW_TYPE_RET;
			break;
		case INSTR_JMP:
			switch (instraddmode[opcode].addmode) {
				case ADDMODE_ABS:
					*new_pc = RAM[pc+1] | RAM[pc+2]<<8;
					*flow_type = FLOW_TYPE_JUMP;
					break;
				case ADDMODE_IND:
#ifdef WARNINGS
					printf("Warning: JMP ($%04X) at $%04X\n", RAM[pc+1] | RAM[pc+2]<<8, pc);
#endif
					/* TODO: handle this, if address is in ROM */
					*flow_type = FLOW_TYPE_ERR;
					break;
				default:
					printf("Table error at %s:%d\n", __FILE__, __LINE__);
					exit(1);
			}
			break;
		case INSTR_JSR:
			*new_pc = RAM[pc+1] | RAM[pc+2]<<8;
			*flow_type = FLOW_TYPE_CALL;
			break;
		case INSTR_BCC:
		case INSTR_BCS:
		case INSTR_BEQ:
		case INSTR_BMI:
		case INSTR_BNE:
		case INSTR_BPL:
		case INSTR_BVC:
		case INSTR_BVS:
			*new_pc = pc+2 + (int8_t)RAM[pc+1];
			*flow_type = FLOW_TYPE_BRANCH;
			break;
		default:
			*flow_type = FLOW_TYPE_CONTINUE;
			break;
	}
	return length[instraddmode[opcode].addmode] + 1;
}

