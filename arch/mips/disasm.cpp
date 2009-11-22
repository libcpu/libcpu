#include "libcpu.h"
#include "cpu_generic.h"
#include "mips_internal.h"

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

typedef u64 TRegister;

typedef TRegister TRegisters[32];

typedef union {
	u32 ByIndex[32];
	struct {
		u32 Index;
		u32 Random; // alias NextFreeTLBEntry...?
		u32 EntryLo0;
		u32 EntryLo1;
		u32 Context;
		u32 PageMask;
		u32 Wired;
		u32 Res0;
		u32 BadVAddr;
		u32 Count;
		u32 EntryHi;
		u32 Compare;
		u32 Status;
		u32 Cause;
		u32 ExceptPC;
		u32 PRevID;
		u32 Config;
		u32 LLAddr;
		u32 WatchLo;
		u32 WatchHi;
		u32 XContext;
		u32 Res1;
		u32 Res2;
		u32 Res3;
		u32 Res4;
		u32 Res5;
		u32 PErr;
		u32 CacheErr;
		u32 TagLo;
		u32 TagHi;
		u32 ErrorEPC;
		u32 Res6;
	};
} TCOP0Registers;

typedef union {
	u32 R32[32];
	float F32[32];
	u64 R64[16];
	double F64[16];
} TCOP1Registers;

typedef union {
	struct {
		u32 lo;
		u32 hi;
	} R32;
	u64 R64;
} TCOP1Registers64[64];

typedef struct {
	u32 ll;
	u32 hl;
	u32 lh;
	u32 PageMask;
	u32 Valid0;
	u32 Valid1;
	u32 Global;
	u32 Dirty0;
	u32 Dirty1;
	u32 Asid;
	u32 VPN;
	u32 PFN0;
	u32 PFN1;
	u32 AddressVPNShift;
	u32 AddressVPNMask;
	u32 PFNShift;
	u32 OffsetMask;
} TTLB;

typedef struct {
	u32 MODE_REG;
	u32 CONFIG_REG;
	u32 CURRENT_LOAD_REG;
	u32 SELECT_REG;
	u32 REFRESH_REG;
	u32 LATENCY_REG;
	u32 RERROR_REG;
	u32 WERROR_REG;
} TRIReg;

typedef struct {
	u32 MEM_ADDR_REG;
	u32 DRAM_ADDR_REG;
	u32 RD_LEN_REG;
	u32 WR_LEN_REG;
	u32 STATUS_REG;
	u32 SEMAPHORE_REG;
	u32 PC_REG;
	u32 IBIST_REG;
} TSPReg;

typedef struct {
	u32 MODE_REG;
	u32 INTR_MASK_REG;
	u32 INTR_REG;
} TMIReg;

typedef struct {
	u32 DRAM_ADDR_REG;
	u32 DBUF_DRAM_ADDR_REG;
	u32 LEN_REG;
	u32 DBUF_LEN_REG;
	u32 CONTROL_REG;
	u32 STATUS_REG;
	u32 DACRATE_REG;
	u32 BITRATE_REG;
	u32 DBUF;
} TAIReg;

typedef struct {
	u32 DRAM_ADDR_REG;
} TSIReg;

typedef struct {
	u32 CONFIG_REG;
	u32 DEVICE_ID_REG;
	u32 DELAY_REG;
	u32 MODE_REG;
	u32 REF_INTERVAL_REG;
	u32 REF_ROW_REG;
	u32 RAS_INTERVAL_REG;
	u32 MIN_INTERVAL_REG;
	u32 ADDR_SELECT_REG;
	u32 DEVICE_MANUF_REG;
} TRDRAMReg;

typedef struct {
	u32 STATUS_REG;
	u32 DRAM_ADDR_REG;
	u32 H_WIDTH_REG;
	u32 V_INTR_REG;
	u32 V_CURRENT_LINE_REG;
	u32 TIMING_REG;
	u32 V_SYNC_REG;
	u32 H_SYNC_REG;
	u32 H_SYNC_LEAP_REG;
	u32 H_VIDEO_REG;
	u32 V_VIDEO_REG;
	u32 V_BURST_REG;
	u32 X_SCALE_REG;
	u32 Y_SCALE_REG;
} TVIReg;

typedef struct {
	u32 DRAM_ADDR_REG;
	u32 CART_ADDR_REG;
	u32 RD_LEN_REG;
	u32 WR_LEN_REG;
	u32 STATUS_REG;
	u32 BSD_DOM1_LAT_REG;
	u32 BSD_DOM1_PWD_REG;
	u32 BSD_DOM1_PGS_REG;
	u32 BSD_DOM1_RLS_REG;
	u32 BSD_DOM2_LAT_REG;
	u32 BSD_DOM2_PWD_REG;
	u32 BSD_DOM2_PGS_REG;
	u32 BSD_DOM2_RLS_REG;
} TPIReg;

