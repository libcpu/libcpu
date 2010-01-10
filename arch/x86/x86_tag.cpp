/*
 * libcpu: x86_tag.cpp
 *
 * tagging code
 */

#include "libcpu.h"
#include "x86_decode.h"
#include "x86_isa.h"
#include "tag.h"

int
arch_8086_tag_instr(cpu_t *cpu, addr_t pc, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	struct x86_instr instr;
	int len;

	if (arch_8086_decode_instr(&instr, cpu->RAM, pc) != 0)
		return -1;

	len = arch_8086_instr_length(&instr);

	if (cpu->RAM[pc] == 0xCD && cpu->RAM[pc+1] == 0x20) {
		//XXX DOS-specific hack to end tagging when an "int $0x20" is encountered
		*tag = TAG_RET;
	} else {
		*tag = TAG_CONTINUE;
	}	

	*next_pc = pc + len;

	return len;
}
