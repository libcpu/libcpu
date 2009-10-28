#include "libcpu.h"
#include "types.h"
#include "isa.h"
#include "cpu_generic.h"
#include "arch/6502/libcpu_6502.h"

using namespace llvm;
extern Function* func_jitmain;
extern Value *ptr_reg;
extern Value* ptr_RAM;
extern PointerType* type_pfunc_callout;
extern Value *ptr_func_debug;

extern Value *get_struct_member_pointer(Value *s, int index, BasicBlock *bb);
extern const BasicBlock *lookup_basicblock(Function* f, addr_t pc);

// 6502
extern Value* ptr_PC;
extern Value* ptr_r8[32];
extern Value* ptr_r16[32];
extern Value* ptr_r32[32];
extern Value* ptr_r64[32];

Value* ptr_C;
Value* ptr_Z;
Value* ptr_I;
Value* ptr_D;
Value* ptr_V;
Value* ptr_N;


#define ptr_A ptr_r8[0]
#define ptr_X ptr_r8[1]
#define ptr_Y ptr_r8[2]
#define ptr_S ptr_r8[3]
#define ptr_P ptr_r8[4]


//////////////////////////////////////////////////////////////////////
// 6502
//////////////////////////////////////////////////////////////////////

/* optimizations - these lead to inaccurate 6502 emulation */
//#define IGNORE_INDY_WRAPAROUND 1

#define OPCODE RAM[pc]
#define OPERAND_8 RAM[(pc+1)&0xFFFF]
#define OPERAND_16 ((RAM[pc+1]&0xFFFF) | (RAM[pc+2]&0xFFFF)<<8)
#define BRANCH_TARGET ((pc+2+(int8_t)RAM[(pc+1)&0xFFFF])&0xFFFF)

Value *
arch_6502_get_op8(uint8_t *RAM, uint16_t pc, BasicBlock *bb) {
	return ConstantInt::get(Type::Int32Ty, OPERAND_8);
}

Value *
arch_6502_get_op16(uint8_t *RAM, uint16_t pc, BasicBlock *bb) {
	return ConstantInt::get(Type::Int32Ty, OPERAND_16);
}

static Value *
arch_6502_get_x(BasicBlock *bb) {
	return new LoadInst(ptr_X, "", false, bb);
}

static Value *
arch_6502_get_y(BasicBlock *bb) {
	return new LoadInst(ptr_Y, "", false, bb);
}

static Value *
arch_6502_load_ram_8(Value *addr, BasicBlock *bb) {
	Value* ptr = GetElementPtrInst::Create(ptr_RAM, addr, "", bb);
	return new LoadInst(ptr, "", false, bb);
}

static Value *
arch_6502_load_ram_16(int is_32, Value *addr, BasicBlock *bb) {
	ConstantInt* const_int32_0001 = ConstantInt::get(Type::Int32Ty, 0x0001);
	ConstantInt* const_int32_0008 = ConstantInt::get(is_32? Type::Int32Ty : Type::Int16Ty, 0x0008);

	/* get lo */
	Value *lo = arch_6502_load_ram_8(addr, bb);
	Value *lo32 = new ZExtInst(lo, IntegerType::get(is_32? 32:16), "", bb);

	/* get hi */
	addr = BinaryOperator::Create(Instruction::Add, addr, const_int32_0001, "", bb);
	Value *hi = arch_6502_load_ram_8(addr, bb);
	Value *hi32 = new ZExtInst(hi, IntegerType::get(is_32? 32:16), "", bb);

	/* combine */
	BinaryOperator* hi32shifted = BinaryOperator::Create(Instruction::Shl, hi32, const_int32_0008, "", bb);
	return BinaryOperator::Create(Instruction::Add, lo32, hi32shifted, "", bb);
}

static Value *
arch_6502_add_index(Value *ea, Value *index_register, BasicBlock *bb) {
	/* load index register, extend to 32 bit */
	Value *index = new LoadInst(index_register, "", false, bb);
	CastInst* index32 = new ZExtInst(index, IntegerType::get(32), "", bb);

	/* add base and index */
	return BinaryOperator::Create(Instruction::Add, index32, ea, "", bb);
}

