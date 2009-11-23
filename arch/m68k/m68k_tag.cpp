#include "types.h"
#include "tag.h"
#include "m68k_isa.h"

#include "m68k_internal.h"

int
arch_m68k_tag_instr(cpu_t *cpu, addr_t pc, tag_t *tag, addr_t *new_pc, addr_t *next_pc) {
	uint16_t opcode = cpu->RAM[pc]<<8 | cpu->RAM[pc+1];
	int32_t disp;

	if (bits(opcode,6,15)==0x13A) {	/* JSR */
		*tag = TAG_CALL;
		*new_pc = NEW_PC_NONE;
	} else if (bits(opcode,12,15)==6) {		/* Bcc */
		*tag = TAG_COND_BRANCH;
		disp = arch_disasm_get_disp(cpu, pc, opcode);
		*new_pc = pc+2+disp;
	} else if (bits(opcode,12,15)==5 && bits(opcode,3,5)==1 && bits(opcode,6,7)==3) {
		*tag = TAG_COND_BRANCH;
		*new_pc = pc+2+RAM16(pc+2);
	} else if (opcode==0x4e75) {			/* RTS */
		*tag = TAG_RET;
	} else {
		*tag = TAG_CONTINUE;
	}
//printf("opcode = %x, tag = %d\n", opcode, *tag);

	int bytes = arch_m68k_instr_length(cpu, pc);
	*next_pc = pc + bytes;
	return bytes;
}

