#ifndef __i386_disasm_h__
#define __i386_disasm_h__

#include <llvm/MC/MCInst.h>

MCInst DecodeInstruction(cpu_t *cpu, addr_t pc, uint64_t *_size);
std::string DisasmInstruction(MCInst insn);

#endif  /* __i386_disasm_h__ */
