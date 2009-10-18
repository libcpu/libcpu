#include "CPUInterpreter.h"
#include <string>
using std::string;

int main() {

	VM *vm = new VM(true, 512, "MARIO64_US_.V64");
	CPUEmulator *cpuEmulator = new CPUInterpreter(vm);
	for (;;) {
		cpuEmulator->Run();
	}
}

VM::VM(bool hasRamExpansion, u32 eepromSize, string romFilename) {
	_eepromPresent = true;
	_eepromSize = eepromSize;
	_ramSize = (hasRamExpansion ? 8 : 4) * MB;
	_ram = (u8*) malloc(_ramSize);
//	_rom = RomLoader::LoadRomFile(romFilename, _romSize);

	PC = 0xA4000040;
	memcpy(&_spmem[0x40], &_rom[0x40], 0x1000 - 0x40);
	Registers[0] = (s32) (0x00000000); // you didn't know this one, did ya ;)
	Registers[1] = (s32) (0x00000000);
	Registers[2] = (s32) (0xD1731BE9);
	Registers[3] = (s32) (0xD1731BE9);

	Registers[4] = (s32) (0x00001BE9);
	Registers[5] = (s32) (0xF45231E5);
	Registers[6] = (s32) (0xA4001F0C);
	Registers[7] = (s32) (0xA4001F08);

	Registers[8] = (s32) (0x000000C0);
	Registers[9] = (s32) (0x00000000);
	Registers[10] = (s32) (0x00000040);
	Registers[11] = (s32) (0xA4000040);

	Registers[12] = (s32) (0xD1330BC3);
	Registers[13] = (s32) (0xD1330BC3);
	Registers[14] = (s32) (0x25613A26);
	Registers[15] = (s32) (0x2EA04317);

	Registers[16] = (s32) (0x00000000);
	Registers[17] = (s32) (0x00000000);
	Registers[18] = (s32) (0x00000000);
	Registers[19] = (s32) (0x00000000); // bulk or cartridge ROM (0=cartridge ROM, 1=bulk, 2=64dd)

//	Registers[20] = (s32) DetectTVModeFromRom(); // TV type (1=standard NTSC, 0=standard PAL)
	Registers[21] = (s32) 0;//(0x00000000);	// reset type (1=NMI, 0=cold reset)
//	Registers[22] = (s32) DetectCicFromRom();
//	Registers[23] = (s32) DetectVersionFromRom();

	Registers[24] = (s32) (0x00000000);
	Registers[25] = (s32) (0xD73F2993);
	Registers[26] = (s32) (0x00000000);
	Registers[27] = (s32) (0x00000000);

	Registers[28] = (s32) (0x00000000);
	Registers[29] = (s32) (0xA4001FF0);
	Registers[30] = (s32) (0x00000000);
	Registers[31] = (s32) (0xA4001554);

	// COP0
	Cop0Registers.Status = 0x34000000;
	Cop0Registers.PRevID = 0x00000B00;
	Cop0Registers.Config = 0x0006E463;
	//  Cop0Registers.Context = 0x00520000;	 // After Zelda's bootcode
	//	Cop0Registers.Context = 0xF7D20000;	 // After Mario's bootcode
	Cop0Registers.Context = 0;

	// PI
	u32 PIInitValue = ReadMem32(PI_DOM1_ADDR2);
	PiReg.BSD_DOM1_LAT_REG = PIInitValue & 0xFF;
	PiReg.BSD_DOM1_PWD_REG = (PIInitValue >> 8) & 0xFF;
	PiReg.BSD_DOM1_PGS_REG = (PIInitValue >> 16) & 0xF;
	PiReg.BSD_DOM1_RLS_REG = (PIInitValue >> 20) & 0x3;

	SpReg.STATUS_REG = SP_STATUS_HALT | 0x40;

	// RDP Init
	DpcReg.STATUS_REG = DPC_STATUS_START_GCLK | DPC_STATUS_CBUF_READY;

	HandledOverflow = true;
	ViInterruptTime = 6250000;
	NextVIInterrupt = ViInterruptTime;
	CurrentLineCount = 0;

	Controllers[0].PluggedIn = true;

	// TODO: Mempack, SRAM, EEPROM
}
