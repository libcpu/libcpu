/*
 * libcpu: optimize.cpp
 *
 * Tell LLVM to run optimizers over the IR.
 */

#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/IR/Module.h"
#include "llvm/PassManager.h"
#include "llvm/IR/DataLayout.h"

#include "libcpu.h"

void
optimize(cpu_t *cpu)
{
	FunctionPassManager pm = FunctionPassManager(cpu->mod);

	pm.add(createPromoteMemoryToRegisterPass());
	pm.add(createInstructionCombiningPass());
	pm.add(createConstantPropagationPass());
	pm.add(createDeadCodeEliminationPass());
	pm.run(*cpu->cur_func);
}

