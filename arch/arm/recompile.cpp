#include "libcpu.h"
#include "cpu_generic.h"
#include "arm_internal.h"
#include "tag_generic.h"

using namespace llvm;
extern Function* func_jitmain;

extern Value* ptr_PC;

static Value* ptr_N;
static Value* ptr_Z;
static Value* ptr_C;
static Value* ptr_V;
static Value* ptr_I;

#define ptr_CPSR ptr_r32[16]

#define BAD printf("%s:%d\n", __func__, __LINE__); exit(1);
#define LOG printf("%s:%d\n", __func__, __LINE__);

//////////////////////////////////////////////////////////////////////
// tagging
//////////////////////////////////////////////////////////////////////

int arch_arm_tag_instr(uint8_t* RAM, addr_t pc, int *flow_type, addr_t *new_pc) {
	uint32_t instr = *(uint32_t*)&RAM[pc];

	if (instr == 0xE1A0F00E) /* MOV r15, r0, r14 */
		*flow_type = FLOW_TYPE_RET;
//	else if (instr >> 24 < 0x0E)
//		*flow_type = FLOW_TYPE_BRANCH;
	else 
		*flow_type = FLOW_TYPE_CONTINUE;

	return 4;
}

#define RD ((instr>>12)&0xF)
#define RN ((instr>>16)&0xF)
#define RM (instr&0xF)

#define GETADDR(r) ((r==15)?(armregs[15]&r15mask):armregs[r])

static inline unsigned shift4(unsigned opcode)
{
	BAD;
#if 0
        unsigned shiftmode=opcode&0x60;
        unsigned shiftamount=(opcode&0x10)?(armregs[(opcode>>8)&15]&0xFF):((opcode>>7)&31);
        uint32_t rm=armregs[RM];
        if ((shiftamount-1)>=31)
        {
                return shift5(opcode,shiftmode,shiftamount,rm);
        }
        else
        {
                switch (shiftmode)
                {
                        case 0: /*LSL*/
                        return rm<<shiftamount;
                        case 0x20: /*LSR*/
                        return rm>>shiftamount;
                        case 0x40: /*ASR*/
                        return (int)rm>>shiftamount;
                        default: /*ROR*/
                        return (rm>>shiftamount)|(rm<<(32-shiftamount));
                }
        }
#endif
}

static inline void setsub(Value *op1, Value *op2, BasicBlock *bb)
{
	Value *v = BinaryOperator::Create(Instruction::Sub, op1, op2, "", bb);
	Value* z = new ICmpInst(*bb, ICmpInst::ICMP_EQ, v, CONST(0));
	Value* n = new ICmpInst(*bb, ICmpInst::ICMP_SLT, v, CONST(0));
	new StoreInst(z, ptr_Z, bb);
	new StoreInst(n, ptr_N, bb);
	new StoreInst(ICMP_SLE(v, op1), ptr_C, false, bb);
	new StoreInst(TRUNC1(LSHR(AND(XOR(op1, op2),XOR(op1,v)),CONST(31))), ptr_V, false, bb);
	return;
}

static uint32_t rotate2(uint32_t instr)
{
	uint32_t res;
	int c;

	res = instr & 0xFF;
	c = ((instr >> 8) & 0xF) << 1;
	return (res >> c) | (res << (32 - c));
}

#define shift2(o) ((o&0xFF0)?shift4(o):armregs[RM])

int arch_arm_recompile_instr(uint8_t* RAM, addr_t pc, BasicBlock *bb_dispatch, BasicBlock *bb, BasicBlock *bb_target, BasicBlock *bb_cond, BasicBlock *bb_next) {
	uint32_t instr = *(uint32_t*)&RAM[pc];

	/* hack to finish basic block */
	if (instr == 0xE1A0F00E) { /* MOV r15, r0, r14 */
		BranchInst::Create(bb_dispatch, bb);
	}

	int cond = instr >> 28;
	int op1 = (instr>>20)&0xFF;
	int op2 = (instr>>4)&0xF;
	int shift_bits = (instr>>4)&0xFF;

	printf("cond=%x, op1=%x, op2=%x, shift_bits=%x\n", cond, op1, op2, shift_bits);

	switch ((instr>>20)&0xFF) {
		case 0x1A: /* MOV */
			if (RD==15) {
				LOG;//BAD;
				//armregs[15]=(armregs[15]&~r15mask)|((shift2(opcode)+4)&r15mask);
			} else {
				LOG;//BAD;
				//armregs[RD]=shift2(opcode);
			}
			break;
		case 0x35: /* CMP */
			if (RD==15) {
				BAD;
			} else {
				setsub(R(RN),CONST(rotate2(instr)), bb);
			}
			break;
		default:
			LOG;//BAD;	
	}

//	BAD;

	return 4;
}

#define N_SHIFT 31
#define Z_SHIFT 30
#define C_SHIFT 29
#define V_SHIFT 28
#define I_SHIFT 27

static Value *
arch_arm_flags_encode(BasicBlock *bb)
{
	Value *flags = ConstantInt::get(getIntegerType(32), 0);

	flags = arch_encode_bit(flags, ptr_N, N_SHIFT, 32, bb);
	flags = arch_encode_bit(flags, ptr_Z, Z_SHIFT, 32, bb);
	flags = arch_encode_bit(flags, ptr_C, C_SHIFT, 32, bb);
	flags = arch_encode_bit(flags, ptr_V, V_SHIFT, 32, bb);
	flags = arch_encode_bit(flags, ptr_I, I_SHIFT, 32, bb);

	return flags;
}

static void
arch_arm_flags_decode(Value *flags, BasicBlock *bb)
{
	arch_decode_bit(flags, ptr_N, N_SHIFT, 32, bb);
	arch_decode_bit(flags, ptr_Z, Z_SHIFT, 32, bb);
	arch_decode_bit(flags, ptr_C, C_SHIFT, 32, bb);
	arch_decode_bit(flags, ptr_V, V_SHIFT, 32, bb);
	arch_decode_bit(flags, ptr_I, I_SHIFT, 32, bb);
}

void
arch_arm_emit_decode_reg(BasicBlock *bb)
{
	// declare flags
	ptr_N = new AllocaInst(getIntegerType(1), "N", bb);
	ptr_Z = new AllocaInst(getIntegerType(1), "Z", bb);
	ptr_C = new AllocaInst(getIntegerType(1), "C", bb);
	ptr_V = new AllocaInst(getIntegerType(1), "V", bb);
	ptr_I = new AllocaInst(getIntegerType(1), "I", bb);

	// decode CPSR
	Value *flags = new LoadInst(ptr_CPSR, "", false, bb);
	arch_arm_flags_decode(flags, bb);
}

void
arch_arm_spill_reg_state(BasicBlock *bb)
{
	Value *flags = arch_arm_flags_encode(bb);
	new StoreInst(flags, ptr_CPSR, false, bb);
}
//printf("%s:%d PC=$%04X\n", __func__, __LINE__, pc);
//printf("%s:%d\n", __func__, __LINE__);