static Value *
arch_6502_get_operand_lvalue(uint8_t* RAM, addr_t pc, BasicBlock* bb) {
	int am = instraddmode[OPCODE].addmode;
	Value *index_register_before;
	Value *index_register_after;
	bool is_indirect;
	bool is_8bit;

	switch (am) {
		case ADDMODE_ACC:
			return ptr_A;
		case ADDMODE_BRA:
		case ADDMODE_IMM:
		case ADDMODE_IMPL:
			return NULL;
	}

	is_indirect = ((am == ADDMODE_IND) || (am == ADDMODE_INDX) || (am == ADDMODE_INDY));
	is_8bit = !((am == ADDMODE_ABS) || (am == ADDMODE_ABSX) || (am == ADDMODE_ABSY));
	index_register_before = NULL;
	if ((am == ADDMODE_ABSX) || (am == ADDMODE_INDX) || (am == ADDMODE_ZPX))
		index_register_before = ptr_X;
	if ((am == ADDMODE_ABSY) || (am == ADDMODE_ZPY))
		index_register_before = ptr_Y;
	index_register_after = (am == ADDMODE_INDY)? ptr_Y : NULL;

#if 0
	printf("pc = %x\n", pc);
	printf("index_register_before = %x\n", index_register_before);
	printf("index_register_after = %x\n", index_register_after);
	printf("is_indirect = %x\n", is_indirect);
	printf("is_8bit = %x\n", is_8bit);
#endif
	ConstantInt* const_int32_FFFF = ConstantInt::get(Type::Int32Ty, 0xFFFF);
	ConstantInt* const_int32_00FF = ConstantInt::get(Type::Int32Ty, 0x00FF);

	/* create base constant */
	uint16_t base = is_8bit? (OPERAND_8):(OPERAND_16);
	Value *ea = ConstantInt::get(Type::Int32Ty, base);

	if (index_register_before)
		ea = arch_6502_add_index(ea, index_register_before, bb);

	/* wrap around in zero page */
	if (is_8bit)
		ea = BinaryOperator::Create(Instruction::And, ea, const_int32_00FF, "", bb);
	else if (base >= 0xFF00) /* wrap around in memory */
		ea = BinaryOperator::Create(Instruction::And, ea, const_int32_FFFF, "", bb);

	if (is_indirect)
		ea = arch_6502_load_ram_16(true, ea, bb);

	if (index_register_after)
		ea = arch_6502_add_index(ea, index_register_after, bb);

	return GetElementPtrInst::Create(ptr_RAM, ea, "", bb);
}

static Value *
arch_6502_get_operand_rvalue(uint8_t* RAM, addr_t pc, BasicBlock* bb)
{
	switch (instraddmode[OPCODE].addmode) {
		case ADDMODE_IMM:
			return ConstantInt::get(Type::Int8Ty, OPERAND_8);
		default:
			Value *lvalue = arch_6502_get_operand_lvalue(RAM, pc, bb);
			return new LoadInst(lvalue, "", false, bb);
	}
}

static void
arch_6502_set_nz(Value *data, BasicBlock *bb)
{
	ConstantInt* const_int8_00 = ConstantInt::get(Type::Int8Ty, 0x00);
	ICmpInst* z = new ICmpInst(ICmpInst::ICMP_EQ, data, const_int8_00, "", bb);
	ICmpInst* n = new ICmpInst(ICmpInst::ICMP_SLT, data, const_int8_00, "", bb);
	new StoreInst(z, ptr_Z, bb);
	new StoreInst(n, ptr_N, bb);
}

static void
arch_6502_copy_reg(Value *src, Value *dst, BasicBlock *bb)
{
	Value *v = new LoadInst(src, "", false, bb);
	new StoreInst(v, dst, bb);
	arch_6502_set_nz(v, bb);
}

static void
arch_6502_store_reg(uint8_t* RAM, addr_t pc, Value *src, BasicBlock *bb)
{
	Value *lvalue = arch_6502_get_operand_lvalue(RAM, pc, bb);
	Value *v = new LoadInst(src, "", false, bb);
	new StoreInst(v, lvalue, bb);
}

