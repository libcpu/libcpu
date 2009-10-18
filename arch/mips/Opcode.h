#ifndef OPCODE_H_
#define OPCODE_H_

#include "Typedefs.h"

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

#endif /* OPCODE_H_ */
