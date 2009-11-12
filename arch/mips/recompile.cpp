#define OPT_LOCAL_REGISTERS //XXX

#include "libcpu.h"
#include "cpu_generic.h"
#include "arch/mips/libcpu_mips.h"
#include "mips_internal.h"

using namespace llvm;

//////////////////////////////////////////////////////////////////////
// MIPS: instruction decoding
//////////////////////////////////////////////////////////////////////

#define RS	((instr >> 21) & 0x1F)
#define RT	((instr >> 16) & 0x1F)
#define RD	((instr >> 11) & 0x1F)

#define GetSA	((instr >> 6) & 0x1F)
#define GetImmediate (instr & 0xFFFF)
#define GetTarget (instr & 0x3FFFFFF)

#define GetFunction (instr & 0x3F)
//#define GetSpecialInstruction GetFunction
#define GetRegimmInstruction RT
#define GetFMT RS
//#define GetCacheType (RT&BitM2)
//#define GetCacheInstr ((RT>>2)&BitM3)
#define GetCOP1FloatInstruction GetFunction

#define MIPS_BRANCH_TARGET ((uint32_t)(pc + 4 + (uint32_t)(((sint32_t)(sint16_t)GetImmediate<<2))))

//////////////////////////////////////////////////////////////////////
// tagging
//////////////////////////////////////////////////////////////////////