static void
arch_6502_load_reg(uint8_t* RAM, addr_t pc, Value *dst, BasicBlock *bb)
{
	Value *rvalue = arch_6502_get_operand_rvalue(RAM, pc, bb);
	new StoreInst(rvalue, dst, bb);
	arch_6502_set_nz(rvalue, bb);
}

static void
arch_6502_log(uint8_t* RAM, addr_t pc, Instruction::BinaryOps o, BasicBlock *bb)
{
	Value *v1 = arch_6502_get_operand_rvalue(RAM, pc, bb);
	Value *v2 = new LoadInst(ptr_A, "", false, bb);
	v1 = BinaryOperator::Create(o, v1, v2, "", bb);
	new StoreInst(v1, ptr_A, bb);
	arch_6502_set_nz(v1, bb);
}

static void
arch_6502_trap(addr_t pc, BasicBlock *bb)
{
	ConstantInt* v_pc = ConstantInt::get(Type::Int16Ty, pc);
	new StoreInst(v_pc, ptr_PC, bb);
	ReturnInst::Create(ConstantInt::get(Type::Int32Ty, -1), bb);//XXX needs #define
}

static void
arch_6502_branch(uint8_t* RAM, addr_t pc, Value *flag, bool flag_state, BasicBlock *bb) {
	BRANCH(flag_state, BRANCH_TARGET, pc+2, new LoadInst(flag, "", false, bb));
}

static void
arch_6502_rmw(uint8_t *RAM, uint16_t pc, Instruction::BinaryOps o, Value *c, BasicBlock *bb) {
	Value *lvalue = arch_6502_get_operand_lvalue(RAM, pc, bb);
	Value *v = new LoadInst(lvalue, "", false, bb);
	v = BinaryOperator::Create(o, v, c, "", bb);
	new StoreInst(v, lvalue, bb);
	arch_6502_set_nz(v, bb);
}

static void
arch_6502_shiftrotate(uint8_t *RAM, uint16_t pc, bool left, bool rotate, BasicBlock *bb)
{
	ConstantInt* const_int8_0000 = ConstantInt::get(Type::Int8Ty, 0x0000);
	ConstantInt* const_int8_0001 = ConstantInt::get(Type::Int8Ty, 0x0001);
	ConstantInt* const_int8_0007 = ConstantInt::get(Type::Int8Ty, 0x0007);

	/* load operand */
	Value *lvalue = arch_6502_get_operand_lvalue(RAM, pc, bb);
	Value *v1 = new LoadInst(lvalue, "", false, bb);

	/* shift */
	Value *v2 = BinaryOperator::Create(left? Instruction::Shl : Instruction::LShr, v1, const_int8_0001, "", bb);
	
	if (rotate) {	/* shift in carry */
		/* zext carry to i8 */
		Value *c = new LoadInst(ptr_C, "", false, bb);
		c = new ZExtInst(c, IntegerType::get(8), "", bb);
		if (!left)
			c = BinaryOperator::Create(Instruction::Shl, c, const_int8_0007, "", bb);
		v2 = BinaryOperator::Create(Instruction::Or, v2, c, "", bb);
	}

	/* store */	
	new StoreInst(v2, lvalue, false, bb);
	arch_6502_set_nz(v2, bb);

	Value *c;
	if (left)	/* old MSB to carry */
		c = new ICmpInst(ICmpInst::ICMP_SLT, v1, const_int8_0000, "", bb);
	else		/* old LSB to carry */
		c = new TruncInst(v1, IntegerType::get(1), "", bb);
	new StoreInst(c, ptr_C, bb);
}

/*
 * This is used for ADC, SBC and CMP
 * - for ADC, we add A + B + C
 * - for SBC, we add A + ~B + C
 * - for CMP, we add A + ~B + 1
 */
