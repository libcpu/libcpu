#include "cpu_generic.h"

//////////////////////////////////////////////////////////////////////

#define SWAP16(x)		(OR(SHL(AND(x, CONST(0xff00)), CONST(8)), \
							LSHR(AND(x, CONST(0x00ff)), CONST(8))))
#define SWAP32(x)		(OR(OR(OR(										\
						 LSHR(AND(x, CONST(0xff000000)), CONST(24)),	\
						 LSHR(AND(x, CONST(0x00ff0000)), CONST(8))),	\
						 SHL(AND(x, CONST(0x0000ff00)), CONST(8))), 	\
						 SHL(AND(x, CONST(0x000000ff)), CONST(24))))

//////////////////////////////////////////////////////////////////////
// GENERIC: register access
//////////////////////////////////////////////////////////////////////

static Value **
ptr_r(cpu_t *cpu)
{
	switch (cpu->reg_size) {
		case 8:
			return cpu->ptr_r8;
		case 16:
			return cpu->ptr_r16;
		case 32:
			return cpu->ptr_r32;
		case 64:
			return cpu->ptr_r64;
		default:
			return 0; /* can't happen */
	}
}

// GET REGISTER
Value *
arch_get_reg(cpu_t *cpu, uint32_t index, uint32_t bits, BasicBlock *bb) {
	Value *v;

	/* R0 is always 0 (on certain RISCs) */
	if (cpu->has_special_r0 && !index)
		return CONSTs(bits? bits : cpu->reg_size, 0);

	/* get the register */
	v = new LoadInst(ptr_r(cpu)[index], "", false, bb);

	/* optionally truncate it */
	if (bits && cpu->reg_size != bits)
		v = TRUNC(bits, v);
	
	return v;
}

// PUT REGISTER
void
arch_put_reg(cpu_t *cpu, uint32_t index, Value *v, uint32_t bits, bool sext, BasicBlock *bb) {
	/*
	 * if the caller cares about bit size and
	 * the size is not the register size, we'll zext or sext
	 */
	if (bits && cpu->reg_size != bits)
		if (sext)
			v = SEXT(cpu->reg_size, v);
		else
			v = ZEXT(cpu->reg_size, v);

	/* store value, unless it's R0 (on certain RISCs) */
	if (!cpu->has_special_r0 || index)
		new StoreInst(v, ptr_r(cpu)[index], bb);
}

//////////////////////////////////////////////////////////////////////
// GENERIC: host memory access
//////////////////////////////////////////////////////////////////////
uint32_t
RAM32BE(uint8_t *RAM, addr_t a) {
	uint32_t v;
	v  = RAM[a+0] << 24;
	v |= RAM[a+1] << 16;
	v |= RAM[a+2] << 8;
	v |= RAM[a+3] << 0;
	return v;
}

//////////////////////////////////////////////////////////////////////
// GENERIC: memory access
//////////////////////////////////////////////////////////////////////

/* get a RAM pointer to a 32 bit value */
static Value *
arch_gep32(cpu_t *cpu, Value *a, BasicBlock *bb) {
	a = GetElementPtrInst::Create(cpu->ptr_RAM, a, "", bb);
	return new BitCastInst(a, PointerType::get(getType(Int32Ty), 0), "", bb);
}

/* load 32 bit ALIGNED value from RAM */
Value *
arch_load32_aligned(cpu_t *cpu, Value *a, BasicBlock *bb) {
	a = arch_gep32(cpu, a, bb);
#ifdef __LITTLE_ENDIAN__
	bool swap = !cpu->is_little_endian;
#else
	bool swap = cpu->is_little_endian;
#endif
	if(swap)
		return SWAP32(new LoadInst(a, "", false, bb));
	else
		return new LoadInst(a, "", false, bb);
}

/* store 32 bit ALIGNED value to RAM */
void
arch_store32_aligned(cpu_t *cpu, Value *v, Value *a, BasicBlock *bb) {
	a = arch_gep32(cpu, a, bb);
#ifdef __LITTLE_ENDIAN__
	bool swap = !cpu->is_little_endian;
#else
	bool swap = cpu->is_little_endian;
#endif
	new StoreInst(swap ? SWAP32(v) : v, a, bb);
}

