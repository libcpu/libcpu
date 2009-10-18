#ifndef VM_H_
#define VM_H_

#include "Typedefs.h"
#include "stdincludes.h"
#include "N64Constants.h"
#include "Graphic/DisplayListInterpreter.h"
#include "Endian.h"

class VM {
private:
	u8* _ram;
	u8* _rom;
	u8 _pifRam[PIF_RAM_SIZE];
	u8 _pifRom[PIF_ROM_SIZE];
	// todo: check whether those can be turned into variables in the Pif-Simulator
	u8 _pifRamT[64];  // needed inside of the Pif Simulator
	u8 _pifRamR[64];  // needed inside of the Pif Simulator
	u8 _pifRamW[64];  // needed inside of the Pif Simulator
	u8 _memPackBuff[32];
	u8 _spmem[SP_MEM_SIZE << 1]; // make it twice as large as needed. I do believe the Zelda UCode overflows a bit
	u32 _romSize;
	u32 _ramSize;
	bool _eepromPresent;
	u32 _eepromSize;
	u8 _eepromData[2048];
	u8 _mempackData[MaxMempackSize];
	u32 _tlbError;
	u32 _tlbInvalidVAddress;
	u32 _tlbInvalidWrite;
	Comnd _pifCommands[256];
	bool _pifLastWasCic;
	DisplayListInterpreter* _displayListInterpreter;
	u32 InnerRead32(u32 address);
	void WriteTlb(u32 index);
	std::string _saveStateFilename;
public:
	u32 PC;
	u32 CurrentLineCount;
	u32 ViInterruptTime;
	u32 DoSomething;
	u32 Cop1Fcr31;
	s32 NextVIInterrupt;
	bool HandledOverflow; // for Compare Interrupt
	bool CompareCheck; // for Compare Interrupt
	bool Cop1UnusableInDelay;
	bool UsesEEProm;
	bool UsesMempack;
	TController Controllers[4];

	// Audio Interrupt
	bool UseAiCounter;
	s32 AiCounter;
	void CalculateAiCounter();

	// Graphic implementation
	void SetDisplayListInterpreter(DisplayListInterpreter* interpreter) {
		_displayListInterpreter = interpreter;
	}

	// Various DoSomething constants
	static const u32 Do_DP_Intr = 1;
	static const u32 Do_PI_Intr = 2;
	static const u32 Do_SP_Intr = 4;
	static const u32 Do_SI_Intr = 8;
	static const u32 Do_VI_Intr = 16;
	static const u32 Do_AI_Intr = 32;
	static const u32 Do_Compare_Intr = 64;
	static const u32 Do_COP1Unusable = 128;
	static const u32 Do_SaveState = 256;
	static const u32 Do_LoadState = 512;

	VM(bool hasRamExpansion, u32 eepromSize, std::string romFilename);
	virtual ~VM();

	u32 DetectCicFromRom();
	u32 DetectTVModeFromRom();
	u32 DetectVersionFromRom();

	// CPU Registers
	TRegisters Registers;
	TRegister* GetRegisters();
	u64 Lo;
	u64 Hi;

	// COP0 Registers
	TCOP0Registers Cop0Registers;

	// COP1 Registers
	TCOP1Registers Cop1Registers;
	TCOP1Registers64 Cop1Registers64;

	// TLB
	TTLB Tlb[32];
	u32 NextFreeTLBEntry;

	// Various in-memory special registers
	TRIReg RiReg;
	TSPReg SpReg;
	TMIReg MiReg;
	TAIReg AiReg;
	TSIReg SiReg;
	TRDRAMReg RdramReg;
	TPIReg PiReg;
	TVIReg ViReg;
	TDPCReg DpcReg;

	// Virtual Memory
	inline u32 Map(u32 address) {
		if ((address & 0xC0000000) == 0x80000000)
			return address & BitM29;

		return MapTLB(address, false);
	}
	u32 MapTLB(u32 address, bool write);

	// Rom
	u8* GetRom();
	u32 GetRomSize();

	// Pif
	void HandlePifRam();
	void DoCICEmu(u32 address);
	void InitPifCommands();
	u8 PifGetMempackCrc();

	// TLB
	void TLBR();
	void TLBP();
	void TLBWI();
	void TLBWR();

	// Ram and Memory Access
	u32 GetRamSize();
	u8* GetRam();
	u64 ReadMem64(u32 address);
	inline u32 ReadMem32(u32 address) {
		if (address < _ramSize) { // RDRAM
			return Endian::Read32(_ram, address);
		}

		return InnerRead32(address);
	}
	u16 ReadMem16(u32 address);
	u8 ReadMem8(u32 address);
	void WriteMem64(u32 address, u32 valueLo, u32 valueHi);
	void WriteMem32(u32 address, u32 value);
	void WriteMem16(u32 address, u16 value);
	void WriteMem8(u32 address, u8 value);

	// SPMEM
	inline u8* GetSPMEM() {
		return &_spmem[0];
	}

	// States
	void QueueLoadState(const std::string filename);
	void QueueSaveState(const std::string filename);
	void LoadState();
	void SaveState();
};

#endif /* VM_H_ */
