#ifndef __i386_disasm_h__
#define __i386_disasm_h__

#include <llvm/MC/MCInst.h>
#include <llvm/MC/MCInstrInfo.h>
#include <llvm/MC/MCInstrDesc.h>

MCInst DecodeInstruction(cpu_t *cpu, addr_t pc, uint64_t *_size);
std::string DisasmInstruction(MCInst insn);
const MCInstrInfo *GetInstInfo();
const MCInstrDesc &DescForInst(const MCInst &insn);

#endif  /* __i386_disasm_h__ */
