#define OPT_LOCAL_REGISTERS //XXX

#include "libcpu.h"
#include "cpu_generic.h"
#include "arch/m88k/libcpu_m88k.h"
#include "m88k_internal.h"
#include "m88k_insn.h"

using namespace llvm;

extern Value* m88k_ptr_C; // Carry

#define ptr_PSR    ptr_r32[32]
#define ptr_TRAPNO ptr_r32[33]

//////////////////////////////////////////////////////////////////////
// TAGGING
//////////////////////////////////////////////////////////////////////

#include "tag.h"

static bool
arch_m88k_fix_return_pc(m88k_insn const &insn, addr_t *next_pc)
{
	if (insn.rd() != 1 || insn.rs1() != 1 || insn.format() != M88K_IFMT_REG)
		return false;

	//assert(insn.rs1() == 1 && "only r1 as source is supported");
	//assert(insn.format() == M88K_IFMT_REG && "only immediate format is supported");

	switch (insn.opcode()) {
		case M88K_OPC_ADD:
		case M88K_OPC_ADDU:
			*next_pc += insn.immediate();
			return true;
		case M88K_OPC_SUB:
		case M88K_OPC_SUBU:
			*next_pc -= insn.immediate();
			return true;
		default:
			//assert(0 && "the opcode alters r1 but it is not handled");
			return false;
	}
}

int arch_m88k_tag_instr(cpu_t *cpu, addr_t pc, tag_t *tag, addr_t *new_pc, addr_t *next_pc)
{
	m88k_insn instr = INSTR(pc);

	switch (instr.opcode()) {
		case M88K_OPC_ILLEGAL:
			*tag = TAG_TRAP;
			break;

		case M88K_OPC_JMP:
		case M88K_OPC_JMP_N:
			*tag = TAG_RET;
			break;

		case M88K_OPC_JSR:
		case M88K_OPC_JSR_N:
			*tag = TAG_CALL;
			*new_pc = NEW_PC_NONE;
			break;

		case M88K_OPC_BR:
		case M88K_OPC_BR_N:
			*new_pc = pc + (instr.branch26() << 2);
			*tag = TAG_BRANCH;
			break;

		case M88K_OPC_BSR:
		case M88K_OPC_BSR_N:
			*new_pc = pc + (instr.branch26() << 2);
			*tag = TAG_CALL;
			break;

		case M88K_OPC_BB0:
		case M88K_OPC_BB0_N:
		case M88K_OPC_BB1:
		case M88K_OPC_BB1_N:
			*new_pc = pc + (instr.branch16() << 2);
			*tag = TAG_COND_BRANCH;
			break;

		case M88K_OPC_BCND:
		case M88K_OPC_BCND_N:
			if (instr.mb() == M88K_BCND_NEVER)
				*tag = TAG_CONTINUE;
			else {
				*new_pc = pc + (instr.branch16() << 2);
				if (instr.mb() == M88K_BCND_ALWAYS)
					*tag = TAG_BRANCH;
				else
					*tag = TAG_COND_BRANCH;
			}
			break;

		case M88K_OPC_TB1:
			if (instr.rs1() == 0) {
				*tag = TAG_CONTINUE;
				break;
			}
		case M88K_OPC_TB0:
		case M88K_OPC_TBND:
		case M88K_OPC_TCND:
			*tag = TAG_TRAP; //XXX COND_TRAP
			break;

		default:
			*tag = TAG_CONTINUE;
			break;
	}

	if (instr.is_delaying()) {
		*tag |= TAG_DELAY_SLOT;
		*next_pc = pc + 8;

		//
		// In case of subroutine call, check
		// the instruction in the delay slot
		// modifes the return addess.
		//
		switch (instr.opcode()) {
			case M88K_OPC_BSR_N:
			case M88K_OPC_JSR_N:
				arch_m88k_fix_return_pc(INSTR(pc+4), next_pc);
				break;
			default:
				break;
		}
	} else {
		*next_pc = pc + 4;
	}

	return 4;
}

//////////////////////////////////////////////////////////////////////

enum { I_SEXT = 1, I_UPPER = 2, I_BS = 4 };

static Value *
arch_m88k_get_imm(cpu_t *cpu, m88k_insn const &instr, uint32_t bits,
  unsigned flags, BasicBlock *bb)
{
	uint64_t imm;
	if (flags & I_SEXT)
		imm = (uint64_t)instr.immediate();
	else
		imm = (uint64_t)(uint16_t)instr.immediate();

	if (flags & I_UPPER) {
		imm <<= 16;
		if (flags & I_BS)
			imm |= 0xffff;
	} else if (flags & I_BS)
		imm |= 0xffff0000;

	return ConstantInt::get(getIntegerType(bits ? bits : cpu->reg_size), imm);
}

#define IMM    arch_m88k_get_imm(cpu, instr, 0, I_SEXT, bb)
#define UIMM   arch_m88k_get_imm(cpu, instr, 0, 0, bb)

#define IMM_U  arch_m88k_get_imm(cpu, instr, 0, I_UPPER, bb)
#define IMM_B  arch_m88k_get_imm(cpu, instr, 0, I_BS, bb)
#define IMM_UB arch_m88k_get_imm(cpu, instr, 0, I_UPPER | I_BS, bb)

#define IMM32  arch_m88k_get_imm(cpu, instr, 32, I_SEXT, bb)

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

