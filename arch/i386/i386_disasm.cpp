/*
 * libcpu: i386_decoder.cpp
 */

#include <llvm/MC/MCAsmInfo.h>
#include <llvm/MC/MCDisassembler.h>
#include <llvm/MC/MCInst.h>
#include <llvm/MC/MCInstPrinter.h>
#include <llvm/MC/MCRegisterInfo.h>
#include <llvm/MC/MCSubtargetInfo.h>
#include <llvm/ADT/OwningPtr.h>
#include <llvm/ADT/Triple.h>
#include <llvm/Support/MemoryObject.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>

#include "i386_arch.h"


static const MCDisassembler *DisAsm = NULL;
static MCInstPrinter *InstPrinter = NULL;
static const MCInstrInfo *InstInfo = NULL;


/*
// A subclass of LLVM's MemoryObject that wraps a pointer as an unbounded array
// of bytes.
// 
// When we have an MCDisassembler object, we'll point it into this MemoryObject
// to decode an instruction. 
*/	 
namespace {
	class DirectMemoryObject : public MemoryObject {
	private:
		const uint8_t *Bytes;
	public:
		DirectMemoryObject(const uint8_t *bytes) : Bytes(bytes) {}

		uint64_t getBase() const { return 0; }
		uint64_t getExtent() const { return 0; }

		int readByte(uint64_t Addr, uint8_t *Byte) const {
			*Byte = Bytes[Addr];
			return 0;
		}
	};
}


/*
// Initializes our global disassembler and printer.
// Largely ripped out of llvm-mc.
*/
static void InitializeGlobals()
{
	// Initialize the X86 disassembler
	LLVMInitializeX86Disassembler();
	
	// We need to make a target triple for i386
	Triple triple(sys::getDefaultTargetTriple());
	triple.setArch(Triple::getArchTypeForLLVMName("x86"));
	
	// Look up this target in LLVM's registry
	std::string Err;
	const Target *T = TargetRegistry::lookupTarget(triple.getTriple(), Err);
	if (!T)
	{
		errs() << "Failed to get target for " << triple.getTriple() << "\n";
		abort();
	}

	OwningPtr<const MCRegisterInfo> MRI(T->createMCRegInfo(triple.getTriple()));
	if (!MRI) {
		errs() << "Failed to get register info for target\n";
		abort();
	}

	OwningPtr<const MCAsmInfo> AsmInfo(T->createMCAsmInfo(*T->createMCRegInfo(triple.getTriple()), triple.getTriple()));
	if (!AsmInfo) {
		errs() << "Failed to get target for target\n";
		abort();
	}
	
	OwningPtr<const MCSubtargetInfo> STI(T->createMCSubtargetInfo(triple.getTriple(), "", ""));
	if (!STI) {
		errs() << "Failed to get subtarget for target\n";
		abort();
	}
	
	DisAsm = T->createMCDisassembler(*STI);
	if (!DisAsm) {
		errs() << "Failed to get disassembler for target\n";
		abort();
	}
	
	InstInfo = (T->createMCInstrInfo());
	if (!InstInfo) {
		errs() << "Failed to get instruction info for target\n";
		abort();
	}
	
	
	int AsmPrinterVariant = AsmInfo->getAssemblerDialect();
	InstPrinter = T->createMCInstPrinter(AsmPrinterVariant, *AsmInfo,
		                                 *InstInfo, *MRI, *STI);
	if (!InstPrinter) {
		errs() << "Failed to get instruction printer for target\n";
		abort();
	}
}


/*
// Vends a shared MCDisassembler object.
*/
static const MCDisassembler *GetDisassembler()
{
	if (!DisAsm)
		InitializeGlobals();
	
	return DisAsm;
}


/*
// Vends a shared MCInstPrinter object.
*/
static MCInstPrinter *GetInstPrinter()
{
	if (!InstPrinter)
		InitializeGlobals();
	
	return InstPrinter;
}


/*
// Decodes one instruction from the buffer.
*/
MCInst DecodeInstruction(cpu_t *cpu, addr_t pc, uint64_t *_size)
{
	uint64_t size = 0;
	uint64_t index = pc;
	MCInst insn;
	
	// Decode the instruction
	DirectMemoryObject memoryObject(cpu->RAM);
	
	MCDisassembler::DecodeStatus S;
    S = GetDisassembler()->getInstruction(insn, size, memoryObject, index,
                                          nulls(), nulls());
	
	switch (S)
	{
		case MCDisassembler::Fail:
			errs() << "Warning: Invalid instruction encoding\n";
			if (size == 0)
				size = 1;
			break;
		
		case MCDisassembler::SoftFail:
			errs() << "Warning: Potentially undefined instruction encoding\n";
			// Fall through
		
		case MCDisassembler::Success:
			break;
	}
	
	if (_size)
		*_size = size;
	
	return insn;
}


/*
// Returns an MCInstrDesc for the given opcode.
*/
const MCInstrDesc &DescForInst(const MCInst &insn)
{
	if (!InstInfo)
		InitializeGlobals();
	
	return InstInfo->get(insn.getOpcode());
} 


/*
// Returns an MCInstInfo instance.
*/
const MCInstrInfo *GetInstInfo()
{
	if (!InstInfo)
		InitializeGlobals();
	
	return InstInfo;
} 



/*
// Returns a human-readable version of the instruction.
*/
std::string DisasmInstruction(MCInst insn)
{
	std::string output;
	raw_string_ostream sos(output);
	GetInstPrinter()->printInst(&insn, sos, "");
	return sos.str();
}


int arch_i386_disasm_instr(cpu_t *cpu, addr_t pc, char *line, unsigned int max_line)
{
	uint64_t size;
	MCInst insn = DecodeInstruction(cpu, pc, &size);
	strlcpy(line, DisasmInstruction(insn).c_str(), max_line);
	return size;
}
