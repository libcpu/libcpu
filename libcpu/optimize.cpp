/*
 * libcpu: optimize.cpp
 *
 * Tell LLVM to run optimizers over the IR.
 */

#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/DataLayout.h"

#include "libcpu.h"

void
optimize(cpu_t *cpu)
{
	llvm::legacy::FunctionPassManager pm = llvm::legacy::FunctionPassManager(cpu->mod);

	pm.add(createPromoteMemoryToRegisterPass());
	pm.add(createInstructionCombiningPass());
	pm.add(createConstantPropagationPass());
	pm.add(createDeadCodeEliminationPass());
	pm.run(*cpu->cur_func);
}

