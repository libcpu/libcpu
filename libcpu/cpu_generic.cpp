#include "cpu_generic.h"

uint32_t reg_size;
bool is_little_endian;
bool has_special_r0;

//////////////////////////////////////////////////////////////////////
// GENERIC: register access
//////////////////////////////////////////////////////////////////////

static Value **
ptr_r()
{
	switch (reg_size) {
		case 8:
			return ptr_r8;
		case 16:
			return ptr_r16;
		case 32:
			return ptr_r32;
		case 64:
			return ptr_r64;
		default:
			return 0; /* can't happen */
	}
}

// GET REGISTER
Value *
arch_get_reg(uint32_t index, uint32_t bits, BasicBlock *bb) {
	Value *v;

	/* R0 is always 0 (on certain RISCs) */
	if (has_special_r0 && !index)
		return CONSTs(bits? bits : reg_size, 0);

	/* get the register */
	v = new LoadInst(ptr_r()[index], "", false, bb);

	/* optionally truncate it */
	if (bits && reg_size != bits)
		v = TRUNC(bits, v);
	
	return v;
}

// PUT REGISTER
void
arch_put_reg(uint32_t index, Value *v, uint32_t bits, bool sext, BasicBlock *bb) {
	/*
	 * if the caller cares about bit size and
	 * the size is not the register size, we'll zext or sext
	 */
	if (bits && reg_size != bits)
		if (sext)
			v = SEXT(reg_size, v);
		else
			v = ZEXT(reg_size, v);

	/* store value, unless it's R0 (on certain RISCs) */
	if (!has_special_r0 || index)
		new StoreInst(v, ptr_r()[index], bb);
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
arch_gep32(Value *a, BasicBlock *bb) {
	a = GetElementPtrInst::Create(ptr_RAM, a, "", bb);
	return new BitCastInst(a, PointerType::get(Type::Int32Ty, 0), "", bb);
}

/* load 32 bit ALIGNED value from RAM */
Value *
arch_load32_aligned(Value *a, BasicBlock *bb) {
	a = arch_gep32(a, bb);
	return new LoadInst(a, "", false, bb);
}

/* store 32 bit ALIGNED value to RAM */
void
arch_store32_aligned(Value *v, Value *a, BasicBlock *bb) {
	a = arch_gep32(a, bb);
	new StoreInst(v, a, bb);
}

//////////////////////////////////////////////////////////////////////
// GENERIC: endianness
//////////////////////////////////////////////////////////////////////

static Value *
arch_get_shift8(Value *addr, BasicBlock *bb)
{
	Value *shift = AND(addr,CONST(3));
	if (!is_little_endian)
		shift = XOR(shift, CONST(3));
	return SHL(CONST(3), shift);
}

static Value *
arch_get_shift16(Value *addr, BasicBlock *bb)
{
	Value *shift = AND(addr,CONST(1));
	if (!is_little_endian)
		shift = XOR(shift, CONST(1));
	return SHL(CONST(4), shift);
}

Value *
arch_load8(Value *addr, BasicBlock *bb) {
	Value *shift = arch_get_shift8(addr, bb);
	Value *val = arch_load32_aligned(AND(addr, CONST(~3ULL)), bb);
	return TRUNC8(LSHR(val, shift));
}

Value *
arch_load16_aligned(Value *addr, BasicBlock *bb) {
	Value *shift = arch_get_shift16(addr, bb);
	Value *val = arch_load32_aligned(AND(addr, CONST(~3ULL)), bb);
	return TRUNC16(LSHR(val, shift));
}

void
arch_store8(Value *val, Value *addr, BasicBlock *bb) {
	Value *shift = arch_get_shift8(addr, bb);
	addr = AND(addr, CONST(~3ULL));
	Value *mask = XOR(SHL(CONST(255), shift),CONST(-1ULL));
	Value *old = AND(arch_load32_aligned(addr, bb), mask);
	val = OR(old, SHL(AND(val, CONST(255)), shift));
	arch_store32_aligned(val, addr, bb);
}

void
arch_store16(Value *val, Value *addr, BasicBlock *bb) {
	Value *shift = arch_get_shift16(addr, bb);
	addr = AND(addr, CONST(~3ULL));
	Value *mask = XOR(SHL(CONST(65535), shift),CONST(-1ULL));
	Value *old = AND(arch_load32_aligned(addr, bb), mask);
	val = OR(old, SHL(AND(val, CONST(65535)), shift));
	arch_store32_aligned(val, addr, bb);
}


// branches

extern const BasicBlock *lookup_basicblock(Function *f, addr_t pc);
void
arch_branch(bool flag_state, addr_t pc1, addr_t pc2, Value *v, Function *f, BasicBlock *bb) {
printf("BRANCH(%llx,%llx)\n", pc1, pc2);
	BasicBlock *target1 = (BasicBlock*)lookup_basicblock(f, pc1);
	BasicBlock *target2 = (BasicBlock*)lookup_basicblock(f, pc2);
	if (!target1) {
		printf("error: unknown branch target $%04llx!\n", (unsigned long long)pc1);
		exit(1);
	}
	if (!target2) {
		printf("error: unknown branch continue $%04llx!\n", (unsigned long long)pc2);
		exit(1);
	}
	if (flag_state)
		BranchInst::Create(target1, target2, v, bb);
	else
		BranchInst::Create(target2, target1, v, bb);
}