//////////////////////////////////////////////////////////////////////
// GENERIC: endianness
//////////////////////////////////////////////////////////////////////

static Value *
arch_get_shift8(cpu_t *cpu, Value *addr, BasicBlock *bb)
{
	Value *shift = AND(addr,CONST(3));
	if (!cpu->is_little_endian)
		shift = XOR(shift, CONST(3));
	return SHL(CONST(3), shift);
}

static Value *
arch_get_shift16(cpu_t *cpu, Value *addr, BasicBlock *bb)
{
	Value *shift = AND(addr,CONST(1));
	if (!cpu->is_little_endian)
		shift = XOR(shift, CONST(1));
	return SHL(CONST(4), shift);
}

Value *
arch_load8(cpu_t *cpu, Value *addr, BasicBlock *bb) {
	Value *shift = arch_get_shift8(cpu, addr, bb);
	Value *val = arch_load32_aligned(cpu, AND(addr, CONST(~3ULL)), bb);
	return TRUNC8(LSHR(val, shift));
}

Value *
arch_load16_aligned(cpu_t *cpu, Value *addr, BasicBlock *bb) {
	Value *shift = arch_get_shift16(cpu, addr, bb);
	Value *val = arch_load32_aligned(cpu, AND(addr, CONST(~3ULL)), bb);
	return TRUNC16(LSHR(val, shift));
}

void
arch_store8(cpu_t *cpu, Value *val, Value *addr, BasicBlock *bb) {
	Value *shift = arch_get_shift8(cpu, addr, bb);
	addr = AND(addr, CONST(~3ULL));
	Value *mask = XOR(SHL(CONST(255), shift),CONST(-1ULL));
	Value *old = AND(arch_load32_aligned(cpu, addr, bb), mask);
	val = OR(old, SHL(AND(val, CONST(255)), shift));
	arch_store32_aligned(cpu, val, addr, bb);
}

void
arch_store16(cpu_t *cpu, Value *val, Value *addr, BasicBlock *bb) {
	Value *shift = arch_get_shift16(cpu, addr, bb);
	addr = AND(addr, CONST(~3ULL));
	Value *mask = XOR(SHL(CONST(65535), shift),CONST(-1ULL));
	Value *old = AND(arch_load32_aligned(cpu, addr, bb), mask);
	val = OR(old, SHL(AND(val, CONST(65535)), shift));
	arch_store32_aligned(cpu, val, addr, bb);
}


// branches

void
arch_branch(bool flag_state, BasicBlock *target1, BasicBlock *target2, Value *v, BasicBlock *bb) {
	if (flag_state)
		BranchInst::Create(target1, target2, v, bb);
	else
		BranchInst::Create(target2, target1, v, bb);
}

void
arch_jump(BasicBlock *bb, BasicBlock *bb_target) {
	if (!bb_target) {
		printf("error: unknown jump target!\n");
		exit(1);
	}
	BranchInst::Create(bb_target, bb);
}

// decoding and encoding of bits in a bitfield (e.g. flags)

Value *
arch_encode_bit(Value *flags, Value *bit, int shift, int width, BasicBlock *bb)
{
	Value *n = new LoadInst(bit, "", false, bb);
	bit = new ZExtInst(n, getIntegerType(width), "", bb);
	bit = BinaryOperator::Create(Instruction::Shl, bit, ConstantInt::get(getIntegerType(width), shift), "", bb);
	return BinaryOperator::Create(Instruction::Or, flags, bit, "", bb);
}

void
arch_decode_bit(Value *flags, Value *bit, int shift, int width, BasicBlock *bb)
{
	Value *n = BinaryOperator::Create(Instruction::LShr, flags, ConstantInt::get(getIntegerType(width), shift), "", bb);
	n = new TruncInst(n, getIntegerType(1), "", bb);
	new StoreInst(n, bit, bb);
}
