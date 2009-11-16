#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "isa.h"
#include "tag_generic.h"
#include "libcpu.h"

int
arch_6502_tag_instr(cpu_t *cpu, addr_t pc, int *flow_type, addr_t *new_pc) {
	uint8_t opcode = cpu->RAM[pc];

	switch (instraddmode[opcode].instr) {
		case INSTR_BRK:
			*flow_type = FLOW_TYPE_ERR;
			break;
		case INSTR_RTS:
			*flow_type = FLOW_TYPE_RETURN;
			break;
		case INSTR_JMP:
			if (instraddmode[opcode].addmode == ADDMODE_ABS)
				*new_pc = cpu->RAM[pc+1] | cpu->RAM[pc+2]<<8;
			else 
				*new_pc = NEW_PC_NONE;	/* jmp indirect */
			*flow_type = FLOW_TYPE_BRANCH;
			break;
		case INSTR_JSR:
			*new_pc = cpu->RAM[pc+1] | cpu->RAM[pc+2]<<8;
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
			*new_pc = pc+2 + (int8_t)cpu->RAM[pc+1];
			*flow_type = FLOW_TYPE_COND_BRANCH;
			break;
		default:
			//XXX only known instrunctions should be FLOW_TYPE_CONTINUE,
			//XXX all others should be FLOW_TYPE_ERR
			*flow_type = FLOW_TYPE_CONTINUE;
			break;
	}
	return length[instraddmode[opcode].addmode] + 1;
}

