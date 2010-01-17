#include "llvm/Instructions.h"

#define OPT_LOCAL_REGISTERS //XXX

#include "libcpu.h"
#include "libcpu_llvm.h"
#include "frontend.h"
#include "fapra_internal.h"

using namespace llvm;

//////////////////////////////////////////////////////////////////////
// fapra: instruction decoding
//////////////////////////////////////////////////////////////////////

#define RS	((instr >> 21) & 0x1F)
#define RT	((instr >> 16) & 0x1F)
#define RD	((instr >> 11) & 0x1F)


#define GetSA	((instr >> 6) & 0x1F)
#define GetTarget (instr & 0x3FFFFFF)

#define GetFunction (instr & 0x3F)
//#define GetSpecialInstruction GetFunction
#define GetRegimmInstruction RT
#define GetFMT RS
//#define GetCacheType (RT&BitM2)
//#define GetCacheInstr ((RT>>2)&BitM3)
#define GetCOP1FloatInstruction GetFunction

#define fapra_BRANCH_TARGET ((uint32_t)(pc + 4 + (uint32_t)(((sint32_t)(sint16_t)GetImmediate<<2))))

enum {
  LDW = 0x10,
  STW = 0x11,
  LDB = 0x1C,
  STB = 0x1D,

  LDIH = 0x20,
  LDIL = 0x21,

  JMP = 0x30,
  BRA = 0x34,
  BZ = 0x35,
  BNZ = 0x36,
  NOP = 0x32,
  CALL = 0x33,
  BL = 0x37,
  RFE = 0x3F,

  ADDI = 0x0F,
  ADD = 0x00,
  SUB = 0x01,
  AND = 0x02,
  OR = 0x03,
  NOT = 0x05,
  SARI = 0x0B,
  SAL = 0x06,
  SAR = 0x07,
  MUL = 0x08,

  PERM = 0x09,
  RDC8 = 0x0A,
  TGE = 0x0C,
  TSE = 0x0D
};

// Instruction decoding helper functions.
static inline uint32_t opc(uint32_t ins) {
  return ins >> 26;
}

static inline uint32_t rd(uint32_t ins) {
  return (ins >> 21) & 0x1F;
}

static inline uint32_t ra(uint32_t ins) {
  return (ins >> 16) & 0x1F;
}

static inline uint32_t rb(uint32_t ins) {
  return (ins >> 11) & 0x1F;
}

static inline uint32_t simm(uint32_t ins) {
  return ((int32_t) ((ins & 0xFFFF) << 16)) >> 16;
}

static inline uint32_t imm(uint32_t ins) {
  return ins & 0xFFFF;
}

#define RD ((instr >> 21) & 0x1F)
#define RA ((instr >> 16) & 0x1F)
#define RB ((instr >> 11) & 0x1F)
#define GetImmediate (instr & 0xFFFF)

//////////////////////////////////////////////////////////////////////
// tagging
//////////////////////////////////////////////////////////////////////

#include "tag.h"
int arch_fapra_tag_instr(cpu_t *cpu, addr_t pc, tag_t *tag, addr_t *new_pc,
						 addr_t *next_pc) {
	uint32_t ins = INSTR(pc);

	switch (opc(ins)) {
	case RFE:
	case PERM:
	case RDC8:
	case TGE:
	case TSE:
	default:
	  fprintf(stderr, "Illegal instruction!\n");
	  exit(EXIT_FAILURE);
	case LDW:
	case STW:
	case LDB:
	case STB:
	case LDIH:
	case LDIL:
	case ADDI:
	case ADD:
	case SUB:
	case AND:
	case OR:
	case NOT:
	case SARI:
	case SAL:
	case SAR:
	case MUL:
	case NOP:
		*tag = TAG_CONTINUE;
		break;
	case JMP:
		*tag = TAG_RET;
		break;
	case BRA:
		*tag = TAG_BRANCH;
		*new_pc = pc + simm(ins);
		break;
	case BZ:
	case BNZ:
		*tag = TAG_COND_BRANCH;
		*new_pc = pc + simm(ins);
		break;
	case CALL:
		*tag = TAG_CALL;
		*new_pc = NEW_PC_NONE;
		break;
	case BL:
		*tag = TAG_CALL;
		*new_pc = pc + simm(ins);
		break;
	}

	*next_pc = pc + 4;

	return 4;
}

//////////////////////////////////////////////////////////////////////

Value *
arch_fapra_get_imm(cpu_t *cpu, uint32_t instr, uint32_t bits, bool sext,
  BasicBlock *bb) {
	uint64_t imm;
	if (sext)
		imm = (uint64_t)(sint16_t)GetImmediate;
	else
		imm = (uint64_t)(uint16_t)GetImmediate;

	return ConstantInt::get(getIntegerType(bits? bits : cpu->info.word_size), imm);
}

#define IMM arch_fapra_get_imm(cpu, instr, 0, true, bb)
#define IMMU arch_fapra_get_imm(cpu, instr, 0, false, bb)
#define IMM32 arch_fapra_get_imm(cpu, instr, 32, true, bb)

