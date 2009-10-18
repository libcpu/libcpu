#include "CPUInterpreter.h"
#include "Opcode.h"
#include "../stdincludes.h"

#include "../../../../arch/mips/CPUDisassembler.h"

using namespace std;

CPUInterpreter::CPUInterpreter(VM* vm) :
	CPUEmulator(vm) {
	_registers = vm->GetRegisters();
}

void CPUInterpreter::InvalidOpcode() {
	logf("Invalid Opcode found\n");
}

void CPUInterpreter::UnhandledOpcode(string desc) {
	logf("%s\n", ("Unhandled Opcode: " + desc).c_str());
}

void CPUInterpreter::CheckMIInterrupt(u32 DoValue, u32 MIValue) {
	if (_vm->DoSomething & DoValue) {
		if ((_vm->Cop0Registers.Status & SR_IBIT3)
				&& (_vm->Cop0Registers.Status & SR_IE)
				&& ((_vm->Cop0Registers.Status & (SR_ERL | SR_EXL)) == 0)) {
			_vm->DoSomething &= ~DoValue;
			/*logf(
			 "Taking Interrupt %X (TODO: Check whether it is even enabled)\n",
			 MIValue);*/
			if (_vm->MiReg.INTR_MASK_REG & MIValue) {
				_vm->MiReg.INTR_REG |= MIValue;
				_vm->Cop0Registers.Cause |= CAUSE_IP3;
				_vm->Cop0Registers.Cause &= ~(CAUSE_EXCMASK | CAUSE_BD);
				_vm->Cop0Registers.Status |= SR_EXL;
				_vm->Cop0Registers.ExceptPC = _vm->PC;
				_vm->PC = E_VEC;
			}
		}
	}
}

void CPUInterpreter::Run() {
//printf("PC: %llx\n", (long long)_vm->PC);
//printf(".");
	TOpcode op;
	op.all = _vm->ReadMem32(_vm->Map(_vm->PC));

#if 1
	CPUDisassembler *disassembler = new CPUDisassembler();
	printf("%08llx %s\n", (long long)_vm->PC, disassembler->Disassemble(_vm->PC, op, false).c_str());

//	if (bytes == 8) {// delay slot
//		TOpcode op2;
//		op2.all = _vm->ReadMem32(_vm->Map(_vm->PC));
//		snprintf(line+strlen(line), max_line-strlen(line), " [%s]", disassembler->Disassemble(pc+4, op2, false).c_str());
//	}
#endif

	ExecuteCommand(op);
	IncreaseCounter(1);
	_vm->PC += 4;

	if (_vm->NextVIInterrupt < 0) {
		_vm->NextVIInterrupt += _vm->ViInterruptTime;
		if (_vm->MiReg.INTR_MASK_REG & MI_INTR_VI) {
			_vm->DoSomething |= VM::Do_VI_Intr;
		}
		// TODO: render screen
		//logf("TODO: UpdateScreen (VI Interrupt)\n");
		/*if (GFXPlugin->UpdateScreen){
		 GFXPlugin->UpdateScreen();
		 }*/
	}

	if (_vm->UseAiCounter && (_vm->AiCounter < 0)) {
		_vm->DoSomething |= VM::Do_AI_Intr;
		_vm->AiReg.STATUS_REG &= (~AI_STATUS_DMA_BUSY);
		_vm->AiReg.STATUS_REG &= (~AI_STATUS_FIFO_FULL);
		// double buffered addresses?
		if (_vm->AiReg.DBUF_LEN_REG) {
			_vm->AiReg.DRAM_ADDR_REG = _vm->AiReg.DBUF_DRAM_ADDR_REG;
			_vm->AiReg.DBUF_DRAM_ADDR_REG = 0;
			_vm->AiReg.LEN_REG = _vm->AiReg.DBUF_LEN_REG;
			_vm->AiReg.DBUF_LEN_REG = 0;
			_vm->CalculateAiCounter();
			//AISetReg(0, 0); //hack that calls AIPlay(); in audio.dll
			//AIReg.STATUS_REG &= ~AI_STATUS_FIFO_FULL;
		} else {
			_vm->AiReg.DRAM_ADDR_REG = 0;
			_vm->AiReg.LEN_REG = 0;
			_vm->UseAiCounter = false;
		}
	}
	// if there was an overflow set CompareCheck
	if (_vm->Cop0Registers.Count < 2 * CountTime) {
		if (!_vm->HandledOverflow) {
			_vm->CompareCheck = true;
			_vm->HandledOverflow = true;
		}
	} else {
		_vm->HandledOverflow = false;
	}

	if (_vm->CompareCheck && (_vm->Cop0Registers.Count
			>= _vm->Cop0Registers.Compare)) {
		_vm->DoSomething |= VM::Do_Compare_Intr;
		_vm->CompareCheck = false;
	}
	if (_vm->DoSomething != 0) {
		CheckMIInterrupt(VM::Do_VI_Intr, MI_INTR_VI);
		CheckMIInterrupt(VM::Do_SI_Intr, MI_INTR_SI);
		CheckMIInterrupt(VM::Do_PI_Intr, MI_INTR_PI);
		CheckMIInterrupt(VM::Do_SP_Intr, MI_INTR_SP);
		CheckMIInterrupt(VM::Do_AI_Intr, MI_INTR_AI);
		CheckMIInterrupt(VM::Do_DP_Intr, MI_INTR_DP);
		if (_vm->DoSomething & VM::Do_Compare_Intr) {
			logf("Compare Interrupt\n");
			if ((_vm->Cop0Registers.Status & SR_IBIT8)
					&& (_vm->Cop0Registers.Status & SR_IE)
					&& ((_vm->Cop0Registers.Status & (SR_ERL | SR_EXL)) == 0)) {
				_vm->Cop0Registers.Cause |= CAUSE_IP8;
				_vm->Cop0Registers.Cause &= ~(CAUSE_EXCMASK | CAUSE_BD);
				_vm->Cop0Registers.Status |= SR_EXL;
				_vm->Cop0Registers.ExceptPC = _vm->PC;
				_vm->PC = E_VEC;
				_vm->DoSomething &= ~VM::Do_Compare_Intr;
			}
		}
		if (_vm->DoSomething & VM::Do_COP1Unusable) {
			if ((_vm->Cop0Registers.Status & SR_EXL) == 0) {
				_vm->Cop0Registers.ExceptPC = _vm->PC;
				if (_vm->Cop1UnusableInDelay) {
					_vm->Cop0Registers.Cause |= CAUSE_BD;
				} else {
					_vm->Cop0Registers.Cause &= ~CAUSE_BD;
				}
			} else {
				logf(
						"Doing cop1 unusable exception while exception is already being executed. Dunno if this will work\n");
			}
			_vm->Cop0Registers.Cause &= ~CAUSE_EXCMASK;
			_vm->Cop0Registers.Cause |= EXC_CPU;
			_vm->Cop0Registers.Status |= SR_EXL;
			_vm->Cop0Registers.Cause &= ~CAUSE_CEMASK;
			_vm->Cop0Registers.Cause |= (1 << CAUSE_CESHIFT); // set cop number
			_vm->PC = E_VEC;
			_vm->DoSomething &= ~VM::Do_COP1Unusable;
		}
		if (_vm->DoSomething & VM::Do_LoadState) {
			_vm->LoadState();
			_vm->DoSomething &= ~VM::Do_LoadState;
		}
		if (_vm->DoSomething & VM::Do_SaveState) {
			_vm->DoSomething &= ~VM::Do_SaveState;
			_vm->SaveState();
		}
		/*if(_vm->DoSomething&Do_COP1Exception){
		 ShowCPUError("COP1 Exception trap taken");
		 if ((_vm->Cop0Registers.Status & SR_EXL) == 0){
		 _vm->Cop0Registers.ExceptPC = PC;
		 if (COP1ExceptionInDelay){
		 _vm->Cop0Registers.Cause |= CAUSE_BD;
		 } else {
		 _vm->Cop0Registers.Cause &= ~CAUSE_BD;
		 }
		 } else {
		 ShowCPUError("Doing cop1 trap exception while exception is already being executed. Dunno if this will work");
		 }
		 _vm->Cop0Registers.Cause &= ~CAUSE_EXCMASK;
		 _vm->Cop0Registers.Cause |= EXC_FPE;
		 _vm->Cop0Registers.Status |= SR_EXL;
		 PC = E_VEC;
		 _vm->DoSomething &= ~Do_COP1Exception;
		 }
		 if(_vm->DoSomething&Do_TLBException){
		 if ((_vm->Cop0Registers.Status & SR_EXL) == 0){
		 _vm->Cop0Registers.ExceptPC = PC;
		 if (TLBExceptionInDelay){
		 _vm->Cop0Registers.Cause |= CAUSE_BD;
		 TLBExceptionInDelay = false;
		 } else {
		 _vm->Cop0Registers.Cause &= ~CAUSE_BD;
		 }
		 } else {
		 ShowCPUError("Doing tlb exception while exception is already being executed. Dunno if this will work");
		 }
		 _vm->Cop0Registers.Cause &= ~(CAUSE_EXCMASK);
		 switch (TLBError){
		 case 1: // TLB Miss
		 if (TLBInvalidWrite){
		 _vm->Cop0Registers.Cause |= EXC_WMISS;
		 } else {
		 _vm->Cop0Registers.Cause |= EXC_RMISS;
		 }
		 if (_vm->Cop0Registers.Status & SR_EXL){
		 PC = E_VEC;
		 } else {
		 PC = UT_VEC;
		 }
		 break;
		 case 2: // TLB Entry 0 not valid
		 case 3: // TLB Entry 1 not valid
		 if (TLBInvalidWrite){
		 _vm->Cop0Registers.Cause |= EXC_WMISS;
		 } else {
		 _vm->Cop0Registers.Cause |= EXC_RMISS;
		 }
		 PC = E_VEC;
		 break;
		 case 4: // TLB Entry 0 not dirty, but written to
		 case 5: // TLB Entry 1 not dirty, but written to
		 _vm->Cop0Registers.Cause |= EXC_MOD;
		 PC = E_VEC;
		 break;
		 }
		 _vm->Cop0Registers.Context = (_vm->Cop0Registers.Context & 0xff800000) | ((TLBInvalidVAddress >> 9) & 0x7FFFF0);
		 _vm->Cop0Registers.BadVAddr = TLBInvalidVAddress;
		 _vm->Cop0Registers.EntryHi = (_vm->Cop0Registers.EntryHi&0xFF)|(TLBInvalidVAddress & 0xFFFFE000);
		 _vm->Cop0Registers.Status |= SR_EXL;
		 _vm->DoSomething &= ~Do_TLBException;
		 }
		 if(_vm->DoSomething&Do_Syscall){
		 _vm->Cop0Registers.Cause &= ~(CAUSE_EXCMASK | CAUSE_BD);
		 _vm->Cop0Registers.Cause|=EXC_SYSCALL;
		 _vm->Cop0Registers.ExceptPC = SyscallPC;
		 _vm->Cop0Registers.Status|=SR_EXL;
		 PC = E_VEC;
		 _vm->DoSomething &= ~Do_Syscall;
		 }
		 if (_vm->DoSomething&Do_Pause){
		 SuspendThread(MyThreadHandle);
		 }
		 if (_vm->DoSomething&Do_SaveState){
		 _vm->DoSomething&=~Do_SaveState;
		 WriteSaveState();
		 }
		 if (_vm->DoSomething&Do_LoadState){
		 _vm->DoSomething&=~Do_LoadState;
		 ReadSaveState(false);
		 }
		 if(_vm->DoSomething&Do_Reset){
		 InitN64();
		 _vm->DoSomething &= ~Do_Reset;
		 }
		 */
	}
}

