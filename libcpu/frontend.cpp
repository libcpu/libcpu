#include "frontend.h"

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

static Value **
ptr_f(cpu_t *cpu)
{
	switch (cpu->fp_reg_size) {
		case 32:
			return cpu->ptr_f32;
		case 64:
			return cpu->ptr_f64;
		case 80:
			return cpu->ptr_f80;
		case 128:
			return cpu->ptr_f128;
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

Value *
arch_get_fp_reg(cpu_t *cpu, uint32_t index, uint32_t bits, BasicBlock *bb) {
	Value *v;

	/* get the register */
	return new LoadInst(ptr_f(cpu)[index], "", false, bb);
}

// PUT REGISTER
Value *
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

	return v;
}

void
arch_put_fp_reg(cpu_t *cpu, uint32_t index, Value *v, uint32_t bits, BasicBlock *bb) {
	/* store value, unless it's R0 (on certain RISCs) */
	if (!cpu->has_special_fr0 || index)
		new StoreInst(v, ptr_f(cpu)[index], bb);
}

//XXX TODO
// The guest CPU can be little endian or big endian, so we need both
// host mode access routines as well as IR generators that deal with
// both modes. In practice, we need two sets of functions, one that
// deals with host native endianness, and one that deals with the other
// endianness; and interface functions that dispatch calls to endianness
// functions depending on the host endianness.
// i.e. there should be RAM32NE() which reads a 32 bit address from RAM,
// using the "Native Endianness" of the host, and RAM32SW() which does
// a swapped read. RAM32BE() and RAM32LE() should choose either of
// RAM32NE() and RAM32SW() depending on the endianness of the host.
//
// Swapped endianness can be implemented in different ways: by swapping
// every non-byte memory read and write, or by keeping aligned words
// in the host's native endianness, not swapping aligned reads and
// writes, and correcting unaligned accesses. (This makes more sense
// on guest CPUs that only allow aligned memory access.)
// The client should be able to specify a specific strategy, but each
// CPU frontend should default to the typically best strategy. Functions
// like RAM32SW() will have to respect the setting, so that all memory
// access is consistent with the strategy.

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
	bool swap = (cpu->exec_engine->getTargetData()->isLittleEndian () ^ cpu->is_little_endian);
	if(swap)
		return SWAP32(new LoadInst(a, "", false, bb));
	else
		return new LoadInst(a, "", false, bb);
}

/* store 32 bit ALIGNED value to RAM */
void
arch_store32_aligned(cpu_t *cpu, Value *v, Value *a, BasicBlock *bb) {
	a = arch_gep32(cpu, a, bb);
	bool swap = (cpu->exec_engine->getTargetData()->isLittleEndian () ^ cpu->is_little_endian);
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
	return SHL(shift, CONST(3));
}

static Value *
arch_get_shift16(cpu_t *cpu, Value *addr, BasicBlock *bb)
{
	Value *shift = AND(LSHR(addr,CONST(1)),CONST(1));
	if (!cpu->is_little_endian)
		shift = XOR(shift, CONST(1));
	return SHL(shift, CONST(4));
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

//////////////////////////////////////////////////////////////////////

Value *
arch_bswap(cpu_t *cpu, size_t width, Value *v, BasicBlock *bb) {
	Type const *ty = getIntegerType(width);
	return CallInst::Create(Intrinsic::getDeclaration(cpu->mod, Intrinsic::bswap, &ty, 1), v, "", bb);
}

Value *
arch_ctlz(cpu_t *cpu, size_t width, Value *v, BasicBlock *bb) {
	Type const *ty = getIntegerType(width);
	return CallInst::Create(Intrinsic::getDeclaration(cpu->mod, Intrinsic::ctlz, &ty, 1), v, "", bb);
}

Value *
arch_cttz(cpu_t *cpu, size_t width, Value *v, BasicBlock *bb) {
	Type const *ty = getIntegerType(width);
	return CallInst::Create(Intrinsic::getDeclaration(cpu->mod, Intrinsic::cttz, &ty, 1), v, "", bb);
}

// branches

void
arch_branch(bool flag_state, BasicBlock *target1, BasicBlock *target2, Value *v, BasicBlock *bb) {
	if (!target1) {
		printf("target1 is NULL\n");
		exit(1);
	}
	if (!target2) {
		printf("target2 is NULL\n");
		exit(1);
	}
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

// FP

Value *
arch_sqrt(cpu_t *cpu, size_t width, Value *v, BasicBlock *bb) {
	Type const *ty = getFloatType(width);
	return CallInst::Create(Intrinsic::getDeclaration(cpu->mod, Intrinsic::sqrt, &ty, 1), v, "", bb);
}

// Invoke debug_function

void
arch_debug_me(cpu_t *cpu, BasicBlock *bb)
{
	if (cpu->ptr_func_debug == NULL)
		return;

	Type const *intptr_type = cpu->exec_engine->getTargetData()->getIntPtrType(_CTX());
	Constant *v_cpu = ConstantInt::get(intptr_type, (uintptr_t)cpu);
	Value *v_cpu_ptr = ConstantExpr::getIntToPtr(v_cpu, PointerType::getUnqual(intptr_type));

	// XXX synchronize cpu context!
	CallInst::Create(cpu->ptr_func_debug, v_cpu_ptr, "", bb);
}