#define LET_PC(v)		new StoreInst(v, cpu->ptr_PC, bb)
#define LET_TRAPNO(v)	new StoreInst(v, cpu->ptr_TRAPNO, bb)

#define TRAP(n) do { \
	LET_TRAPNO(CONST32(instr.vec9())); \
	LET_PC(CONST(pc + 4)); \
} while(0)

#define JMP_BB(b) 		BranchInst::Create(b, bb)

#define LINKr(i, d)		LET(i, CONST((uint64_t)(sint64_t)(sint32_t)pc+(4<<(d))))

#define LINK(d)			LINKr(1, d)

//////////////////////////////////////////////////////////////////////

#define GET_CARRY()		(SEXT32(new LoadInst(m88k_ptr_C, "", false, bb)))
#define SET_CARRY(v)	(new StoreInst(v, m88k_ptr_C, bb))

//////////////////////////////////////////////////////////////////////
// FLOATING POINT
//////////////////////////////////////////////////////////////////////

static Value *
arch_m88k_get_fpr(cpu_t *cpu, bool xfr, m88k_reg_t r, uint32_t t,
	BasicBlock *bb)
{
	if (xfr) {
		switch (t) {
			case 0: return FPBITCAST32(FR80(r));
			case 1: return FPBITCAST64(FR80(r));
			case 2: return FR80(r);
		}
	} else {
		switch (t) {
			case 0: return FPBITCAST32(R32(r));
			case 1: if (r < 2)
						return FPCONST64(0);
					else
						return FPBITCAST64(OR(SHL(ZEXT64(R32(r & ~1)), CONST64(32)), ZEXT64(R32(r | 1))));
			case 2: abort(); // can't happen
		}
	}
}

static void
arch_m88k_set_fpr(cpu_t *cpu, bool xfr, m88k_reg_t r,
	uint32_t t, Value *v, BasicBlock *bb)
{
	if (xfr) {
		switch(t) {
			case 0: LETFP(r, FPBITCAST80(FPBITCAST32(v))); break;
			case 1: LETFP(r, FPBITCAST80(FPBITCAST64(v))); break;
			case 2: LETFP(r, FPBITCAST80(v)); break;
		}
	} else {
		Value *iv;
		switch (t) {
			case 0: LET32(r, IBITCAST32(FPBITCAST32(v))); break;
			case 1: iv = IBITCAST64(FPBITCAST64(v));
					LET32(r & ~1, TRUNC32(LSHR(iv, CONST64(32))));
					LET32(r | 1, TRUNC32(iv));
					break;
			case 2: abort(); // can't happen
		}
	}
}

#define GET_FPR(x,r,t)   arch_m88k_get_fpr(cpu, x, r, t, bb)
#define SET_FPR(x,r,t,v) arch_m88k_set_fpr(cpu, x, r, t, v, bb)

enum {
	FP_OP_CMP,
	FP_OP_ADD,
	FP_OP_SUB,
	FP_OP_MUL,
	FP_OP_DIV,
	FP_OP_REM,
	FP_OP_CMP_OEQ,
	FP_OP_CMP_OGE
};

#undef getType // XXX clash!

/*
 * The Motorola 88K can operate on three different
 * types of floating point using the same register set,
 * and can mix them in a single instruction so that,
 * for example, you can add a double to a single and
 * put the result in an extended float.
 *
 * LLVM mandates both operands of a binary operation
 * shall be the same size. This function overrides 
 * cpu_generic.h macros and extends the operation to
 * the biggest of the two operands.
 */
static Value *
arch_m88k_fp_op(cpu_t *cpu, unsigned op, Value *a, Value *b,
	BasicBlock *bb)
{
	Type::TypeID typeid_a, typeid_b;
	Type const *type_a = a->getType();
	Type const *type_b = b->getType();

	typeid_a = type_a->getTypeID();
	typeid_b = type_b->getTypeID();

	// Both a and b shall be of the same size,
	// extend to the biggest.
	if (typeid_a != typeid_b) {
		if (typeid_a > typeid_b)
			b = new FPExtInst(b, type_a, "", bb);
		else
			a = new FPExtInst(a, type_b, "", bb);
	}

	switch (op) {
		case FP_OP_ADD:		return FPADD(a, b);
		case FP_OP_SUB:		return FPSUB(a, b);
		case FP_OP_MUL:		return FPMUL(a, b);
		case FP_OP_DIV:		return FPDIV(a, b);
		case FP_OP_REM:		return FPREM(a, b);
		case FP_OP_CMP_OEQ:	return FPCMP_OEQ(a, b);
		case FP_OP_CMP_OGE:	return FPCMP_OGE(a, b);
		default:			assert(0 && "Invalid FP operation");
							return NULL;
	}
}

#undef FPADD
#undef FPSUB
#undef FPMUL
#undef FPDIV
#undef FPREM
#undef FPCMP_OEQ
#undef FPCMP_OGE
#define FPADD(a, b) arch_m88k_fp_op(cpu, FP_OP_ADD, a, b, bb)
#define FPSUB(a, b) arch_m88k_fp_op(cpu, FP_OP_SUB, a, b, bb)
#define FPMUL(a, b) arch_m88k_fp_op(cpu, FP_OP_MUL, a, b, bb)
#define FPDIV(a, b) arch_m88k_fp_op(cpu, FP_OP_DIV, a, b, bb)
#define FPREM(a, b) arch_m88k_fp_op(cpu, FP_OP_REM, a, b, bb)
#define FPCMP_OGE(a, b) arch_m88k_fp_op(cpu, FP_OP_CMP_OGE, a, b, bb)
#define FPCMP_OEQ(a, b) arch_m88k_fp_op(cpu, FP_OP_CMP_OEQ, a, b, bb)