void CPUInterpreter::ExecuteDelay() {
	TOpcode op;
	op.all = _vm->ReadMem32(_vm->Map(_vm->PC + 4));
	ExecuteCommand(op);
	IncreaseCounter(1);
}

void CPUInterpreter::SaveLinkRegister(int index) {
	_registers[31] = (s32) (_vm->PC + 8);
}

void CPUInterpreter::JumpTo(u32 targetPC) {
	_vm->PC = targetPC - 4;
}

void CPUInterpreter::BranchCompareRegister(TOpcode op, bool equal, bool likely) {
	// there is no logical XOR. but as both types are bool (0=false, 1=true) it shouldn't matter
	if ((!equal) ^ (_registers[op.rs()] == _registers[op.rt()])) {
		ExecuteDelay();
		u32 targetPC = _vm->PC + 4 + ((u32) (s16) op.immediate() << 2);
		JumpTo(targetPC);
	} else {
		if (likely) {
			JumpTo(_vm->PC + 8);
		}
	}
}

void CPUInterpreter::BranchCompareZero(TOpcode op, TComparison comparison,
		bool likely, bool Link) {
	if (Link) {
		SaveLinkRegister(31);
	}
	bool DoJump;
	switch (comparison) {
	case LE:
		DoJump = (s64) _registers[op.rs()] <= 0;
		break;
	case LT:
		DoJump = (s64) _registers[op.rs()] < 0;
		break;
	case GE:
		DoJump = (s64) _registers[op.rs()] >= 0;
		break;
	case GT:
		DoJump = (s64) _registers[op.rs()] > 0;
		break;
	default:
		logf("Invalid comparison in BranchCompareZero\n");
		DoJump = false;
		break;
	}
	if (DoJump) {
		ExecuteDelay();
		u32 targetPC = _vm->PC + 4 + ((u32) (s16) op.immediate() << 2);
		JumpTo(targetPC);
	} else {
		if (likely) {
			JumpTo(_vm->PC + 8);
		}
	}
}

