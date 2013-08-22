/*
 * libcpu: i386_tcond.cpp
 */

#include <llvm/IR/Instructions.h>

#include "i386_arch.h"
#include "i386_disasm.h"
#include "libcpu/libcpu_llvm.h"


llvm::Value *
arch_i386_translate_cond(cpu_t *cpu, addr_t pc, llvm::BasicBlock *bb)
{
	MCInst insn;
	insn = DecodeInstruction(cpu, pc, NULL);

	return FALSE;
}
