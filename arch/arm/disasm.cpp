#include "libcpu.h"
#include "cpu_generic.h"
#include "arm_internal.h"

/**********************************************************************/
/* START Nemu64 Disassembler - libcpu code below!                     */
/**********************************************************************/
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

/**********************************************************************/
/* END Nemu64 Disassembler                                            */
/**********************************************************************/


int
arch_arm_disasm_instr(uint8_t* RAM, addr_t pc, char *line, unsigned int max_line) {

	int dummy1;
	addr_t dummy2;
	int bytes = arch_arm_tag_instr(RAM, pc, &dummy1, &dummy2);

	uint32_t instr = INSTR(pc);
//	TOpcode op;
//	op.all = instr;

	snprintf(line, max_line, "%s", "???");

	if (bytes == 8) {// delay slot
		instr = INSTR(pc+4);
//		op.all = instr;
//		snprintf(line+strlen(line), max_line-strlen(line), " [%s]", disassembler->Disassemble(pc+4, op, false).c_str());
	}

	return bytes;
}