static void
arch_6502_addsub(uint8_t *RAM, uint16_t pc, Value *reg, Value *reg2, int is_sub, int with_carry, BasicBlock *bb) {
	ConstantInt* const_int16_0001 = ConstantInt::get(Type::Int16Ty, 0x0001);
	ConstantInt* const_int16_0008 = ConstantInt::get(Type::Int16Ty, 0x0008);
	ConstantInt* const_int8_00FF = ConstantInt::get(Type::Int8Ty, 0x00FF);

	Value *old_c = NULL; //XXX GCC

	/* load operand, A and C */
	Value *v1 = new LoadInst(reg, "", false, bb);
	Value *v2 = arch_6502_get_operand_rvalue(RAM, pc, bb);

	/* NOT operand (if subtraction) */
	if (is_sub)
		v2 = BinaryOperator::Create(Instruction::Xor, v2, const_int8_00FF, "", bb);

	/* convert to 16 bits */
	v1 = new ZExtInst(v1, IntegerType::get(16), "", bb);
	v2 = new ZExtInst(v2, IntegerType::get(16), "", bb);

	/* add them together */
	v1 = BinaryOperator::Create(Instruction::Add, v1, v2, "", bb);

	/* add C or 1 */
	if (with_carry) {
		old_c = new LoadInst(ptr_C, "", false, bb);
		old_c = new ZExtInst(old_c, IntegerType::get(16), "", bb);
		v1 = BinaryOperator::Create(Instruction::Add, v1, old_c, "", bb);
	} else {
		v1 = BinaryOperator::Create(Instruction::Add, v1, const_int16_0001, "", bb);
	}

	/* get C */
	Value *c = BinaryOperator::Create(Instruction::LShr, v1, const_int16_0008, "", bb);
	c = new TruncInst(c, IntegerType::get(1), "", bb);
	new StoreInst(c, ptr_C, bb);

	/* get result */
	v1 = new TruncInst(v1, IntegerType::get(8), "", bb);
	if (reg2)
		new StoreInst(v1, reg2, bb);

	/* set flags */
	arch_6502_set_nz(v1, bb);
}

static void
arch_6502_push(Value *v, BasicBlock *bb) {
	ConstantInt* const_int32_0100 = ConstantInt::get(Type::Int32Ty, 0x0100);
	ConstantInt* const_int8_0001 = ConstantInt::get(Type::Int8Ty, 0x0001);

	/* get pointer to TOS */
	Value *s = new LoadInst(ptr_S, "", false, bb);
	Value *s_ptr = new ZExtInst(s, IntegerType::get(32), "", bb);
	s_ptr = BinaryOperator::Create(Instruction::Or, s_ptr, const_int32_0100, "", bb);
	s_ptr = GetElementPtrInst::Create(ptr_RAM, s_ptr, "", bb);

	/* store value */
	new StoreInst(v, s_ptr, false, bb);

	/* update S */
	s = BinaryOperator::Create(Instruction::Sub, s, const_int8_0001, "", bb);
	new StoreInst(s, ptr_S, false, bb);
}

static Value *
arch_6502_pull(BasicBlock *bb) {
	ConstantInt* const_int32_0100 = ConstantInt::get(Type::Int32Ty, 0x0100);
	ConstantInt* const_int8_0001 = ConstantInt::get(Type::Int8Ty, 0x0001);

	/* update S */
	Value *s = new LoadInst(ptr_S, "", false, bb);
	s = BinaryOperator::Create(Instruction::Add, s, const_int8_0001, "", bb);
	new StoreInst(s, ptr_S, false, bb);

	/* get pointer to TOS */
	Value *s_ptr = new ZExtInst(s, IntegerType::get(32), "", bb);
	s_ptr = BinaryOperator::Create(Instruction::Or, s_ptr, const_int32_0100, "", bb);
	s_ptr = GetElementPtrInst::Create(ptr_RAM, s_ptr, "", bb);

	/* load value */
	return new LoadInst(s_ptr, "", false, bb);
}

//XXX this generates inefficient IR code - worth fixing?
//	store i8 %408, i8* %S
//	%409 = load i8* %S
static void
arch_6502_push_c16(uint16_t v, BasicBlock *bb) {
	arch_6502_push(ConstantInt::get(Type::Int8Ty, v >> 8), bb);
	arch_6502_push(ConstantInt::get(Type::Int8Ty, v & 0xFF), bb);
}

