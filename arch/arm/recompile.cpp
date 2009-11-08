#include "libcpu.h"
#include "cpu_generic.h"
#include "arm_internal.h"
#include "tag_generic.h"

using namespace llvm;
extern Function* func_jitmain;

extern const BasicBlock *lookup_basicblock(Function* f, addr_t pc);

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

static inline void setsub(uint32_t op1, uint32_t op2, uint32_t res)
{
	BAD;
#if 0
        unsigned long temp=0;
        if (!res)                           temp=ZFLAG;
        else if (checkneg(res))             temp=NFLAG;
        if (res<=op1)                       temp|=CFLAG;
        if ((op1^op2)&(op1^res)&0x80000000) temp|=VFLAG;
        *pcpsr=((*pcpsr)&0xFFFFFFF)|(temp);
#endif
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

int arch_arm_recompile_instr(uint8_t* RAM, addr_t pc, BasicBlock *bb_dispatch, BasicBlock *bb) {
	uint32_t instr = *(uint32_t*)&RAM[pc];


	int cond = instr >> 28;
	int op1 = (instr>>20)&0xFF;
	int op2 = (instr>>4)&0xF;
	int shift_bits = (instr>>4)&0xFF;

	printf("cond=%x, op1=%x, op2=%x, shift_bits=%x\n", cond, op1, op2, shift_bits);

	switch ((instr>>20)&0xFF) {
		case 0x1A: /* MOV */
			if (RD==15) {
				BAD;
				//armregs[15]=(armregs[15]&~r15mask)|((shift2(opcode)+4)&r15mask);
			} else {
				BAD;
				//armregs[RD]=shift2(opcode);
			}
			break;
		case 0x35: /* CMP */
			if (RD==15) {
				BAD;
			} else {
				uint32_t templ, templ2;
				templ=rotate2(instr);
				printf("rotate2=%x\n", templ);
//				templ2=GETADDR(RN);
//				setsub(templ2,templ,templ2-templ);
			}
			break;
		default:
			BAD;	
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
	ConstantInt* const_int8_n_shift = ConstantInt::get(getType(Int32Ty), N_SHIFT);
	ConstantInt* const_int8_z_shift = ConstantInt::get(getType(Int32Ty), Z_SHIFT);
	ConstantInt* const_int8_c_shift = ConstantInt::get(getType(Int32Ty), C_SHIFT);
	ConstantInt* const_int8_v_shift = ConstantInt::get(getType(Int32Ty), V_SHIFT);
	ConstantInt* const_int8_i_shift = ConstantInt::get(getType(Int32Ty), I_SHIFT);
	Value *n = new LoadInst(ptr_N, "", false, bb);
	Value *z = new LoadInst(ptr_Z, "", false, bb);
	Value *c = new LoadInst(ptr_C, "", false, bb);
	Value *v = new LoadInst(ptr_V, "", false, bb);
	Value *i = new LoadInst(ptr_I, "", false, bb);
	n = new ZExtInst(n, getIntegerType(32), "", bb);
	z = new ZExtInst(z, getIntegerType(32), "", bb);
	c = new ZExtInst(c, getIntegerType(32), "", bb);
	v = new ZExtInst(v, getIntegerType(32), "", bb);
	i = new ZExtInst(i, getIntegerType(32), "", bb);
	n = BinaryOperator::Create(Instruction::Shl, n, const_int8_n_shift, "", bb);
	z = BinaryOperator::Create(Instruction::Shl, z, const_int8_z_shift, "", bb);
	c = BinaryOperator::Create(Instruction::Shl, i, const_int8_c_shift, "", bb);
	v = BinaryOperator::Create(Instruction::Shl, v, const_int8_v_shift, "", bb);
	i = BinaryOperator::Create(Instruction::Shl, i, const_int8_i_shift, "", bb);
	Value *flags = BinaryOperator::Create(Instruction::Or, n, z, "", bb);
	flags = BinaryOperator::Create(Instruction::Or, flags, c, "", bb);
	flags = BinaryOperator::Create(Instruction::Or, flags, v, "", bb);
	flags = BinaryOperator::Create(Instruction::Or, flags, i, "", bb);
	return flags;
}

static void
arch_arm_flags_decode(Value *flags, BasicBlock *bb)
{

	ConstantInt* const_int8_n_shift = ConstantInt::get(getType(Int32Ty), N_SHIFT);
	ConstantInt* const_int8_z_shift = ConstantInt::get(getType(Int32Ty), Z_SHIFT);
	ConstantInt* const_int8_c_shift = ConstantInt::get(getType(Int32Ty), C_SHIFT);
	ConstantInt* const_int8_v_shift = ConstantInt::get(getType(Int32Ty), V_SHIFT);
	ConstantInt* const_int8_i_shift = ConstantInt::get(getType(Int32Ty), I_SHIFT);
	Value *n = BinaryOperator::Create(Instruction::LShr, flags, const_int8_n_shift, "", bb);
	Value *z = BinaryOperator::Create(Instruction::LShr, flags, const_int8_z_shift, "", bb);
	Value *c = BinaryOperator::Create(Instruction::LShr, flags, const_int8_c_shift, "", bb);
	Value *v = BinaryOperator::Create(Instruction::LShr, flags, const_int8_v_shift, "", bb);
	Value *i = BinaryOperator::Create(Instruction::LShr, flags, const_int8_i_shift, "", bb);
	n = new TruncInst(n, getIntegerType(1), "", bb);
	z = new TruncInst(z, getIntegerType(1), "", bb);
	c = new TruncInst(c, getIntegerType(1), "", bb);
	v = new TruncInst(v, getIntegerType(1), "", bb);
	i = new TruncInst(i, getIntegerType(1), "", bb);
	new StoreInst(n, ptr_N, bb);
	new StoreInst(z, ptr_Z, bb);
	new StoreInst(c, ptr_C, bb);
	new StoreInst(v, ptr_V, bb);
	new StoreInst(i, ptr_I, bb);
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

	// decode P
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