#include "tag_generic.h"
int arch_mips_tag_instr(cpu_t *cpu, addr_t pc, int *flow_type, addr_t *new_pc) {
	uint32_t instr = INSTR(pc);

	switch(instr >> 26) {
		case 0x00: //INCPU_SPECIAL
			switch(instr & 0x3F) {
				case 0x08: //INCPUS_JR
					*flow_type = FLOW_TYPE_RET;
					break;
				case 0x01: //IN_invalid
				case 0x05: //IN_invalid
				case 0x0A: //IN_invalid
				case 0x0B: //IN_invalid
				case 0x0C: //INCPUS_SYSCALL
				case 0x0D: //INCPUS_BREAK
				case 0x0E: //IN_invalid
				case 0x15: //IN_invalid
				case 0x28: //IN_invalid
				case 0x29: //IN_invalid
				case 0x35: //IN_invalid
				case 0x37: //IN_invalid
				case 0x39: //IN_invalid
				case 0x3D: //IN_invalid
					*flow_type = FLOW_TYPE_ERR;
					break;
				default:
					*flow_type = FLOW_TYPE_CONTINUE;
					break;
			}
			break;
		case 0x01: //INCPU_REGIMM
			switch (GetRegimmInstruction) {
				case 0x00: //INCPUR_BLTZ
				case 0x01: //INCPUR_BGEZ
					*new_pc = MIPS_BRANCH_TARGET;
					*flow_type = FLOW_TYPE_BRANCH;
					break;
				case 0x10: //INCPUR_BLTZAL
				case 0x11: //INCPUR_BGEZAL
					*new_pc = MIPS_BRANCH_TARGET;
					*flow_type = FLOW_TYPE_CALL;
					break;
				case 0x02: //INCPUR_BLTZL
				case 0x03: //INCPUR_BGEZL
					*new_pc = MIPS_BRANCH_TARGET;
					*flow_type = FLOW_TYPE_BRANCH;
					break;
				case 0x12: //INCPUR_BLTZALL
				case 0x13: //INCPUR_BGEZALL
					*new_pc = MIPS_BRANCH_TARGET;
					*flow_type = FLOW_TYPE_CALL;
					break;
				case 0x04: //IN_invalid
				case 0x05: //IN_invalid
				case 0x06: //IN_invalid
				case 0x07: //IN_invalid
				case 0x0D: //IN_invalid
				case 0x0F: //IN_invalid
				case 0x14: //IN_invalid
				case 0x15: //IN_invalid
				case 0x16: //IN_invalid
				case 0x17: //IN_invalid
				case 0x18: //IN_invalid
				case 0x19: //IN_invalid
				case 0x1A: //IN_invalid
				case 0x1B: //IN_invalid
				case 0x1C: //IN_invalid
				case 0x1D: //IN_invalid
				case 0x1E: //IN_invalid
				case 0x1F: //IN_invalid
					*flow_type = FLOW_TYPE_ERR;
					break;
				default:
					*flow_type = FLOW_TYPE_CONTINUE;
					break;
			}
		case 0x02: //INCPU_J
			*new_pc = (pc & 0xF0000000) | (GetTarget << 2);
			*flow_type = FLOW_TYPE_BRANCH;
		case 0x03: //INCPU_JAL
			*new_pc = (pc & 0xF0000000) | (GetTarget << 2);
			*flow_type = FLOW_TYPE_CALL;
			break;
		case 0x04: //INCPU_BEQ
			if (!RS && !RT) { // special case: B
				*new_pc = MIPS_BRANCH_TARGET;
				*flow_type = FLOW_TYPE_JUMP;
			} else {
				*new_pc = MIPS_BRANCH_TARGET;
				*flow_type = FLOW_TYPE_BRANCH;
			}
			break;
		case 0x05: //INCPU_BNE
		case 0x06: //INCPU_BLEZ
		case 0x07: //INCPU_BGTZ
			*new_pc = MIPS_BRANCH_TARGET;
			*flow_type = FLOW_TYPE_BRANCH;
			break;
		case 0x10: //INCPU_COP0
			// we don't translate any of the INCPU_COP0 branch
			*flow_type = FLOW_TYPE_ERR;
			break;
		case 0x11: //INCPU_COP1
			switch(GetFMT) {
				case 0x03: //IN_invalid
				case 0x07: //IN_invalid
				case 0x09: //IN_invalid
				case 0x0A: //IN_invalid
				case 0x0B: //IN_invalid
				case 0x0C: //IN_invalid
				case 0x0D: //IN_invalid
				case 0x0E: //IN_invalid
				case 0x0F: //IN_invalid
					*flow_type = FLOW_TYPE_ERR;
					break;
				case 0x10: //INCOP1_S
					switch(GetCOP1FloatInstruction) {
						case 0x00: //INCOP1_ADD
								case 0x10: //IN_invalid
								case 0x11: //IN_invalid
								case 0x12: //IN_invalid
								case 0x13: //IN_invalid
								case 0x14: //IN_invalid
								case 0x15: //IN_invalid
								case 0x16: //IN_invalid
								case 0x17: //IN_invalid
								case 0x18: //IN_invalid
								case 0x19: //IN_invalid
								case 0x1A: //IN_invalid
								case 0x1B: //IN_invalid
								case 0x1C: //IN_invalid
								case 0x1D: //IN_invalid
								case 0x1E: //IN_invalid
								case 0x1F: //IN_invalid
								case 0x22: //IN_invalid
								case 0x23: //IN_invalid
								case 0x26: //IN_invalid
								case 0x27: //IN_invalid
								case 0x28: //IN_invalid
								case 0x29: //IN_invalid
								case 0x2A: //IN_invalid
								case 0x2B: //IN_invalid
								case 0x2C: //IN_invalid
								case 0x2D: //IN_invalid
								case 0x2E: //IN_invalid
								case 0x2F: //IN_invalid
									*flow_type = FLOW_TYPE_ERR;
									break;
								default:
									*flow_type = FLOW_TYPE_CONTINUE;
									break;
							}
				case 0x12: //IN_invalid
				case 0x13: //IN_invalid
				case 0x16: //IN_invalid
				case 0x17: //IN_invalid
				case 0x18: //IN_invalid
				case 0x19: //IN_invalid
				case 0x1A: //IN_invalid
				case 0x1B: //IN_invalid
				case 0x1C: //IN_invalid
				case 0x1D: //IN_invalid
				case 0x1E: //IN_invalid
				case 0x1F: //IN_invalid
					*flow_type = FLOW_TYPE_ERR;
					break;
				default:
					*flow_type = FLOW_TYPE_CONTINUE;
					break;
			}
		case 0x14: //INCPU_BEQL
		case 0x15: //INCPU_BNEL
		case 0x16: //INCPU_BLEZL
		case 0x17: //INCPU_BGTZL
			*new_pc = MIPS_BRANCH_TARGET;
			*flow_type = FLOW_TYPE_BRANCH;
			break;
		case 0x12: //IN_invalid
		case 0x13: //IN_invalid
		case 0x1C: //IN_invalid
		case 0x1D: //IN_invalid
		case 0x1E: //IN_invalid
		case 0x1F: //IN_invalid
		case 0x32: //IN_invalid
		case 0x33: //IN_invalid
		case 0x36: //IN_invalid
		case 0x3A: //IN_invalid
		case 0x3B: //IN_invalid
		case 0x3E: //IN_invalid
			*flow_type = FLOW_TYPE_ERR;
			break;
		default:
			*flow_type = FLOW_TYPE_CONTINUE;
			break;
	}
	return (*flow_type == FLOW_TYPE_BRANCH ||
			*flow_type == FLOW_TYPE_JUMP ||
			*flow_type == FLOW_TYPE_CALL ||
			*flow_type == FLOW_TYPE_RET)? 8 : 4;
}