void CPUInterpreter::ExecuteCommand(TOpcode op) {
	switch (op.op()) {
	case _SPECIAL:
		switch (op.funct()) {
		case _SLL:
			_registers[op.rd()] = (s32) ((u32) _registers[op.rt()] << op.sa());
			break;
		case _SRL:
			_registers[op.rd()] = (s32) ((u32) _registers[op.rt()] >> op.sa());
			break;
		case _SRA:
			_registers[op.rd()] = (s32) ((s32) _registers[op.rt()] >> op.sa());
			break;
		case _SLLV:
			_registers[op.rd()] = (s32) ((u32) _registers[op.rt()]
					<< _registers[op.rs()]);
			break;
		case _SRLV:
			_registers[op.rd()] = (s32) ((u32) _registers[op.rt()]
					>> _registers[op.rs()]);
			break;
		case _SRAV:
			_registers[op.rd()] = (s32) (((s32) _registers[op.rt()])
					>> _registers[op.rs()]);
			break;
		case _JR:
			ExecuteDelay();
			JumpTo((u32) _registers[op.rs()]);
			break;
		case _JALR:
			ExecuteDelay();
			SaveLinkRegister(op.rd());
			JumpTo((u32) _registers[op.rs()]);
			break;
		case _SYSCALL:
			UnhandledOpcode("Special: _SYSCALL");
			break;
		case _BREAK:
			UnhandledOpcode("Special: _BREAK");
			break;
		case _SYNC:
			UnhandledOpcode("Special: _SYNC");
			break;
		case _MFHI:
			_registers[op.rd()] = _vm->Hi;
			break;
		case _MTHI:
			_vm->Hi = _registers[op.rs()];
			break;
		case _MFLO:
			_registers[op.rd()] = _vm->Lo;
			break;
		case _MTLO:
			_vm->Lo = _registers[op.rs()];
			break;
		case _DSLLV:
			_registers[op.rd()] = _registers[op.rt()] << _registers[op.rs()];
			break;
		case _DSRLV:
			_registers[op.rd()] = (u64) _registers[op.rt()]
					>> _registers[op.rs()];
			break;
		case _DSRAV:
			_registers[op.rd()] = (s64) _registers[op.rt()]
					>> _registers[op.rs()];
			break;
		case _MULT: {
			s64 Result = ((s64) (s32) _registers[op.rs()])
					* ((s64) (s32) _registers[op.rt()]);
			_vm->Lo = (s64) (s32) (Result);
			_vm->Hi = (s64) (s32) (Result >> 32);
			break;
		}
		case _MULTU: {
			u64 Result = ((u64) (u32) _registers[op.rs()])
					* ((u64) (u32) _registers[op.rt()]);
			_vm->Lo = (s64) (s32) (Result);
			Result >>= 32;
			_vm->Hi = (s64) (s32) (Result);
			break;
		}
		case _DIV: {
			if ((u32) _registers[op.rt()] != 0) {
				_vm->Lo = (s32) _registers[op.rs()] / (s32) _registers[op.rt()];
				_vm->Hi = (s32) _registers[op.rs()] % (s32) _registers[op.rt()];
			}
			break;
		}
		case _DIVU: {
			if ((u32) _registers[op.rt()] != 0) {
				_vm->Lo = (u32) _registers[op.rs()] / (u32) _registers[op.rt()];
				_vm->Hi = (u32) _registers[op.rs()] % (u32) _registers[op.rt()];
			}
			break;
		}
		case _DMULT: {
			logf("Faking DMULT\n");
			_vm->Lo = (s64) _registers[op.rs()] * (s64) _registers[op.rt()];
			_vm->Hi = 0;
			break;
		}
		case _DMULTU: {
			logf("Faking DMULTU\n");
			_vm->Lo = (u64) _registers[op.rs()] * (u64) _registers[op.rt()];
			_vm->Hi = 0;
			break;
		}
		case _DDIV: {
			if (_registers[op.rt()] != 0) {
				_vm->Lo = (s64) _registers[op.rs()] / (s64) _registers[op.rt()];
				_vm->Hi = (s64) _registers[op.rs()] % (s64) _registers[op.rt()];
			}
			break;
		}
		case _DDIVU: {
			if (_registers[op.rt()] != 0) {
				_vm->Lo = (u64) _registers[op.rs()] / (u64) _registers[op.rt()];
				_vm->Hi = (u64) _registers[op.rs()] % (u64) _registers[op.rt()];
			}
			break;
		}
		case _ADD:
			_registers[op.rd()] = (s32) ((u32) _registers[op.rs()]
					+ (u32) _registers[op.rt()]);
			break;
		case _ADDU:
			_registers[op.rd()] = (s32) ((u32) _registers[op.rs()]
					+ (u32) _registers[op.rt()]);
			break;
		case _SUB:
			_registers[op.rd()] = (s32) ((u32) _registers[op.rs()]
					- (u32) _registers[op.rt()]);
			break;
		case _SUBU:
			_registers[op.rd()] = (s32) ((u32) _registers[op.rs()]
					- (u32) _registers[op.rt()]);
			break;
		case _AND:
			_registers[op.rd()] = _registers[op.rs()] & _registers[op.rt()];
			break;
		case _OR:
			_registers[op.rd()] = _registers[op.rs()] | _registers[op.rt()];
			break;
		case _XOR:
			_registers[op.rd()] = _registers[op.rs()] ^ _registers[op.rt()];
			break;
		case _NOR:
			_registers[op.rd()] = ~(_registers[op.rs()] | _registers[op.rt()]);
			break;
		case _SLT:
			_registers[op.rd()] = ((s64) _registers[op.rs()]
					< (s64) _registers[op.rt()]);
			break;
		case _SLTU:
			_registers[op.rd()] = ((u64) _registers[op.rs()]
					< (u64) _registers[op.rt()]);
			break;
		case _DADD:
			_registers[op.rd()] = _registers[op.rs()] + _registers[op.rt()];
			break;
		case _DADDU:
			_registers[op.rd()] = _registers[op.rs()] + _registers[op.rt()];
			break;
		case _DSUB:
			_registers[op.rd()] = (_registers[op.rs()] - _registers[op.rt()]);
			break;
		case _DSUBU:
			_registers[op.rd()] = (_registers[op.rs()] - _registers[op.rt()]);
			break;
		case _TGE:
			UnhandledOpcode("Special: _TGE");
			break;
		case _TGEU:
			UnhandledOpcode("Special: _TGEU");
			break;
		case _TLT:
			UnhandledOpcode("Special: _TLT");
			break;
		case _TLTU:
			UnhandledOpcode("Special: _TLTU");
			break;
		case _TEQ:
			UnhandledOpcode("Special: _TEQ");
			break;
		case _TNE:
			UnhandledOpcode("Special: _TNE");
			break;
		case _DSLL:
			_registers[op.rd()] = _registers[op.rt()] << op.sa();
			break;
		case _DSRL:
			_registers[op.rd()] = (u64) _registers[op.rt()] >> op.sa();
			break;
		case _DSRA:
			_registers[op.rd()] = (s64) _registers[op.rt()] >> op.sa();
			break;
		case _DSLL32:
			_registers[op.rd()] = _registers[op.rt()] << (op.sa() + 32);
			break;
		case _DSRL32:
			_registers[op.rd()] = (u64) _registers[op.rt()] >> (op.sa() + 32);
			break;
		case _DSRA32:
			_registers[op.rd()] = (s64) _registers[op.rt()] >> (op.sa() + 32);
			break;
		}
		break;
	case _REGIMM:
		switch (op.rt()) {
		case _BLTZ:
			BranchCompareZero(op, LT, false, false);
			break;
		case _BGEZ:
			BranchCompareZero(op, GE, false, false);
			break;
		case _BLTZL:
			BranchCompareZero(op, LT, true, false);
			break;
		case _BGEZL:
			BranchCompareZero(op, GE, true, false);
			break;
		case _TGEI:
			break;
		case _TGEIU:
			break;
		case _TLTI:
			break;
		case _TLTIU:
			break;
		case _TEQI:
			break;
		case _TNEI:
			break;
		case _BLTZAL:
			BranchCompareZero(op, LT, false, true);
			break;
		case _BGEZAL:
			BranchCompareZero(op, GE, false, true);
			break;
		case _BLTZALL:
			BranchCompareZero(op, LT, true, true);
			break;
		case _BGEZALL:
			BranchCompareZero(op, GE, true, true);
			break;
		}
		break;
	case _J: {
		u32 TargetPC = (_vm->PC & 0xF0000000) | (op.target() << 2);
		if (TargetPC == _vm->PC) {// Infinite loop
			_vm->Cop0Registers.Count += 100;
		}
		ExecuteDelay();
		JumpTo(TargetPC);
		break;
	}
	case _JAL: {
		u32 TargetPC = (_vm->PC & 0xF0000000) | (op.target() << 2);
		if (TargetPC == _vm->PC) {// Infinite loop
			_vm->Cop0Registers.Count += 100;
		}
		SaveLinkRegister(31);
		ExecuteDelay();
		JumpTo(TargetPC);
		break;
	}
	case _BEQ:
		if (op.all == (0x1000FFFF)) { // Infinite loop
			ExecuteDelay();
			_vm->Cop0Registers.Count += 100;
			JumpTo(_vm->PC);
		} else {
			if ((op.rt() == 0) && (op.rs() == 0)) {
				ExecuteDelay();
				JumpTo((_vm->PC + 4 + ((u32) (s16) op.immediate() << 2)));
			} else {
				BranchCompareRegister(op, true, false);
			}
		}
		break;
	case _BNE:
		BranchCompareRegister(op, false, false);
		break;
	case _BLEZ:
		BranchCompareZero(op, LE, false, false);
		break;
	case _BGTZ:
		BranchCompareZero(op, GT, false, false);
		break;
	case _ADDI:
		_registers[op.rt()] = (s32) ((u32) _registers[op.rs()]
				+ (u32) (s16) op.immediate());
		break;
	case _ADDIU:
		_registers[op.rt()] = (s32) ((u32) _registers[op.rs()]
				+ (u32) (s16) op.immediate());
		break;
	case _SLTI:
		_registers[op.rt()] = (s64) _registers[op.rs()]
				< (s64) (s16) op.immediate();
		break;
	case _SLTIU:
		_registers[op.rt()] = (u64) _registers[op.rs()]
				< (u64) (s16) op.immediate();
		break;
	case _ANDI:
		_registers[op.rt()] = _registers[op.rs()] & (u16) op.immediate();
		break;
	case _ORI:
		_registers[op.rt()] = _registers[op.rs()] | (u16) op.immediate();
		break;
	case _XORI:
		_registers[op.rt()] = _registers[op.rs()] ^ (u16) op.immediate();
		break;
	case _LUI:
		_registers[op.rt()] = (s32) (((u32) (u16) op.immediate()) << 16);
		break;
	case _COP0:
		switch (op.rs()) {
		case _MFC0:
			switch (op.rd()) {
			case 0x00:
				_registers[op.rt()] = (s32) _vm->Cop0Registers.Index;
				break;
			case 0x01:
				_registers[op.rt()] = (s32) _vm->Cop0Registers.Random;
				break;
			case 0x02:
				_registers[op.rt()] = (s32) _vm->Cop0Registers.EntryLo0;
				break;
			case 0x03:
				_registers[op.rt()] = (s32) _vm->Cop0Registers.EntryLo1;
				break;
			case 0x04:
				_registers[op.rt()] = (s32) _vm->Cop0Registers.Context;
				break;
			case 0x05:
				_registers[op.rt()] = (s32) _vm->Cop0Registers.PageMask;
				break;
			case 0x06:
				_registers[op.rt()] = (s32) _vm->Cop0Registers.Wired;
				break;
			case 0x08:
				_registers[op.rt()] = (s32) _vm->Cop0Registers.BadVAddr;
				break;
			case 0x09:
				_registers[op.rt()] = (s32) _vm->Cop0Registers.Count;
				break;
			case 0x0A:
				_registers[op.rt()] = (s32) _vm->Cop0Registers.EntryHi;
				break;
			case 0x0B:
				_registers[op.rt()] = (s32) _vm->Cop0Registers.Compare;
				break;
			case 0x0C:
				_registers[op.rt()] = (s32) _vm->Cop0Registers.Status;
				break;
			case 0x0D:
				_registers[op.rt()] = (s32) _vm->Cop0Registers.Cause;
				break;
			case 0x0E:
				_registers[op.rt()] = (s32) _vm->Cop0Registers.ExceptPC;
				break;
			case 0x0F:
				_registers[op.rt()] = (s32) _vm->Cop0Registers.PRevID;
				break;
			case 0x10:
				_registers[op.rt()] = (s32) _vm->Cop0Registers.Config;
				break;
			case 0x11:
				_registers[op.rt()] = (s32) _vm->Cop0Registers.LLAddr;
				break;
			case 0x12:
				_registers[op.rt()] = (s32) _vm->Cop0Registers.WatchLo;
				break;
			case 0x13:
				_registers[op.rt()] = (s32) _vm->Cop0Registers.WatchHi;
				break;
			case 0x14:
				_registers[op.rt()] = (s32) _vm->Cop0Registers.XContext;
				break;
			case 0x1A:
				_registers[op.rt()] = (s32) _vm->Cop0Registers.PErr;
				break;
			case 0x1B:
				_registers[op.rt()] = (s32) _vm->Cop0Registers.CacheErr;
				break;
			case 0x1C:
				_registers[op.rt()] = (s32) _vm->Cop0Registers.TagLo;
				break;
			case 0x1D:
				_registers[op.rt()] = (s32) _vm->Cop0Registers.TagHi;
				break;
			case 0x1E:
				_registers[op.rt()] = (s32) _vm->Cop0Registers.ErrorEPC;
				break;
			}
			break;
		case _MTC0:
			switch (op.rd()) {
			case 0x00:
				_vm->Cop0Registers.Index = (u32) _registers[op.rt()];
				break;
			case 0x02:
				_vm->Cop0Registers.EntryLo0 = (u32) _registers[op.rt()];
				break;
			case 0x03:
				_vm->Cop0Registers.EntryLo1 = (u32) _registers[op.rt()];
				break;
			case 0x04:
				_vm->Cop0Registers.Context = (u32) _registers[op.rt()];
				break;
			case 0x05:
				_vm->Cop0Registers.PageMask = (u32) _registers[op.rt()];
				break;
			case 0x06:
				_vm->Cop0Registers.Wired = (u32) _registers[op.rt()];
				if (_vm->Cop0Registers.Random < _vm->Cop0Registers.Wired) {
					_vm->Cop0Registers.Random = _vm->Cop0Registers.Wired;
				}
				break;
			case 0x08:
				_vm->Cop0Registers.BadVAddr = (u32) _registers[op.rt()];
				break;
			case 0x09:
				_vm->Cop0Registers.Count = (u32) _registers[op.rt()];
				_vm->CompareCheck = _vm->Cop0Registers.Count
						< _vm->Cop0Registers.Compare;
				break;
			case 0x0A:
				_vm->Cop0Registers.EntryHi = (u32) _registers[op.rt()];
				break;
			case 0x0B:
				_vm->Cop0Registers.Compare = (u32) _registers[op.rt()];
				_vm->CompareCheck = _vm->Cop0Registers.Count
						< _vm->Cop0Registers.Compare;
				break;
			case 0x0C:
				_vm->Cop0Registers.Status = (u32) _registers[op.rt()];
				if ((_vm->Cop0Registers.Status & SR_FR) != 0) {
					logf("TODO: Set FPUMode to 64x64\n");
				}
				break;
			case 0x0D:
				_vm->Cop0Registers.Cause = (u32) _registers[op.rt()];
				break;
			case 0x0E:
				_vm->Cop0Registers.ExceptPC = (u32) _registers[op.rt()];
				break;
			case 0x0F:
				_vm->Cop0Registers.PRevID = (u32) _registers[op.rt()];
				break;
			case 0x10:
				_vm->Cop0Registers.Config = (u32) _registers[op.rt()];
				break;
			case 0x11:
				_vm->Cop0Registers.LLAddr = (u32) _registers[op.rt()];
				break;
			case 0x12:
				_vm->Cop0Registers.WatchLo = (u32) _registers[op.rt()];
				break;
			case 0x13:
				_vm->Cop0Registers.WatchHi = (u32) _registers[op.rt()];
				break;
			case 0x14:
				_vm->Cop0Registers.XContext = (u32) _registers[op.rt()];
				break;
			case 0x1A:
				_vm->Cop0Registers.PErr = (u32) _registers[op.rt()];
				break;
			case 0x1B:
				_vm->Cop0Registers.CacheErr = (u32) _registers[op.rt()];
				break;
			case 0x1C:
				_vm->Cop0Registers.TagLo = (u32) _registers[op.rt()];
				break;
			case 0x1D:
				_vm->Cop0Registers.TagHi = (u32) _registers[op.rt()];
				break;
			case 0x1E:
				_vm->Cop0Registers.ErrorEPC = (u32) _registers[op.rt()];
				break;
			}
			break;
		case _TLB:
			switch (op.funct()) {
			case _TLBR:
				_vm->TLBR();
				break;
			case _TLBWI:
				_vm->TLBWI();
				break;
			case _TLBWR:
				_vm->TLBWR();
				break;
			case _TLBP:
				_vm->TLBP();
				break;
			case _ERET:
				if (_vm->Cop0Registers.Status & SR_ERL) {
					_vm->Cop0Registers.Status &= ~SR_ERL;
					JumpTo(_vm->Cop0Registers.ErrorEPC);
				} else {
					_vm->Cop0Registers.Status &= ~SR_EXL;
					JumpTo(_vm->Cop0Registers.ExceptPC);
				}
				break;
			}
			break;
		default:
			UnhandledOpcode("Unhandled COP0\n");
			break;
		}
		break;
	case _COP1:
		switch (op.rs()) {
		case _MFC1:
			_registers[op.rt()] = (s32) (_vm->Cop1Registers.R32[op.fs()]);
			break;
		case _DMFC1:
			_registers[op.rt()] = _vm->Cop1Registers.R64[op.fs() >> 1];
			break;
		case _CFC1:
			switch (op.fs()) {
			case 0:
				_registers[op.rt()] = 0xA00;
				break;
			case 31:
				_registers[op.rt()] = (s32) _vm->Cop1Fcr31;
				break;
			default:
				UnhandledOpcode("CFC1 with invalid register");
				break;
			}
			break;
		case _MTC1:
			_vm->Cop1Registers.R32[op.fs()] = (u32) _registers[op.rt()];
			break;
		case _DMTC1:
			_vm->Cop1Registers.R64[op.fs() >> 1] = _registers[op.rt()];
			break;
		case _CTC1:
			switch (op.fs()) {
			case 31:
				_vm->Cop1Fcr31 = (u32) _registers[op.rt()];
				break;
			default:
				UnhandledOpcode("CTC1 with invalid register");
				break;
			}
			break;
		case _BC1: {
			bool likely = (op.rt() == _BC1_FL) || (op.rt() == _BC1_TL);
			bool unset = ((op.rt() == _BC1_F) || (op.rt() == _BC1_FL));

			if ((unset) ^ ((_vm->Cop1Fcr31 & FPCSR_C) != 0)) {
				ExecuteDelay();
				u32 targetPC = _vm->PC + 4 + ((u32) (s16) op.immediate() << 2);
				JumpTo(targetPC);
			} else {
				if (likely) {
					JumpTo(_vm->PC + 8);
				}
			}
			break;
		}
		case _COP1_S:
			switch (op.funct()) {
			case _COP1_ADD:
				_vm->Cop1Registers.F32[op.fd()]
						= _vm->Cop1Registers.F32[op.fs()]
								+ _vm->Cop1Registers.F32[op.ft()];
				break;
			case _COP1_SUB:
				_vm->Cop1Registers.F32[op.fd()]
						= _vm->Cop1Registers.F32[op.fs()]
								- _vm->Cop1Registers.F32[op.ft()];
				break;
			case _COP1_MUL:
				_vm->Cop1Registers.F32[op.fd()]
						= _vm->Cop1Registers.F32[op.fs()]
								* _vm->Cop1Registers.F32[op.ft()];
				break;
			case _COP1_DIV:
				_vm->Cop1Registers.F32[op.fd()]
						= _vm->Cop1Registers.F32[op.fs()]
								/ _vm->Cop1Registers.F32[op.ft()];
				break;
			case _COP1_SQRT:
				_vm->Cop1Registers.F32[op.fd()] = sqrtf(
						_vm->Cop1Registers.F32[op.fs()]);
				break;
			case _COP1_ABS:
				_vm->Cop1Registers.F32[op.fd()] = fabsf(
						_vm->Cop1Registers.F32[op.fs()]);
				break;
			case _COP1_MOV:
				_vm->Cop1Registers.F32[op.fd()]
						= _vm->Cop1Registers.F32[op.fs()];
				break;
			case _COP1_NEG:
				_vm->Cop1Registers.F32[op.fd()]
						= -(_vm->Cop1Registers.F32[op.fs()]);
				break;
			case _COP1_ROUND_L:
				_vm->Cop1Registers.R64[op.fd() >> 1]
						= (s64) _vm->Cop1Registers.F32[op.fs()];
				break;
			case _COP1_TRUNC_L:
				_vm->Cop1Registers.R64[op.fd() >> 1]
						= (s64) _vm->Cop1Registers.F32[op.fs()];
				break;
			case _COP1_CEIL_L:
				_vm->Cop1Registers.R64[op.fd() >> 1]
						= (s64) _vm->Cop1Registers.F32[op.fs()];
				break;
			case _COP1_FLOOR_L:
				_vm->Cop1Registers.R64[op.fd() >> 1]
						= (s64) _vm->Cop1Registers.F32[op.fs()];
				break;
			case _COP1_ROUND_W:
				_vm->Cop1Registers.R32[op.fd()]
						= (s32) _vm->Cop1Registers.F32[op.fs()];
				break;
			case _COP1_TRUNC_W:
				_vm->Cop1Registers.R32[op.fd()]
						= (s32) _vm->Cop1Registers.F32[op.fs()];
				break;
			case _COP1_CEIL_W:
				_vm->Cop1Registers.R32[op.fd()]
						= (s32) _vm->Cop1Registers.F32[op.fs()];
				break;
			case _COP1_FLOOR_W:
				_vm->Cop1Registers.R32[op.fd()]
						= (s32) _vm->Cop1Registers.F32[op.fs()];
				break;
			case _COP1_CVT_D:
				_vm->Cop1Registers.F64[op.fd() >> 1]
						= (double) _vm->Cop1Registers.F32[op.fs()];
				break;
			case _COP1_CVT_W:
				_vm->Cop1Registers.R32[op.fd()]
						= (s32) _vm->Cop1Registers.F32[op.fs()];
				break;
			case _COP1_CVT_L:
				_vm->Cop1Registers.R64[op.fd() >> 1]
						= (s64) _vm->Cop1Registers.F32[op.fs()];
				break;
			case _COP1_C_F:
				_vm->Cop1Fcr31 &= ~FPCSR_C;
				break;
			case _COP1_C_UN:
				_vm->Cop1Fcr31 &= ~FPCSR_C;
				break;
			case _COP1_C_EQ:
				if (_vm->Cop1Registers.F32[op.fs()]
						== _vm->Cop1Registers.F32[op.ft()]) {
					_vm->Cop1Fcr31 |= FPCSR_C;
				} else {
					_vm->Cop1Fcr31 &= ~FPCSR_C;
				}
				break;
			case _COP1_C_UEQ:
				if (_vm->Cop1Registers.F32[op.fs()]
						== _vm->Cop1Registers.F32[op.ft()]) {
					_vm->Cop1Fcr31 |= FPCSR_C;
				} else {
					_vm->Cop1Fcr31 &= ~FPCSR_C;
				}
				break;
			case _COP1_C_OLT:
				if (_vm->Cop1Registers.F32[op.fs()]
						< _vm->Cop1Registers.F32[op.ft()]) {
					_vm->Cop1Fcr31 |= FPCSR_C;
				} else {
					_vm->Cop1Fcr31 &= ~FPCSR_C;
				}
				break;
			case _COP1_C_ULT:
				if (_vm->Cop1Registers.F32[op.fs()]
						< _vm->Cop1Registers.F32[op.ft()]) {
					_vm->Cop1Fcr31 |= FPCSR_C;
				} else {
					_vm->Cop1Fcr31 &= ~FPCSR_C;
				}
				break;
			case _COP1_C_OLE:
				if (_vm->Cop1Registers.F32[op.fs()]
						<= _vm->Cop1Registers.F32[op.ft()]) {
					_vm->Cop1Fcr31 |= FPCSR_C;
				} else {
					_vm->Cop1Fcr31 &= ~FPCSR_C;
				}
				break;
			case _COP1_C_ULE:
				if (_vm->Cop1Registers.F32[op.fs()]
						<= _vm->Cop1Registers.F32[op.ft()]) {
					_vm->Cop1Fcr31 |= FPCSR_C;
				} else {
					_vm->Cop1Fcr31 &= ~FPCSR_C;
				}
				break;
			case _COP1_C_SF:
				_vm->Cop1Fcr31 &= ~FPCSR_C;
				break;
			case _COP1_C_NGLE:
				UnhandledOpcode("C.SEQ.S unhandled");
				break;
			case _COP1_C_SEQ:
				if (_vm->Cop1Registers.F32[op.fs()]
						== _vm->Cop1Registers.F32[op.ft()]) {
					_vm->Cop1Fcr31 |= FPCSR_C;
				} else {
					_vm->Cop1Fcr31 &= ~FPCSR_C;
				}
				break;
			case _COP1_C_NGL:
				if (_vm->Cop1Registers.F32[op.fs()]
						== _vm->Cop1Registers.F32[op.ft()]) {
					_vm->Cop1Fcr31 |= FPCSR_C;
				} else {
					_vm->Cop1Fcr31 &= ~FPCSR_C;
				}
				break;
			case _COP1_C_LT:
				if (_vm->Cop1Registers.F32[op.fs()]
						< _vm->Cop1Registers.F32[op.ft()]) {
					_vm->Cop1Fcr31 |= FPCSR_C;
				} else {
					_vm->Cop1Fcr31 &= ~FPCSR_C;
				}
				break;
			case _COP1_C_NGE:
				if (_vm->Cop1Registers.F32[op.fs()]
						< _vm->Cop1Registers.F32[op.ft()]) {
					_vm->Cop1Fcr31 |= FPCSR_C;
				} else {
					_vm->Cop1Fcr31 &= ~FPCSR_C;
				}
				break;
			case _COP1_C_LE:
				if (_vm->Cop1Registers.F32[op.fs()]
						<= _vm->Cop1Registers.F32[op.ft()]) {
					_vm->Cop1Fcr31 |= FPCSR_C;
				} else {
					_vm->Cop1Fcr31 &= ~FPCSR_C;
				}
				break;
			case _COP1_C_NGT:
				if (_vm->Cop1Registers.F32[op.fs()] <= _vm->Cop1Registers.F32[op.ft()]){
					_vm->Cop1Fcr31 |= FPCSR_C;
				} else {
					_vm->Cop1Fcr31 &= ~FPCSR_C;
				}
				break;
			default:
				UnhandledOpcode("???.S unhandled");
				break;
			}
			break;
		case _COP1_D:
			switch (op.funct()) {
			case _COP1_ADD:
				_vm->Cop1Registers.F64[op.fd() >> 1]
						= _vm->Cop1Registers.F64[op.fs() >> 1]
								+ _vm->Cop1Registers.F64[op.ft() >> 1];
				break;
			case _COP1_SUB:
				_vm->Cop1Registers.F64[op.fd() >> 1]
						= _vm->Cop1Registers.F64[op.fs() >> 1]
								- _vm->Cop1Registers.F64[op.ft() >> 1];
				break;
			case _COP1_MUL:
				_vm->Cop1Registers.F64[op.fd() >> 1]
						= _vm->Cop1Registers.F64[op.fs() >> 1]
								* _vm->Cop1Registers.F64[op.ft() >> 1];
				break;
			case _COP1_DIV:
				_vm->Cop1Registers.F64[op.fd() >> 1]
						= _vm->Cop1Registers.F64[op.fs() >> 1]
								/ _vm->Cop1Registers.F64[op.ft() >> 1];
				break;
			case _COP1_SQRT:
				_vm->Cop1Registers.F64[op.fd() >> 1] = sqrt(
						_vm->Cop1Registers.F64[op.fs() >> 1]);
				break;
			case _COP1_ABS:
				_vm->Cop1Registers.F64[op.fd() >> 1] = fabs(
						_vm->Cop1Registers.F64[op.fs() >> 1]);
				break;
			case _COP1_MOV:
				_vm->Cop1Registers.F64[op.fd() >> 1]
						= _vm->Cop1Registers.F64[op.fs() >> 1];
				break;
			case _COP1_NEG:
				_vm->Cop1Registers.F64[op.fd() >> 1]
						= -(_vm->Cop1Registers.F64[op.fs() >> 1]);
				break;
			case _COP1_ROUND_L:
				_vm->Cop1Registers.R64[op.fd() >> 1]
						= (s64) _vm->Cop1Registers.F64[op.fs() >> 1];
				break;
			case _COP1_TRUNC_L:
				_vm->Cop1Registers.R64[op.fd() >> 1]
						= (s64) _vm->Cop1Registers.F64[op.fs() >> 1];
				break;
			case _COP1_CEIL_L:
				_vm->Cop1Registers.R64[op.fd() >> 1]
						= (s64) _vm->Cop1Registers.F64[op.fs() >> 1];
				break;
			case _COP1_FLOOR_L:
				_vm->Cop1Registers.R64[op.fd() >> 1]
						= (s64) _vm->Cop1Registers.F64[op.fs() >> 1];
				break;
			case _COP1_ROUND_W:
				_vm->Cop1Registers.R32[op.fd()]
						= (s32) _vm->Cop1Registers.F64[op.fs() >> 1];
				break;
			case _COP1_TRUNC_W:
				_vm->Cop1Registers.R32[op.fd()]
						= (s32) _vm->Cop1Registers.F64[op.fs() >> 1];
				break;
			case _COP1_CEIL_W:
				_vm->Cop1Registers.R32[op.fd()]
						= (s32) _vm->Cop1Registers.F64[op.fs() >> 1];
				break;
			case _COP1_FLOOR_W:
				_vm->Cop1Registers.R32[op.fd()]
						= (s32) _vm->Cop1Registers.F64[op.fs() >> 1];
				break;
			case _COP1_CVT_S:
				_vm->Cop1Registers.F32[op.fd()]
						= (float) _vm->Cop1Registers.F64[op.fs() >> 1];
				break;
			case _COP1_CVT_W:
				_vm->Cop1Registers.R32[op.fd()]
						= (s32) _vm->Cop1Registers.F64[op.fs() >> 1];
				break;
			case _COP1_CVT_L:
				_vm->Cop1Registers.R64[op.fd() >> 1]
						= (s64) _vm->Cop1Registers.F64[op.fs() >> 1];
				break;
			case _COP1_C_F:
				_vm->Cop1Fcr31 &= ~FPCSR_C;
				break;
			case _COP1_C_UN:
				_vm->Cop1Fcr31 &= ~FPCSR_C;
				break;
			case _COP1_C_EQ:
				if (_vm->Cop1Registers.F64[op.fs() >> 1]
						== _vm->Cop1Registers.F64[op.ft() >> 1]) {
					_vm->Cop1Fcr31 |= FPCSR_C;
				} else {
					_vm->Cop1Fcr31 &= ~FPCSR_C;
				}
				break;
			case _COP1_C_UEQ:
				if (_vm->Cop1Registers.F64[op.fs() >> 1]
						== _vm->Cop1Registers.F64[op.ft() >> 1]) {
					_vm->Cop1Fcr31 |= FPCSR_C;
				} else {
					_vm->Cop1Fcr31 &= ~FPCSR_C;
				}
				break;
			case _COP1_C_OLT:
				if (_vm->Cop1Registers.F64[op.fs() >> 1]
						< _vm->Cop1Registers.F64[op.ft() >> 1]) {
					_vm->Cop1Fcr31 |= FPCSR_C;
				} else {
					_vm->Cop1Fcr31 &= ~FPCSR_C;
				}
				break;
			case _COP1_C_ULT:
				if (_vm->Cop1Registers.F64[op.fs() >> 1]
						< _vm->Cop1Registers.F64[op.ft() >> 1]) {
					_vm->Cop1Fcr31 |= FPCSR_C;
				} else {
					_vm->Cop1Fcr31 &= ~FPCSR_C;
				}
				break;
			case _COP1_C_OLE:
				if (_vm->Cop1Registers.F64[op.fs() >> 1]
						<= _vm->Cop1Registers.F64[op.ft() >> 1]) {
					_vm->Cop1Fcr31 |= FPCSR_C;
				} else {
					_vm->Cop1Fcr31 &= ~FPCSR_C;
				}
				break;
			case _COP1_C_ULE:
				if (_vm->Cop1Registers.F64[op.fs() >> 1]
						<= _vm->Cop1Registers.F64[op.ft() >> 1]) {
					_vm->Cop1Fcr31 |= FPCSR_C;
				} else {
					_vm->Cop1Fcr31 &= ~FPCSR_C;
				}
				break;
			case _COP1_C_SF:
				_vm->Cop1Fcr31 &= ~FPCSR_C;
				break;
			case _COP1_C_NGLE:
				_vm->Cop1Fcr31 &= ~FPCSR_C;
				break;
			case _COP1_C_SEQ:
				if (_vm->Cop1Registers.F64[op.fs() >> 1]
						== _vm->Cop1Registers.F64[op.ft() >> 1]) {
					_vm->Cop1Fcr31 |= FPCSR_C;
				} else {
					_vm->Cop1Fcr31 &= ~FPCSR_C;
				}
				break;
			case _COP1_C_NGL:
				if (_vm->Cop1Registers.F64[op.fs() >> 1]
						== _vm->Cop1Registers.F64[op.ft() >> 1]) {
					_vm->Cop1Fcr31 |= FPCSR_C;
				} else {
					_vm->Cop1Fcr31 &= ~FPCSR_C;
				}
				break;
			case _COP1_C_LT:
				if (_vm->Cop1Registers.F64[op.fs() >> 1]
						< _vm->Cop1Registers.F64[op.ft() >> 1]) {
					_vm->Cop1Fcr31 |= FPCSR_C;
				} else {
					_vm->Cop1Fcr31 &= ~FPCSR_C;
				}
				break;
			case _COP1_C_NGE:
				if (_vm->Cop1Registers.F64[op.fs() >> 1]
						< _vm->Cop1Registers.F64[op.ft() >> 1]) {
					_vm->Cop1Fcr31 |= FPCSR_C;
				} else {
					_vm->Cop1Fcr31 &= ~FPCSR_C;
				}
				break;
			case _COP1_C_LE:
				if (_vm->Cop1Registers.F64[op.fs() >> 1]
						<= _vm->Cop1Registers.F64[op.ft() >> 1]) {
					_vm->Cop1Fcr31 |= FPCSR_C;
				} else {
					_vm->Cop1Fcr31 &= ~FPCSR_C;
				}
				break;
			case _COP1_C_NGT:
				if (_vm->Cop1Registers.F64[op.fs() >> 1]
						<= _vm->Cop1Registers.F64[op.ft() >> 1]) {
					_vm->Cop1Fcr31 |= FPCSR_C;
				} else {
					_vm->Cop1Fcr31 &= ~FPCSR_C;
				}
				break;
			default:
				UnhandledOpcode("COP1.D ??");
				break;
			}
			break;
		case _COP1_W:
			switch (op.funct()) {
			case _COP1_CVT_S:
				_vm->Cop1Registers.F32[op.fd()]
						= (float) (s32) _vm->Cop1Registers.R32[op.fs()];
				break;
			case _COP1_CVT_D:
				_vm->Cop1Registers.F64[op.fd() >> 1]
						= (double) (s32) _vm->Cop1Registers.R32[op.fs()];
				break;
			default:
				UnhandledOpcode("? COP1 - W");
				break;
			}
			break;
		case _COP1_L:
			UnhandledOpcode("L");
			break;
		default:
			UnhandledOpcode("Unhandled COP1\n");
			break;
		}
		break;
	case _BEQL:
		BranchCompareRegister(op, true, true);
		break;
	case _BNEL:
		BranchCompareRegister(op, false, true);
		break;
	case _BLEZL:
		BranchCompareZero(op, LE, true, false);
		break;
	case _BGTZL:
		BranchCompareZero(op, GT, true, false);
		break;
	case _DADDI:
		_registers[op.rt()] = _registers[op.rs()] + (u64) (s16) op.immediate();
		break;
	case _DADDIU:
		_registers[op.rt()] = _registers[op.rs()] + (u64) (s16) op.immediate();
		break;
	case _LDL:
		UnhandledOpcode("LDL");
		break;
	case _LDR:
		UnhandledOpcode("LDR");
		break;
	case _LB:
		_registers[op.rt()] = (u64) (s8) _vm->ReadMem8(_vm->Map(
				(u32) _registers[op.rs()] + (u32) (s16) op.immediate()));
		break;
	case _LH:
		_registers[op.rt()] = (u64) (s16) _vm->ReadMem16(_vm->Map(
				(u32) _registers[op.rs()] + (u32) (s16) op.immediate()));
		break;
	case _LWL: {
		u32 addr = _vm->Map((u32) _registers[op.rs()]
				+ (u32) (s16) op.immediate());
		u32 memContents = _vm->ReadMem32(addr & ~BitM2);
		u32 addr2 = (addr & BitM2) << 3;
		_registers[op.rt()] = (s32) ((memContents << addr2)
				| ((u32) _registers[op.rt()] & (0x00FFFFFF >> (24 - addr2))));
		break;
	}
	case _LW:
		_registers[op.rt()] = (u64) (s32) _vm->ReadMem32(_vm->Map(
				(u32) _registers[op.rs()] + (u32) (s16) op.immediate()));
		break;
	case _LBU:
		_registers[op.rt()] = (u64) (u8) _vm->ReadMem8(_vm->Map(
				(u32) _registers[op.rs()] + (u32) (s16) op.immediate()));
		break;
	case _LHU:
		_registers[op.rt()] = (u64) (u16) _vm->ReadMem16(_vm->Map(
				(u32) _registers[op.rs()] + (u32) (s16) op.immediate()));
		break;
	case _LWR: {
		u32 addr = _vm->Map((u32) _registers[op.rs()]
				+ (u32) (s16) op.immediate());
		u32 memContents = _vm->ReadMem32(addr & ~BitM2);
		u32 addr2 = (addr & BitM2) << 3;
		_registers[op.rt()] = (s32) ((memContents >> (24 - addr2))
				| ((u32) _registers[op.rt()] & (0xFFFFFF00 << addr2)));
		break;
	}
	case _LWU:
		_registers[op.rt()] = (u64) (u32) _vm->ReadMem32(_vm->Map(
				(u32) _registers[op.rs()] + (u32) (s16) op.immediate()));
		break;
	case _SB:
		_vm->WriteMem8(_vm->Map((u32) _registers[op.rs()]
				+ (u32) (s16) op.immediate()), (u8) _registers[op.rt()]);
		break;
	case _SH:
		_vm->WriteMem16(_vm->Map((u32) _registers[op.rs()]
				+ (u32) (s16) op.immediate()), (u16) _registers[op.rt()]);
		break;
	case _SWL: {
		u32 addr = _vm->Map((u32) _registers[op.rs()]
				+ (u32) (s16) op.immediate());
		u32 oldContents = _vm->ReadMem32(addr & ~BitM2);
		u32 addr2 = (addr & BitM2) << 3;
		u32 newContents = (oldContents & (0xFFFFFF00 << (24 - addr2))
				| ((u32) _registers[op.rt()] >> addr2));
		_vm->WriteMem32(addr & ~BitM2, newContents);
		break;
	}
	case _SW:
		_vm->WriteMem32(_vm->Map((u32) _registers[op.rs()]
				+ (u32) (s16) op.immediate()), (u32) _registers[op.rt()]);
		break;
	case _SDL:
		UnhandledOpcode("SDL");
		break;
	case _SDR:
		UnhandledOpcode("SDR");
		break;
	case _SWR: {
		u32 addr = _vm->Map((u32) _registers[op.rs()]
				+ (u32) (s16) op.immediate());
		u32 oldContents = _vm->ReadMem32(addr & ~BitM2);
		u32 addr2 = (addr & BitM2) << 3;
		u32 newContents = (oldContents & (0x00FFFFFF >> (addr2))
				| ((u32) _registers[op.rt()] << (24 - addr2)));
		_vm->WriteMem32(addr & ~BitM2, newContents);
		break;
	}
	case _CACHE:
		//UnhandledOpcode("CACHE");
		break;
	case _LL:
		UnhandledOpcode("LL");
		break;
	case _LWC1:
		_vm->Cop1Registers.R32[op.rt()] = _vm->ReadMem32(_vm->Map(
				(u32) _registers[op.rs()] + (u32) (s16) op.immediate()));
		break;
	case _LLD:
		UnhandledOpcode("LLD");
		break;
	case _LDC1:
		_vm->Cop1Registers.R64[op.rt() >> 1] = _vm->ReadMem64(_vm->Map(
				(u32) _registers[op.rs()] + (u32) (s16) op.immediate()));
		break;
	case _LD:
		_registers[op.rt()] = _vm->ReadMem64(_vm->Map((u32) _registers[op.rs()]
				+ (u32) (s16) op.immediate()));
		break;
	case _SC:
		UnhandledOpcode("SC");
		break;
	case _SWC1:
		_vm->WriteMem32(_vm->Map((u32) _registers[op.rs()]
				+ (u32) (s16) op.immediate()), _vm->Cop1Registers.R32[op.rt()]);
		break;
	case _SCD:
		UnhandledOpcode("SCD");
		break;
	case _SDC1:
		_vm->WriteMem64(_vm->Map((u32) _registers[op.rs()]
				+ (u32) (s16) op.immediate()), _vm->Cop1Registers.R32[op.rt()
				& ~1], _vm->Cop1Registers.R32[op.rt() | 1]);
		break;
	case _SD:
		_vm->WriteMem64(_vm->Map((u32) _registers[op.rs()]
				+ (u32) (s16) op.immediate()), (u32) _registers[op.rt()],
				(u32) (_registers[op.rt()] >> 32));
		break;
	}
}
