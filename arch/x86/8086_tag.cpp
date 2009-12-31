#include <stdio.h>
#include <stdlib.h>
#include "libcpu.h"
#include "8086_isa.h"
#include "tag.h"

int
arch_8086_tag_instr(cpu_t *cpu, addr_t pc, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	*tag = TAG_TRAP;

	return 1;
}