//////////////////////////////////////////////////////////////////////

Value *
arch_fapra_get_sa(cpu_t *cpu, uint32_t instr, uint32_t bits, BasicBlock *bb) {
	return ConstantInt::get(getIntegerType(bits? bits : cpu->info.word_size), GetSA);
}

#define SA arch_fapra_get_sa(cpu, instr, 0, bb)
#define SA32 arch_fapra_get_sa(cpu, instr, 32, bb)

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

#define LET_PC(v) new StoreInst(v, cpu->ptr_PC, bb)

#define LINKr(i) LET32(i, CONST((uint64_t)(sint64_t)(sint32_t)pc+8))

#define LINK LINKr(31)

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

Value *
arch_fapra_translate_cond(cpu_t *cpu, addr_t pc, BasicBlock *bb)
{
	uint32_t instr = INSTR(pc);

	LOG("cond (%08llx) %08x\n", pc, instr);

	switch(instr >> 26) {
	case 0x01: /* INCPU_REGIMM */
		switch (GetRegimmInstruction) {
			case 0x00: /* INCPUR_BLTZ */	return ICMP_SLT(R(RS),CONST(0));
			case 0x01: /* INCPUR_BGEZ */	return ICMP_SGE(R(RS),CONST(0));
			case 0x02: /* INCPUR_BLTZL */	return ICMP_SLT(R(RS),CONST(0));
			case 0x03: /* INCPUR_BGEZL */	return ICMP_SGE(R(RS),CONST(0));
			case 0x10: /* INCPUR_BLTZAL */	return ICMP_SLT(R(RS),CONST(0));
			case 0x11: /* INCPUR_BGEZAL */	return ICMP_SGE(R(RS),CONST(0));
			case 0x12: /* INCPUR_BLTZALL */	return ICMP_SLT(R(RS),CONST(0));
			case 0x13: /* INCPUR_BGEZALL */	return ICMP_SGE(R(RS),CONST(0));
		}
	case 0x04: /* INCPU_BEQ */		
		if (!RS && !RT) // special case: B
			return NULL; /* should never be reached */
		else
			return ICMP_EQ(R(RS), R(RT));
	case 0x05: /* INCPU_BNE */		return ICMP_NE(R(RS), R(RT));
	case 0x06: /* INCPU_BLEZ */		return ICMP_SLE(R(RS),CONST(0));
	case 0x07: /* INCPU_BGTZ */		return ICMP_SGT(R(RS),CONST(0));
	case 0x14: /* INCPU_BEQL */		return ICMP_EQ(R(RS), R(RT));
	case 0x15: /* INCPU_BNEL */		return ICMP_NE(R(RS), R(RT));
	case 0x16: /* INCPU_BLEZL */	return ICMP_SLE(R(RS), CONST(0));
	case 0x17: /* INCPU_BGTZL */	return ICMP_SGT(R(RS), CONST(0));
  default: assert(0 && "Not handled"); return NULL;
	}
}

int
arch_fapra_translate_instr(cpu_t *cpu, addr_t pc, BasicBlock *bb)
{
#define BAD printf("%s:%d\n", __func__, __LINE__); exit(1);
#define LOGX LOG("%s:%d\n", __func__, __LINE__);

	uint32_t instr = INSTR(pc);

	LOG("translating (%08llx) %08x\n", pc, instr);

	switch (opc(instr)) {
	case PERM:
	case RDC8:
	case TGE:
	case TSE:
	case SARI:
	case RFE:
	default:
	  fprintf(stderr, "Illegal instruction!\n");
	  exit(EXIT_FAILURE);
	case LDW:
	  break;
	case STW:
	  break;
	case LDB:
	  break;
	case STB:
	  break;
	case LDIH:
		LET(RD, OR(AND(R(RD), CONST(0xFFFF)), CONST(GetImmediate << 16)));
		break;
	case LDIL:
		LET(RD, OR(AND(R(RD), CONST(0xFFFF0000)), IMMU));
		break;
	case JMP:
	  break;
	case BRA:
	  break;
	case BZ:
	  break;
	case BNZ:
	  break;
	case NOP:
		// Emit nothing.
		break;
	case CALL:
	  break;
	case BL:
	  break;
	case ADDI:
		LET(RD, ADD(R(RA), IMM));
		break;
	case ADD:
		LET(RD, ADD(R(RA), R(RB)));
		break;
	case SUB:
		LET(RD, SUB(R(RA), R(RB)));
		break;
	case AND:
		LET(RD, AND(R(RA), R(RB)));
		break;
	case OR:
		LET(RD, OR(R(RA), R(RB)));
		break;
	case NOT:
		LET(RD, NOT(R(RA)));
		break;
	case SAL:
		LET(RD, SHL(R(RA), R(RB)));
		break;
	case SAR:
		LET(RD, ASHR(R(RA), R(RB)));
		break;
	case MUL:
		LET(RD, MUL(R(RA), R(RB)));
		break;
	}

	tag_t dummy1;
	addr_t dummy2, dummy3;
	return arch_fapra_tag_instr(cpu, pc, &dummy1, &dummy2, &dummy3);
}
