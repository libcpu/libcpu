#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "isa.h"
#include "tag.h"
#include "libcpu.h"

int
arch_6502_tag_instr(cpu_t *cpu, addr_t pc, tag_t *tag, addr_t *new_pc) {
	uint8_t opcode = cpu->RAM[pc];

	switch (instraddmode[opcode].instr) {
		case INSTR_BRK:
			*tag = TAG_TRAP;
			break;
		case INSTR_RTS:
			*tag = TAG_RET;
			break;
		case INSTR_JMP:
			if (instraddmode[opcode].addmode == ADDMODE_ABS)
				*new_pc = cpu->RAM[pc+1] | cpu->RAM[pc+2]<<8;
			else 
				*new_pc = NEW_PC_NONE;	/* jmp indirect */
			*tag = TAG_BRANCH;
			break;
		case INSTR_JSR:
			*new_pc = cpu->RAM[pc+1] | cpu->RAM[pc+2]<<8;
			*tag = TAG_CALL;
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
			*tag = TAG_COND_BRANCH;
			break;
		default:
			//XXX only known instrunctions should be TAG_CONTINUE,
			//XXX all others should be TAG_TRAP
			*tag = TAG_CONTINUE;
			break;
	}
	return length[instraddmode[opcode].addmode] + 1;
}

