#include "VM.h"
#include "N64Constants.h"
#include "Endian.h"
#include "RomLoader.h"
#include "stdincludes.h"
//#include "Timing.h"

using namespace std;


VM::VM(bool hasRamExpansion, u32 eepromSize, string romFilename) {
	_eepromPresent = true;
	_eepromSize = eepromSize;
	_ramSize = (hasRamExpansion ? 8 : 4) * MB;
	_ram = (u8*) malloc(_ramSize);
	_rom = RomLoader::LoadRomFile(romFilename, _romSize);

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

	Registers[20] = (s32) DetectTVModeFromRom(); // TV type (1=standard NTSC, 0=standard PAL)
	Registers[21] = (s32) 0;//(0x00000000);	// reset type (1=NMI, 0=cold reset)
	Registers[22] = (s32) DetectCicFromRom();
	Registers[23] = (s32) DetectVersionFromRom();

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

VM::~VM() {
	free(_ram);
	free(_rom);
}

u32 VM::DetectTVModeFromRom() {
	// Autodetect TVMode by country code
	switch (Endian::Read8(_rom, 0x3E)) {
	case 'P': // Europe:
	case 'D': // Germany:
	case 'U': // Australia:
		return 0; // PAL
	case 'E': // US:
	case 'J': // Japan:
	case 'A': // dunno country, but ntsc *should* be correct
		return 1; // NTSC
	default: // Others:
		logf("TV Mode could not be detected. Assuming NTSC\n");
		return 1; // should assume NTSC since most demo roms with invalid header are NTSC
	}
}

u32 VM::DetectVersionFromRom() {
	if (DetectTVModeFromRom() == 0) {
		return 6;
	}

	return 1;
}

u32 VM::DetectCicFromRom() {
	// Detect CIC
	u32 pos = 0x40;
	u32 hash = 0;
	do {
		hash += ((u32) (_rom[Endian::Address8(pos)]) * pos);
		pos++;
	} while (pos < 0x1000);

	switch (hash) {
	case 0x18ED367A: // Mario, most other games
	case 0x18DECFBD:
		return 0x3F;
	case 0x15886C07: // F Zero
		return 0x85;
	case 0x19F83396: // Banjo Kazooi, Excitebike64
		return 0x78;
	case 0x2976F0DC: // Zelda
		return 0x91;
	default:
		logf(
				"Unknown bootcode (hash %x). Chances that this game works aren't high",
				hash);
		return 0x0;
	}
}

void VM::DoCICEmu(u32 address) {
	logf("DoCICEmu is stubbed\n");
}

void CIRumble(int channel) {
	// TODO: Get rid of this
	//logf("CIRumble!!\n");
}

void CIGetJoyData() {
	// TODO: Get rid of this
	//logf("CIGetJoyData!!\n");
}

void WriteMempack(int index, int size) {
	// TODO: Get rid of this
	logf("Writing Mempack to disk\n");
}

void WriteEEProm(int index, int size) {
	// TODO: Get rid of this
	//logf("Writing EEProm to disk\n");
}

void VM::HandlePifRam() {
	/*logf("Entering PifRam at PC=%X. Data before:", PC);
	 for (u32 i = 0; i < PIF_RAM_SIZE; i++) {
	 if ((i & BitM4) == 0)
	 logf("\n");
	 logf("%2X", Endian::Read8(_pifRam, i));
	 }
	 logf("\n");*/
	int CommandIndex;
	int i;
	bool ExecuteOk;
	int MemPackAddr;
	int Channel;
	bool JoyRead;
	u8 tx;
	u8 rx;
	//U32   tx;
	//int   rx;


	//if(Endian::Read8(_pifRam, 0)==0x01 && Endian::Read8(_pifRam, 1)==0x03)
	//{
	//Do_Something|=Do_Step;
	//}

	InitPifCommands();
	JoyRead = false;
	Channel = -1;
	CommandIndex = 0;
	/*
	 mario = 3f
	 yoshi,fzero = 85 (most popular besides 3f)
	 zelda = 91
	 banjo/dkr/1080 = 78
	 */
	if ((PC >= 0xA4001000) && (PC < 0xA4001FFF)) { // Hack for PIFRom
		Endian::Write8(_pifRam, 63, Endian::Read8(_pifRam, 63) | 0x80);
		Endian::Write8(_pifRam, 36, 0x0);
		Endian::Write8(_pifRam, 37, 0x04); // cold boot with version bit set
		Endian::Write8(_pifRam, 38, 0x3f);
		Endian::Write8(_pifRam, 39, 0x3f);
		return;
	}

	// this cant be a good way to check. but really shouldnt be a problem. (famous last words)
	// does this work for jfg? might cause a bug there. but since tooie and jfg are only ones
	// worth worrying about, we should be ok.
	if (_pifLastWasCic && !(Endian::Read8(_pifRam, 63) & 0x02)) //&& (Endian::Read8(_pifRam, 62)!=0x00 || Endian::Read8(_pifRam, 62)!=0xff ))
	{
		_pifLastWasCic = false;
		return;
	}

	if ((Endian::Read8(_pifRam, 63) & 0x02) && (Endian::Read8(_pifRam, 46)
			== 0x0F) && (Endian::Read8(_pifRam, 47) == 0x0F)) // 6105 cic retrieval (jfg and tooie)
	{
		DoCICEmu(46);
		Endian::Write8(_pifRam, 63, Endian::Read8(_pifRam, 63) & ~0x3);
		_pifLastWasCic = true;
		return;
	}

	while ((Endian::Read8(_pifRam, CommandIndex) != 0xFE)
			&& (CommandIndex < 64) && (Channel < 6)) // step thru each byte
	{
		while ((Endian::Read8(_pifRam, CommandIndex) == 0xFF)
				|| (Endian::Read8(_pifRam, CommandIndex) == 0x00) && (Channel
						< 6)) //ff00 could not possibly send a command
		{
			if ((Endian::Read8(_pifRam, CommandIndex) == 0x00)
					|| (Endian::Read8(_pifRam, CommandIndex) == 0xFD))
				Channel++; //00 or fd will increment channel (its a nop)
			CommandIndex++; //skip fillers
		}
		if ((Endian::Read8(_pifRam, CommandIndex) == 0xFE) || (Channel > 4))
			goto exitpif;
		Channel++;

		//if(Channel < 4)
		//if (Controllers[Channel].PluggedIn && Controllers[Channel].HasRumble && Controllers[Channel].MemPack)

		//	ShowCPUDebugMessage("ERROR: both mempack and rumblepack in at the same time? impossible!");

		tx = Endian::Read8(_pifRam, CommandIndex);
		rx = Endian::Read8(_pifRam, CommandIndex + 1) & (~0xc0); // and's off error bits if any (hack)
		CommandIndex += 2;
		if (tx > (64 - CommandIndex))
			tx = 64 - CommandIndex; //clip it  (command data length error check later on...)
		if (tx > 64)
			tx = 64;
		for (int temp = 0; temp < tx; temp++) {
			Endian::Write8(_pifRamT, temp, Endian::Read8(_pifRam, CommandIndex
					+ temp));
		}
		if (Endian::Read8(_pifRamT, 0) == 0xFF)
			Endian::Write8(_pifRamT, 0, 0x00);//goto exitpif; //dont simulate reset
		CommandIndex += tx;
		ExecuteOk = (tx > 0);
		//clip rx/tx values
		if (rx > _pifCommands[Endian::Read8(_pifRamT, 0)].rxmax) { // execute data length error
			Endian::Write8(_pifRam, CommandIndex - tx - 1, 0x40);
			rx = _pifCommands[Endian::Read8(_pifRamT, 0)].rxmax;
		}
		if (tx > _pifCommands[Endian::Read8(_pifRamT, 0)].txmax) { // execute data length error
			Endian::Write8(_pifRam, CommandIndex - tx - 1, 0x40);
			tx = _pifCommands[Endian::Read8(_pifRamT, 0)].txmax;
			ExecuteOk = false;
		}
		/*
		 if(Endian::Read8(_pifRam, 63) = 0)
		 then begin
		 if (not((Endian::Read8(_pifRamT, 0) = $00) or (Endian::Read8(_pifRamT, 0) = $01))) // infinite commands
		 then goto exitpif;
		 end;
		 */

		/* ** pif execution ** */

		if (ExecuteOk)
			switch (Endian::Read8(_pifRamT, 0)) {
			case 0x00:
				if (Channel < 4) {
					if (Controllers[Channel].PluggedIn) {
						//setup pif response
						Endian::Write8(_pifRamR, 0, 0x05);
						Endian::Write8(_pifRamR, 1, 0x00);
						if (Controllers[Channel].MemPack
								|| Controllers[Channel].HasRumble) {
							Endian::Write8(_pifRamR, 2, 0x01);
						} else {
							Endian::Write8(_pifRamR, 2, 0x02);
						}
						for (int temp = 0; temp < rx; temp++) {
							Endian::Write8(_pifRam, CommandIndex + temp,
									Endian::Read8(_pifRamR, temp));
						}
					} else {
						Endian::Write8(_pifRam, CommandIndex - 2,
								Endian::Read8(_pifRam, CommandIndex - 2) | 0x80);
					}
				} else if (Channel < 6) //eeprom probe (right now roms can support two eeproms in channel 4 and 5, altho the latter has yet to happen to my knowledge)
				{
					if (_eepromPresent) // this should not be needed anymore (but leave it in for future)
					{
						//setup pif response
						Endian::Write8(_pifRamR, 0, 0x00);
						if (_eepromSize == 2048)
							Endian::Write8(_pifRamR, 1, 0xC0);
						else
							Endian::Write8(_pifRamR, 1, 0x80);
						Endian::Write8(_pifRamR, 2, 0x00);
						for (int temp = 0; temp < rx; temp++) {
							Endian::Write8(_pifRam, CommandIndex + temp,
									Endian::Read8(_pifRamR, temp));
						}
					} else {
						Endian::Write8(_pifRam, CommandIndex - 2,
								Endian::Read8(_pifRam, CommandIndex - 2) | 0x80);
					}
				} else if (rx > 0)
					Endian::Write8(_pifRam, CommandIndex - 2, Endian::Read8(
							_pifRam, CommandIndex - 2) | 0x80);
				break;
			case 0x01: // Read button values (t1 r4)
				if (Channel >= 4)
					Endian::Write8(_pifRam, CommandIndex - tx - 1,
							Endian::Read8(_pifRam, CommandIndex - tx - 1)
									| 0x40);
				else if (Controllers[Channel].PluggedIn) {
					if (!JoyRead) {

						CIGetJoyData();
						JoyRead = true;
					}
					Endian::Write8(_pifRamR, 0,
							(u8) (Controllers[Channel].KeyState >> 24));
					Endian::Write8(_pifRamR, 1,
							(u8) (Controllers[Channel].KeyState >> 16));
					Endian::Write8(_pifRamR, 2,
							(u8) (Controllers[Channel].KeyState >> 8));
					Endian::Write8(_pifRamR, 3,
							(u8) (Controllers[Channel].KeyState));

					for (int temp = 0; temp < rx; temp++) {
						Endian::Write8(_pifRam, CommandIndex + temp,
								Endian::Read8(_pifRamR, temp));
					}
				} else if (rx > 0)
					Endian::Write8(_pifRam, CommandIndex - 2, Endian::Read8(
							_pifRam, CommandIndex - 2) | 0x80);
				break;
			case 0x02: // Controller RAM Read (mempack slot) (t3 r33)
				//				ShowCPUDebugMessage("PIFRAM command read ram.");
				if (Channel >= 4)
					Endian::Write8(_pifRam, CommandIndex - tx - 1,
							Endian::Read8(_pifRam, CommandIndex - tx - 1)
									| 0x40);
				else if (Controllers[Channel].PluggedIn) {
					MemPackAddr = (Endian::Read8(_pifRamT, 1) << 8)
							| (Endian::Read8(_pifRamT, 2));
					//					MemPackAddrCRC = (MemPackAddr & 0x1f);
					MemPackAddr = (MemPackAddr >> 5) & 0xffff; //this is bad programming
					if (MemPackAddr >= 1024) {
						for (i = 0; i < 32; i++) {
							Endian::Write8(_memPackBuff, i,
									Controllers[Channel].HasRumble ? 0x80
											: 0x00);
						}
					} else {
						if (Controllers[Channel].HasRumble) // rumble-pak
						{
							for (i = 0; i < 32; i++)
								Endian::Write8(_memPackBuff, i, 0x80);
						} else {
							for (int temp = 0; temp < 32; temp++)
								Endian::Write8(_memPackBuff, temp,
										Endian::Read8(_mempackData,
												(MemPackAddr * 32) + temp));
							UsesMempack = true;
						}
					}
					for (int temp = 0; temp < 32; temp++)
						Endian::Write8(_pifRam, CommandIndex + temp,
								Endian::Read8(_memPackBuff, temp));
					if (Controllers[Channel].MemPack
							|| Controllers[Channel].HasRumble)
						Endian::Write8(_pifRam, CommandIndex + rx - 1,
								PifGetMempackCrc());
					else
						Endian::Write8(_pifRam, CommandIndex + rx - 1,
								~(PifGetMempackCrc())); // crc is not'd when there is nothing to write to
				} else
					//not PluggedIn
					Endian::Write8(_pifRam, CommandIndex - tx - 1,
							Endian::Read8(_pifRam, CommandIndex - tx - 1)
									| 0x80);
				break;
			case 0x03: // Controller RAM Write (t35 r1)
				//						  ShowCPUDebugMessage("PIFRAM command write ram.");
				if (Channel >= 4)
					Endian::Write8(_pifRam, CommandIndex - tx - 1,
							Endian::Read8(_pifRam, CommandIndex - tx - 1)
									| 0x40);
				else if (Controllers[Channel].PluggedIn) {
					MemPackAddr = (Endian::Read8(_pifRamT, 1) << 8)
							| (Endian::Read8(_pifRamT, 2));
					//					MemPackAddrCRC = (MemPackAddr & 0x1f);
					MemPackAddr = (MemPackAddr >> 5) & 0xffff;

					if ((tx - 3) > 32)
						tx = 35;
					if ((((int) tx) - 3) < 0)
						logf("PIFRAM command write ram. tx too small\n");

					for (int temp = 0; temp < tx - 3; temp++)
						Endian::Write8(_memPackBuff, temp, Endian::Read8(
								_pifRamT, 3 + temp));
					if (MemPackAddr >= 1024) {
						// rumble-pak//(PIFRAM[CommandIndex+4]=$c0) and (PIFRAM[CommandIndex+5]=$1b))//rumble pack
						if ((MemPackAddr == 0x600)
								&& Controllers[Channel].HasRumble) {
							if ((Endian::Read8(_pifRamT, 5) == 0x01)
									&& (Endian::Read8(_pifRamT, 6) == 0x01)) //quick check/hack
							{
								Controllers[Channel].Rumble = true; // there is only one intensity of rumble,
								CIRumble(Channel); // actual rumble effects are simulated by starting up the motor and stopping it,
							} // turning it off and on quickly will result in low intensity rumbles,
							else {
								Controllers[Channel].Rumble = false; // turning it off and on slowly will result in high intensity rumbles.
								CIRumble(Channel);
							}
						} else {
							//ShowCPUError("PIFRAM command write mempack out of address range.");
						}

					}
					if (Controllers[Channel].MemPack
							|| Controllers[Channel].HasRumble) {
						if ((MemPackAddr < 1024)
								&& !(Controllers[Channel].HasRumble)) {
							if ((MemPackAddr * 32) + tx - 3 >= MaxMempackSize)
								logf(
										"PIFRAM command write ram. MemPackAddr too big\n");
							memcpy(&_mempackData[(MemPackAddr * 32)],
									_memPackBuff, tx - 3);
							WriteMempack((MemPackAddr * 32), tx - 3);
							UsesMempack = true;
						}
						Endian::Write8(_pifRam, CommandIndex + rx - 1,
								PifGetMempackCrc());
					} else {
						Endian::Write8(_pifRam, CommandIndex + rx - 1,
								~(PifGetMempackCrc())); // crc is not'd when there is nothing to write to
					}
				} else
					//not PluggedIn
					Endian::Write8(_pifRam, CommandIndex - tx - 1,
							Endian::Read8(_pifRam, CommandIndex - tx - 1)
									| 0x80);
				break;
			case 0x04: // read eeprom (t2 r8)
				if ((Channel == 4) || (Channel == 5)) //check if doing it in the right channel
				{
					UsesEEProm = true;
					int dataIndex = Endian::Read8(_pifRamT, 1) << 3;
					for (int temp = 0; temp < rx; temp++)
						Endian::Write8(_pifRam, CommandIndex + temp,
								Endian::Read8(_eepromData, dataIndex + temp));
				} else
					logf("Pif command read eeprom (unsupported channel)\n");
				break;
			case 0x05:// write eeprom (t10 r1)
				//                       ShowCPUError("PIFRAM command write eeprom.");
				if ((Channel == 4) || (Channel == 5)) //check if doing it in the right channel
				{
					UsesEEProm = true;

					int dataIndex = Endian::Read8(_pifRamT, 1) << 3;
					for (int temp = 0; temp < tx - 2; temp++)
						Endian::Write8(_eepromData, dataIndex + temp,
								Endian::Read8(_pifRamT, 2 + temp));
					WriteEEProm(dataIndex, tx - 2);

					//sets byte to tell eeprom was written ok.
					Endian::Write8(_pifRam, CommandIndex, 0x00); //could be dangerous... what if this is FE? probably wouldnt matter in emulation.
				} else
					logf("Pif command write eeprom (unsupported channel)\n");

				break;
			}
		CommandIndex += rx;
	}
	exitpif:

	bool ResetSwitchPressed = false;
	if ((ResetSwitchPressed) || (PC == 0x1FC0056C))//|| ((PC&0xFFF00000) == 0x1FC00000)) )
		Endian::Write8(_pifRam, 63, 0x80); // PifROM Hack  and reset switch detection.
	else
		Endian::Write8(_pifRam, 63, 0x00); // Clear Pif Status Control Byte


	/*logf("Leaving PifRam. Data after:");
	 for (u32 i = 0; i < PIF_RAM_SIZE; i++) {
	 if ((i & BitM4) == 0)
	 logf("\n");
	 logf("%2X", Endian::Read8(_pifRam, i));
	 }
	 logf("\n");*/
}

u8 VM::PifGetMempackCrc() {
	u8 temp = 0;
	for (int i = 0; i < 33; i++) {
		for (int j = 7; j > -1; j--) {
			u32 temp2 = (u8) ((temp & 0x80) ? 0x85 : 0);
			temp <<= 1;
			if (i == 32)
				temp |= 0;
			else
				temp
						|= ((Endian::Read8(_memPackBuff, i) & (0x01 << j)) ? 1
								: 0);
			temp ^= temp2;
		}
	}
	return temp;
}

void VM::InitPifCommands() {
	_pifCommands[0].txmax = 1;
	_pifCommands[0].rxmax = 3;

	_pifCommands[1].txmax = 1;
	_pifCommands[1].rxmax = 4;

	_pifCommands[2].txmax = 3;
	_pifCommands[2].rxmax = 33;

	_pifCommands[3].txmax = 35;
	_pifCommands[3].rxmax = 1;

	_pifCommands[4].txmax = 2;
	_pifCommands[4].rxmax = 8;

	_pifCommands[5].txmax = 10;
	_pifCommands[5].rxmax = 1;

	_pifCommands[255].txmax = 1;
	_pifCommands[255].rxmax = 3;
}

void VM::TLBR() {
	Cop0Registers.PageMask = Tlb[Cop0Registers.Index & BitM6].PageMask;
	Cop0Registers.EntryHi = (Tlb[Cop0Registers.Index & BitM6].hl
			& ~Tlb[Cop0Registers.Index].PageMask) & ~Bit13;
	Cop0Registers.EntryLo1 = Tlb[Cop0Registers.Index & BitM6].lh;
	Cop0Registers.EntryLo0 = Tlb[Cop0Registers.Index & BitM6].ll;
}

void VM::TLBP() {
	// Probe TLB for matching entry
	u32 index = 0;
	do {
		// possible bug: this is code copied from my actual tlb mapper but the code above should also work
		// in perfect dark the code above returns 0x80000000 when called the second time, the code below finds an entry

		if (Tlb[index].Global) { // or asid!!
			u32 addressVPN =
					(Cop0Registers.EntryHi & Tlb[index].AddressVPNMask)
							>> Tlb[index].AddressVPNShift;
			if (Tlb[index].VPN == (addressVPN >> 1)) {
				Cop0Registers.Index = index;
				return;
			}
		}
		index++;
	} while (index < 32);
	Cop0Registers.Index = Bit32;
}

void VM::TLBWR() {
	WriteTlb(NextFreeTLBEntry);
	NextFreeTLBEntry++;
	if (NextFreeTLBEntry >= 30) {
		NextFreeTLBEntry = Cop0Registers.Wired;
	}
}

void VM::TLBWI() {
	WriteTlb(Cop0Registers.Index);
}

void VM::WriteTlb(u32 index) {
	if (index > 31)
		logf("TLB Error 0 in WriteTlb: index is too big\n");
	Tlb[index].PageMask = Cop0Registers.PageMask;
	Tlb[index].hl = Cop0Registers.EntryHi & ~Cop0Registers.PageMask;
	Tlb[index].lh = Cop0Registers.EntryLo1 & 0xFFFFFFFE;
	Tlb[index].ll = Cop0Registers.EntryLo0 & 0xFFFFFFFE;
	Tlb[index].Asid = Cop0Registers.EntryHi & BitM8;
	if (Tlb[index].Asid)
		logf("Missing feature: ASID used by ROM\n");
	Tlb[index].Global = (Cop0Registers.EntryLo0 & Cop0Registers.EntryLo1
			& TLBLO_G) != 0;
	Tlb[index].Valid0 = (Cop0Registers.EntryLo0 & TLBLO_V) != 0;
	Tlb[index].Valid1 = (Cop0Registers.EntryLo1 & TLBLO_V) != 0;
	Tlb[index].Dirty0 = (Cop0Registers.EntryLo0 & TLBLO_D) != 0;
	Tlb[index].Dirty1 = (Cop0Registers.EntryLo1 & TLBLO_D) != 0;
	Tlb[index].PFN0 = Cop0Registers.EntryLo0 >> 6 & BitM24;
	Tlb[index].PFN1 = Cop0Registers.EntryLo1 >> 6 & BitM24;
	switch (Tlb[index].PageMask) {
	case TLBPGMASK_4K:
		Tlb[index].AddressVPNShift = 12;
		Tlb[index].AddressVPNMask = BitM20 << 12;
		Tlb[index].OffsetMask = BitM12;
		Tlb[index].PFNShift = 12;
		Tlb[index].VPN = (Cop0Registers.EntryHi >> 13) & BitM19;
		break;
	case TLBPGMASK_16K:
		Tlb[index].AddressVPNShift = 14;
		Tlb[index].AddressVPNMask = BitM18 << 14;
		Tlb[index].OffsetMask = BitM14;
		Tlb[index].PFNShift = 12;
		Tlb[index].VPN = (Cop0Registers.EntryHi >> 15) & BitM17;
		break;
	case TLBPGMASK_64K:
		Tlb[index].AddressVPNShift = 16;
		Tlb[index].AddressVPNMask = BitM16 << 16;
		Tlb[index].OffsetMask = BitM16;
		Tlb[index].PFNShift = 12;
		Tlb[index].VPN = (Cop0Registers.EntryHi >> 17) & BitM15;
		break;
	case TLBPGMASK_256K:
		Tlb[index].AddressVPNShift = 18;
		Tlb[index].AddressVPNMask = BitM14 << 18;
		Tlb[index].OffsetMask = BitM18;
		Tlb[index].PFNShift = 12;
		Tlb[index].VPN = (Cop0Registers.EntryHi >> 19) & BitM13;
		break;
	case TLBPGMASK_1M:
		Tlb[index].AddressVPNShift = 20;
		Tlb[index].AddressVPNMask = BitM12 << 20;
		Tlb[index].OffsetMask = BitM20;
		Tlb[index].PFNShift = 12;
		Tlb[index].VPN = (Cop0Registers.EntryHi >> 21) & BitM11;
		break;
	case TLBPGMASK_4M:
		Tlb[index].AddressVPNShift = 22;
		Tlb[index].AddressVPNMask = BitM10 << 22;
		Tlb[index].OffsetMask = BitM22;
		Tlb[index].PFNShift = 12;
		Tlb[index].VPN = (Cop0Registers.EntryHi >> 23) & BitM9;
		break;
	}
	/*
	 if (((Cop0Registers.EntryHi!=0xC0000000) || (index!=31)) && (Tlb[index].Valid0||Tlb[index].Valid1)) {
	 // TODO: Reset Dynarec here
	 if (!EnableTLB){
	 EnableTLB = true;
	 OptimizeCode = false;
	 DoSomething |= Do_ResetDynrec;
	 }
	 */
	// TODO: Detect we can use a quicker test for non-tlb'ed addresses
	/*	if (EnableTLB && !SlowTLB && (Tlb[index].Valid0 || Tlb[index].Valid1)){
	 if (Tlb[index].hl < RDRAMSize) {
	 SlowTLB = true;
	 OptimizeCode = InitOptimizeCode;
	 Do_Something |= Do_ResetDynrec;
	 }
	 }
	 */
}

TRegister* VM::GetRegisters() {
	return Registers;
}

u8* VM::GetRom() {
	return _rom;
}

u32 VM::GetRomSize() {
	return _romSize;
}

u32 VM::GetRamSize() {
	return _ramSize;
}

u8* VM::GetRam() {
	return _ram;
}

u32 VM::MapTLB(u32 address, bool write) {
	u32 index = 0;
	_tlbError = 0;

	do {
		if (Tlb[index].Global) { // or asid!!
			u32 AddressVPN = (address & Tlb[index].AddressVPNMask)
					>> Tlb[index].AddressVPNShift;
			if (Tlb[index].VPN == (AddressVPN >> 1)) {
				// Now check which Tlb Entry (odd/even) matches
				if (!(AddressVPN & BitM1)) {
					if (!(Tlb[index].Valid0)) {
						_tlbError = 2;
						break;
					}
					if (write && !Tlb[index].Dirty0) {
						_tlbError = 4;
						break;
					}
					return (address & Tlb[index].OffsetMask | (Tlb[index].PFN0
							<< Tlb[index].PFNShift));
				} else {
					if (!(Tlb[index].Valid1)) {
						_tlbError = 3;
						break;
					}
					if (write && !Tlb[index].Dirty1) {
						_tlbError = 5;
						break;
					}
					return (address & Tlb[index].OffsetMask | (Tlb[index].PFN1
							<< Tlb[index].PFNShift));
				}
			}
		}
		index++;
	} while (index < 32);
	if (_tlbError == 0)
		_tlbError = 1;
	_tlbInvalidVAddress = address;
	_tlbInvalidWrite = write;
	logf("MapTLB didn't finish properly.");
	return 0;
}

u64 VM::ReadMem64(u32 address) {
	return ReadMem32(address + 4) | ((u64) (ReadMem32(address)) << 32);
}

u32 VM::InnerRead32(u32 address) {
	if ((address >= SP_DMEM_START) && (address <= SP_IMEM_END)) { // SPMEM
		return Endian::Read32(_spmem, address - SP_DMEM_START);
	}

	if ((address >= PIF_RAM_START) && (address <= PIF_RAM_END)) { // PIFRAM
		return Endian::Read32(_pifRam, address - PIF_RAM_START);
	}

	if ((address >= PIF_ROM_START) && (address <= PIF_ROM_END)) { // PIFROM
		return Endian::Read32(_pifRom, address - PIF_ROM_START);
	}

	if ((address >= PI_DOM1_ADDR2) && (address <= 0x1FBFFFFF)) { // ROM area
		if (address - PI_DOM1_ADDR2 < _romSize) {
			return Endian::Read32(_rom, address - PI_DOM1_ADDR2);
		}
		/*
		 // (goldeneye) conkers bfd hack
		 u32 address2;
		 address2 = address-PI_DOM1_ADDR2-64*MB+0x34b30;  // goldeneye
		 //address2 = address-PI_DOM1_ADDR2-64*MB+0x34b30;  // conkers bfd
		 if (address2 < romsize){
		 return _Read32Endian(ROM, address2);
		 }
		 */
	}

	if ((address >= RI_BASE_REG) && (address <= RI_WERROR_REG)) {
		switch (address) {
		case RI_MODE_REG:
			return RiReg.MODE_REG;
		case RI_CONFIG_REG:
			return RiReg.CONFIG_REG;
		case RI_SELECT_REG:
			return RiReg.SELECT_REG;
		case RI_REFRESH_REG:
			return RiReg.REFRESH_REG;
		case RI_LATENCY_REG:
			return RiReg.LATENCY_REG;
		case RI_RERROR_REG:
			return RiReg.RERROR_REG;
		}
	}

	if ((address >= MI_BASE_REG) && (address <= MI_INTR_MASK_REG)) {
		switch (address) {
		case MI_MODE_REG:
			return MiReg.MODE_REG;
		case MI_VERSION_REG:
			return 0x02020202;
		case MI_INTR_REG:
			return MiReg.INTR_REG;
		case MI_INTR_MASK_REG:
			return MiReg.INTR_MASK_REG;
		}
	}

	if ((address >= AI_BASE_REG) && (address <= AI_BITRATE_REG)) {
		s32 value = 0;//Value = AIGetReg(address);
		switch (address) {
		case AI_LEN_REG: {
			u32 ClockRate;
			s32 value;
			if (UseAiCounter) {
				if (DetectTVModeFromRom() != 0) {
					ClockRate = 0x2E6D354;
				} else {
					ClockRate = 0x2F5B2D2;
				}
				s64 tempValue;
				tempValue = (s64) (AiCounter) * ClockRate / 46875000;
				tempValue = (tempValue / (AiReg.DACRATE_REG + 1)) << 2;
				value = (s32) tempValue;
				if (value < 0) {
					value = 0;
				}
			} else {
				value = 0;
			}
			break;
		}
		case AI_STATUS_REG:
			/*if (CPUAILenRegBuffer) {
			 value |= AI_STATUS_FIFO_FULL;
			 } else {
			 value &= ~AI_STATUS_FIFO_FULL;
			 }*/

			value = 0;//AIGetReg(address);
			break;
		case AI_DRAM_ADDR_REG:
			value = AiReg.DRAM_ADDR_REG;
			break;
		default:
			logf("Getting AIReg: %X\n", address);
			value = 0;//AiGetReg(address);
			break;

		}
		return value;
	}

	if ((address >= PI_BASE_REG) && (address <= PI_BSD_DOM2_RLS_REG)) {
		switch (address) {
		case PI_DRAM_ADDR_REG:
			return PiReg.DRAM_ADDR_REG;
		case PI_CART_ADDR_REG:
			return PiReg.CART_ADDR_REG;
		case PI_RD_LEN_REG:
			return PiReg.RD_LEN_REG;
		case PI_WR_LEN_REG:
			return PiReg.WR_LEN_REG;
		case PI_STATUS_REG:
			return PiReg.STATUS_REG;
		case PI_BSD_DOM1_LAT_REG:
			return PiReg.BSD_DOM1_LAT_REG;
		case PI_BSD_DOM1_PWD_REG:
			return PiReg.BSD_DOM1_PWD_REG;
		case PI_BSD_DOM1_PGS_REG:
			return PiReg.BSD_DOM1_PGS_REG;
		case PI_BSD_DOM1_RLS_REG:
			return PiReg.BSD_DOM1_RLS_REG;
		case PI_BSD_DOM2_LAT_REG:
			return PiReg.BSD_DOM2_LAT_REG;
		case PI_BSD_DOM2_PWD_REG:
			return PiReg.BSD_DOM2_PWD_REG;
		case PI_BSD_DOM2_PGS_REG:
			return PiReg.BSD_DOM2_PGS_REG;
		case PI_BSD_DOM2_RLS_REG:
			return PiReg.BSD_DOM2_RLS_REG;
		}
	}

	if (address == SP_PC_REG) {
		return SpReg.PC_REG;
	}

	if (address == SP_IBIST_REG) {
		return SpReg.IBIST_REG;
	}

	if ((address >= RDRAM_BASE_REG) && (address <= 0x03FFFFFF)) {
		switch (RDRAM_BASE_REG | (address & BitM8)) {
		case RDRAM_CONFIG_REG:
			switch (address >> 10) {
			case 0:
			case 1:
				return 2 * MB;
			case 2:
			case 3:
				if (_ramSize == 8 * MB) {
					return 2 * MB;
				} else {
					return 0;
				}
			}
		default:
			return 0;
		case RDRAM_DEVICE_ID_REG:
			return RdramReg.DEVICE_ID_REG;
		case RDRAM_DELAY_REG:
			return RdramReg.DELAY_REG;
		case RDRAM_MODE_REG:
			return RdramReg.MODE_REG;
		case RDRAM_REF_INTERVAL_REG:
			return RdramReg.REF_INTERVAL_REG;
		case RDRAM_REF_ROW_REG:
			return RdramReg.REF_ROW_REG;
		case RDRAM_RAS_INTERVAL_REG:
			return RdramReg.RAS_INTERVAL_REG;
		case RDRAM_MIN_INTERVAL_REG:
			return RdramReg.MIN_INTERVAL_REG;
		case RDRAM_ADDR_SELECT_REG:
			return RdramReg.ADDR_SELECT_REG;
		case RDRAM_DEVICE_MANUF_REG:
			return RdramReg.DEVICE_MANUF_REG;
		}
	}

	if ((address >= SI_BASE_REG) && (address <= SI_STATUS_REG)) {
		switch (address) {
		case SI_DRAM_ADDR_REG:
			return SiReg.DRAM_ADDR_REG;
		case SI_STATUS_REG:
			return ((MiReg.INTR_REG & MI_INTR_SI) << 11);
		}
	}

	if ((address >= SP_BASE_REG) && (address <= SP_SEMAPHORE_REG)) {
		switch (address) {
		case SP_MEM_ADDR_REG:
			return SpReg.MEM_ADDR_REG;
		case SP_DRAM_ADDR_REG:
			return SpReg.DRAM_ADDR_REG;
		case SP_RD_LEN_REG:
			return SpReg.RD_LEN_REG;
		case SP_WR_LEN_REG:
			return SpReg.WR_LEN_REG;
		case SP_STATUS_REG:
			return SpReg.STATUS_REG;
		case SP_DMA_FULL_REG:
			return (u32) ((SpReg.STATUS_REG & SP_STATUS_DMA_FULL) != 0);
		case SP_DMA_BUSY_REG:
			return (u32) ((SpReg.STATUS_REG & SP_STATUS_DMA_BUSY) != 0);
		case SP_SEMAPHORE_REG:
			u32 returnValue;
			returnValue = SpReg.SEMAPHORE_REG;
			SpReg.SEMAPHORE_REG = 1;
			return returnValue;
		}
	}

	if ((address >= VI_BASE_REG) && (address <= VI_Y_SCALE_REG)) {
		switch (address) {
		case VI_STATUS_REG:
			return ViReg.STATUS_REG;
		case VI_ORIGIN_REG:
			return ViReg.DRAM_ADDR_REG;
		case VI_WIDTH_REG:
			return ViReg.H_WIDTH_REG;
		case VI_INTR_REG:
			return ViReg.V_INTR_REG;
		case VI_CURRENT_REG:
			CurrentLineCount += 2;
			/*
			 if (CurrentLineCount>=512){
			 CurrentLineCount = 0;
			 }
			 return CurrentLineCount;
			 */
			return (CurrentLineCount >> 4) & BitM10;
			//			CurrentLineCount+=233;
			//			return CurrentLineCount&BitM10;
			/*
			 if(CurrentLineCount<1023){
			 CurrentLineCount+=2;
			 } else {
			 CurrentLineCount=0;
			 }
			 return CurrentLineCount;
			 */
		case VI_BURST_REG:
			return ViReg.TIMING_REG;
		case VI_V_SYNC_REG:
			return ViReg.V_SYNC_REG;
		case VI_H_SYNC_REG:
			return ViReg.H_SYNC_REG;
		case VI_LEAP_REG:
			return ViReg.H_SYNC_LEAP_REG;
		case VI_H_START_REG:
			return ViReg.H_VIDEO_REG;
		case VI_V_START_REG:
			return ViReg.V_VIDEO_REG;
		case VI_V_BURST_REG:
			return ViReg.V_BURST_REG;
		case VI_X_SCALE_REG:
			return ViReg.X_SCALE_REG;
		case VI_Y_SCALE_REG:
			return ViReg.Y_SCALE_REG;
		}
	}

	if ((address >= DPC_BASE_REG) && (address <= DPC_TMEM_REG)) {
		switch (address) {
		case DPC_START_REG:
			return DpcReg.START_REG;
		case DPC_END_REG:
			return DpcReg.END_REG;
		case DPC_CURRENT_REG:
			return DpcReg.CURRENT_REG;
		case DPC_STATUS_REG:
			return DpcReg.STATUS_REG;
		case DPC_CLOCK_REG:
			return DpcReg.CLOCK_REG;
		case DPC_BUFBUSY_REG:
			return DpcReg.BUFBUSY_REG;
		case DPC_PIPEBUSY_REG:
			return DpcReg.PIPEBUSY_REG;
		case DPC_TMEM_REG:
			return DpcReg.TMEM_REG;
		}
	}

	if ((address >= PI_DOM2_ADDR2) && (address <= PI_DOM2_ADDR2 + 0x3FFFF)) // Sram / Flashram
	{
		logf("Ignoring Flash....\n");
		//		if(!tempf)
		//			tempf=fopen("flash.log","wb");
		//		flogf(tempf,"Flash read %s \n",(address==0x08000000)?"status":"control");
		//		fflush(tempf);
		return 0;

		/*if(flash_control!=0xf0000000) // and (flash_on) then
		 return flash_status;
		 else
		 return _Read32(SRAM, 0);*/
	}

	/*
	 if((address==0x06001010)){
	 return 0;//0x2129FFF8;
	 }

	 if((address==0x06000000)){
	 return 0;//0xDEADBEAF;
	 }
	 */

	/*if((address>=0x06000000) && (address<0x06400000))
	 {
	 extern U8 *ddROM;
	 u32 addr=address-0x06000000;
	 if(!ddROM)
	 Load64DDImage(psz64DDBootImage);
	 return *(u32 *)&ddROM[addr];
	 }*/
	if (address == 0x05000508) {
		// 64dd detection, 0x05080508=none, 0=plugged in
		return 0x05080508;
	}

	if (address < (_ramSize + 4 * MB)) { // RDRAM extension
		// RDRAM size detection
		return 0;
	}
	if (address == 0x1FF00000) {
		logf("Trying to Read32 from some debug device 0x1FF00000\n");
		return 0;
	}
	// Unmapped memory
	logf("Trying to Read32 from unhandled %x at %x\n", address, PC);
	return 0;
}

u16 VM::ReadMem16(u32 address) {
	if (address < _ramSize) { // RDRAM
		return Endian::Read16(_ram, address);
	}

	if ((address >= PI_DOM1_ADDR2) && (address <= 0x1FBFFFFF)) { // ROM area
		address = address - PI_DOM1_ADDR2;
		if (address <= _romSize) {
			if (address & Bit2) {
				return Endian::Read16(_rom, address + 2);
			} else {
				return Endian::Read16(_rom, address);
			}
		} else {
			return 0;
		}
	}

	if ((address >= PIF_RAM_START) && (address <= PIF_RAM_END)) { // PIFRAM (Vector demo does 16 bit read)
		return Endian::Read16(_pifRam, address - PIF_RAM_START);
	}

	if ((address >= PIF_ROM_START) && (address <= PIF_ROM_END)) { // PIFROM
		return Endian::Read16(_pifRom, address - PIF_ROM_START);
	}

	if ((address >= SP_DMEM_START) && (address <= SP_IMEM_END)) { // SPMEM
		return Endian::Read16(_spmem, address - SP_DMEM_START);
	}

	// Unmapped memory
	logf("Trying to Read16 from unhandled or invalid memory: %x\n", address);
	return 0;
}

u8 VM::ReadMem8(u32 address) {
	if (address < _ramSize) { // RDRAM
		return Endian::Read8(_ram, address);
	}

	if ((address >= SP_DMEM_START) && (address <= SP_IMEM_END)) { // SPMEM
		return Endian::Read8(_spmem, address - SP_DMEM_START);
	}

	if ((address >= PIF_RAM_START) && (address <= PIF_RAM_END)) { // PIFRAM
		return Endian::Read8(_pifRam, address - PIF_RAM_START);
	}

	if ((address >= PIF_ROM_START) && (address <= PIF_ROM_END)) { // PIFROM
		return Endian::Read8(_pifRom, address - PIF_ROM_START);
	}

	// reading bytes from rom is very strange.
	// u can read from the first two bytes of a 4 byte aligned address.
	// if u try to read from the 2nd two bytes of a 4 byte aligned address then u
	// get the first two bytes of the next 4 byte aligned address.

	if ((address >= PI_DOM1_ADDR2) && (address <= 0x1FBFFFFF)) { // ROM area
		if (address <= _romSize + PI_DOM1_ADDR2 - 2) {
			address -= PI_DOM1_ADDR2;
			if ((address & BitM2) > 1) {
				return Endian::Read8(_rom, address + 2);
			} else {
				return Endian::Read8(_rom, address);
			}
		} else {
			return 0;
		}
	}

	// Unmapped memory
	logf("Trying to Read8 from unhandled or invalid memory: %x\n", address);
	return 0;
}

void VM::WriteMem64(u32 address, u32 valueLo, u32 valueHi) {
	WriteMem32(address, valueHi);
	WriteMem32(address + 4, valueLo);
}

void VM::WriteMem32(u32 address, u32 value) {
	if (address < _ramSize) { // RDRAM
		Endian::Write32(_ram, address, value);
		return;
	}

	if ((address >= SP_DMEM_START) && (address <= SP_IMEM_END)) { // SPMEM
		Endian::Write32(_spmem, address - SP_DMEM_START, value);
		return;
	}

	if ((address >= PIF_RAM_START) && (address <= PIF_RAM_END)) {
		Endian::Write32(_pifRam, address - PIF_RAM_START, value);
		logf("Handling controllers...\n");
		HandlePifRam();
		return;
	}

	if ((address >= RI_BASE_REG) && (address <= RI_WERROR_REG)) {
		switch (address) {
		case RI_MODE_REG:
			RiReg.MODE_REG = value;
			return;
		case RI_CONFIG_REG:
			RiReg.CONFIG_REG = value;
			return;
		case RI_CURRENT_LOAD_REG:
			RiReg.CURRENT_LOAD_REG = value;
			return;
		case RI_SELECT_REG:
			RiReg.SELECT_REG = value;
			return;
		case RI_REFRESH_REG:
			RiReg.REFRESH_REG = value;
			return;
		case RI_LATENCY_REG:
			RiReg.LATENCY_REG = value;
			return;
		case RI_WERROR_REG:
			RiReg.WERROR_REG = value;
			return;
		}
	}

	if ((address >= SP_BASE_REG) && (address <= SP_SEMAPHORE_REG)) {
		switch (address) {
		case SP_MEM_ADDR_REG:
			SpReg.MEM_ADDR_REG = value;
			return;
		case SP_DRAM_ADDR_REG:
			SpReg.DRAM_ADDR_REG = value;
			return;
		case SP_RD_LEN_REG: {
			SpReg.RD_LEN_REG = value;
			u32 pos = SpReg.DRAM_ADDR_REG & BitM24;
			u32 length = (((SpReg.RD_LEN_REG & BitM12) | BitM3) + 1);
			if (pos + length < _ramSize) {
				u32 count = SpReg.RD_LEN_REG >> 12 & BitM8;
				u32 skip = SpReg.RD_LEN_REG >> 20 & BitM12;
				u32 spPos = SpReg.MEM_ADDR_REG & BitM13;
				// Initiate SP DMA (RDRAM to SPMEM)
				for (u32 c = 0; c <= count; c++) {
					if ((spPos + length <= 0x2000)
							&& (pos + length <= _ramSize)) {
						Endian::UnalignedMemCopy(&_spmem[spPos], &_ram[pos],
								length);
					} else {
						logf("SP DMA out of bounds\n");
						break;
					}
					pos += length + skip;
					spPos += length;
				}
			} else {
				logf("Error: Start+Size of sp dma not in rdram\n");
			}
			return;
		}
		case SP_WR_LEN_REG: {
			SpReg.WR_LEN_REG = value;
			u32 length = (((SpReg.WR_LEN_REG & BitM12) | BitM3) + 1);
			u32 count = (SpReg.WR_LEN_REG >> 12 & BitM8);
			u32 skip = SpReg.WR_LEN_REG >> 20 & BitM12;
			u32 pos = SpReg.DRAM_ADDR_REG & BitM24;
			u32 spPos = SpReg.MEM_ADDR_REG & BitM13;
			// Initiate SP DMA (SPMEM to RDRAM)
			for (u32 c = 0; c <= count; c++) {
				Endian::UnalignedMemCopy(&_ram[pos], &_spmem[spPos], length);
				pos += length + skip;
				spPos += length;
			}
			return;
		}
		case SP_STATUS_REG:
			if (value & SP_CLR_HALT)
				SpReg.STATUS_REG &= ~SP_STATUS_HALT;
			if (value & SP_SET_HALT)
				SpReg.STATUS_REG |= SP_STATUS_HALT;
			if (value & SP_CLR_BROKE)
				SpReg.STATUS_REG &= ~SP_STATUS_BROKE;
			if (value & SP_CLR_INTR) {
				MiReg.INTR_REG &= ~MI_INTR_SP;
			}
			if (value & SP_SET_INTR) {
				DoSomething |= Do_SP_Intr;
			}
			if (value & SP_CLR_SSTEP)
				SpReg.STATUS_REG &= ~SP_STATUS_SSTEP;
			if (value & SP_SET_SSTEP)
				SpReg.STATUS_REG |= SP_STATUS_SSTEP;
			if (value & SP_CLR_INTR_BREAK)
				SpReg.STATUS_REG &= ~SP_STATUS_INTR_BREAK;
			if (value & SP_SET_INTR_BREAK)
				SpReg.STATUS_REG |= SP_STATUS_INTR_BREAK;

			if (value & SP_CLR_SIG0)
				SpReg.STATUS_REG &= ~SP_STATUS_SIG0;
			if (value & SP_SET_SIG0)
				SpReg.STATUS_REG |= SP_STATUS_SIG0;

			if (value & SP_CLR_SIG1)
				SpReg.STATUS_REG &= ~SP_STATUS_SIG1;
			if (value & SP_SET_SIG1)
				SpReg.STATUS_REG |= SP_STATUS_SIG1;

			if (value & SP_CLR_SIG2)
				SpReg.STATUS_REG &= ~SP_STATUS_SIG2;
			if (value & SP_SET_SIG2)
				SpReg.STATUS_REG |= SP_STATUS_SIG2;

			if (value & SP_CLR_SIG3)
				SpReg.STATUS_REG &= ~SP_STATUS_SIG3;
			if (value & SP_SET_SIG3)
				SpReg.STATUS_REG |= SP_STATUS_SIG3;

			if (value & SP_CLR_SIG4)
				SpReg.STATUS_REG &= ~SP_STATUS_SIG4;
			if (value & SP_SET_SIG4)
				SpReg.STATUS_REG |= SP_STATUS_SIG4;

			if (value & SP_CLR_SIG5)
				SpReg.STATUS_REG &= ~SP_STATUS_SIG5;
			if (value & SP_SET_SIG5)
				SpReg.STATUS_REG |= SP_STATUS_SIG5;

			if (value & SP_CLR_SIG6)
				SpReg.STATUS_REG &= ~SP_STATUS_SIG6;
			if (value & SP_SET_SIG6)
				SpReg.STATUS_REG |= SP_STATUS_SIG6;

			if (value & SP_CLR_SIG7)
				SpReg.STATUS_REG &= ~SP_STATUS_SIG7;
			if (value & SP_SET_SIG7)
				SpReg.STATUS_REG |= SP_STATUS_SIG7;

			if ((SpReg.STATUS_REG & SP_STATUS_HALT) == 0) {
				u32 dlistType = Endian::Read32(_spmem, 0xFC0);
				switch (dlistType) {
				case 1:
					if (_displayListInterpreter != NULL) {
						_displayListInterpreter->Interprete();
					}
					DoSomething |= Do_DP_Intr;
					break;
				case 2:
					//logf("Ignoring Audio DList from %X...\n", PC);
					//AUDList(HLEDListAddr);
					//Do_Something |= Do_Step;
					break;
				case 4:
					logf("Ignoring DCT List\n");
					//if (DCTList){
					//DCTList(HLEDListAddr);
					//}
					break;
				case 7:
					logf("Ignoring MPEG List\n");
					// MPEG Ucode
					break;
				default:
					logf("Unhandled RCP List sent\n");
					break;
				}
				if (SpReg.STATUS_REG & SP_STATUS_INTR_BREAK) {
					DoSomething |= Do_SP_Intr;
				}
				SpReg.STATUS_REG |= (SP_STATUS_HALT | SP_STATUS_TASKDONE);
			}
			return;
		case SP_SEMAPHORE_REG:
			if (value == 0) {
				SpReg.SEMAPHORE_REG = 0;
			}
			return;
		}
	}

	if (address == SP_PC_REG) {
		SpReg.PC_REG = value & BitM12;
		return;
	}

	if (address == SP_IBIST_REG) {
		SpReg.IBIST_REG = value;
		return;
	}

	if ((address >= MI_BASE_REG) && (address <= MI_INTR_MASK_REG)) {
		switch (address) {
		case MI_MODE_REG:
			MiReg.MODE_REG = (MiReg.MODE_REG & ~BitM7) | (value & BitM7);
			if (value & MI_CLR_INIT)
				MiReg.MODE_REG &= ~MI_MODE_INIT;
			if (value & MI_SET_INIT)
				MiReg.MODE_REG |= MI_MODE_INIT;
			if (value & MI_CLR_EBUS)
				MiReg.MODE_REG &= ~MI_MODE_EBUS;
			if (value & MI_SET_EBUS)
				MiReg.MODE_REG |= MI_MODE_EBUS;
			if (value & MI_CLR_DP_INTR) {
				MiReg.INTR_REG &= ~MI_INTR_DP;
			}
			if (value & MI_CLR_RDRAM)
				MiReg.MODE_REG &= ~MI_MODE_RDRAM;
			if (value & MI_SET_RDRAM)
				MiReg.MODE_REG |= MI_MODE_RDRAM;
			return;
		case MI_INTR_MASK_REG:
			if (value & MI_INTR_MASK_CLR_SP)
				MiReg.INTR_MASK_REG &= ~MI_INTR_SP;
			if (value & MI_INTR_MASK_SET_SP)
				MiReg.INTR_MASK_REG |= MI_INTR_SP;

			if (value & MI_INTR_MASK_CLR_SI)
				MiReg.INTR_MASK_REG &= ~MI_INTR_SI;
			if (value & MI_INTR_MASK_SET_SI)
				MiReg.INTR_MASK_REG |= MI_INTR_SI;

			if (value & MI_INTR_MASK_CLR_AI)
				MiReg.INTR_MASK_REG &= ~MI_INTR_AI;
			if (value & MI_INTR_MASK_SET_AI)
				MiReg.INTR_MASK_REG |= MI_INTR_AI;

			if (value & MI_INTR_MASK_CLR_VI)
				MiReg.INTR_MASK_REG &= ~MI_INTR_VI;
			if (value & MI_INTR_MASK_SET_VI)
				MiReg.INTR_MASK_REG |= MI_INTR_VI;

			if (value & MI_INTR_MASK_CLR_PI)
				MiReg.INTR_MASK_REG &= ~MI_INTR_PI;
			if (value & MI_INTR_MASK_SET_PI)
				MiReg.INTR_MASK_REG |= MI_INTR_PI;

			if (value & MI_INTR_MASK_CLR_DP)
				MiReg.INTR_MASK_REG &= ~MI_INTR_DP;
			if (value & MI_INTR_MASK_SET_DP)
				MiReg.INTR_MASK_REG |= MI_INTR_DP;
			return;
		}
	}

	if ((address >= AI_BASE_REG) && (address <= AI_BITRATE_REG)) {
		switch (address) {
		case AI_LEN_REG:
			if (AiReg.LEN_REG) {
				AiReg.DBUF_LEN_REG = (value & 0x0003FFFF);
				AiReg.STATUS_REG |= AI_STATUS_FIFO_FULL;
			} else {
				AiReg.LEN_REG = (value & 0x0003FFFF);
				CalculateAiCounter();
				//AISetReg(0,0); //hack that calls AIPlay(); in audio.dll
			}
			break;
		case AI_STATUS_REG:
			MiReg.INTR_REG &= ~MI_INTR_AI;
			//AISetReg(address, value);
			break;
		default:
			//AISetReg(address, value);
			break;
		}
		return;
	}

	if ((address >= VI_BASE_REG) && (address <= VI_Y_SCALE_REG)) {
		switch (address) {
		case VI_CONTROL_REG:
			if (ViReg.STATUS_REG != value) {
				ViReg.STATUS_REG = value;
				logf(
						"Should be sending VI Status to the Graphics plugin here. But let's not\n");
				/*if (GFXPlugin->ViStatusChanged) {
				 GFXPlugin->ViStatusChanged();
				 }*/
			}
			return;
		case VI_DRAM_ADDR_REG:
			ViReg.DRAM_ADDR_REG = value;
			return;
		case VI_H_WIDTH_REG:
			if (ViReg.H_WIDTH_REG != value) {
				ViReg.H_WIDTH_REG = value;
				logf(
						"Should be sending VI Width change to the Graphics plugin here. But let's not\n");
				/*				if (GFXPlugin->ViWidthChanged) {
				 GFXPlugin->ViWidthChanged();
				 }*/
			}
			return;
		case VI_V_INTR_REG:
			ViReg.V_INTR_REG = value;
			return;
		case VI_V_CURRENT_LINE_REG:
			ViReg.V_CURRENT_LINE_REG = value;
			MiReg.INTR_REG &= ~MI_INTR_VI;
			return;
		case VI_TIMING_REG:
			ViReg.TIMING_REG = value;
			return;
		case VI_V_SYNC_REG:
			ViReg.V_SYNC_REG = value;
			/*if (DetectTVModeFromRom() == 0) {
			 ViInterruptTime = Value * 751;//1500;//1253;
			 } else {
			 ViInterruptTime = Value * 557;//1500;//1253;
			 }*/
			ViInterruptTime = value * 1500;
			return;
		case VI_H_SYNC_REG:
			ViReg.H_SYNC_REG = value;
			return;
		case VI_LEAP_REG:
			ViReg.H_SYNC_LEAP_REG = value;
			return;
		case VI_H_VIDEO_REG:
			ViReg.H_VIDEO_REG = value;
			return;
		case VI_V_VIDEO_REG:
			ViReg.V_VIDEO_REG = value;
			return;
		case VI_V_BURST_REG:
			ViReg.V_BURST_REG = value;
			return;
		case VI_X_SCALE_REG:
			ViReg.X_SCALE_REG = value;
			return;
		case VI_Y_SCALE_REG:
			ViReg.Y_SCALE_REG = value;
			return;
		}
	}

	if ((address >= RDRAM_BASE_REG) && (address <= 0x03FFFFFF)) {
		switch (RDRAM_BASE_REG | (address & BitM8)) {
		case RDRAM_CONFIG_REG:
			RdramReg.CONFIG_REG = value;
			return;
		case RDRAM_DEVICE_ID_REG:
			RdramReg.DEVICE_ID_REG = value;
			return;
		case RDRAM_DELAY_REG:
			RdramReg.DELAY_REG = value;
			return;
		case RDRAM_MODE_REG:
			RdramReg.MODE_REG = value;
			return;
		case RDRAM_REF_INTERVAL_REG:
			RdramReg.REF_INTERVAL_REG = value;
			return;
		case RDRAM_REF_ROW_REG:
			RdramReg.REF_ROW_REG = value;
			return;
		case RDRAM_RAS_INTERVAL_REG:
			RdramReg.RAS_INTERVAL_REG = value;
			return;
		case RDRAM_MIN_INTERVAL_REG:
			RdramReg.MIN_INTERVAL_REG = value;
			return;
		case RDRAM_ADDR_SELECT_REG:
			RdramReg.ADDR_SELECT_REG = value;
			return;
		case RDRAM_DEVICE_MANUF_REG:
			RdramReg.DEVICE_MANUF_REG = value;
			return;
		}
	}

	if ((address >= PI_BASE_REG) && (address <= PI_BSD_DOM2_RLS_REG)) {
		switch (address) {
		case PI_DRAM_ADDR_REG:
			PiReg.DRAM_ADDR_REG = value & ~BitM2; // could be divisable by 8 only
			return;
		case PI_CART_ADDR_REG:
			PiReg.CART_ADDR_REG = value & ~BitM1; // this was tested on the real n64
			return;
		case PI_RD_LEN_REG: {
			PiReg.RD_LEN_REG = value;
			//u32 size = (PiReg.RD_LEN_REG + 1) & 0xFFFFFF;
			//u32 c1 = PiReg.DRAM_ADDR_REG;
			if ((PiReg.CART_ADDR_REG >= PI_DOM2_ADDR2) && (PiReg.CART_ADDR_REG
					<= 0x0803FFFF)) {
				logf("Ignoring DMA to FLASH RAM\n");
				/*u32 StartRom = (PiReg.CART_ADDR_REG & ~1) - PI_DOM2_ADDR2;
				 if((StartRom < 0x20000) && (flash_on))
				 {
				 flash_dram=c1;
				 }
				 else
				 {
				 CopyRdramToSram(&RDRAM[c1], StartRom, Size);
				 }*/
			} else {
				logf("PI read (ram to unhandled memory): %i",
						PiReg.CART_ADDR_REG);
			}
			DoSomething |= Do_PI_Intr;
			return;
		}
		case PI_WR_LEN_REG: {
			// Timing: countincrease=(bytes*9)/2
			PiReg.WR_LEN_REG = value;
			u32 Size = (PiReg.WR_LEN_REG + 1) & 0xFFFFFF; // 16 megs max
			u32 c1 = PiReg.DRAM_ADDR_REG;
			if (Size + c1 >= _ramSize - 1) {
				Size = _ramSize - 1 - c1;
			}
			if ((PiReg.CART_ADDR_REG >= PI_DOM1_ADDR2) && (PiReg.CART_ADDR_REG
					<= 0x1FBFFFFF)) {
				// Copy from CART TO RDRAM
				u32 StartRom = PiReg.CART_ADDR_REG - PI_DOM1_ADDR2;
				if (Size + StartRom > _romSize) {
					if (StartRom < _romSize) {
						Size = _romSize - StartRom - 1;
						logf("Clamping a PI DMA\n");
					} else {
						logf("Not doing some PI DMA\n");
						return;
					}
				}
				// rdram and rom ALWAYS have the same endian mode, so memcpy is fine.
				// but in pc endian we have to be careful if StartRom is not aligned to 4 bytes
				Endian::UnalignedMemCopy(&_ram[c1], &_rom[StartRom], Size);
			} else if ((PiReg.CART_ADDR_REG >= PI_DOM2_ADDR2)
					&& (PiReg.CART_ADDR_REG <= 0x0803FFFF)) {
				logf("Ignoring DMA from Flash to RAM\n");
				/*StartRom = PiReg.CART_ADDR_REG - PI_DOM2_ADDR2;
				 CopySramToRdram(&RDRAM[c1], StartRom, Size);
				 if ((flash_on) && (StartRom==0) && (flash_control != 0xF0000000))
				 {
				 _Write32Endian(RDRAM, c1, flash_status);
				 _Write32Endian(RDRAM, c1+4, 0x00c2001d);
				 }
				 */
			} else if ((PiReg.CART_ADDR_REG >= PI_DOM1_ADDR1)
					&& (PiReg.CART_ADDR_REG <= 0x063FFFFF)) {
				logf("Ignoring DMA from 64dd to RAM\n");
				/*StartRom = PiReg.CART_ADDR_REG - PI_DOM1_ADDR1;
				 if(!ddROM)
				 Load64DDImage(psz64DDBootImage);
				 unaligned_memcpy(&RDRAM[c1],&ddROM[StartRom], Size);
				 */
			} else {
				if (PiReg.CART_ADDR_REG < _ramSize) {
					Endian::UnalignedMemCopy(&_ram[c1],
							&_ram[PiReg.CART_ADDR_REG], Size);
				} else {
					logf("PI write (unhandled cart memory to rdram): %i",
							PiReg.CART_ADDR_REG);
				}
			}
			DoSomething |= Do_PI_Intr;
			return;
		}
		case PI_STATUS_REG:
			if (value & PI_CLR_INTR) {
				MiReg.INTR_REG &= ~MI_INTR_PI;
			}
			return;
		case PI_BSD_DOM1_LAT_REG:
			PiReg.BSD_DOM1_LAT_REG = value & 0xFF;
			return;
		case PI_BSD_DOM1_PWD_REG:
			PiReg.BSD_DOM1_PWD_REG = value & 0xFF;
			return;
		case PI_BSD_DOM1_PGS_REG:
			PiReg.BSD_DOM1_PGS_REG = value & 0xF;
			return;
		case PI_BSD_DOM1_RLS_REG:
			PiReg.BSD_DOM1_RLS_REG = value & 0x3;
			return;
		case PI_BSD_DOM2_LAT_REG:
			PiReg.BSD_DOM2_LAT_REG = value & 0xFF;
			return;
		case PI_BSD_DOM2_PWD_REG:
			PiReg.BSD_DOM2_PWD_REG = value & 0xFF;
			return;
		case PI_BSD_DOM2_PGS_REG:
			PiReg.BSD_DOM2_PGS_REG = value & 0xF;
			return;
		case PI_BSD_DOM2_RLS_REG:
			PiReg.BSD_DOM2_RLS_REG = value & 0x3;
			return;
		}
	}

	if ((address >= SI_BASE_REG) && (address <= SI_STATUS_REG)) {
		switch (address) {
		case SI_DRAM_ADDR_REG:
			SiReg.DRAM_ADDR_REG = value & BitM24;
			return;
		case SI_PIF_ADDR_RD64B_REG:
			// Copy from PifRAM to RDRAM
			//logf("DMAing from PifRam to Ram\n");
			HandlePifRam();
			Endian::UnalignedMemCopy(&_ram[SiReg.DRAM_ADDR_REG], &_pifRam[0],
					64);
			// TODO: Old emu had a way of delaying this interrupt
			DoSomething |= Do_SI_Intr;
			return;
		case SI_PIF_ADDR_WR64B_REG:
			// Copy from RDRAM to PifRAM
			Endian::UnalignedMemCopy(&_pifRam[0], &_ram[SiReg.DRAM_ADDR_REG],
					64);
			// TODO: Old emu had a way of delaying this interrupt
			DoSomething |= Do_SI_Intr;
			//logf("DMAing from Ram to PifRam\n");
			HandlePifRam();
			return;
		case SI_STATUS_REG:
			MiReg.INTR_REG &= ~MI_INTR_SI;
			return;
		}
	}

	if ((address >= DPC_BASE_REG) && (address <= DPC_TMEM_REG)) {
		switch (address) {
		case DPC_START_REG:
			DpcReg.START_REG = value & BitM24;
			DpcReg.CURRENT_REG = DpcReg.START_REG;
			return;
		case DPC_END_REG:
			DpcReg.END_REG = value & BitM24;
			logf("Ignoring write to DPC_END_REG (should raise a DP Int here)\n");
			/*u32 RDPCommand[2];
			 if (DpcReg.CURRENT_REG<DpcReg.END_REG){
			 do {
			 if (DpcReg.STATUS_REG & DPC_STATUS_XBUS_DMEM_DMA){
			 RDPCommand[0] = Read32(SP_DMEM_START +     DpcReg.CURRENT_REG);
			 RDPCommand[1] = Read32(SP_DMEM_START + 4 + DpcReg.CURRENT_REG);
			 } else {
			 RDPCommand[0] = Read32(DpcReg.CURRENT_REG);
			 RDPCommand[1] = Read32(DpcReg.CURRENT_REG + 4);
			 }
			 if ((RDPCommand[0]>>26)==0x3A){ // RDP_FULLSYNC
			 Do_Something|=Do_DP_Intr;
			 }
			 RDPCommandIndex &= 2047;
			 RDPCommands[RDPCommandIndex] = RDPCommand[0];
			 RDPCommandIndex++;
			 RDPCommandIndex &= 2047;
			 RDPCommands[RDPCommandIndex] = RDPCommand[1];
			 RDPCommandIndex++;
			 DpcReg.CURRENT_REG+=8;
			 } while (DpcReg.CURRENT_REG<DpcReg.END_REG);
			 }*/
			return;
		case DPC_STATUS_REG:
			if (value & DPC_CLR_XBUS_DMEM_DMA)
				DpcReg.STATUS_REG &= ~DPC_STATUS_XBUS_DMEM_DMA;
			if (value & DPC_SET_XBUS_DMEM_DMA)
				DpcReg.STATUS_REG |= DPC_STATUS_XBUS_DMEM_DMA;
			if (value & DPC_CLR_FREEZE)
				DpcReg.STATUS_REG &= ~DPC_STATUS_FREEZE;
			//if (Value&DPC_SET_FREEZE) DpcReg.STATUS_REG |= DPC_STATUS_FREEZE;
			if (value & DPC_CLR_FLUSH)
				DpcReg.STATUS_REG &= ~DPC_STATUS_FLUSH;
			if (value & DPC_SET_FLUSH)
				DpcReg.STATUS_REG |= DPC_STATUS_FLUSH;
			if (value & DPC_CLR_TMEM_CTR)
				DpcReg.TMEM_REG = 0;
			if (value & DPC_CLR_PIPE_CTR)
				DpcReg.PIPEBUSY_REG = 0;
			if (value & DPC_CLR_CMD_CTR)
				DpcReg.BUFBUSY_REG = 0;
			if (value & DPC_CLR_CLOCK_CTR)
				DpcReg.CLOCK_REG = 0;
			return;
		}
	}

	if ((address >= PI_DOM2_ADDR2) && (address <= PI_DOM2_ADDR2 + 0x3FFFF)) // Sram / Flashram
	{
		logf("Ignoring write to flash area\n");
		return;
		//		if(!tempf)
		//			tempf=fopen("flash.log","wb");
		//		flogf(tempf,"Flash write %s <- 0x%08X \n",(address==0x08000000)?"status":"control",Value);
		//		fflush(tempf);
		/*		flash_on=TRUE;
		 flash_control=0;
		 if(value&0xFFFF)
		 flash_control=value;
		 if((value&0xFF000000)==0x4b000000)
		 flash_control=value;
		 if(value==0xD2000000)
		 {
		 if((flash_status&0xffff)==0x0080)
		 flash_status=0x00800080;
		 else
		 flash_status=((flash_status & 0xFFFF0000) | 0x0080);
		 }
		 if(value==0xE1000000)
		 flash_status=0x11118001;
		 if(value==0xF0000000)
		 flash_control=0xF0000000;

		 if((value & 0xFF000000)==0x78000000)
		 {
		 flash_status=0x00080008;
		 }

		 if((value & 0xFF000000)==0xA5000000)
		 {
		 //			if(!tempf)
		 //				tempf=fopen("flash.log","wb");
		 //			flogf(tempf,"program sector 0x%04X from address 0x%08X \n",Value&0xfff,flash_dram);
		 //			fflush(tempf);
		 CopyRdramToSram(&RDRAM[flash_dram], ((value & 0xffff)*0x80), 0x80);
		 flash_status=0x00040004;
		 }
		 return;
		 */
	}

	if ((address >= PI_DOM1_ADDR2) && (address <= 0x1FBFFFFF)) { // ROM area (for code caching)
		/* if (AllowRomWrite){
		 u32 address2;
		 address2 = address - PI_DOM1_ADDR2;
		 if (address2<romsize){
		 _Write32Endian(ROM, address2, value);
		 }
		 // goldeneye hack
		 //address2 = address-PI_DOM1_ADDR2-64*MB+0x34b30;  // goldeneye
		 address2 = address-PI_DOM1_ADDR2-64*MB+0x34b30;  // conkers bfd
		 if (address2 < romsize){
		 _Write32Endian(ROM, address2, value);
		 }
		 return;
		 }*/
		logf("Ignoring write to ROM area...\n");
		return;
	}

	if (address < (_ramSize + 4 * 1024 * 1024)) { // RDRAM extension
		// RDRAM size detection
		logf("Writing outside of rdram-bounds (for size detection)\n");
		return;
	}

	// Unmapped memory
	logf("Trying to Write32 '%x' to unhandled or invalid memory: %x\n", value,
			address);
}

void VM::WriteMem16(u32 address, u16 value) {
	if (address < _ramSize) { // RDRAM
		Endian::Write16(_ram, address, value);
		return;
	}
	if ((address >= SP_DMEM_START) && (address <= SP_IMEM_END)) { // SPMEM
		Endian::Write16(_spmem, address - SP_DMEM_START, value);
		return;
	}
	if ((address >= PIF_RAM_START) && (address <= PIF_RAM_END)) { // PIFRAM
		Endian::Write16(_pifRam, address - PIF_RAM_START, value);
		logf("Handling controllers...\n");
		HandlePifRam();
		return;
	}

	// Unmapped memory
	logf("Trying to Write16 '%x' to unhandled or invalid memory: %x\n", value,
			address);
}

void VM::WriteMem8(u32 address, u8 value) {
	if (address < _ramSize) { // RDRAM
		Endian::Write8(_ram, address, value);
		return;
	}
	if ((address >= SP_DMEM_START) && (address <= SP_IMEM_END)) { // SPMEM
		Endian::Write8(_spmem, address - SP_DMEM_START, value);
		return;
	}
	if ((address >= PIF_RAM_START) && (address <= PIF_RAM_END)) {
		Endian::Write8(_pifRam, address - PIF_RAM_START, value);
		logf("Handling controllers...\n");
		HandlePifRam();
		return;
	}

	// Unmapped memory
	logf("Trying to Write8 '%x' to unhandled or invalid memory: %x\n", value,
			address);
}

void VM::CalculateAiCounter() {
	float AiCountFactor = 0.98f;
	u32 clockRate = DetectTVModeFromRom() != 0 ? 0x2E6D354 : 0x2F5B2D2;
	UseAiCounter = true;
	s32 temp = (s32) ((s64) (AiReg.LEN_REG / 4) * (AiReg.DACRATE_REG + 1)
			* 46875000 / clockRate);
	AiCounter = (s32) ((float) temp * AiCountFactor);
}

void VM::QueueLoadState(const std::string filename) {
	_saveStateFilename = filename;
	DoSomething |= Do_LoadState;
}

void VM::QueueSaveState(const std::string filename) {
	_saveStateFilename = filename;
	DoSomething |= Do_SaveState;
}

void VM::LoadState() {
	std::ifstream file(_saveStateFilename.c_str(), std::ios::in);

	// Ram Size
	u32 ramSize;
	file.read((char*) &ramSize, 4);
	if (_ramSize != ramSize) {
		free(_ram);
		_ramSize = _ramSize;
		_ram = (u8*) malloc(_ramSize);
	}

	// Ram
	file.read((char*) &_ram[0], _ramSize);

	// PifRam
	file.read((char*) &_pifRam[0], PIF_RAM_SIZE);

	// PifRom
	file.read((char*) &_pifRom[0], PIF_ROM_SIZE);

	file.read((char*) &_spmem[0], SP_MEM_SIZE);

	file.read((char*) &_eepromSize, 4);
	file.read((char*) &_eepromPresent, 1);
	file.read((char*) &_eepromData[0], 2048);

	file.read((char*) &_tlbError, 4);
	file.read((char*) &_tlbInvalidVAddress, 4);
	file.read((char*) &_tlbInvalidWrite, 4);

	file.read((char*)&PC, 4);
	file.read((char*)&CurrentLineCount, 4);
	file.read((char*)&ViInterruptTime, 4);
	file.read((char*)&DoSomething, 4);
	file.read((char*)&Cop1Fcr31, 4);
	file.read((char*)&NextVIInterrupt, 4);

	file.read((char*)&HandledOverflow, 1);
	file.read((char*)&CompareCheck, 1);
	file.read((char*)&Cop1UnusableInDelay, 1);
	file.read((char*)&UsesEEProm, 1);
	file.read((char*)&UsesMempack, 1);
	file.read((char*)&Controllers[0], sizeof(TController) * 4);

	file.read((char*)&UseAiCounter, 1);
	file.read((char*)&AiCounter, 4);

	file.read((char*)&Registers, sizeof(TRegisters));
	file.read((char*)&Lo, 8);
	file.read((char*)&Hi, 8);
	file.read((char*)&Cop0Registers, sizeof(TCOP0Registers));
	file.read((char*)&Cop1Registers, sizeof(TCOP1Registers));
	file.read((char*)&Cop1Registers64, sizeof(TCOP1Registers64));
	file.read((char*)&Tlb[0], sizeof(TTLB) * 32);
	file.read((char*)&NextFreeTLBEntry, 4);

	file.read((char*)&RiReg, sizeof(TRIReg));
	file.read((char*)&SpReg, sizeof(TSPReg));
	file.read((char*)&MiReg, sizeof(TMIReg));
	file.read((char*)&AiReg, sizeof(TAIReg));
	file.read((char*)&SiReg, sizeof(TSIReg));
	file.read((char*)&RdramReg, sizeof(TRDRAMReg));
	file.read((char*)&PiReg, sizeof(TPIReg));
	file.read((char*)&ViReg, sizeof(TVIReg));
	file.read((char*)&DpcReg, sizeof(TDPCReg));
	file.close();
}

void VM::SaveState() {
	std::ofstream file(_saveStateFilename.c_str(), std::ios::out);
	file.write((char*)&_ramSize, 4);
	file.write((char*)&_ram[0], _ramSize);
	file.write((char*)&_pifRam[0], PIF_RAM_SIZE);
	file.write((char*)&_pifRom[0], PIF_ROM_SIZE);
	file.write((char*)&_spmem[0], SP_MEM_SIZE); // don't store the overflow
	file.write((char*)&_eepromSize, 4);
	file.write((char*)&_eepromPresent, 1);
	file.write((char*)&_eepromData[0], 2048);
	file.write((char*)&_tlbError, 4);
	file.write((char*)&_tlbInvalidVAddress, 4);
	file.write((char*)&_tlbInvalidWrite, 4);

	file.write((char*)&PC, 4);
	file.write((char*)&CurrentLineCount, 4);
	file.write((char*)&ViInterruptTime, 4);
	file.write((char*)&DoSomething, 4);
	file.write((char*)&Cop1Fcr31, 4);
	file.write((char*)&NextVIInterrupt, 4);

	file.write((char*)&HandledOverflow, 1);
	file.write((char*)&CompareCheck, 1);
	file.write((char*)&Cop1UnusableInDelay, 1);
	file.write((char*)&UsesEEProm, 1);
	file.write((char*)&UsesMempack, 1);
	file.write((char*)&Controllers[0], sizeof(TController) * 4);

	file.write((char*)&UseAiCounter, 1);
	file.write((char*)&AiCounter, 4);

	file.write((char*)&Registers, sizeof(TRegisters));
	file.write((char*)&Lo, 8);
	file.write((char*)&Hi, 8);
	file.write((char*)&Cop0Registers, sizeof(TCOP0Registers));
	file.write((char*)&Cop1Registers, sizeof(TCOP1Registers));
	file.write((char*)&Cop1Registers64, sizeof(TCOP1Registers64));
	file.write((char*)&Tlb[0], sizeof(TTLB) * 32);
	file.write((char*)&NextFreeTLBEntry, 4);

	file.write((char*)&RiReg, sizeof(TRIReg));
	file.write((char*)&SpReg, sizeof(TSPReg));
	file.write((char*)&MiReg, sizeof(TMIReg));
	file.write((char*)&AiReg, sizeof(TAIReg));
	file.write((char*)&SiReg, sizeof(TSIReg));
	file.write((char*)&RdramReg, sizeof(TRDRAMReg));
	file.write((char*)&PiReg, sizeof(TPIReg));
	file.write((char*)&ViReg, sizeof(TVIReg));
	file.write((char*)&DpcReg, sizeof(TDPCReg));
	file.close();
}
