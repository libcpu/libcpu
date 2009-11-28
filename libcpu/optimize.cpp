/*
 * libcpu: optimize.cpp
 *
 * Tell LLVM to run optimizers over the IR.
 */
#include "libcpu.h"
//XXX how do we know this is a suitable set of optimizations?
void
optimize(cpu_t *cpu)
{
	PassManager pm;
	uint64_t flags = cpu->flags_optimize;

	std::string data_layout = cpu->exec_engine->getTargetData()->getStringRepresentation();
	TargetData *TD = new TargetData(data_layout);
	pm.add(TD);

	//pm.add(createStripDeadPrototypesPass());
	if (flags & (1ULL<<0))
		pm.add(createGlobalDCEPass());

	if (flags & (1ULL<<1))
		pm.add(createRaiseAllocationsPass());
	if (flags & (1ULL<<2))
		pm.add(createCFGSimplificationPass());
	if (flags & (1ULL<<3))
		pm.add(createPromoteMemoryToRegisterPass());
	if (flags & (1ULL<<4))
		pm.add(createGlobalOptimizerPass());
	if (flags & (1ULL<<5))
		pm.add(createGlobalDCEPass());

	if (flags & (1ULL<<6))
		pm.add(createIPConstantPropagationPass());
	if (flags & (1ULL<<7))
		pm.add(createDeadArgEliminationPass());
	if (flags & (1ULL<<8))
		pm.add(createInstructionCombiningPass());
	if (flags & (1ULL<<9))
		pm.add(createCFGSimplificationPass());
	if (flags & (1ULL<<10))
		pm.add(createPruneEHPass());

	if (flags & (1ULL<<11))
		pm.add(createFunctionInliningPass());

	if (flags & (1ULL<<12))
		pm.add(createArgumentPromotionPass());
	if (flags & (1ULL<<13))
		pm.add(createTailDuplicationPass());
	if (flags & (1ULL<<14))
		pm.add(createInstructionCombiningPass());
	if (flags & (1ULL<<15))
		pm.add(createCFGSimplificationPass());
	if (flags & (1ULL<<16))
		pm.add(createScalarReplAggregatesPass());
	if (flags & (1ULL<<17))
		pm.add(createInstructionCombiningPass());
	if (flags & (1ULL<<18))
		pm.add(createCondPropagationPass());

	if (flags & (1ULL<<19))
		pm.add(createTailCallEliminationPass());
	if (flags & (1ULL<<20))
		pm.add(createCFGSimplificationPass());
	if (flags & (1ULL<<21))
		pm.add(createReassociatePass());
	if (flags & (1ULL<<22))
		pm.add(createLoopRotatePass());
	if (flags & (1ULL<<23))
		pm.add(createLICMPass());
	if (flags & (1ULL<<24))
		pm.add(createLoopUnswitchPass());
	if (flags & (1ULL<<25))
		pm.add(createInstructionCombiningPass());
	if (flags & (1ULL<<26))
		pm.add(createIndVarSimplifyPass());
	if (flags & (1ULL<<27))
		pm.add(createLoopUnrollPass());
	if (flags & (1ULL<<28))
		pm.add(createInstructionCombiningPass());
	if (flags & (1ULL<<29))
		pm.add(createGVNPass());
	if (flags & (1ULL<<30))
		pm.add(createSCCPPass());

	if (flags & (1ULL<<31))
		pm.add(createInstructionCombiningPass());
	if (flags & (1ULL<<32))
		pm.add(createCondPropagationPass());

	if (flags & (1ULL<<33))
		pm.add(createDeadStoreEliminationPass());
	if (flags & (1ULL<<34))
		pm.add(createAggressiveDCEPass());
	if (flags & (1ULL<<35))
		pm.add(createCFGSimplificationPass());
	if (flags & (1ULL<<36))
		pm.add(createSimplifyLibCallsPass());
	if (flags & (1ULL<<37))
		pm.add(createDeadTypeEliminationPass());
	if (flags & (1ULL<<38))
		pm.add(createConstantMergePass());

	pm.run(*cpu->mod);
}