typedef struct {
	u32 START_REG;
	u32 END_REG;
	u32 CURRENT_REG;
	u32 STATUS_REG;
	u32 CLOCK_REG;
	u32 BUFBUSY_REG;
	u32 PIPEBUSY_REG;
	u32 TMEM_REG;
} TDPCReg;

// PifCommand
typedef struct Comnd_s {
	int txmax, rxmax;
} Comnd;

typedef struct TController_s {
	bool PluggedIn;
	bool MemPack;
	bool HasRumble;
	bool Rumble;
	u32 KeyState;
} TController;

const int BitM1 = 0x1;
const int BitM2 = 0x3;
const int BitM3 = 0x7;
const int BitM4 = 0xF;
const int BitM5 = 0x1F;
const int BitM6 = 0x3F;
const int BitM7 = 0x7F;
const int BitM8 = 0xFF;
const int BitM9 = 0x1FF;
const int BitM10 = 0x3FF;
const int BitM11 = 0x7FF;
const int BitM12 = 0xFFF;
const int BitM13 = 0x1FFF;
const int BitM14 = 0x3FFF;
const int BitM15 = 0x7FFF;
const int BitM16 = 0xFFFF;
const int BitM17 = 0x1FFFF;
const int BitM18 = 0x3FFFF;
const int BitM19 = 0x7FFFF;
const int BitM20 = 0xFFFFF;
const int BitM21 = 0x1FFFFF;
const int BitM22 = 0x3FFFFF;
const int BitM23 = 0x7FFFFF;
const int BitM24 = 0xFFFFFF;
const int BitM25 = 0x1FFFFFF;
const int BitM26 = 0x3FFFFFF;
const int BitM27 = 0x7FFFFFF;
const int BitM28 = 0xFFFFFFF;
const int BitM29 = 0x1FFFFFFF;
const int BitM30 = 0x3FFFFFFF;
const int BitM31 = 0x7FFFFFFF;
const int BitM32 = 0xFFFFFFFF;

const int Bit1 = 0x1;
const int Bit2 = 0x2;
const int Bit3 = 0x4;
const int Bit4 = 0x8;
const int Bit5 = 0x10;
const int Bit6 = 0x20;
const int Bit7 = 0x40;
const int Bit8 = 0x80;
const int Bit9 = 0x100;
const int Bit10 = 0x200;
const int Bit11 = 0x400;
const int Bit12 = 0x800;
const int Bit13 = 0x1000;
const int Bit14 = 0x2000;
const int Bit15 = 0x4000;
const int Bit16 = 0x8000;
const int Bit17 = 0x10000;
const int Bit18 = 0x20000;
const int Bit19 = 0x40000;
const int Bit20 = 0x80000;
const int Bit21 = 0x100000;
const int Bit22 = 0x200000;
const int Bit23 = 0x400000;
const int Bit24 = 0x800000;
const int Bit25 = 0x1000000;
const int Bit26 = 0x2000000;
const int Bit27 = 0x4000000;
const int Bit28 = 0x8000000;
const int Bit29 = 0x10000000;
const int Bit30 = 0x20000000;
const int Bit31 = 0x40000000;
const int Bit32 = 0x80000000;

const int KB = 1024;
const int MB = (1024 * KB);
const int GB = (1024 * MB);

enum TComparison {
	GT, GE, LT, LE
};

// this has to be implemented by our concrete platform (e.g. android)
extern int logf(const char *format, ...);
extern int logef(const char *format, ...);

typedef struct {
	u32 all;

	inline u32 immediate() {
		return all & BitM16;
	}

	inline u32 rt() {
		return all >> 16 & BitM5;
	}

	inline u32 rs() {
		return all >> 21 & BitM5;
	}

	inline u32 op() {
		return all >> 26 & BitM6;
	}

	inline u32 target() {
		return all & BitM26;
	}

	inline u32 funct() {
		return all & BitM6;
	}

	inline u32 sa() {
		return all >> 6 & BitM5;
	}

	inline u32 rd() {
		return all >> 11 & BitM5;
	}

	inline u32 fd() {
		return all >> 6 & BitM5;
	}

	inline u32 fs() {
		return all >> 11 & BitM5;
	}

	inline u32 ft() {
		return all >> 16 & BitM5;
	}
} TOpcode;

const std::string OpcodeGPRNames[] = { "R0", "AT", "V0", "V1", "A0", "A1",
		"A2", "A3", "T0", "T1", "T2", "T3", "T4", "T5", "T6", "T7", "S0", "S1",
		"S2", "S3", "S4", "S5", "S6", "S7", "T8", "T9", "K0", "K1", "GP", "SP",
		"S8", "RA" };

const std::string OpcodeCOP0RegNames[] = { "Index", "Random", "EntryLo0",
		"EntryLo1", "Context", "PageMask", "Wired", "reserved", "BadVAddr",
		"Count", "EntryHi", "Compare", "Status", "Cause", "ExceptPC",
		"reserved", "reserved", "reserved", "reserved", "reserved", "PRevID",
		"Config", "LLAddr", "WatchLo", "WatchHi", "XContext", "PErr",
		"CacheErr", "TagLo", "TagHi", "ErrorEPC", "reserved" };