//////////////////////////////////////////////////////////////////////
// CONDITIONAL EXECUTION
//////////////////////////////////////////////////////////////////////

static Value *
arch_m88k_bcnd_cond(cpu_t *cpu, Value *src1, m88k_bcnd_t cond, BasicBlock *bb)
{
	switch (cond) {
		case M88K_BCND_EQ0: return ICMP_EQ(src1, CONST32(0));
		case M88K_BCND_NE0: return ICMP_NE(src1, CONST32(0));
		case M88K_BCND_GT0: return ICMP_SGT(src1, CONST32(0));
		case M88K_BCND_LT0: return ICMP_SLT(src1, CONST32(0));
		case M88K_BCND_LE0: return ICMP_SLE(src1, CONST32(0));
		case M88K_BCND_GE0: return ICMP_SGE(src1, CONST32(0));
		case M88K_BCND_4: // rs1 != 0x80000000 && rs1 < 0
			return AND(ICMP_NE(src1, CONST32(0x80000000)),
				ICMP_SLT(src1, CONST32(0)));

		case M88K_BCND_5: // (rs1 & 0x7fffffff) != 0
			return ICMP_NE(AND(src1, CONST32(0x7fffffff)),
				CONST32(0));

		case M88K_BCND_6: // rs != 0x80000000 && rs <= 0 
			return AND(ICMP_NE(src1, CONST32(0x80000000)),
				ICMP_SLE(src1, CONST32(0)));

		case M88K_BCND_7: // rs != 0x80000000
			return ICMP_NE(src1, CONST32(0x80000000));

		case M88K_BCND_8: // rs == 0x80000000
			return ICMP_EQ(src1, CONST32(0x80000000));

		case M88K_BCND_9: // rs > 0 || rs == 0x80000000
			return OR(ICMP_SGT(src1, CONST(0)),
				ICMP_EQ(src1, CONST32(0x80000000)));

		case M88K_BCND_10: // (rs1 & 0x7fffffff) == 0
			return ICMP_EQ(AND(src1, CONST32(0x7fffffff)),
				CONST32(0));

		case M88K_BCND_11: // rs1 >= 0 || rs1 == 0x80000000
			return OR(ICMP_SGE(src1, CONST(0)),
				ICMP_EQ(src1, CONST32(0x80000000)));

		case M88K_BCND_NEVER:
		case M88K_BCND_ALWAYS:
			return NULL;
	}
}

Value *
arch_m88k_recompile_cond(cpu_t *cpu, addr_t pc, BasicBlock *bb)
{
	m88k_insn insn(INSTR(pc));
	uint8_t bit = insn.mb();
	switch (insn.opcode()) {
		case M88K_OPC_BCND:
		case M88K_OPC_BCND_N:
			return arch_m88k_bcnd_cond(cpu, R32(insn.rs1()), (m88k_bcnd_t)bit, bb);
		case M88K_OPC_BB0:
		case M88K_OPC_BB0_N:
			return ICMP_EQ(AND(R32(insn.rs1()), CONST32(1 << bit)), CONST(0));
		case M88K_OPC_BB1:
		case M88K_OPC_BB1_N:
			return ICMP_NE(AND(R32(insn.rs1()), CONST32(1 << bit)), CONST(0));
		default:
			return NULL;
	}
}

//////////////////////////////////////////////////////////////////////
// COMPLEX INSTRUCTIONS
//////////////////////////////////////////////////////////////////////

#define COMPUTE_CARRY(src1, src2, result) \
	(AND(ICMP_NE(src2, CONST(0)), ICMP_ULT(result, src1)))

#define COMPUTE_BORROW(src1, src2, result) \
	(AND(ICMP_NE(src2, CONST(0)), ICMP_UGT(result, src1)))

static void
arch_m88k_addsub(cpu_t *cpu, addr_t pc, m88k_reg_t dst, Value *src1, Value *src2,
	bool sub, unsigned carry, bool overflow_trap,
	BasicBlock *bb)
{
	Value *result;

	if (sub)
		result = SUB(src1, src2);
	else
		result = ADD(src1, src2);

	if (overflow_trap) {
		//XXX trap on overflow!
	}

	if (carry & M88K_CARRY_IN) {
		if (sub)
			result = SUB(result, GET_CARRY());
		else
			result = ADD(result, GET_CARRY());
	}

	if (carry & M88K_CARRY_OUT) {
		if (sub)
			SET_CARRY(COMPUTE_BORROW(src1, src2, result));
		else
			SET_CARRY(COMPUTE_CARRY(src1, src2, result));
	}

	LET32(dst, result);
}