//////////////////////////////////////////////////////////////////////

Value *
arch_mips_get_imm(cpu_t *cpu, uint32_t instr, uint32_t bits, bool sext,
  BasicBlock *bb) {
	uint64_t imm;
	if (sext)
		imm = (uint64_t)(sint16_t)GetImmediate;
	else
		imm = (uint64_t)(uint16_t)GetImmediate;

	return ConstantInt::get(getIntegerType(bits? bits : cpu->reg_size), imm);
}

#define IMM arch_mips_get_imm(cpu, instr, 0, true, bb)
#define IMMU arch_mips_get_imm(cpu, instr, 0, false, bb)
#define IMM32 arch_mips_get_imm(cpu, instr, 32, true, bb)

//////////////////////////////////////////////////////////////////////

Value *
arch_mips_get_sa(cpu_t *cpu, uint32_t instr, uint32_t bits, BasicBlock *bb) {
	return ConstantInt::get(getIntegerType(bits? bits : cpu->reg_size), GetSA);
}

#define SA arch_mips_get_sa(cpu, instr, 0, bb)
#define SA32 arch_mips_get_sa(cpu, instr, 32, bb)

//////////////////////////////////////////////////////////////////////

void
arch_mips_branch(Value *v, bool likely, BasicBlock *bb, BasicBlock *bb_target, BasicBlock *bb_next) {

	if (likely) {
		printf("error: \"likely\" is broken!\n");
		exit(1);
	}

	BRANCH(true, bb_target, bb_next, v);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

#define LET_PC(v) new StoreInst(v, cpu->ptr_PC, bb)


#define DELAY_SLOT arch_mips_recompile_instr(cpu, pc+4, bb_dispatch, bb, bb_target, bb_cond, bb_next)
#define JMP_BB(b) BranchInst::Create(b, bb)

#define BRANCH_TRUE(tag)  arch_mips_branch(tag, false, bb, bb_target, bb_next);
#define BRANCH_TRUE_LIKELY(tag)  arch_mips_branch(tag, true, bb, bb_target, bb_next);

#define BRANCH_DELAY_TRUE(test) {	\
	Value *f = test;				\
	DELAY_SLOT;						\
	BRANCH_TRUE(f);					\
}
#define BRANCH_DELAY_TRUE_LIKELY(test) {	\
	Value *f = test;				\
	DELAY_SLOT;						\
	BRANCH_TRUE_LIKELY(f);					\
}

#define LINKr(i) LET32(i, CONST((uint64_t)(sint64_t)(sint32_t)pc+8))

#define LINK LINKr(31)

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

