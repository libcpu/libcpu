/*
 * libcpu: fp.cpp
 *
 * IEEE754 Floating Point Handling
 */

#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/Instructions.h"
#include "llvm/Target/TargetData.h"

#include "libcpu.h"
#include "libcpu_llvm.h"
#include "frontend.h"

#define HAS_SPECIAL_FPR0(cpu) ((cpu)->info.common_flags & CPU_FLAG_HARDWIRE_FPR0)

/* 
 * Don't expose these macros!
 */
#define FP_BITCASTs(s, v) new BitCastInst(v, getFloatType(s), "", bb)
#define FP_BITCAST32(v) FP_BITCASTs(32, v)
#define FP_BITCAST64(v) FP_BITCASTs(64, v)

//
// The following code is used to synthesize FP80 and FP128
// registers. Internally, the core works with the widest
// precision available eventually performs conversion between
// formats during load and store.
//
// This code is valid only for IEEE754 floating point.
//
// XXX This code does not support fp128 and doesn't deal when
// fp128 is natively supported and fp80 is not; in the latter
// case fp80 is an alias for fp64.
//

// FP64
static inline Value *fp64_mantissa(cpu_t *cpu, Value *v, BasicBlock *bb)
{ return AND(v, CONST64((1ULL << 52) - 1)); }

static inline Value *fp64_exponent(cpu_t *cpu, Value *v, BasicBlock *bb)
{ return AND(LSHR(v, CONST64(52)), CONST64(0x7ff)); }

static inline Value *fp64_sign(cpu_t *cpu, Value *v, BasicBlock *bb)
{ return LSHR(v, CONST64(63)); }

#define FP64_MANTISSA(v) fp64_mantissa(cpu, v, bb)
#define FP64_EXPONENT(v) fp64_exponent(cpu, v, bb)
#define FP64_SIGN(v)     fp64_sign(cpu, v, bb)

// FP80 
static inline Value *fp80_mantissa(cpu_t *cpu, Value *hi, Value *lo, BasicBlock *bb)
{ return SHL(lo, CONST32(1)); }

static inline Value *fp80_exponent(cpu_t *cpu, Value *hi, Value *lo, BasicBlock *bb)
{ return AND(hi, CONST32(0x7fff)); }

static inline Value *fp80_sign(cpu_t *cpu, Value *hi, Value *lo, BasicBlock *bb)
{ return LSHR(hi, CONST32(15)); }

#define FP80_MANTISSA(v) fp80_mantissa(cpu, v+0, v+1, bb)
#define FP80_EXPONENT(v) fp80_exponent(cpu, v+0, v+1, bb)
#define FP80_SIGN(v)     fp80_sign(cpu, v+0, v+1, bb)

//////////////////////////////////////////////////////////////////////

// defined in frontend.cpp
Value *arch_get_fp_reg(cpu_t *cpu, uint32_t index, uint32_t bits,
	BasicBlock *bb);
void arch_put_fp_reg(cpu_t *cpu, uint32_t index, Value *v, uint32_t bits,
	BasicBlock *bb);

//////////////////////////////////////////////////////////////////////

static void
arch_synthesize_fp80_store(cpu_t *cpu, uint32_t index, Value *v, BasicBlock *bb)
{
	Value *v64 = IBITCAST64(arch_cast_fp64(cpu, v, bb));
	Value *sign = FP64_SIGN(v64);
	Value *exp = FP64_EXPONENT(v64);
	Value *mantissa = FP64_MANTISSA(v64);

	//
	// expand mantissa:
	//
	// mantissa = (1 << 63) | (mantissa << 11)
	//
	mantissa = OR(SHL(mantissa, CONST64(11)), CONST64(1ULL << 63));

	//
	// expand exponent:
	// 
	// ((exp & 0x700) << 4) |
	//  ((-((exp >> 9) & 1)) & 0xf00) |
	//  (exp & 0x3ff)
	//
	exp = OR(OR(SHL(AND(exp, CONST64(0x700)), CONST64(4)),
				AND(NEGs(64, AND(LSHR(exp, CONST64(9)), CONST64(1))),
					CONST64(0xf00))),
			 AND(exp, CONST64(0x3ff)));

	Value *hi = OR(SHL(sign, CONST64(15)), exp);
	Value *lo = mantissa;

	// store
	if (cpu->exec_engine->getTargetData()->isLittleEndian()) {
		arch_put_fp_reg(cpu, index + 0, lo, 64, bb);
		arch_put_fp_reg(cpu, index + 1, hi, 64, bb);
	} else {
		arch_put_fp_reg(cpu, index + 0, hi, 64, bb);
		arch_put_fp_reg(cpu, index + 1, lo, 64, bb);
	}
}

