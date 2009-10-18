#ifndef CPUDISASSEMBLER_H_
#define CPUDISASSEMBLER_H_

#include "Opcode.h"

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

#endif /* CPUDISASSEMBLER_H_ */
