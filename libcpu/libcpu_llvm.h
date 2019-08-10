#ifndef _LIBCPU_LLVM_H_
#define _LIBCPU_LLVM_H_

#include "llvm/ADT/APFloat.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/LLVMContext.h"

//////////////////////////////////////////////////////////////////////
// LLVM Helpers
//////////////////////////////////////////////////////////////////////

#define _CTX() (*cpu->ctx)

#define XgetType(x) (Type::get##x(_CTX()))
#define getIntegerType(x) (IntegerType::get(_CTX(), x))
#define getNamedStructType(x, ...) (StructType::create(_CTX(), x, name,    \
					       #__VA_ARGS__))

#define getFloatType(bits) getFloatType0(*cpu->ctx, bits)

static inline Type *getFloatType0(LLVMContext &ctx, unsigned bits)
{
	switch(bits) {
		case 32: return Type::getFloatTy(ctx);
		case 64: return Type::getDoubleTy(ctx);
		case 80: return Type::getX86_FP80Ty(ctx);
		case 128: return Type::getFP128Ty(ctx);
		default: return 0;
	}
}

#endif