static void
arch_m88k_shift(cpu_t *cpu, m88k_opcode_t opc, m88k_reg_t rd, Value *src1, Value *src2,
	uint32_t width, uint32_t offset, bool ifmt,
	BasicBlock *bb)
{
	uint32_t wmask = (width != 0) ? (1 << width) - 1 : (-1U);

	switch (opc) {
		case M88K_OPC_ROT: // (rs1 >> bits)|(rs1 << (32-bits))
			if (ifmt)
				LET32(rd, OR(LSHR(src1, CONST32(offset)),
							 SHL(src1, CONST32(32 - offset))));
			else
				LET32(rd, OR(LSHR(src1, AND(src2, CONST32(0x1f))),
							 SHL(src1, SUB(CONST32(32),
										   AND(src2, CONST32(0x1f))))));
			break;

		case M88K_OPC_MAK: // (rs1 & wmask) << offset
			if (ifmt) 
				LET32(rd, SHL(AND(src1, CONST32(wmask)), CONST32(offset)));
			else
				LET32(rd, SHL(src1, src2));
			break;

		case M88K_OPC_SET: // rs1 | (wmask << n)
			if (ifmt)
				LET32(rd, OR(src1, CONST32(wmask << offset)));
			else
				LET32(rd, OR(src1, SHL(CONST32(wmask),
					AND(src2, CONST32(0x1f)))));
			break;

		case M88K_OPC_CLR: // rs1 & ~(wmask << n)
			if (ifmt)
				LET32(rd, AND(src1, CONST32(~(wmask << offset))));
			else
				LET32(rd, AND(src1, COM(SHL(CONST32(wmask),
					AND(src2, CONST32(0x1f))))));
			break;

		case M88K_OPC_EXT: // ((int)rs1 >> n) & wmask
			if (ifmt) {
				if (width == 0) {
					LET32(rd, ASHR(src1, CONST32(offset)));
				} else {
					unsigned nb = 32 - (offset + width);
					LET32(rd, ASHR(SHL(src1, CONST32(nb & 0x1f)), CONST32((offset + nb) & 0x1f)));
				}
			} else
				LET32(rd, ASHR(src1, AND(src2, CONST32(0x1f))));
			break;

		case M88K_OPC_EXTU: // (rs1 >> n) & wmask
			if (ifmt)
				LET32(rd, AND(LSHR(src1, CONST32(offset)), CONST32(wmask)));
			else
				LET32(rd, LSHR(src1, AND(src2, CONST32(0x1f))));
			break;

		default:
			abort();
	}
}

/*
 * Motorola 88000 optimized cmp:
 *
 * if (s1 == s2)
 *     dst = 0x5aa4;
 * else {
 *     dst = 0xa008;
 *     if ((u)s1 > (u)s2) dst |= 0x900; else dst |= 0x600;
 *     if ((s)s1 > (s)s2) dst |= 0x90;  else dst |= 0x60;
 * }
 *
 * bit name meaning/pseudo-code
 *   2 eq   s1 == s2
 *   3 ne   s1 != s2
 *   4 gt   (s)s1 > (s)s2
 *   5 le   (s)s1 <= (s)s2
 *   6 lt   (s)s1 < (s)s2
 *   7 ge   (s)s1 >= (s)s2
 *   8 hi   (u)s1 > (u)s2
 *   9 ls   (u)s1 <= (u)s2
 *  10 lo   (u)s1 < (u)s2
 *  11 hs   (u)s1 >= (u)s2
 *  12 be   any_byte_eq(s1, s2)  (supported partially)
 *  13 nb   !any_byte_eq(s1, s2) (supported partially)
 *  14 he   any_half_eq(s1, s2)  (supported partially)
 *  15 nh   !any_half_eq(s1, s2) (supported partially)
 */
static void
arch_m88k_cmp(cpu_t *cpu, m88k_reg_t dst, Value *src1, Value *src2, BasicBlock *bb)
{
	LET32(dst,
		SELECT(ICMP_EQ(src1, src2),
			CONST32(0x5aa4), // EQ | LE | GE | LS | HS | BE | HE
			OR(CONST(0xa008), // NE | NB | NH
			   // (((-X) ^ 6) & 0xf) << Y will do the trick.
			   OR(SHL(AND(XOR(SEXT32(ICMP_UGT(src1, src2)), CONST32(6)),
					CONST32(0xf)), CONST32(8)),
				  SHL(AND(XOR(SEXT32(ICMP_SGT(src1, src2)), CONST32(6)),
					CONST32(0xf)), CONST32(4))))));
}

/* Small inline optimization for always true case (reg1 == reg2). */
static inline void
arch_m88k_cmp_reg(cpu_t *cpu, m88k_reg_t dst, m88k_reg_t src1, m88k_reg_t src2, BasicBlock *bb)
{
	if (src1 == src2)
		LET32(dst, CONST32(0x5aa4));
	else
		arch_m88k_cmp(cpu, dst, R32(src1), R32(src2), bb);
}