static Value *
arch_synthesize_fp80_load(cpu_t *cpu, uint32_t index, BasicBlock *bb)
{
	Value *hi, *lo;
	Value *v64;

	if (index == 0 && HAS_SPECIAL_FPR0(cpu))
		return FPCONST64(0);

	// load
	if (cpu->exec_engine->getTargetData()->isLittleEndian()) {
		lo = arch_get_fp_reg(cpu, index + 0, 64, bb);
		hi = arch_get_fp_reg(cpu, index + 1, 64, bb);
	} else {
		hi = arch_get_fp_reg(cpu, index + 0, 64, bb);
		lo = arch_get_fp_reg(cpu, index + 1, 64, bb);
	}

	Value *sign = LSHR(hi, CONST64(15));
	Value *exp = AND(hi, CONST64(0x7fff));
	Value *mantissa = SHL(lo, CONST64(1));

	// compress fp80 -> fp64
	//
	// (sign << 63) |
	// ((exp & 0x7000) << 48) |
	// ((exp & 0x3ff) << 52) |
	// (mantissa >> 12)
	//
	v64 = OR(OR(OR(SHL(sign, CONST64(63)),
				   SHL(AND(exp, CONST64(0x7000)), CONST64(48))),
				SHL(AND(exp, CONST64(0x3ff)), CONST64(52))),
			 LSHR(mantissa, CONST64(12)));

	return FP_BITCAST64(v64);
}

static inline Value *
fp_cast(cpu_t *cpu, uint32_t bits, Value *v, BasicBlock *bb)
{
	switch (bits) {
		case 32: return arch_cast_fp32(cpu, v, bb);
		case 64: return arch_cast_fp64(cpu, v, bb);
		case 80: return arch_cast_fp80(cpu, v, bb);
		case 128: return arch_cast_fp128(cpu, v, bb);
		default: abort();
	}
	/* NOTREACHED */
	return NULL;
}

//////////////////////////////////////////////////////////////////////
// PUBLIC API
//////////////////////////////////////////////////////////////////////

void
arch_store_fp_reg(cpu_t *cpu, uint32_t index, Value *v, uint32_t bits,
	BasicBlock *bb)
{
	uint32_t size = cpu->info.register_size[CPU_REG_FPR];

	/*
	 * if the caller cares about bit size and
	 * the size is not the register size, we'll extend.
	 */
	if (bits && size != bits)
		v = fp_cast(cpu, size, v, bb);

	if (size == 80 && (cpu->flags & CPU_FLAG_FP80) == 0) {
		arch_synthesize_fp80_store(cpu, index * 2, v, bb);
	} else if (size == 128 && (cpu->flags & CPU_FLAG_FP128) == 0) {
		assert(0 && "FP128 not yet implemented");
	} else {
		arch_put_fp_reg(cpu, index, v, bits, bb);
	}
}

