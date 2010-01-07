/*
 * Add big switches for testing here.
 * libcpu shouldn't use many ifdefs. Consider dropping one of
 * the alteratives or make it a runtime option (i.e. let the
 * client decide).
 */

// Copy register set paramters into a local array this hints LLVM to
// not care about writing back the contents too often.
// This works particularly well with PromoteMemoryToReg optimization
// pass, which removes the local arrays and make LLVM do the register
// allocation for us
#define OPT_LOCAL_REGISTERS

// DFS limit when CPU_CODEGEN_TAG_LIMIT is set by the client.
// '6' is the optimum for OpenBSD's 'date' on M88K.
#define LIMIT_TAGGING_DFS 6