int arch_mips_recompile_instr(cpu_t *cpu, addr_t pc, BasicBlock *bb_dispatch, BasicBlock *bb, BasicBlock *bb_target, BasicBlock *bb_cond, BasicBlock *bb_next) {
#define BAD printf("%s:%d\n", __func__, __LINE__); exit(1);
#define LOG printf("%s:%d\n", __func__, __LINE__);

//printf("%s:%d %p, %p\n", __func__, __LINE__, bb_dispatch, bb);

	uint32_t instr = INSTR(pc);

	printf("translating (%08llx) %08x\n", pc, instr);

	switch(instr >> 26) {
	case 0x00: /* INCPU_SPECIAL */
		switch(instr & 0x3F) {
			case 0x00: /* INCPUS_SLL */		LET32(RD,SHL(R32(RT),SA32));	break; /* XXX special case NOP? */
			case 0x02: /* INCPUS_SRL */		LET32(RD,LSHR(R32(RT),SA32));	break;
			case 0x03: /* INCPUS_SRA */		LET32(RD,ASHR(R32(RT),SA32));	break;
			case 0x04: /* INCPUS_SLLV */	LET32(RD,SHL(R32(RT),R32(RS)));	break;
			case 0x06: /* INCPUS_SRLV */	LET32(RD,LSHR(R32(RT),R32(RS)));	break;
			case 0x07: /* INCPUS_SRAV */	LET32(RD,ASHR(R32(RT),R32(RS)));	break;
			case 0x08: /* INCPUS_JR */
			{
				LET_PC(R(RS));
				DELAY_SLOT;
				JMP_BB(bb_dispatch);
				break;
			}
			case 0x09: /* INCPUS_JALR */
			{
				LET_PC(SUB(R(RS),CONST(4)));
				LINKr(RD);
				DELAY_SLOT;
				JMP_BB(bb_dispatch);
				break;
			}
			case 0x0C: /* INCPUS_SYSCALL */	BAD;
			case 0x0D: /* INCPUS_BREAK */	BAD;
			case 0x0F: /* INCPUS_SYNC */	BAD;
			case 0x10: /* INCPUS_MFHI */	BAD;
			case 0x11: /* INCPUS_MTHI */	BAD;
			case 0x12: /* INCPUS_MFLO */	BAD;
			case 0x13: /* INCPUS_MTLO */	BAD;
			case 0x14: /* INCPUS_DSLLV */	BAD;
			case 0x16: /* INCPUS_DSRLV */	BAD;
			case 0x17: /* INCPUS_DSRAV */	BAD;
			case 0x18: /* INCPUS_MULT */	BAD;
			case 0x19: /* INCPUS_MULTU */	BAD;
			case 0x1A: /* INCPUS_DIV */		BAD;
			case 0x1B: /* INCPUS_DIVU */	BAD;
			case 0x1C: /* INCPUS_DMULT */	BAD;
			case 0x1D: /* INCPUS_DMULTU */	BAD;
			case 0x1E: /* INCPUS_DDIV */	BAD;
			case 0x1F: /* INCPUS_DDIVU */	BAD;
			case 0x20: /* INCPUS_ADD */		LET32(RD,ADD(R32(RS), R32(RT)));		break; //XXX same??
			case 0x21: /* INCPUS_ADDU */	LET32(RD,ADD(R32(RS), R32(RT)));		break; //XXX same??
			case 0x22: /* INCPUS_SUB */		LET32(RD,SUB(R32(RS), R32(RT)));		break; //XXX same??
			case 0x23: /* INCPUS_SUBU */	LET32(RD,SUB(R32(RS), R32(RT)));		break; //XXX same??
			case 0x24: /* INCPUS_AND */		LET32(RD,AND(R32(RS), R32(RT)));		break;
			case 0x25: /* INCPUS_OR */		LET32(RD,OR(R32(RS), R32(RT)));		break;
			case 0x26: /* INCPUS_XOR */		LET32(RD,XOR(R32(RS), R32(RT)));		break;
			case 0x27: /* INCPUS_NOR */		LET32(RD,XOR(OR(R32(RS), R32(RT)),CONST32((unsigned long)-1)));	break;
			case 0x2A: /* INCPUS_SLT */		LET_ZEXT(RD,ICMP_SLT(R(RS),R(RT)));	break;
			case 0x2B: /* INCPUS_SLTU */	LET_ZEXT(RD,ICMP_ULT(R(RS),R(RT)));	break;
			case 0x2C: /* INCPUS_DADD */	LET(RD,ADD(R(RS), R(RT)));			break; //XXX same??
			case 0x2D: /* INCPUS_DADDU */	LET(RD,ADD(R(RS), R(RT)));			break; //XXX same??
			case 0x2E: /* INCPUS_DSUB */	LET(RD,SUB(R(RS), R(RT)));			break; //XXX same??
			case 0x2F: /* INCPUS_DSUBU */	LET(RD,SUB(R(RS), R(RT)));			break; //XXX same??
			case 0x30: /* INCPUS_TGE */		BAD;
			case 0x31: /* INCPUS_TGEU */	BAD;
			case 0x32: /* INCPUS_TLT */		BAD;
			case 0x33: /* INCPUS_TLTU */	BAD;
			case 0x34: /* INCPUS_TEQ */		BAD;
			case 0x36: /* INCPUS_TNE */		BAD;
			case 0x38: /* INCPUS_DSLL */	LET(RD,SHL(R(RT),SA));				break;
			case 0x3A: /* INCPUS_DSRL */	LET(RD,LSHR(R(RT),SA));			break;
			case 0x3B: /* INCPUS_DSRA */	LET(RD,ASHR(R(RT),SA));			break;
			case 0x3C: /* INCPUS_DSLL32 */	LET(RD,SHL(R(RT),ADD(SA,CONST(32))));		break;
			case 0x3E: /* INCPUS_DSRL32 */	LET(RD,LSHR(R(RT),ADD(SA,CONST(32))));		break;
			case 0x3F: /* INCPUS_DSRA32 */	LET(RD,ASHR(R(RT),ADD(SA,CONST(32))));		break;
			default:
				printf("INVALID %s:%d\n", __func__, __LINE__); exit(1);
		}
		break;
	case 0x01: /* INCPU_REGIMM */
		switch (GetRegimmInstruction) {
			case 0x00: /* INCPUR_BLTZ */	BRANCH_DELAY_TRUE(ICMP_SLT(R(RS),CONST(0)));	break;
			case 0x01: /* INCPUR_BGEZ */	BRANCH_DELAY_TRUE(ICMP_SGE(R(RS),CONST(0)));	break;
			case 0x02: /* INCPUR_BLTZL */	BRANCH_DELAY_TRUE_LIKELY(ICMP_SLT(R(RS),CONST(0)));	break;
			case 0x03: /* INCPUR_BGEZL */	BRANCH_DELAY_TRUE_LIKELY(ICMP_SGE(R(RS),CONST(0)));	break;
			case 0x08: /* INCPUR_TGEI */	BAD;
			case 0x09: /* INCPUR_TGEIU */	BAD;
			case 0x0A: /* INCPUR_TLTI */	BAD;
			case 0x0B: /* INCPUR_TLTIU */	BAD;
			case 0x0C: /* INCPUR_TEQI */	BAD;
			case 0x0E: /* INCPUR_TNEI */	BAD;
			case 0x10: /* INCPUR_BLTZAL */	LINK; BRANCH_DELAY_TRUE(ICMP_SLT(R(RS),CONST(0)));	break;
			case 0x11: /* INCPUR_BGEZAL */	LINK; BRANCH_DELAY_TRUE(ICMP_SGE(R(RS),CONST(0)));	break;
			case 0x12: /* INCPUR_BLTZALL */	LINK; BRANCH_DELAY_TRUE_LIKELY(ICMP_SLT(R(RS),CONST(0)));	break;
			case 0x13: /* INCPUR_BGEZALL */	LINK; BRANCH_DELAY_TRUE_LIKELY(ICMP_SGE(R(RS),CONST(0)));	break;
			default:
				printf("INVALID %s:%d\n", __func__, __LINE__); exit(1);
		}
	case 0x02: /* INCPU_J */
		{
		DELAY_SLOT;
		JUMP;
		break;
		}
	case 0x03: /* INCPU_JAL */
		{
		LINK;
		DELAY_SLOT;
		JUMP;
		break;
		}
	case 0x04: /* INCPU_BEQ */		
		if (!RS && !RT) { // special case: B
			DELAY_SLOT;
			JUMP;
		} else {
			BRANCH_DELAY_TRUE(ICMP_EQ(R(RS), R(RT)));
		}
		break;
	case 0x05: /* INCPU_BNE */		BRANCH_DELAY_TRUE(ICMP_NE(R(RS), R(RT)));			break;
	case 0x06: /* INCPU_BLEZ */		BRANCH_DELAY_TRUE(ICMP_SLE(R(RS),CONST(0)));	break;
	case 0x07: /* INCPU_BGTZ */		BRANCH_DELAY_TRUE(ICMP_SGT(R(RS),CONST(0)));	break;
	case 0x08: /* INCPU_ADDI */		LET32(RT, ADD(R32(RS), IMM32));					break; //XXX same??
	case 0x09: /* INCPU_ADDIU */	LET32(RT, ADD(R32(RS), IMM32));					break; //XXX same??
	case 0x0A: /* INCPU_SLTI */		LET_ZEXT(RT,ICMP_ULT(R(RS),IMM));				break; //XXX same??
	case 0x0B: /* INCPU_SLTIU */	LET_ZEXT(RT,ICMP_ULT(R(RS),IMM));				break; //XXX same??
	case 0x0C: /* INCPU_ANDI */		LET(RT,AND(R(RS), IMMU));						break;
	case 0x0D: /* INCPU_ORI */		LET(RT,OR(R(RS), IMMU));						break;
	case 0x0E: /* INCPU_XORI */		LET(RT,XOR(R(RS), IMMU));						break;
	case 0x0F: /* INCPU_LUI */		LET(RT,SHL(IMMU,CONST(16)));				break;
	case 0x10: /* INCPU_COP0 */
		switch (GetFMT) {
			case 0x00: /* INCOP0_MFC0 */	BAD;
			case 0x04: /* INCOP0_MTC0 */	BAD;
			case 0x10: /* INCOP0_TLB */
				switch(GetFunction) {
					case 0x01: /* INCOP0TLB_TLBR */	BAD;
					case 0x02: /* INCOP0TLB_TLBWI */	BAD;
					case 0x06: /* INCOP0TLB_TLBWR */	BAD;
					case 0x08: /* INCOP0TLB_TLBP */	BAD;
					case 0x18: /* INCOP0TLB_ERET */	BAD;
					default:
						printf("INVALID %s:%d\n", __func__, __LINE__); exit(1);
				}
			default:
				printf("INVALID %s:%d\n", __func__, __LINE__); exit(1);
		}
	case 0x11: /* INCPU_COP1 */
		switch(GetFMT) {
			case 0x00: /* INCOP1_MFC1 */	BAD;
			case 0x01: /* INCOP1_DMFC1 */	BAD;
			case 0x02: /* INCOP1_CFC1 */	BAD;
			case 0x04: /* INCOP1_MTC1 */	BAD;
			case 0x05: /* INCOP1_DMTC1 */	BAD;
			case 0x06: /* INCOP1_CTC1 */	BAD;
			case 0x08: /* INCOP1_BC */	BAD;
			case 0x10: /* INCOP1_S */
				switch(GetCOP1FloatInstruction) {
					case 0x00: /* INCOP1_ADD */	BAD;
					case 0x01: /* INCOP1_SUB */	BAD;
					case 0x02: /* INCOP1_MUL */	BAD;
					case 0x03: /* INCOP1_DIV */	BAD;
					case 0x04: /* INCOP1_SQRT */	BAD;
					case 0x05: /* INCOP1_ABS */	BAD;
					case 0x06: /* INCOP1_MOV */	BAD;
					case 0x07: /* INCOP1_NEG */	BAD;
					case 0x08: /* INCOP1_ROUND_L */	BAD;
					case 0x09: /* INCOP1_TRUNC_L */	BAD;
					case 0x0A: /* INCOP1_CEIL_L */	BAD;
					case 0x0B: /* INCOP1_FLOOR_L */	BAD;
					case 0x0C: /* INCOP1_ROUND_W */	BAD;
					case 0x0D: /* INCOP1_TRUNC_W */	BAD;
					case 0x0E: /* INCOP1_CEIL_W */	BAD;
					case 0x0F: /* INCOP1_FLOOR_W */	BAD;
					case 0x20: /* INCOP1_CVT_S */	BAD;
					case 0x21: /* INCOP1_CVT_D */	BAD;
					case 0x24: /* INCOP1_CVT_W */	BAD;
					case 0x25: /* INCOP1_CVT_L */	BAD;
					case 0x30: /* INCOP1_C_F */	BAD;
					case 0x31: /* INCOP1_C_UN */	BAD;
					case 0x32: /* INCOP1_C_EQ */	BAD;
					case 0x33: /* INCOP1_C_UEQ */	BAD;
					case 0x34: /* INCOP1_C_OLT */	BAD;
					case 0x35: /* INCOP1_C_ULT */	BAD;
					case 0x36: /* INCOP1_C_OLE */	BAD;
					case 0x37: /* INCOP1_C_ULE */	BAD;
					case 0x38: /* INCOP1_C_SF */	BAD;
					case 0x39: /* INCOP1_C_NGLE */	BAD;
					case 0x3A: /* INCOP1_C_SEQ */	BAD;
					case 0x3B: /* INCOP1_C_NGL */	BAD;
					case 0x3C: /* INCOP1_C_LT */	BAD;
					case 0x3D: /* INCOP1_C_NGE */	BAD;
					case 0x3E: /* INCOP1_C_LE */	BAD;
					case 0x3F: /* INCOP1_C_NGT */	BAD;
					default:
						printf("INVALID %s:%d\n", __func__, __LINE__); exit(1);
				}
			case 0x11: /* INCOP1_D */	BAD;
			case 0x14: /* INCOP1_W */	BAD;
			case 0x15: /* INCOP1_L */	BAD;
			default:
				printf("INVALID %s:%d\n", __func__, __LINE__); exit(1);
		}
	case 0x14: /* INCPU_BEQL */		BRANCH_DELAY_TRUE_LIKELY(ICMP_EQ(R(RS), R(RT)));			break;
	case 0x15: /* INCPU_BNEL */		BRANCH_DELAY_TRUE_LIKELY(ICMP_NE(R(RS), R(RT)));			break;
	case 0x16: /* INCPU_BLEZL */	BRANCH_DELAY_TRUE_LIKELY(ICMP_SLE(R(RS), CONST(0)));	break;
	case 0x17: /* INCPU_BGTZL */	BRANCH_DELAY_TRUE_LIKELY(ICMP_SGT(R(RS), CONST(0)));	break;
	case 0x18: /* INCPU_DADDI */	LET(RT,ADD(R(RS),IMM));								break; //XXX same??
	case 0x19: /* INCPU_DADDIU */	LET(RT,ADD(R(RS),IMM));								break; //XXX same??
	case 0x1A: /* INCPU_LDL */		BAD;
	case 0x1B: /* INCPU_LDR */		BAD;
	case 0x20: /* INCPU_LB */		LOAD8S(RT,ADD(R32(RS),IMM32));							break;
	case 0x21: /* INCPU_LH */		LOAD16S(RT,ADD(R32(RS),IMM32));						break;
	case 0x22: /* INCPU_LWL */		BAD;
	case 0x23: /* INCPU_LW */		LOAD32(RT,ADD(R32(RS),IMM32));							break; //XXX ignores misalignment
	case 0x24: /* INCPU_LBU */		LOAD8(RT,ADD(R32(RS),IMM32));							break;
	case 0x25: /* INCPU_LHU */		LOAD16(RT,ADD(R32(RS),IMM32));							break;
	case 0x26: /* INCPU_LWR */		BAD;
	case 0x27: /* INCPU_LWU */		BAD;
	case 0x28: /* INCPU_SB */		STORE8(R(RT),ADD(R32(RS),IMM32));							break;
	case 0x29: /* INCPU_SH */		STORE16(R(RT),ADD(R32(RS),IMM32));							break;
	case 0x2A: /* INCPU_SWL */		BAD;
	case 0x2B: /* INCPU_SW */		STORE32(R(RT),ADD(R32(RS),IMM32));						break;
	case 0x2C: /* INCPU_SDL */	BAD;
	case 0x2D: /* INCPU_SDR */	BAD;
	case 0x2E: /* INCPU_SWR */	BAD;
	case 0x2F: /* INCPU_CACHE */	break; /* no-op */
	case 0x30: /* INCPU_LL */		break; /* no-op */
	case 0x31: /* INCPU_LWC1 */	BAD;
	case 0x34: /* INCPU_LLD */		break; /* no-op */
	case 0x35: /* INCPU_LDC1 */	BAD;
	case 0x37: /* INCPU_LD */	BAD;
	case 0x38: /* INCPU_SC */		break; /* no-op */
	case 0x39: /* INCPU_SWC1 */	BAD;
	case 0x3C: /* INCPU_SCD */	BAD;
	case 0x3D: /* INCPU_SDC1 */	BAD;
	case 0x3F: /* INCPU_SD */	BAD;
	default:
		printf("INVALID %s:%d\n", __func__, __LINE__); exit(1);
	}

	int dummy1;
	addr_t dummy2;
	return arch_mips_tag_instr(cpu, pc, &dummy1, &dummy2);
}


//printf("%s:%d PC=$%04X\n", __func__, __LINE__, pc);
//printf("%s:%d\n", __func__, __LINE__);
