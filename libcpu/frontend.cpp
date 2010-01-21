/*
 * libcpu: frontend.cpp
 *
 * This is the interface that frontends use. But frontends
 * don't use this directly, but go through the macros in
 * frontend.h
 */

#include <assert.h>

#include "llvm/Constants.h"
#include "llvm/Intrinsics.h"
#include "llvm/Support/IRBuilder.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/Target/TargetData.h"

#include "libcpu.h"
#include "libcpu_llvm.h"
#include "frontend.h"

//////////////////////////////////////////////////////////////////////
// GENERIC: register access
//////////////////////////////////////////////////////////////////////

#define IS_LITTLE_ENDIAN(x)   (((cpu)->info.common_flags & CPU_FLAG_ENDIAN_MASK) == CPU_FLAG_ENDIAN_LITTLE)
#define HAS_SPECIAL_GPR0(cpu) ((cpu)->info.common_flags & CPU_FLAG_HARDWIRE_GPR0)
#define HAS_SPECIAL_FPR0(cpu) ((cpu)->info.common_flags & CPU_FLAG_HARDWIRE_FPR0)

// GET REGISTER
Value *
arch_get_reg(cpu_t *cpu, uint32_t index, uint32_t bits, BasicBlock *bb) {
	Value *v;
	Value **regs = cpu->ptr_gpr;
	uint32_t size = cpu->info.register_size[CPU_REG_GPR];
	uint32_t count = cpu->info.register_count[CPU_REG_GPR];

	/*
	 * XXX If the index is past the number of the available GPRs,
	 * then it's considered an XR.  This is in order to maintain
	 * compatibility with the current implementation.
	 */
	if (index >= count) {
		index -= count;
		regs = cpu->ptr_xr;
		size = cpu->info.register_size[CPU_REG_XR];
		count = cpu->info.register_count[CPU_REG_XR];
		if (index >= count) {
			assert(0 && "GPR/XR register index is out of range!");
			return NULL;
		}
	} else {
		/* R0 is always 0 (on certain RISCs) */
		if (HAS_SPECIAL_GPR0(cpu) && index == 0)
			return CONSTs(bits ? bits : size, 0);
	}

	/* get the register */
	v = new LoadInst(regs[index], "", false, bb);

	/* optionally truncate it */
	if (bits != 0 && bits < size)
		v = TRUNC(bits, v);

	return v;
}

Value *
arch_get_fp_reg(cpu_t *cpu, uint32_t index, uint32_t bits, BasicBlock *bb)
{
	assert(index < cpu->info.register_count[CPU_REG_FPR]);
	/* get the register */
	return new LoadInst(cpu->ptr_fpr[index], "", false, bb);
}

// PUT REGISTER
Value *
arch_put_reg(cpu_t *cpu, uint32_t index, Value *v, uint32_t bits, bool sext,
	BasicBlock *bb)
{
	Value **regs = cpu->ptr_gpr;
	uint32_t size = cpu->info.register_size[CPU_REG_GPR];
	uint32_t count = cpu->info.register_count[CPU_REG_GPR];

	/*
	 * XXX If the index is past the number of the available GPRs,
	 * then it's considered an XR.  This is in order to maintain
	 * compatibility with the current implementation.
	 */
	if (index >= count) {
		index -= count;
		regs = cpu->ptr_xr;
		size = cpu->info.register_size[CPU_REG_XR];
		count = cpu->info.register_count[CPU_REG_XR];
		if (index >= count) {
			assert(0 && "GPR/XR register index is out of range!");
			return NULL;
		}
	} 

	/*
	 * if the caller cares about bit size and
	 * the size is not the register size, we'll zext or sext
	 */
	if (bits != 0 && size != bits) {
		if (sext)
			v = SEXT(size, v);
		else
			v = ZEXT(size, v);
	}

	/* store value, unless it's R0 (on certain RISCs) */
	if (regs == cpu->ptr_xr || !HAS_SPECIAL_GPR0(cpu) || index != 0)
		new StoreInst(v, regs[index], bb);

	return v;
}