static Value *
arch_6502_flags_encode(BasicBlock *bb)
{
	ConstantInt* const_int8_0007 = ConstantInt::get(Type::Int8Ty, 0x0007);
	ConstantInt* const_int8_0006 = ConstantInt::get(Type::Int8Ty, 0x0006);
	ConstantInt* const_int8_0003 = ConstantInt::get(Type::Int8Ty, 0x0003);
	ConstantInt* const_int8_0002 = ConstantInt::get(Type::Int8Ty, 0x0002);
	ConstantInt* const_int8_0001 = ConstantInt::get(Type::Int8Ty, 0x0001);
	Value *n = new LoadInst(ptr_N, "", false, bb);
	Value *v = new LoadInst(ptr_V, "", false, bb);
	Value *d = new LoadInst(ptr_D, "", false, bb);
	Value *i = new LoadInst(ptr_I, "", false, bb);
	Value *z = new LoadInst(ptr_Z, "", false, bb);
	Value *c = new LoadInst(ptr_C, "", false, bb);
	n = new ZExtInst(n, IntegerType::get(8), "", bb);
	v = new ZExtInst(v, IntegerType::get(8), "", bb);
	d = new ZExtInst(d, IntegerType::get(8), "", bb);
	i = new ZExtInst(i, IntegerType::get(8), "", bb);
	z = new ZExtInst(z, IntegerType::get(8), "", bb);
	c = new ZExtInst(c, IntegerType::get(8), "", bb);
	n = BinaryOperator::Create(Instruction::Shl, n, const_int8_0007, "", bb);
	v = BinaryOperator::Create(Instruction::Shl, v, const_int8_0006, "", bb);
	d = BinaryOperator::Create(Instruction::Shl, d, const_int8_0003, "", bb);
	i = BinaryOperator::Create(Instruction::Shl, i, const_int8_0002, "", bb);
	z = BinaryOperator::Create(Instruction::Shl, z, const_int8_0001, "", bb);
	Value *flags = BinaryOperator::Create(Instruction::Or, n, v, "", bb);
	flags = BinaryOperator::Create(Instruction::Or, flags, d, "", bb);
	flags = BinaryOperator::Create(Instruction::Or, flags, i, "", bb);
	flags = BinaryOperator::Create(Instruction::Or, flags, z, "", bb);
	flags = BinaryOperator::Create(Instruction::Or, flags, c, "", bb);
	return flags;
}

static void
arch_6502_flags_decode(Value *flags, BasicBlock *bb)
{
	ConstantInt* const_int8_0007 = ConstantInt::get(Type::Int8Ty, 0x0007);
	ConstantInt* const_int8_0006 = ConstantInt::get(Type::Int8Ty, 0x0006);
	ConstantInt* const_int8_0003 = ConstantInt::get(Type::Int8Ty, 0x0003);
	ConstantInt* const_int8_0002 = ConstantInt::get(Type::Int8Ty, 0x0002);
	ConstantInt* const_int8_0001 = ConstantInt::get(Type::Int8Ty, 0x0001);
	Value *n = BinaryOperator::Create(Instruction::LShr, flags, const_int8_0007, "", bb);
	Value *v = BinaryOperator::Create(Instruction::LShr, flags, const_int8_0006, "", bb);
	Value *d = BinaryOperator::Create(Instruction::LShr, flags, const_int8_0003, "", bb);
	Value *i = BinaryOperator::Create(Instruction::LShr, flags, const_int8_0002, "", bb);
	Value *z = BinaryOperator::Create(Instruction::LShr, flags, const_int8_0001, "", bb);
	n = new TruncInst(n, IntegerType::get(1), "", bb);
	v = new TruncInst(v, IntegerType::get(1), "", bb);
	d = new TruncInst(d, IntegerType::get(1), "", bb);
	i = new TruncInst(i, IntegerType::get(1), "", bb);
	z = new TruncInst(z, IntegerType::get(1), "", bb);
	Value *c = new TruncInst(flags, IntegerType::get(1), "", bb);
	new StoreInst(n, ptr_N, bb);
	new StoreInst(v, ptr_V, bb);
	new StoreInst(d, ptr_D, bb);
	new StoreInst(i, ptr_I, bb);
	new StoreInst(z, ptr_Z, bb);
	new StoreInst(c, ptr_C, bb);
}

