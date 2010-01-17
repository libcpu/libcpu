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

#define INST_SIZE 4

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

	*next_pc = pc + INST_SIZE;

	return INST_SIZE;
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

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

#define LET_PC(v) new StoreInst(v, cpu->ptr_PC, bb)

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

Value *
arch_fapra_translate_cond(cpu_t *cpu, addr_t pc, BasicBlock *bb)
{
	uint32_t instr = INSTR(pc);

	LOG("cond (%08llx) %08x\n", pc, instr);

	switch (opc(instr)) {
	default:
		fprintf(stderr, "Illegal instruction!\n");
		exit(EXIT_FAILURE);
	case BZ:
		// Emit nothing.
		return ICMP_EQ(R(RA), CONST(0));
		break;
	case BNZ:
		return ICMP_NE(R(RA), CONST(0));
		break;
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
		LOAD32(RD, ADD(R(RA), IMM));
		break;
	case STW:
		STORE32(R(RD), ADD(R(RA), IMM));
		break;
	case LDB:
		LOAD8(RD, ADD(R(RA), IMM));
		break;
	case STB:
		STORE8(R(RD), ADD(R(RA), IMM));
		break;
	case LDIH:
		LET(RD, OR(AND(R(RD), CONST(0xFFFF)), CONST(GetImmediate << 16)));
		break;
	case LDIL:
		LET(RD, OR(AND(R(RD), CONST(0xFFFF0000)), IMMU));
		break;
	case JMP:
		LET_PC(R(RA));
		break;
	case BRA:
//		LET_PC(CONST(pc + simm(instr)));
		break;
	case BZ:
		// Emit nothing.
		break;
	case BNZ:
		// Emit nothing.
		break;
	case NOP:
		// Emit nothing.
		break;
	case CALL:
		LET_PC(R(RA));
		LET(RD, CONST(pc + INST_SIZE));
		break;
	case BL:
//		LET_PC(CONST(pc + simm(instr)));
		LET(RD, CONST(pc + INST_SIZE));
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

	return INST_SIZE;
}