void
arch_put_fp_reg(cpu_t *cpu, uint32_t index, Value *v, uint32_t bits,
	BasicBlock *bb)
{
	assert(index < cpu->info.register_count[CPU_REG_FPR]);
	if (bits == 0)
		bits = v->getType()->getPrimitiveSizeInBits();
	assert(bits == cpu->info.float_size);

	/* store value, unless it's R0 (on certain RISCs) */
	if (!HAS_SPECIAL_FPR0(cpu) || index != 0) {
		new StoreInst(v, cpu->ptr_fpr[index], bb);
	}
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

uint32_t
RAM32LE(uint8_t *RAM, addr_t a) {
	uint32_t v;
	v  = RAM[a+0] << 0;
	v |= RAM[a+1] << 8;
	v |= RAM[a+2] << 16;
	v |= RAM[a+3] << 24;
	return v;
}

//////////////////////////////////////////////////////////////////////
// GENERIC: memory access
//////////////////////////////////////////////////////////////////////

/* get a RAM pointer to a 32 bit value */
static Value *
arch_gep32(cpu_t *cpu, Value *a, BasicBlock *bb) {
	a = GetElementPtrInst::Create(cpu->ptr_RAM, a, "", bb);
	return new BitCastInst(a, PointerType::get(XgetType(Int32Ty), 0), "", bb);
}

/* load 32 bit ALIGNED value from RAM */
Value *
arch_load32_aligned(cpu_t *cpu, Value *a, BasicBlock *bb) {
	a = arch_gep32(cpu, a, bb);
	if (cpu->flags & CPU_FLAG_SWAPMEM)
		return SWAP32(new LoadInst(a, "", false, bb));
	else
		return new LoadInst(a, "", false, bb);
}

/* store 32 bit ALIGNED value to RAM */
void
arch_store32_aligned(cpu_t *cpu, Value *v, Value *a, BasicBlock *bb) {
	a = arch_gep32(cpu, a, bb);
	new StoreInst((cpu->flags & CPU_FLAG_SWAPMEM) ? SWAP32(v) : v, a, bb);
}

//////////////////////////////////////////////////////////////////////
// GENERIC: endianness
//////////////////////////////////////////////////////////////////////

static Value *
arch_get_shift8(cpu_t *cpu, Value *addr, BasicBlock *bb)
{
	Value *shift = AND(addr,CONST(3));
	if (!IS_LITTLE_ENDIAN(cpu))
		shift = XOR(shift, CONST(3));
	return SHL(shift, CONST(3));
}

static Value *
arch_get_shift16(cpu_t *cpu, Value *addr, BasicBlock *bb)
{
	Value *shift = AND(LSHR(addr,CONST(1)),CONST(1));
	if (!IS_LITTLE_ENDIAN(cpu))
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

//

Value *
arch_store(Value *v, Value *a, BasicBlock *bb)
{
	new StoreInst(v, a, bb);
	return v;
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

// complex operations

// shifts or rotates an lvalue left or right, regardless of width
Value *
arch_shiftrotate(cpu_t *cpu, Value *dst, Value *src, bool left, bool rotate, BasicBlock *bb)
{
	Value *c;
	Value *v = LOAD(src);

	if (left) {
		c = ICMP_SLT(v, CONSTs(SIZE(v), 0));	/* old MSB to carry */
		v = SHL(v, CONSTs(SIZE(v), 1));
		if (rotate)
			v = OR(v,ZEXT(SIZE(v), LOAD(cpu->ptr_C)));
	} else {
		c = TRUNC1(v);		/* old LSB to carry */
		v = LSHR(v, CONSTs(SIZE(v), 1));
		if (rotate)
			v = OR(v,SHL(ZEXT(SIZE(v), LOAD(cpu->ptr_C)), CONSTs(SIZE(v), SIZE(v)-1)));
	}
	
	LET1(cpu->ptr_C, c);
	return STORE(v, dst);
}

// adds src + v + c and stores it in dst
Value *
arch_adc(cpu_t *cpu, Value *dst, Value *src, Value *v, bool plus_carry, bool plus_one, BasicBlock *bb)
{
	Value *c;
	if (plus_carry)
		c = LOAD(cpu->ptr_C);
	else if (plus_one)
		c = CONST1(1);
	else
		c = CONST1(0);

	if (SIZE(v) == 8) {
		/* calculate intermediate result */
		Value *v1 = ADD(ADD(ZEXT16(LOAD(src)), ZEXT16(v)), ZEXT16(c));

		/* get C */
		STORE(TRUNC1(LSHR(v1, CONST16(8))), cpu->ptr_C);

		/* get result */
		v1 = TRUNC8(v1);

		if (dst)
			STORE(v1, dst);

		return v1;
	} else {
		//XXX TODO use llvm.uadd.with.overflow.*
		//XXX consider using it for 8 bit also, if possible
		printf("TODO: %s() can't do anything but 8 bits yet!\n", __func__);
		exit(1);
	}
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

// flags encoding and decoding

Value *
arch_flags_encode(cpu_t *cpu, BasicBlock *bb)
{
	uint32_t flags_size = cpu->info.psr_size;
	cpu_flags_layout_t const *flags_layout = cpu->info.flags_layout;
	Value *flags = CONSTs(flags_size, 0);

	for (size_t i = 0; i < cpu->info.flags_count; i++)
		flags = arch_encode_bit(flags, cpu->ptr_FLAG[flags_layout[i].shift],
				flags_layout[i].shift, flags_size, bb);

	return flags;
}

void
arch_flags_decode(cpu_t *cpu, Value *flags, BasicBlock *bb)
{
	uint32_t flags_size = cpu->info.psr_size;
	cpu_flags_layout_t const *flags_layout = cpu->info.flags_layout;

	for (size_t i = 0; i < cpu->info.flags_count; i++)
		arch_decode_bit(flags, cpu->ptr_FLAG[flags_layout[i].shift],
				flags_layout[i].shift, flags_size, bb);
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
