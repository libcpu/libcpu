#ifndef _LIBCPU_LLVM_H_
#define _LIBCPU_LLVM_H_

#include "llvm/ADT/APFloat.h"
#include "llvm/DerivedTypes.h"
#include "llvm/LLVMContext.h"

//////////////////////////////////////////////////////////////////////
// LLVM Helpers
//////////////////////////////////////////////////////////////////////

#define _CTX() getGlobalContext()
#define XgetType(x) (Type::get##x(_CTX()))
#define getIntegerType(x) (IntegerType::get(_CTX(), x))
#define getStructType(x, ...) (StructType::get(_CTX(), x,    \
					       #__VA_ARGS__))

static inline fltSemantics const *getFltSemantics(unsigned bits)
{
	switch(bits) {
		case 32: return &APFloat::IEEEsingle;
		case 64: return &APFloat::IEEEdouble;
		case 80: return &APFloat::x87DoubleExtended;
		case 128: return &APFloat::IEEEquad;
		default: return 0;
	}
}

static inline Type const *getFloatType(unsigned bits)
{
	switch(bits) {
		case 32: return Type::getFloatTy(_CTX());
		case 64: return Type::getDoubleTy(_CTX());
		case 80: return Type::getX86_FP80Ty(_CTX());
		case 128: return Type::getFP128Ty(_CTX());
		default: return 0;
	}
}

#endif