Value *
arch_load_fp_reg(cpu_t *cpu, uint32_t index, uint32_t bits,
	BasicBlock *bb)
{
	Value *v = NULL;
	uint32_t size = cpu->info.register_size[CPU_REG_FPR];

	if (size == 80 && (cpu->flags & CPU_FLAG_FP80) == 0) {
		v = arch_synthesize_fp80_load(cpu, index * 2, bb);
	} else if (size == 128 && (cpu->flags & CPU_FLAG_FP128) == 0) {
		assert(0 && "FP128 not yet implemented");
	} else {
		v = arch_get_fp_reg(cpu, index, bits, bb);
	}

	/* optionally cast it */
	if (bits != 0 && size != bits)
		v = fp_cast(cpu, bits, v, bb);

	return v;
}

//////////////////////////////////////////////////////////////////////

Value *
arch_cast_fp32(cpu_t *cpu, Value *v, BasicBlock *bb)
{
	Type const *type = v->getType();
	if (type->isInteger()) {
		if (type->getPrimitiveSizeInBits() < 32)
			v = FP_BITCAST32(ZEXT32(v));
		if (type->getPrimitiveSizeInBits() > 32)
			v = FP_BITCAST32(TRUNC32(v));
		else 
			v = FP_BITCAST32(v);

		type = v->getType();
	}
	if (type->isFloatingPoint()) {
		if (type->getPrimitiveSizeInBits() > 32)
			v = FPTRUNC(32, v);
	} else {
		abort();
	}

	return v;
}

Value *
arch_cast_fp64(cpu_t *cpu, Value *v, BasicBlock *bb)
{
	Type const *type = v->getType();
	if (type->isInteger()) {
		if (type->getPrimitiveSizeInBits() < 32)
			v = FPEXT(64, FP_BITCAST32(ZEXT32(v)));
		else if (type->getPrimitiveSizeInBits() < 64)
			v = FPEXT(64, FP_BITCAST32(v));
		else
			v = FP_BITCAST64(v);

		type = v->getType();
	}
	if (type->isFloatingPoint()) {
		if (type->getPrimitiveSizeInBits() < 64)
			v = FPEXT(64, v);
		else if (type->getPrimitiveSizeInBits() > 64)
			v = FPTRUNC(64, v);
	} else {
		abort();
	}

	return v;
}

//
// If host doesn't support 80bits, cast to fp64.
//
Value *
arch_cast_fp80(cpu_t *cpu, Value *v, BasicBlock *bb)
{
	if (cpu->flags & CPU_FLAG_FP80) {
		Type const *type = v->getType();
		if (type->isInteger()) {
			if (type->getPrimitiveSizeInBits() < 32)
				v = FPEXT(80, FP_BITCAST32(ZEXT32(v)));
			else if (type->getPrimitiveSizeInBits() < 64)
				v = FPEXT(80, FP_BITCAST32(v));
			else
				v = FPEXT(80, FP_BITCAST64(v));

			type = v->getType();
		}
		if (type->isFloatingPoint()) {
			if (type->getPrimitiveSizeInBits() < 80)
				v = FPEXT(80, v);
			else if (type->getPrimitiveSizeInBits() > 80)
				v = FPTRUNC(80, v);
		} else {
			abort();
		}
	} else {
		v = arch_cast_fp64(cpu, v, bb);
	}

	return v;
}

//
// If host doesn't support 80bits, cast to fp80,
// which in turn will fallback to fp64 if not
// supported.
//
Value *
arch_cast_fp128(cpu_t *cpu, Value *v, BasicBlock *bb)
{
	if (cpu->flags & CPU_FLAG_FP128) {
		Type const *type = v->getType();
		if (type->isInteger()) {
			if (type->getPrimitiveSizeInBits() < 32)
				v = FPEXT(128, FP_BITCAST32(ZEXT32(v)));
			else if (type->getPrimitiveSizeInBits() < 64)
				v = FPEXT(128, FP_BITCAST32(v));
			else
				v = FPEXT(128, FP_BITCAST64(v));

			type = v->getType();
		}
		if (type->isFloatingPoint()) {
			if (type->getPrimitiveSizeInBits() < 128)
				v = FPEXT(128, v);
		} else {
			abort();
		}
	} else {
		v = arch_cast_fp80(cpu, v, bb);
	}

	return v;
}