// Main instructions...
const char _SPECIAL = 0;
const char _REGIMM = 1;
const char _J = 2;
const char _JAL = 3;
const char _BEQ = 4;
const char _BNE = 5;
const char _BLEZ = 6;
const char _BGTZ = 7;
const char _ADDI = 8;
const char _ADDIU = 9;
const char _SLTI = 10;
const char _SLTIU = 11;
const char _ANDI = 12;
const char _ORI = 13;
const char _XORI = 14;
const char _LUI = 15;
const char _COP0 = 16;
const char _COP1 = 17;
const char _BEQL = 20;
const char _BNEL = 21;
const char _BLEZL = 22;
const char _BGTZL = 23;
const char _DADDI = 24;
const char _DADDIU = 25;
const char _LDL = 26;
const char _LDR = 27;
const char _LB = 32;
const char _LH = 33;
const char _LWL = 34;
const char _LW = 35;
const char _LBU = 36;
const char _LHU = 37;
const char _LWR = 38;
const char _LWU = 39;
const char _SB = 40;
const char _SH = 41;
const char _SWL = 42;
const char _SW = 43;
const char _SDL = 44;
const char _SDR = 45;
const char _SWR = 46;
const char _CACHE = 47;
const char _LL = 48;
const char _LWC1 = 49;
const char _LLD = 52;
const char _LDC1 = 53;
const char _LD = 55;
const char _SC = 56;
const char _SWC1 = 57;
const char _SCD = 60;
const char _SDC1 = 61;
const char _SD = 63;

// Special Instructions
const char _SLL = 0;
const char _SRL = 2;
const char _SRA = 3;
const char _SLLV = 4;
const char _SRLV = 6;
const char _SRAV = 7;
const char _JR = 8;
const char _JALR = 9;
const char _SYSCALL = 12;
const char _BREAK = 13;
const char _SYNC = 15;
const char _MFHI = 16;
const char _MTHI = 17;
const char _MFLO = 18;
const char _MTLO = 19;
const char _DSLLV = 20;
const char _DSRLV = 22;
const char _DSRAV = 23;
const char _MULT = 24;
const char _MULTU = 25;
const char _DIV = 26;
const char _DIVU = 27;
const char _DMULT = 28;
const char _DMULTU = 29;
const char _DDIV = 30;
const char _DDIVU = 31;
const char _ADD = 32;
const char _ADDU = 33;
const char _SUB = 34;
const char _SUBU = 35;
const char _AND = 36;
const char _OR = 37;
const char _XOR = 38;
const char _NOR = 39;
const char _SLT = 42;
const char _SLTU = 43;
const char _DADD = 44;
const char _DADDU = 45;
const char _DSUB = 46;
const char _DSUBU = 47;
const char _TGE = 48;
const char _TGEU = 49;
const char _TLT = 50;
const char _TLTU = 51;
const char _TEQ = 52;
const char _TNE = 54;
const char _DSLL = 56;
const char _DSRL = 58;
const char _DSRA = 59;
const char _DSLL32 = 60;
const char _DSRL32 = 62;
const char _DSRA32 = 63;

// Regimm Instruction...
const char _BLTZ = 0;
const char _BGEZ = 1;
const char _BLTZL = 2;
const char _BGEZL = 3;
const char _TGEI = 8;
const char _TGEIU = 9;
const char _TLTI = 10;
const char _TLTIU = 11;
const char _TEQI = 12;
const char _TNEI = 14;
const char _BLTZAL = 16;
const char _BGEZAL = 17;
const char _BLTZALL = 18;
const char _BGEZALL = 19;

// COP0 Instructions
const char _MFC0 = 0;
const char _MTC0 = 4;
const char _TLB = 16;

// COP0 - TLB Instruction
const char _TLBR = 1;
const char _TLBWI = 2;
const char _TLBWR = 6;
const char _TLBP = 8;
const char _ERET = 24;

// Cache Instructions (Bits 16,17)
const char _Inst = 0;
const char _Data = 1;
const char _SInst = 2;
const char _SData = 3;

// (18,19,20)
const char _IndInv = 0;
const char _IndLT = 1;
const char _IndST = 2;
const char _CreDEx = 3;
const char _HitInv = 4;
const char _HitWBInv = 5;
const char _HitWB = 6;
const char _HitSetV = 6;

// COP1 Instructions
const char _MFC1 = 0;
const char _DMFC1 = 1;
const char _CFC1 = 2;
const char _MTC1 = 4;
const char _DMTC1 = 5;
const char _CTC1 = 6;
const char _BC1 = 8;
const char _COP1_S = 16;
const char _COP1_D = 17;
const char _COP1_W = 20;
const char _COP1_L = 21;