int
arch_6502_recompile_instr(uint8_t* RAM, addr_t pc, BasicBlock *bb_dispatch, BasicBlock *bb) {
	uint8_t opcode = RAM[pc];

	ConstantInt* const_false = ConstantInt::get(Type::Int1Ty, 0);
	ConstantInt* const_true = ConstantInt::get(Type::Int1Ty, 1);
	ConstantInt* const_int8_0001 = ConstantInt::get(Type::Int8Ty, 0x0001);

//printf("%s:%d PC=$%04X\n", __func__, __LINE__, pc);

#if 0
	// add a call to debug_function()
	ConstantInt* v_pc = ConstantInt::get(Type::Int16Ty, pc);
	new StoreInst(v_pc, ptr_PC, bb);
	// serialize flags
	Value *flags = arch_6502_flags_encode(bb);
	new StoreInst(flags, ptr_P, false, bb);

	create_call(ptr_func_debug, bb);

	flags = new LoadInst(ptr_P, "", false, bb);
	arch_6502_flags_decode(flags, bb);
#endif

//	printf("\naddmode = %i\n", instraddmode[opcode].addmode);
	switch (instraddmode[opcode].instr) {
		case INSTR_ADC:
			arch_6502_addsub(RAM, pc, ptr_A, ptr_A, false, true, bb);
			break;
		case INSTR_AND:
			arch_6502_log(RAM, pc, Instruction::And, bb);
			break;
		case INSTR_ASL:
			arch_6502_shiftrotate(RAM, pc, true, false, bb);
			break;
		case INSTR_BCC:
			arch_6502_branch(RAM, pc, ptr_C, false, bb);
			break;
		case INSTR_BCS:
			arch_6502_branch(RAM, pc, ptr_C, true, bb);
			break;
		case INSTR_BEQ:
			arch_6502_branch(RAM, pc, ptr_Z, true, bb);
			break;
		case INSTR_BIT:
			{
			Value *v1 = arch_6502_get_operand_rvalue(RAM, pc, bb);
			arch_6502_set_nz(v1, bb);
			break;
			}
		case INSTR_BMI:
			arch_6502_branch(RAM, pc, ptr_N, true, bb);
			break;
		case INSTR_BNE:
			arch_6502_branch(RAM, pc, ptr_Z, false, bb);
			break;
		case INSTR_BPL:
			arch_6502_branch(RAM, pc, ptr_N, false, bb);
			break;
		case INSTR_BRK:
			printf("warning: encountered BRK!\n");
			arch_6502_trap(pc, bb);
			break;
		case INSTR_BVC:
			arch_6502_branch(RAM, pc, ptr_V, false, bb);
			break;
		case INSTR_BVS:
			arch_6502_branch(RAM, pc, ptr_V, true, bb);
			break;
		case INSTR_CLC:
			new StoreInst(const_false, ptr_C, false, bb);
			break;
		case INSTR_CLD:
			new StoreInst(const_false, ptr_D, false, bb);
			break;
		case INSTR_CLI:
			new StoreInst(const_false, ptr_I, false, bb);
			break;
		case INSTR_CLV:
			new StoreInst(const_false, ptr_V, false, bb);
			break;
		case INSTR_CMP:
			arch_6502_addsub(RAM, pc, ptr_A, NULL, true, false, bb);
			break;
		case INSTR_CPX:
			arch_6502_addsub(RAM, pc, ptr_X, NULL, true, false, bb);
			break;
		case INSTR_CPY:
			arch_6502_addsub(RAM, pc, ptr_Y, NULL, true, false, bb);
			break;
		case INSTR_DEC:
			arch_6502_rmw(RAM, pc, Instruction::Sub, const_int8_0001, bb);
			break;
		case INSTR_DEX:
			{
			Value *v = arch_6502_get_x(bb);
			v = BinaryOperator::Create(Instruction::Sub, v, const_int8_0001, "", bb);
			new StoreInst(v, ptr_X, bb);
			arch_6502_set_nz(v, bb);
			break;
			}
		case INSTR_DEY:
			{
			Value *v = arch_6502_get_y(bb);
			v = BinaryOperator::Create(Instruction::Sub, v, const_int8_0001, "", bb);
			new StoreInst(v, ptr_Y, bb);
			arch_6502_set_nz(v, bb);
			break;
			}
		case INSTR_EOR:
			arch_6502_log(RAM, pc, Instruction::Xor, bb);
			break;
		case INSTR_INC:
			arch_6502_rmw(RAM, pc, Instruction::Add, const_int8_0001, bb);
			break;
		case INSTR_INX:
			{
			Value *v = arch_6502_get_x(bb);
			v = BinaryOperator::Create(Instruction::Add, v, const_int8_0001, "", bb);
			new StoreInst(v, ptr_X, bb);
			arch_6502_set_nz(v, bb);
			break;
			}
		case INSTR_INY:
			{
			Value *v = arch_6502_get_y(bb);
			v = BinaryOperator::Create(Instruction::Add, v, const_int8_0001, "", bb);
			new StoreInst(v, ptr_Y, bb);
			arch_6502_set_nz(v, bb);
			break;
			}
		case INSTR_JMP:
			if (instraddmode[opcode].addmode == ADDMODE_IND) {
				Value *ea = ConstantInt::get(Type::Int32Ty, OPERAND_16);
				Value *v = arch_6502_load_ram_16(false, ea, bb);
				new StoreInst(v, ptr_PC, bb);
				BranchInst::Create(bb_dispatch, bb);
			} else {
				BasicBlock *target = (BasicBlock*)lookup_basicblock(func_jitmain, OPERAND_16);
				if (target) {
					BranchInst::Create(target, bb);
				} else {
					//printf("warning: unknown jmp at $%04X to $%04X!\n", pc, OPERAND_16);
					ConstantInt* c = ConstantInt::get(Type::Int16Ty, OPERAND_16);
					new StoreInst(c, ptr_PC, bb);
					BranchInst::Create(bb_dispatch, bb);
				}
			}
			break;
		case INSTR_JSR:
			{
			arch_6502_push_c16(pc+2, bb);
			BasicBlock *target = (BasicBlock*)lookup_basicblock(func_jitmain, OPERAND_16);
			if (!target) {
				//printf("warning: unknown jsr at $%04X to $%04X!\n", pc, OPERAND_16);
				ConstantInt* c = ConstantInt::get(Type::Int16Ty, OPERAND_16);
				new StoreInst(c, ptr_PC, bb);
				BranchInst::Create(bb_dispatch, bb);
			} else {
				BranchInst::Create(target, bb);
			}
			break;
			}
		case INSTR_LDA:
			arch_6502_load_reg(RAM, pc, ptr_A, bb);
			break;
		case INSTR_LDX:
			arch_6502_load_reg(RAM, pc, ptr_X, bb);
			break;
		case INSTR_LDY:
			arch_6502_load_reg(RAM, pc, ptr_Y, bb);
			break;
		case INSTR_LSR:
			arch_6502_shiftrotate(RAM, pc, false, false, bb);
			break;
		case INSTR_NOP:
			break;
		case INSTR_ORA:
			arch_6502_log(RAM, pc, Instruction::Or, bb);
			break;
		case INSTR_PHA:
			arch_6502_push(new LoadInst(ptr_A, "", false, bb), bb);
			break;
		case INSTR_PHP:
			arch_6502_push(arch_6502_flags_encode(bb), bb);
			break;
		case INSTR_PLA:
			{
			Value *v1 = arch_6502_pull(bb);
			new StoreInst(v1, ptr_A, bb);
			arch_6502_set_nz(v1, bb);
			break;
			}
		case INSTR_PLP:
			arch_6502_flags_decode(arch_6502_pull(bb), bb);
			break;
		case INSTR_ROL:
			arch_6502_shiftrotate(RAM, pc, true, true, bb);
			break;
		case INSTR_ROR:
			arch_6502_shiftrotate(RAM, pc, false, true, bb);
			break;
		case INSTR_RTI:
			printf("error: encountered RTI!\n");
			arch_6502_trap(pc, bb);
			break;
		case INSTR_RTS:
			{
			ConstantInt* const_int16_0008 = ConstantInt::get(Type::Int16Ty, 0x0008);
			ConstantInt* const_int16_0001 = ConstantInt::get(Type::Int16Ty, 0x0001);
			Value *lo = arch_6502_pull(bb);
			Value *hi = arch_6502_pull(bb);
			lo = new ZExtInst(lo, IntegerType::get(16), "", bb);
			hi = new ZExtInst(hi, IntegerType::get(16), "", bb);
			hi = BinaryOperator::Create(Instruction::Shl, hi, const_int16_0008, "", bb);
			lo = BinaryOperator::Create(Instruction::Add, lo, hi, "", bb);
			lo = BinaryOperator::Create(Instruction::Add, lo, const_int16_0001, "", bb);
			new StoreInst(lo, ptr_PC, bb);
			BranchInst::Create(bb_dispatch, bb);
			break;
			}
		case INSTR_SBC:
			arch_6502_addsub(RAM, pc, ptr_A, ptr_A, true, true, bb);
			break;
		case INSTR_SEC:
			new StoreInst(const_true, ptr_C, false, bb);
			break;
		case INSTR_SED:
			new StoreInst(const_true, ptr_D, false, bb);
			break;
		case INSTR_SEI:
			new StoreInst(const_true, ptr_I, false, bb);
			break;
		case INSTR_STA:
			arch_6502_store_reg(RAM, pc, ptr_A, bb);
			break;
		case INSTR_STX:
			arch_6502_store_reg(RAM, pc, ptr_X, bb);
			break;
		case INSTR_STY:
			arch_6502_store_reg(RAM, pc, ptr_Y, bb);
			break;
		case INSTR_TAX:
			arch_6502_copy_reg(ptr_A, ptr_X, bb);
			break;
		case INSTR_TAY:
			arch_6502_copy_reg(ptr_A, ptr_Y, bb);
			break;
		case INSTR_TSX:
			arch_6502_copy_reg(ptr_S, ptr_X, bb);
			break;
		case INSTR_TXA:
			arch_6502_copy_reg(ptr_X, ptr_A, bb);
			break;
		case INSTR_TXS:
			arch_6502_copy_reg(ptr_X, ptr_S, bb);
			break;
		case INSTR_TYA:
			arch_6502_copy_reg(ptr_Y, ptr_A, bb);
			break;
		case INSTR_XXX:
			printf("warning: encountered XXX!\n");
			arch_6502_trap(pc, bb);
			break;
	}

//printf("%s:%d opcode=%02X, addmode=%d, length=%d\n", __func__, __LINE__, opcode, instraddmode[opcode].addmode, length[instraddmode[opcode].addmode]);
	return length[instraddmode[opcode].addmode]+1;
}