/*
 * Motorola 88000 fcmp(u):
 *
 * Broken optimization: (this works as long as we don't encounter NaNs,
 * it's fine for testing purpose but needs to be reimplemented).
 *
 *    s1 == s2  => 0x6a6
 *    s1 < s2   => 0x66a --+-> 0x666 ^ ( (s1 < s2) ^ 0x66)
 *    s1 >= s2  => 0x69a --+
 *
 * bit name meaning/pseudo-code
 *   0 un   is_unordered(s1) || is_unordered(s2)
 *   1 leg  !(is_unordered(s1) || is_unordered(s2))
 *   2 eq   s1 == s2
 *   3 ne   s1 != s2
 *   4 gt   s1 > s2
 *   5 le   s1 <= s2
 *   6 lt   s1 < s2
 *   7 ge   s1 >= s2
 *   8 ou   (s2 >= 0.0) && (is_unordered(s1 - s2) || (s1 < 0 || s1 > s2))
 *   9 ib   (s2 >= 0.0) && is_ordered(s1 - s2) && (s1 >= 0 && s1 <= s2)
 *  11 in   (s2 >= 0.0) && is_ordered(s1 - s1) && (s1 >= 0 && s1 <= s2)
 *  12 ob   (s2 >= 0.0) && (is_unordered(s1 - s2) || (s1 < 0 || s1 > s2))
 *  13 ue   is_unordered(s1 - s2) || (s1 == s2)
 *  14 lg   is_ordered(s1 - s2) && (s1 < s2 || s1 > s2)
 *  15 ug   is_unordered(s1 - s2) || (s1 > s2)
 *  16 ule  is_unordered(s1 - s2) || (s1 <= s2)
 *  17 ul   is_unordered(s1 - s2) || (s1 < s2)
 *  18 uge  is_unordered(s1 - s2) || (s1 >= s2)
 *
 * TBD Unordered!!
 *     Negative values in 's2' sets OU/IB/IN/OB to zero.
 *     If an operand is either signalling or quiet NaN,
 *     set the FINV bit in the FPSR; if using fcmp,
 *     raise also FPE.
 */
static void
arch_m88k_fcmp(cpu_t *cpu, m88k_reg_t dst, Value *src1, Value *src2, BasicBlock *bb)
{
	LET32(dst,
		SELECT(FPCMP_OEQ(src1, src2),
			CONST32(0x6a6), // LEG | EQ | LE | GE | IB | IN 
			XOR(CONST(0x6a6), //
				// 0x6a6 ^ ((((-(s1 > s2)) << 4) & 0xff) ^ 0xcc) is another trick to
				// satisfy GT/LE/LT/LE correct flags.
	 			XOR(AND(SHL(NEG(SEXT32(FPCMP_OGE(src1, src2))), CONST32(4)),
							CONST32(0xff)), CONST32(0xcc)))));
}

/*
 * Exchange Register/Memory (Word/Byte)
 *
 * NOTE: This operation locks the bus, so when we'll go system
 * emulation, this operation must be atomic.
 */
static void
arch_m88k_xmem(cpu_t *cpu, bool byte, m88k_reg_t rd, Value *src1, Value *src2, BasicBlock *bb)
{
	Value *reg_value;
	Value *mem_value;
	Value *address;

	reg_value = R32(rd);
	if (byte) {
		address = ADD(src1, src2);
		mem_value = arch_load8(cpu, address, bb);
	} else {
		address = ADD(src1, SHL(src2, CONST32(2)));
		mem_value = arch_load32_aligned(cpu, address, bb);
	}
	LET32(rd, mem_value);
	if (byte)
		STORE8(reg_value, address);
	else
		STORE32(reg_value, address);
}

//////////////////////////////////////////////////////////////////////
// TRANSLATOR
//////////////////////////////////////////////////////////////////////