// COP1 BC
const u8 _BC1_F = 0;
const u8 _BC1_T = 1;
const u8 _BC1_FL = 2;
const u8 _BC1_TL = 3;

// COP1 fmt - Instructions
const char _COP1_ADD = 0;
const char _COP1_SUB = 1;
const char _COP1_MUL = 2;
const char _COP1_DIV = 3;
const char _COP1_SQRT = 4;
const char _COP1_ABS = 5;
const char _COP1_MOV = 6;
const char _COP1_NEG = 7;
const char _COP1_ROUND_L = 8;
const char _COP1_TRUNC_L = 9;
const char _COP1_CEIL_L = 10;
const char _COP1_FLOOR_L = 11;
const char _COP1_ROUND_W = 12;
const char _COP1_TRUNC_W = 13;
const char _COP1_CEIL_W = 14;
const char _COP1_FLOOR_W = 15;
const char _COP1_CVT_S = 32;
const char _COP1_CVT_D = 33;
const char _COP1_CVT_W = 36;
const char _COP1_CVT_L = 37;
const char _COP1_C_F = 48;
const char _COP1_C_UN = 49;
const char _COP1_C_EQ = 50;
const char _COP1_C_UEQ = 51;
const char _COP1_C_OLT = 52;
const char _COP1_C_ULT = 53;
const char _COP1_C_OLE = 54;
const char _COP1_C_ULE = 55;
const char _COP1_C_SF = 56;
const char _COP1_C_NGLE = 57;
const char _COP1_C_SEQ = 58;
const char _COP1_C_NGL = 59;
const char _COP1_C_LT = 60;
const char _COP1_C_NGE = 61;
const char _COP1_C_LE = 62;
const char _COP1_C_NGT = 63;

class CPUDisassembler {
private:
	bool _showRegNames;
	TOpcode _op;
	u32 _pc;
	s32 _reg1;
	s32 _reg2;
	s32 _reg3;

	std::string GetRegName(s32 reg) {
		if (_showRegNames)
			return OpcodeGPRNames[reg];

		char temp[20];
		sprintf(temp, "R%i", reg);
		return temp;
	}

	std::string GetFloatRegName(s32 reg) {
		char temp[20];
		sprintf(temp, "F%i", reg);
		return temp;
	}

	std::string DoReg(s32 reg) {
		// Remember this reg for later (allow color-coding in the debugger)
		if (_reg1 == -1)
			_reg1 = reg;
		else if (_reg2 == -1)
			_reg2 = reg;
		else
			_reg3 = reg;

		return GetRegName(reg);
	}

	std::string rd() {
		return DoReg(_op.rd());
	}

	std::string fs() {
		return OpcodeCOP0RegNames[_op.rd()];
	}

	std::string rs() {
		return DoReg(_op.rs());
	}

	std::string base() {
		return "(" + rs() + ")";
	}

	std::string rt() {
		return DoReg(_op.rt());
	}

	std::string sa() {
		return DoReg(_op.sa());
	}

	std::string fdfloat() {
		return GetFloatRegName(_op.fd());
	}

	std::string fsfloat() {
		return GetFloatRegName(_op.fs());
	}

	std::string ftfloat() {
		return GetFloatRegName(_op.ft());
	}

	std::string offset() {
		char temp[20];
		sprintf(temp, "0x%x", _pc + 4 + ((s32)(s16)_op.immediate()<<2));
		return temp;
	}

	std::string target() {
		char temp[20];
		sprintf(temp, "0x%x", (_pc & 0xF0000000) | (_op.target() << 2));
		return temp;
	}

