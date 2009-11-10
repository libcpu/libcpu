#include "types.h"
#include "tag_generic.h"
#include "isa.h"

#include "m68k_internal.h"

int
arch_m68k_tag_instr(cpu_t *cpu, addr_t pc, int *flow_type, addr_t *new_pc) {
	uint16_t opcode = cpu->RAM[pc]<<8 | cpu->RAM[pc+1];
	int32_t disp;

	if (bits(opcode,6,15)==0x13A) {	/* JSR */
		*flow_type = FLOW_TYPE_CALL;
		*new_pc = NEW_PC_NONE;
	} else if (bits(opcode,12,15)==6) {		/* Bcc */
		*flow_type = FLOW_TYPE_BRANCH;
		disp = arch_disasm_get_disp(cpu, pc, opcode);
		*new_pc = pc+2+disp;
	} else if (bits(opcode,12,15)==5 && bits(opcode,3,5)==1 && bits(opcode,6,7)==3) {
		*flow_type = FLOW_TYPE_BRANCH;
		*new_pc = pc+2+RAM16(pc+2);
	} else if (opcode==0x4e75) {			/* RTS */
		*flow_type = FLOW_TYPE_RET;
	} else {
		*flow_type = FLOW_TYPE_CONTINUE;
	}
//printf("opcode = %x, flow_type = %d\n", opcode, *flow_type);

	return arch_m68k_instr_length(cpu, pc);
}