int
arch_m88k_recompile_instr(cpu_t *cpu, addr_t pc, BasicBlock *bb)
{
#define BAD do { printf("%s:%d\n", __func__, __LINE__); exit(1); } while(0)
#define LOG printf("%s:%d\n", __func__, __LINE__);
	m88k_insn instr = INSTR(pc);
	m88k_opcode_t opc = instr.opcode();
	m88k_insnfmt_t fmt = instr.format();

	switch (opc) {
		case M88K_OPC_ILLEGAL:
			BAD;
			break;

		case M88K_OPC_ADD:
		case M88K_OPC_ADDU:
			if (fmt == M88K_IFMT_REG)
				arch_m88k_addsub(cpu, pc, instr.rd(), R32(instr.rs1()),
					(opc == M88K_OPC_ADD) ? IMM : UIMM, false, instr.carry(),
					(opc == M88K_OPC_ADD), bb);
			else
				arch_m88k_addsub(cpu, pc, instr.rd(), R32(instr.rs1()),
					R32(instr.rs2()), false, instr.carry(),
					(opc == M88K_OPC_ADD), bb);
			break;

		case M88K_OPC_SUB:
		case M88K_OPC_SUBU:
			if (fmt == M88K_IFMT_REG)
				arch_m88k_addsub(cpu, pc, instr.rd(), R32(instr.rs1()),
					(opc == M88K_OPC_SUB) ? IMM : UIMM, true, instr.carry(),
					(opc == M88K_OPC_SUB), bb);
			else
				arch_m88k_addsub(cpu, pc, instr.rd(), R32(instr.rs1()),
					R32(instr.rs2()), true, instr.carry(),
					(opc == M88K_OPC_SUB), bb);
			break;

		case M88K_OPC_MULS:
			LET32(instr.rd(), MUL(R32(instr.rs1()), R32(instr.rs2())));
			break;

		case M88K_OPC_MULU:
			if (fmt == M88K_IFMT_REG)
				LET32(instr.rd(), MUL(R32(instr.rs1()), UIMM));
			else
				LET32(instr.rd(), MUL(R32(instr.rs1()), R32(instr.rs2())));
			break;

		case M88K_OPC_DIVS:
			if (fmt == M88K_IFMT_REG)
				LET32(instr.rd(), SDIV(R32(instr.rs1()), IMM));
			else
				LET32(instr.rd(), SDIV(R32(instr.rs1()), R32(instr.rs2())));
			break;

		case M88K_OPC_DIVU:
			if (fmt == M88K_IFMT_REG)
				LET32(instr.rd(), UDIV(R32(instr.rs1()), UIMM));
			else
				LET32(instr.rd(), UDIV(R32(instr.rs1()), R32(instr.rs2())));
			break;

		case M88K_OPC_MASK:
			LET32(instr.rd(), AND(R32(instr.rs1()), UIMM));
			break;

		case M88K_OPC_MASK_U:
			LET32(instr.rd(), AND(R32(instr.rs1()), IMM_U));
			break;

		case M88K_OPC_AND:
			if (fmt == M88K_IFMT_REG)
				LET32(instr.rd(), AND(R32(instr.rs1()), IMM_B));
			else
				LET32(instr.rd(), AND(R32(instr.rs1()), R32(instr.rs2())));
			break;

		case M88K_OPC_AND_C:
			LET32(instr.rd(), AND(R32(instr.rs1()), COM(R32(instr.rs2()))));
			break;

		case M88K_OPC_AND_U:
			LET32(instr.rd(), AND(R32(instr.rs1()), IMM_UB));
			break;

		case M88K_OPC_OR:
			if (fmt == M88K_IFMT_REG)
				LET32(instr.rd(), OR(R32(instr.rs1()), UIMM));
			else
				LET32(instr.rd(), OR(R32(instr.rs1()), R32(instr.rs2())));
			break;

		case M88K_OPC_OR_C:
			LET32(instr.rd(), OR(R32(instr.rs1()), COM(R32(instr.rs2()))));
			break;

		case M88K_OPC_OR_U:
			LET32(instr.rd(), OR(R32(instr.rs1()), IMM_U));
			break;

		case M88K_OPC_XOR:
			if (fmt == M88K_IFMT_REG)
				LET32(instr.rd(), XOR(R32(instr.rs1()), UIMM));
			else
				LET32(instr.rd(), XOR(R32(instr.rs1()), R32(instr.rs2())));
			break;

		case M88K_OPC_XOR_C:
			LET32(instr.rd(), XOR(R32(instr.rs1()), COM(R32(instr.rs2()))));
			break;

		case M88K_OPC_XOR_U:
			LET32(instr.rd(), XOR(R32(instr.rs1()), IMM_U));
			break;

		case M88K_OPC_ROT:
		case M88K_OPC_MAK:
		case M88K_OPC_SET:
		case M88K_OPC_CLR:
		case M88K_OPC_EXT:
		case M88K_OPC_EXTU:
			arch_m88k_shift(cpu, opc, instr.rd(), R32(instr.rs1()),
				(fmt == M88K_BFMT_REG) ? NULL : R32(instr.rs2()),
				instr.bit_width(), instr.bit_offset(),
				(fmt == M88K_BFMT_REG), bb);
			break;

		case M88K_OPC_FF0:
			LET32(instr.rd(), FFC32(R32(instr.rs2())));
			break;

		case M88K_OPC_FF1:
			LET32(instr.rd(), FFS32(R32(instr.rs2())));
			break;

		case M88K_OPC_CMP:
			if (fmt == M88K_IFMT_REG)
				arch_m88k_cmp(cpu, instr.rd(), R32(instr.rs1()), IMM,
					bb);
			else
				arch_m88k_cmp_reg(cpu, instr.rd(), instr.rs1(), instr.rs2(),
					bb);
			break;

		case M88K_OPC_LDA:
			LET32(instr.rd(), ADD(R32(instr.rs1()), SHL(R32(instr.rs2()),
				CONST32(2))));
			break;

		case M88K_OPC_LDA_H:
			LET32(instr.rd(), ADD(R32(instr.rs1()), SHL(R32(instr.rs2()),
				CONST32(1))));
			break;

		case M88K_OPC_LDA_D:
			LET32(instr.rd(), ADD(R32(instr.rs1()), SHL(R32(instr.rs2()),
				CONST32(3))));
			break;

		case M88K_OPC_LDA_X:
			BAD;
			break;

		case M88K_OPC_LD:
			if (fmt == M88K_IFMT_MEM)
				LOAD32(instr.rd(), ADD(R32(instr.rs1()), UIMM));
			else if (fmt == M88K_IFMT_XMEM) {
				BAD;
			} else if (fmt == M88K_TFMT_REG) {
				LOAD32(instr.rd(), ADD(R32(instr.rs1()), R32(instr.rs2())));
			} else if (fmt == M88K_TFMT_REGS) {
				LOAD32(instr.rd(), ADD(R32(instr.rs1()), SHL(R32(instr.rs2()),
					CONST32(2))));
			} else {
				BAD;
			}
			break;

		case M88K_OPC_LD_B:
			if (fmt == M88K_IFMT_MEM)
				LOAD8S(instr.rd(), ADD(R32(instr.rs1()), UIMM));
			else
				LOAD8S(instr.rd(), ADD(R32(instr.rs1()), R32(instr.rs2())));
			break;

		case M88K_OPC_LD_BU:
			if (fmt == M88K_IFMT_MEM)
				LOAD8(instr.rd(), ADD(R32(instr.rs1()), UIMM));
			else
				LOAD8(instr.rd(), ADD(R32(instr.rs1()), R32(instr.rs2())));
			break;

		case M88K_OPC_LD_H:
			if (fmt == M88K_IFMT_MEM) {
				LOAD16S(instr.rd(), ADD(R32(instr.rs1()), UIMM));
			} else if (fmt == M88K_TFMT_REG) {
				LOAD16S(instr.rd(), ADD(R32(instr.rs1()), R32(instr.rs2())));
			} else if (fmt == M88K_TFMT_REGS) {
				LOAD16S(instr.rd(), ADD(R32(instr.rs1()), SHL(R32(instr.rs2()),
					CONST32(1))));
			} else {
				BAD;
			}
			break;

		case M88K_OPC_LD_HU:
			if (fmt == M88K_IFMT_MEM) {
				LOAD16(instr.rd(), ADD(R32(instr.rs1()), UIMM));
			} else if (fmt == M88K_IFMT_XMEM) {
				BAD;
			} else if (fmt == M88K_TFMT_REG) {
				LOAD16(instr.rd(), ADD(R32(instr.rs1()), R32(instr.rs2())));
			} else if (fmt == M88K_TFMT_REGS) {
				LOAD16(instr.rd(), ADD(R32(instr.rs1()), SHL(R32(instr.rs2()),
					CONST32(1))));
			} else {
				BAD;
			}
			break;

		case M88K_OPC_LD_D:
			if (fmt == M88K_IFMT_MEM) {
				LOAD32(instr.rd() & ~1, ADD(R32(instr.rs1()), UIMM));
				LOAD32(instr.rd() | 1, ADD(ADD(R32(instr.rs1()), UIMM),
					CONST32(4)));
			} else if (fmt == M88K_IFMT_XMEM) {
				//arch_m88k_load_xfr(cpu, 64, instr.rd(), ADD(R32(intr.rs1()), UIMM));
			} else if (fmt == M88K_TFMT_REG) {
				LOAD32(instr.rd() & ~1, ADD(R32(instr.rs1()), R32(instr.rs2())));
				LOAD32(instr.rd() | 1, ADD(R32(instr.rs1()),
					ADD(R32(instr.rs2()), CONST32(4))));
			} else if (fmt == M88K_TFMT_REGS) {
				LOAD32(instr.rd() & ~1, ADD(R32(instr.rs1()),
					SHL(R32(instr.rs2()), CONST32(3))));
				LOAD32(instr.rd() | 1, ADD(R32(instr.rs1()),
					ADD(SHL(R32(instr.rs2()), CONST32(3)), CONST32(4))));
			} else {
				BAD;
			}
			break;

		case M88K_OPC_LD_X:
			//BAD;
			break;

		case M88K_OPC_ST:
			if (fmt == M88K_IFMT_MEM)
				STORE32(R(instr.rd()), ADD(R32(instr.rs1()), UIMM));
			else if (fmt == M88K_IFMT_XMEM) {
				//BAD;
			} else if (fmt == M88K_TFMT_REG) {
				STORE32(R(instr.rd()), ADD(R32(instr.rs1()), R32(instr.rs2())));
			} else if (fmt == M88K_TFMT_REGS) {
				STORE32(R(instr.rd()), ADD(R32(instr.rs1()),
					SHL(R32(instr.rs2()), CONST32(2))));
			} else {
				BAD;
			}
			break;

		case M88K_OPC_ST_B:
			if (fmt == M88K_IFMT_MEM)
				STORE8(R(instr.rd()), ADD(R32(instr.rs1()), UIMM));
			else
				STORE8(R(instr.rd()), ADD(R32(instr.rs1()), R32(instr.rs2())));
			break;

		case M88K_OPC_ST_H:
			if (fmt == M88K_IFMT_MEM)
				STORE16(R(instr.rd()), ADD(R32(instr.rs1()), UIMM));
			else if (fmt == M88K_TFMT_REG)
				STORE16(R(instr.rd()), ADD(R32(instr.rs1()), R32(instr.rs2())));
			else if (fmt == M88K_TFMT_REGS)
				STORE16(R(instr.rd()), ADD(R32(instr.rs1()),
					SHL(R32(instr.rs2()), CONST32(1))));
			else {
				BAD;
			}
			break;

		case M88K_OPC_ST_D:
			if (fmt == M88K_IFMT_MEM) {
				STORE32(R(instr.rd() & ~1), ADD(R32(instr.rs1()), UIMM));
				STORE32(R(instr.rd() | 1), ADD(ADD(R32(instr.rs1()), UIMM),
					CONST32(4)));
			} else if (fmt == M88K_IFMT_XMEM) {
				// BAD;
			} else if (fmt == M88K_TFMT_REG) {
				STORE32(R(instr.rd() & ~1), ADD(R32(instr.rs1()), R32(instr.rs2())));
				STORE32(R(instr.rd() | 1), ADD(R32(instr.rs1()),
					ADD(R32(instr.rs2()), CONST32(4))));
			} else if (fmt == M88K_TFMT_REGS) {
				STORE32(R(instr.rd() & ~1), ADD(R32(instr.rs1()),
					SHL(R32(instr.rs2()), CONST32(3))));
				STORE32(R(instr.rd() | 1), ADD(R32(instr.rs1()),
					ADD(SHL(R32(instr.rs2()), CONST32(3)), CONST32(4))));
			}
			break;

		case M88K_OPC_ST_X:
			//BAD;
			break;

		case M88K_OPC_XMEM:
		case M88K_OPC_XMEM_BU:
			arch_m88k_xmem(cpu, (opc == M88K_OPC_XMEM_BU), instr.rd(),
				R32(instr.rs1()), R32(instr.rs2()), bb);
			break;

		case M88K_OPC_JSR:
			LINK(0);
		case M88K_OPC_JMP:
			LET_PC(R(instr.rs2()));
			break;

		case M88K_OPC_JSR_N:
			LINK(1);
		case M88K_OPC_JMP_N:
			LET_PC(R(instr.rs2()));
			break;

		case M88K_OPC_BSR:
			LINK(0);
		case M88K_OPC_BR:
			break;

		case M88K_OPC_BSR_N:
			LINK(1);
		case M88K_OPC_BR_N:
			break;
 
			// jump pc + disp IF !(rS & (1 << bit))
		case M88K_OPC_BB0:
		case M88K_OPC_BB0_N:
			// jump pc + disp IF (rS & (1 << bit))
		case M88K_OPC_BB1:
		case M88K_OPC_BB1_N:
			// jump pc + disp IF rS <cc> 0
		case M88K_OPC_BCND:
		case M88K_OPC_BCND_N:
			break;

		case M88K_OPC_TB1:
			if (instr.rs1() == 0)
				break;
		case M88K_OPC_TB0:
		case M88K_OPC_TBND:
		case M88K_OPC_TCND:
			TRAP(CONST32(instr.vec9()));
			break;

		//////////////////// FPU ///////////////////
		case M88K_OPC_FADD:
			SET_FPR(fmt == M88K_TFMT_XFR, instr.rd(), instr.td(),
				FPADD(GET_FPR(fmt == M88K_TFMT_XFR, instr.rs1(), instr.t1()),
					GET_FPR(fmt == M88K_TFMT_XFR, instr.rs2(), instr.t2())));
			break;

		case M88K_OPC_FSUB:
			SET_FPR(fmt == M88K_TFMT_XFR, instr.rd(), instr.td(),
				FPSUB(GET_FPR(fmt == M88K_TFMT_XFR, instr.rs1(), instr.t1()),
					GET_FPR(fmt == M88K_TFMT_XFR, instr.rs2(), instr.t2())));
			break;

		case M88K_OPC_FMUL:
			SET_FPR(fmt == M88K_TFMT_XFR, instr.rd(), instr.td(),
				FPMUL(GET_FPR(fmt == M88K_TFMT_XFR, instr.rs1(), instr.t1()),
					GET_FPR(fmt == M88K_TFMT_XFR, instr.rs2(), instr.t2())));
			break;

		case M88K_OPC_FDIV:
			SET_FPR(fmt == M88K_TFMT_XFR, instr.rd(), instr.td(),
				FPDIV(GET_FPR(fmt == M88K_TFMT_XFR, instr.rs1(), instr.t1()),
					GET_FPR(fmt == M88K_TFMT_XFR, instr.rs2(), instr.t2())));
			break;

		case M88K_OPC_FCVT:
			SET_FPR(fmt == M88K_TFMT_XFR, instr.rd(), instr.td(),
				GET_FPR(fmt == M88K_TFMT_XFR, instr.rs2(), instr.t2()));
			break;

		case M88K_OPC_INT: // round-to-zero
			LET32(instr.rd(), FPTOSI(32, GET_FPR(fmt == M88K_TFMT_XFR, instr.rs2(), instr.t2())));
			break;

		case M88K_OPC_NINT: // round-to-nearest
			LET32(instr.rd(), FPTOSI(32, GET_FPR(fmt == M88K_TFMT_XFR, instr.rs2(), instr.t2())));
			break;

		case M88K_OPC_TRNC: // truncation
			LET32(instr.rd(), FPTOSI(32, GET_FPR(fmt == M88K_TFMT_XFR, instr.rs2(), instr.t2())));
			break;

		case M88K_OPC_FLT: // flt <- int
			SET_FPR(fmt == M88K_TFMT_XFR, instr.rd(), instr.td(),
				SITOFP(32, R32(instr.rs2())));
			break;

		case M88K_OPC_FSQRT:
			SET_FPR(fmt == M88K_TFMT_XFR, instr.rd(), instr.td(),
				FPSQRT(FPBITCAST64(GET_FPR(fmt == M88K_TFMT_XFR,
					instr.rs2(), instr.t2()))));
			break;

		case M88K_OPC_FCMP:
		case M88K_OPC_FCMPU:
			arch_m88k_fcmp(cpu, instr.rd(),
					GET_FPR(fmt == M88K_TFMT_XFR, instr.rs1(), instr.t1()),
					GET_FPR(fmt == M88K_TFMT_XFR, instr.rs2(), instr.t2()),
					bb);
			break;

		case M88K_OPC_MOV:
			switch(fmt) {
				case M88K_TFMT_XREG: // xfr <- gpr
					SET_FPR(true, instr.rd(), 2, GET_FPR(false, instr.rs2(), instr.t2()));
					break;
				case M88K_TFMT_REGX: // gpr/xfr <- xfr
					if (instr & 0x200)
						SET_FPR(true, instr.rd(), 2, GET_FPR(true, instr.rs2(), 2));
					else
						SET_FPR(false, instr.rd(), instr.td(), GET_FPR(true, instr.rs2(), 2));
					break;
			}
			break;

		default:
			printf("INVALID %s:%d\n", __func__, __LINE__); exit(1);
			break;
	}

	return 4;
}