	std::string imm() {
		char temp[20];
		sprintf(temp, "0x%x", _op.immediate());
		return temp;
	}
	std::string InnerDisassemble(u32 pc, TOpcode op, bool showRegNames) {
		_showRegNames = showRegNames;
		_op = op;
		_pc = pc;
		_reg1 = -1;
		_reg2 = -1;
		_reg3 = -1;
		switch (op.op()) {
		case _SPECIAL: {
			switch (op.funct()) {
			case _ADD:
				return "ADD " + rd() + ", " + rs() + ", " + rt();
			case _ADDU:
				return "ADDU " + rd() + ", " + rs() + ", " + rt();
			case _AND:
				return "AND " + rd() + ", " + rs() + ", " + rt();
			case _BREAK:
				return "BREAK";
			case _DADD:
				return "DADD " + rd() + ", " + rs() + ", " + rt();
			case _DADDU:
				return "DADDU " + rd() + ", " + rs() + ", " + rt();
			case _DDIV:
				return "DDIV " + rs() + ", " + rt();
			case _DDIVU:
				return "DDIVU " + rs() + ", " + rt();
			case _DIV:
				return "DIV " + rs() + ", " + rt();
			case _DIVU:
				return "DIVU " + rs() + ", " + rt();
			case _DMULT:
				return "DMULT " + rs() + ", " + rt();
			case _DMULTU:
				return "DMULTU " + rs() + ", " + rt();
			case _DSLL:
				return "DSLL " + rd() + ", " + rt() + ", " + sa();
			case _DSLL32:
				return "DSLL32 " + rd() + ", " + rt() + ", " + sa();
			case _DSLLV:
				return "DSLLV " + rd() + ", " + rt() + ", " + rs();
			case _DSRA:
				return "DSRA " + rd() + ", " + rt() + ", " + sa();
			case _DSRA32:
				return "DSRA32 " + rd() + ", " + rt() + ", " + sa();
			case _DSRAV:
				return "DSRAV " + rd() + ", " + rt() + ", " + rs();
			case _DSRL:
				return "DSRL " + rd() + ", " + rt() + ", " + sa();
			case _DSRL32:
				return "DSRL32 " + rd() + ", " + rt() + ", " + sa();
			case _DSRLV:
				return "DSRLV " + rd() + ", " + rt() + ", " + rs();
			case _DSUB:
				return "DSUB " + rd() + ", " + rs() + ", " + rt();
			case _DSUBU:
				return "DSUBU " + rd() + ", " + rs() + ", " + rt();
			case _JALR:
				return "JALR " + rd() + ", " + rs();
			case _JR:
				return "JR " + rs();
			case _MFHI:
				return "MFHI " + rd();
			case _MFLO:
				return "MFLO " + rd();
			case _MTHI:
				return "MTHI " + rs();
			case _MTLO:
				return "MTLO " + rs();
			case _MULT:
				return "MULT " + rs() + ", " + rt();
			case _MULTU:
				return "MULTU " + rs() + ", " + rt();
			case _NOR:
				return "NOR " + rd() + ", " + rs() + ", " + rt();
			case _OR:
				return "OR " + rd() + ", " + rs() + ", " + rt();
			case _SLL:
				if (op.all == 0x00000000)
					return "NOP";
				else
					return "SLL " + rd() + ", " + rt() + ", " + sa();
			case _SLLV:
				return "SLLV " + rd() + ", " + rt() + ", " + rs();
			case _SLT:
				return "SLT " + rd() + ", " + rs() + ", " + rt();
			case _SLTU:
				return "SLTU " + rd() + ", " + rs() + ", " + rt();
			case _SRA:
				return "SRA " + rd() + ", " + rt() + ", " + sa();
			case _SRAV:
				return "SRAV " + rd() + ", " + rt() + ", " + rs();
			case _SRL:
				return "SRL " + rd() + ", " + rt() + ", " + sa();
			case _SRLV:
				return "SRLV " + rd() + ", " + rt() + ", " + rs();
			case _SUB:
				return "SUB " + rd() + ", " + rs() + ", " + rt();
			case _SUBU:
				return "SUBU " + rd() + ", " + rs() + ", " + rt();
			case _SYNC:
				return "SYNC";
			case _SYSCALL:
				return "SYSCALL";
			case _TEQ:
				return "TEQ " + rs() + ", " + rt();
			case _TGE:
				return "TGE " + rs() + ", " + rt();
			case _TGEU:
				return "TGEU " + rs() + ", " + rt();
			case _TLT:
				return "TLT " + rs() + ", " + rt();
			case _TLTU:
				return "TLTU " + rs() + ", " + rt();
			case _TNE:
				return "TNE " + rs() + ", " + rt();
			case _XOR:
				return "XOR " + rd() + ", " + rs() + ", " + rt();
			default:
				return "? (Special)";
			}
		}
		case _REGIMM: {
			switch (op.rt()) {
			case _BLTZ:
				return "BLTZ " + rs() + ", " + offset();
			case _BGEZ:
				return "BGEZ " + rs() + ", " + offset();
			case _BLTZL:
				return "BLTZL " + rs() + ", " + offset();
			case _BGEZL:
				return "BGEZL " + rs() + ", " + offset();
			case _TGEI:
				return "TGEI " + rs() + ", " + imm();
			case _TGEIU:
				return "TGEIU " + rs() + ", " + imm();
			case _TLTI:
				return "TLTI " + rs() + ", " + imm();
			case _TLTIU:
				return "TLTIU " + rs() + ", " + imm();
			case _TEQI:
				return "TEQI " + rs() + ", " + imm();
			case _TNEI:
				return "TNEI " + rs() + ", " + imm();
			case _BLTZAL:
				return "BLTZAL " + rs() + ", " + offset();
			case _BGEZAL:
				return "BGEZAL " + rs() + ", " + offset();
			case _BLTZALL:
				return "BLTZALL " + rs() + ", " + offset();
			case _BGEZALL:
				return "BGEZALL " + rs() + ", " + offset();
			default:
				return "? (Regimm)";
			}
		}
		case _COP0: {
			switch (op.rs()) {
			case _MFC0:
				return "MFC0 " + rt() + ", " + fs();
			case _MTC0:
				return "MTC0 " + rt() + ", " + fs();
			case _TLB:
				switch (op.funct()) {
				case _TLBR:
					return "TLBR";
				case _TLBWI:
					return "TLBWI";
				case _TLBWR:
					return "TLBWR";
				case _TLBP:
					return "TLBP";
				case _ERET:
					return "ERET";
				default:
					return "? (COP 0 - TLB)";
				}
			default:
				return "? (COP 0)";
			}
		}
		case _COP1: {
			switch (op.rs()) {
			case _MFC1:
				return "MFC1 " + rt() + ", " + fsfloat();
			case _DMFC1:
				return "DMFC1 " + rt() + ", " + fsfloat();
			case _CFC1:
				return "CFC1 " + rt() + ", " + fsfloat();
			case _MTC1:
				return "MTC1 " + rt() + ", " + fsfloat();
			case _DMTC1:
				return "DMTC1 " + rt() + ", " + fsfloat();
			case _CTC1:
				return "CTC1 " + rt() + ", " + fsfloat();
			case _BC1: {
				switch (op.rt()) {
				case _BC1_F:
					return "BC1F " + offset();
				case _BC1_FL:
					return "BC1FL " + offset();
				case _BC1_T:
					return "BC1T " + offset();
				case _BC1_TL:
					return "BC1TL " + offset();
				default:
					return "? (BC1)";
				}
			}
			case _COP1_S: {
				switch (op.funct()) {
				case _COP1_ADD:
					return "ADD.S " + fdfloat() + ", " + fsfloat() + ", "
							+ ftfloat();
				case _COP1_SUB:
					return "SUB.S " + fdfloat() + ", " + fsfloat() + ", "
							+ ftfloat();
				case _COP1_MUL:
					return "MUL.S " + fdfloat() + ", " + fsfloat() + ", "
							+ ftfloat();
				case _COP1_DIV:
					return "DIV.S " + fdfloat() + ", " + fsfloat() + ", "
							+ ftfloat();
				case _COP1_SQRT:
					return "SQRT.S " + fdfloat() + ", " + fsfloat();
				case _COP1_ABS:
					return "ABS.S " + fdfloat() + ", " + fsfloat();
				case _COP1_MOV:
					return "MOV.S " + fdfloat() + ", " + fsfloat();
				case _COP1_NEG:
					return "NEG.S " + fdfloat() + ", " + fsfloat();
				case _COP1_ROUND_L:
					return "ROUND.L.S " + fdfloat() + ", " + fsfloat();
				case _COP1_TRUNC_L:
					return "TRUNC.L.S " + fdfloat() + ", " + fsfloat();
				case _COP1_CEIL_L:
					return "CEIL.L.S " + fdfloat() + ", " + fsfloat();
				case _COP1_FLOOR_L:
					return "FLOOR.L.S " + fdfloat() + ", " + fsfloat();
				case _COP1_ROUND_W:
					return "ROUND.W.S " + fdfloat() + ", " + fsfloat();
				case _COP1_TRUNC_W:
					return "TRUNC.W.S " + fdfloat() + ", " + fsfloat();
				case _COP1_CEIL_W:
					return "CEIL.W.S " + fdfloat() + ", " + fsfloat();
				case _COP1_FLOOR_W:
					return "FLOOR.W.S " + fdfloat() + ", " + fsfloat();
				case _COP1_CVT_D:
					return "CVT.D.S " + fdfloat() + ", " + fsfloat();
				case _COP1_CVT_W:
					return "CVT.W.S " + fdfloat() + ", " + fsfloat();
				case _COP1_CVT_L:
					return "CVT.L.S " + fdfloat() + ", " + fsfloat();
				case _COP1_C_F:
					return "C.F.S " + fsfloat() + ", " + ftfloat();
				case _COP1_C_UN:
					return "C.UN.S " + fsfloat() + ", " + ftfloat();
				case _COP1_C_EQ:
					return "C.EQ.S " + fsfloat() + ", " + ftfloat();
				case _COP1_C_UEQ:
					return "C.UEQ.S " + fsfloat() + ", " + ftfloat();
				case _COP1_C_OLT:
					return "C.OLT.S " + fsfloat() + ", " + ftfloat();
				case _COP1_C_ULT:
					return "C.ULT.S " + fsfloat() + ", " + ftfloat();
				case _COP1_C_OLE:
					return "C.OLE.S " + fsfloat() + ", " + ftfloat();
				case _COP1_C_ULE:
					return "C.ULE.S " + fsfloat() + ", " + ftfloat();
				case _COP1_C_SF:
					return "C.SF.S " + fsfloat() + ", " + ftfloat();
				case _COP1_C_NGLE:
					return "C.NGLE.S " + fsfloat() + ", " + ftfloat();
				case _COP1_C_SEQ:
					return "C.SEQ.S " + fsfloat() + ", " + ftfloat();
				case _COP1_C_NGL:
					return "C.NGL.S " + fsfloat() + ", " + ftfloat();
				case _COP1_C_LT:
					return "C.LT.S " + fsfloat() + ", " + ftfloat();
				case _COP1_C_NGE:
					return "C.NGE.S " + fsfloat() + ", " + ftfloat();
				case _COP1_C_LE:
					return "C.LE.S " + fsfloat() + ", " + ftfloat();
				case _COP1_C_NGT:
					return "C.NGT.S " + fsfloat() + ", " + ftfloat();
				default:
					return "? COP1 - S";
				}
			}
			case _COP1_D: {
				switch (op.funct()) {
				case _COP1_ADD:
					return "ADD.D " + fdfloat() + ", " + fsfloat() + ", "
							+ ftfloat();
				case _COP1_SUB:
					return "SUB.D " + fdfloat() + ", " + fsfloat() + ", "
							+ ftfloat();
				case _COP1_MUL:
					return "MUL.D " + fdfloat() + ", " + fsfloat() + ", "
							+ ftfloat();
				case _COP1_DIV:
					return "DIV.D " + fdfloat() + ", " + fsfloat() + ", "
							+ ftfloat();
				case _COP1_SQRT:
					return "SQRT.D " + fdfloat() + ", " + fsfloat();
				case _COP1_ABS:
					return "ABS.D " + fdfloat() + ", " + fsfloat();
				case _COP1_MOV:
					return "MOV.D " + fdfloat() + ", " + fsfloat();
				case _COP1_NEG:
					return "NEG.D " + fdfloat() + ", " + fsfloat();
				case _COP1_ROUND_L:
					return "ROUND.L.D " + fdfloat() + ", " + fsfloat();
				case _COP1_TRUNC_L:
					return "TRUNC.L.D " + fdfloat() + ", " + fsfloat();
				case _COP1_CEIL_L:
					return "CEIL.L.D " + fdfloat() + ", " + fsfloat();
				case _COP1_FLOOR_L:
					return "FLOOR.L.D " + fdfloat() + ", " + fsfloat();
				case _COP1_ROUND_W:
					return "ROUND.W.D " + fdfloat() + ", " + fsfloat();
				case _COP1_TRUNC_W:
					return "TRUNC.W.D " + fdfloat() + ", " + fsfloat();
				case _COP1_CEIL_W:
					return "CEIL.W.D " + fdfloat() + ", " + fsfloat();
				case _COP1_FLOOR_W:
					return "FLOOR.W.D " + fdfloat() + ", " + fsfloat();
				case _COP1_CVT_S:
					return "CVT.S.D " + fdfloat() + ", " + fsfloat();
				case _COP1_CVT_W:
					return "CVT.W.D " + fdfloat() + ", " + fsfloat();
				case _COP1_CVT_L:
					return "CVT.L.D " + fdfloat() + ", " + fsfloat();
				case _COP1_C_F:
					return "C.F.D " + fsfloat() + ", " + ftfloat();
				case _COP1_C_UN:
					return "C.UN.D " + fsfloat() + ", " + ftfloat();
				case _COP1_C_EQ:
					return "C.EQ.D " + fsfloat() + ", " + ftfloat();
				case _COP1_C_UEQ:
					return "C.UEQ.D " + fsfloat() + ", " + ftfloat();
				case _COP1_C_OLT:
					return "C.OLT.D " + fsfloat() + ", " + ftfloat();
				case _COP1_C_ULT:
					return "C.ULT.D " + fsfloat() + ", " + ftfloat();
				case _COP1_C_OLE:
					return "C.OLE.D " + fsfloat() + ", " + ftfloat();
				case _COP1_C_ULE:
					return "C.ULE.D " + fsfloat() + ", " + ftfloat();
				case _COP1_C_SF:
					return "C.SF.D " + fsfloat() + ", " + ftfloat();
				case _COP1_C_NGLE:
					return "C.NGLE.D " + fsfloat() + ", " + ftfloat();
				case _COP1_C_SEQ:
					return "C.SEQ.D " + fsfloat() + ", " + ftfloat();
				case _COP1_C_NGL:
					return "C.NGL.D " + fsfloat() + ", " + ftfloat();
				case _COP1_C_LT:
					return "C.LT.D " + fsfloat() + ", " + ftfloat();
				case _COP1_C_NGE:
					return "C.NGE.D " + fsfloat() + ", " + ftfloat();
				case _COP1_C_LE:
					return "C.LE.D " + fsfloat() + ", " + ftfloat();
				case _COP1_C_NGT:
					return "C.NGT.D " + fsfloat() + ", " + ftfloat();
				default:
					return "? COP1 - D";
				}
			}
			case _COP1_W: {
				switch (op.funct()) {
				case _COP1_CVT_S:
					return "CVT.S.W " + fdfloat() + ", " + fsfloat();
				case _COP1_CVT_D:
					return "CVT.D.W " + fdfloat() + ", " + fsfloat();
				default:
					return "? COP1 - W";
				}
			}
			case _COP1_L: {
				switch (op.funct()) {
				case _COP1_CVT_S:
					return "CVT.S.L " + fdfloat() + ", " + fsfloat();
				case _COP1_CVT_D:
					return "CVT.D.L " + fdfloat() + ", " + fsfloat();
				default:
					return "? COP1 - L";
				}
			}
			default:
				return "? (COP 1)";
			}
		}
		case _ADDI:
			return "ADDI " + rt() + ", " + rs() + ", " + imm();
		case _ADDIU:
			return "ADDIU " + rt() + ", " + rs() + ", " + imm();
		case _ANDI:
			return "ANDI " + rt() + ", " + rs() + ", " + imm();
		case _BEQ:
			return "BEQ " + rs() + ", " + rt() + ", " + offset();
		case _BEQL:
			return "BEQL " + rs() + ", " + rt() + ", " + offset();
		case _BGTZ:
			return "BGTZ " + rs() + ", " + offset();
		case _BGTZL:
			return "BGTZL " + rs() + ", " + offset();
		case _BLEZ:
			return "BLEZ " + rs() + ", " + offset();
		case _BLEZL:
			return "BLEZL " + rs() + ", " + offset();
		case _BNE:
			if (op.rt() == 0)
				return "BNEZ " + rs() + ", " + offset();
			else
				return "BNE " + rs() + ", " + rt() + ", " + offset();
		case _BNEL:
			return "BNEL " + rs() + ", " + rt() + ", " + offset();
		case _CACHE:
			return "CACHE";
		case _DADDI:
			return "DADDI " + rt() + ", " + rs() + ", " + imm();
		case _DADDIU:
			return "DADDIU " + rt() + ", " + rs() + ", " + imm();
		case _J:
			return "J " + target();
		case _JAL:
			return "JAL " + target();
		case _LB:
			return "LB " + rt() + ", " + imm() + base();
		case _LBU:
			return "LBU " + rt() + ", " + imm() + base();
		case _LDC1:
			return "LDC1 " + ftfloat() + ", " + imm() + base();
		case _LD:
			return "LD " + rt() + ", " + imm() + base();
		case _LDL:
			return "LDL " + rt() + ", " + imm() + base();
		case _LDR:
			return "LDR " + rt() + ", " + imm() + base();
		case _LH:
			return "LH " + rt() + ", " + imm() + base();
		case _LHU:
			return "LHU " + rt() + ", " + imm() + base();
		case _LUI:
			return "LUI " + rt() + ", " + imm();
		case _LW:
			return "LW " + rt() + ", " + imm() + base();
		case _LWC1:
			return "LWC1 " + ftfloat() + ", " + imm() + base();
		case _LWL:
			return "LWL " + rt() + ", " + imm() + base();
		case _LWR:
			return "LWR " + rt() + ", " + imm() + base();
		case _LWU:
			return "LWU " + rt() + ", " + imm() + base();
		case _ORI:
			return "ORI " + rt() + ", " + rs() + ", " + imm();
		case _SB:
			return "SB " + rt() + ", " + imm() + base();
		case _SDC1:
			return "SDC1 " + ftfloat() + ", " + imm() + base();
		case _SD:
			return "SD " + rt() + ", " + imm() + base();
		case _SDL:
			return "SDL " + rt() + ", " + imm() + base();
		case _SDR:
			return "SDR " + rt() + ", " + imm() + base();
		case _SH:
			return "SH " + rt() + ", " + imm() + base();
		case _SLTI:
			return "SLTI " + rt() + ", " + rs() + ", " + imm();
		case _SLTIU:
			return "SLTIU " + rt() + ", " + rs() + ", " + imm();
		case _SW:
			return "SW " + rt() + ", " + imm() + base();
		case _SWC1:
			return "SWC1 " + ftfloat() + ", " + imm() + base();
		case _SWR:
			return "SWR " + rt() + ", " + imm() + base();
		case _SWL:
			return "SWL " + rt() + ", " + imm() + base();
		case _XORI:
			return "XORI " + rt() + ", " + rs() + ", " + imm();
		default:
			char temp[20];
			sprintf(temp, "? 0x%x, ", op.all);
			return temp;
		}
	}
public:
	std::string Disassemble(u32 pc, TOpcode op, bool showRegNames) {
		return InnerDisassemble(pc, op, showRegNames);
	}
};
/**********************************************************************/
/* END Nemu64 Disassembler                                            */
/**********************************************************************/


int
arch_mips_disasm_instr(cpu_t *cpu, addr_t pc, char *line, unsigned int max_line) {

	int dummy1;
	addr_t dummy2;
	int bytes = arch_mips_tag_instr(cpu, pc, &dummy1, &dummy2);

	uint32_t instr = INSTR(pc);
	TOpcode op;
	op.all = instr;

	CPUDisassembler *disassembler = new CPUDisassembler();
	snprintf(line, max_line, "%s", disassembler->Disassemble(pc, op, false).c_str());

	return bytes;
}