void
arch_6502_init(cpu_t *cpu)
{
	reg_6502_t *reg;
	reg = (reg_6502_t*)malloc(sizeof(reg_6502_t));
	reg->pc = 0;
	reg->a = 0;
	reg->x = 0;
	reg->y = 0;
	reg->s = 0xFF;
	reg->p = 0;
	cpu->reg = reg;

	cpu->pc_width = 16;
	cpu->count_regs_i8 = 5;
	cpu->count_regs_i16 = 0;
	cpu->count_regs_i32 = 0;
	cpu->count_regs_i64 = 0;
}

void
arch_6502_emit_decode_reg(BasicBlock *bb)
{
	// declare flags
	ptr_N = new AllocaInst(IntegerType::get(1), "N", bb);
	ptr_V = new AllocaInst(IntegerType::get(1), "V", bb);
	ptr_D = new AllocaInst(IntegerType::get(1), "D", bb);
	ptr_I = new AllocaInst(IntegerType::get(1), "I", bb);
	ptr_Z = new AllocaInst(IntegerType::get(1), "Z", bb);
	ptr_C = new AllocaInst(IntegerType::get(1), "C", bb);

	// decode P
	Value *flags = new LoadInst(ptr_P, "", false, bb);
	arch_6502_flags_decode(flags, bb);
}

void
arch_6502_spill_reg_state(BasicBlock *bb)
{
	Value *flags = arch_6502_flags_encode(bb);
	new StoreInst(flags, ptr_P, false, bb);
}

addr_t
arch_6502_get_pc(void *reg)
{
	return ((reg_6502_t*)reg)->pc;
}

arch_func_t arch_func_6502 = {
	arch_6502_init,
	arch_6502_get_pc,
	arch_6502_emit_decode_reg,
	arch_6502_spill_reg_state,
	arch_6502_tag_instr,
	arch_6502_disasm_instr,
	arch_6502_recompile_instr
};
