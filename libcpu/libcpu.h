#ifndef _LIBCPU_H_
#define _LIBCPU_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "llvm/Module.h"
#include "llvm/Function.h"
#include "llvm/PassManager.h"
#include "llvm/CallingConv.h"
#include "llvm/Intrinsics.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Assembly/PrintModulePass.h"
#include "llvm/Support/IRBuilder.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ModuleProvider.h"
#include "llvm/Target/TargetData.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/LinkAllPasses.h"
#include "llvm/Config/config.h"
#include "llvm/Target/TargetSelect.h"


#include "types.h"
#include "fp_types.h"

using namespace llvm;

struct cpu;
typedef void        (*fp_init)(struct cpu *cpu);
typedef StructType *(*fp_get_struct_reg)(struct cpu *cpu);
typedef addr_t      (*fp_get_pc)(struct cpu *cpu, void *regs);
typedef void        (*fp_emit_decode_reg)(struct cpu *cpu, BasicBlock *bb);
typedef void        (*fp_spill_reg_state)(struct cpu *cpu, BasicBlock *bb);
typedef int         (*fp_tag_instr)(struct cpu *cpu, addr_t pc, tag_t *tag, addr_t *new_pc, addr_t *next_pc);
typedef int         (*fp_disasm_instr)(struct cpu *cpu, addr_t pc, char *line, unsigned int max_line);
typedef Value      *(*fp_translate_cond)(struct cpu *cpu, addr_t pc, BasicBlock *bb);
typedef int         (*fp_translate_instr)(struct cpu *cpu, addr_t pc, BasicBlock *bb);
// idbg support
typedef uint64_t    (*fp_get_psr)(struct cpu *cpu, void *regs);
typedef int         (*fp_get_reg)(struct cpu *cpu, void *regs, unsigned reg_no, uint64_t *value);
typedef int         (*fp_get_fp_reg)(struct cpu *cpu, void *regs, unsigned reg_no, void *value);

typedef struct {
	fp_init init;
	fp_get_pc get_pc;
	fp_emit_decode_reg emit_decode_reg;
	fp_spill_reg_state spill_reg_state;
	fp_tag_instr tag_instr;
	fp_disasm_instr disasm_instr;
	fp_translate_cond translate_cond;
	fp_translate_instr translate_instr;
	// idbg support
	fp_get_psr get_psr;
	fp_get_reg get_reg;
	fp_get_fp_reg get_fp_reg;
} arch_func_t;

typedef enum {
	CPU_ARCH_INVALID = 0, //XXX unused
	CPU_ARCH_6502,
	CPU_ARCH_M68K,
	CPU_ARCH_MIPS,
	CPU_ARCH_M88K,
	CPU_ARCH_ARM
} cpu_arch_t;

enum {
	CPU_FLAG_FP80  = (1 << 15), // FP80 is natively supported.
	CPU_FLAG_FP128 = (1 << 16), // FP128 is natively supported.
};

typedef struct cpu {
	cpu_arch_t arch;
	arch_func_t f;
	uint16_t pc_offset;
	uint32_t pc_width;
	uint32_t count_regs_i8;
	uint32_t count_regs_i16;
	uint32_t count_regs_i32;
	uint32_t count_regs_i64;
	uint32_t count_regs_f32;
	uint32_t count_regs_f64;
	uint32_t count_regs_f80;
	uint32_t count_regs_f128;
	const char *name;
	addr_t code_start;
	addr_t code_end;
	addr_t code_entry;
	uint64_t flags_optimize;
	uint32_t flags_debug;
	uint32_t flags_hint;
	uint32_t flags;
	uint32_t flags_arch;
	tag_t *tag; /* array of flags, one per byte of code */
	Module *mod;
	Function *func_jitmain;
	ExecutionEngine *exec_engine;
	void *fp;
	void *reg;
	void *fp_reg;
	uint8_t *RAM;
	bool is_little_endian;
	uint32_t reg_size;
	bool has_special_r0;
	uint32_t fp_reg_size;
	bool has_special_fr0;
	Value *ptr_reg;
	Value *ptr_fp_reg;
	Value *ptr_PC;
	Value *ptr_RAM;
	PointerType *type_pfunc_callout;
	Value *ptr_func_debug;
	#define MAX_REGISTERS 64
	Value *ptr_r8[MAX_REGISTERS];
	Value *ptr_r16[MAX_REGISTERS];
	Value *ptr_r32[MAX_REGISTERS];
	Value *ptr_r64[MAX_REGISTERS];
	Value *ptr_f32[MAX_REGISTERS];
	Value *ptr_f64[MAX_REGISTERS];
	Value *ptr_f80[MAX_REGISTERS];
	Value *ptr_f128[MAX_REGISTERS];
	Value *in_ptr_r8[MAX_REGISTERS];
	Value *in_ptr_r16[MAX_REGISTERS];
	Value *in_ptr_r32[MAX_REGISTERS];
	Value *in_ptr_r64[MAX_REGISTERS];
	Value *in_ptr_f32[MAX_REGISTERS];
	Value *in_ptr_f64[MAX_REGISTERS];
	Value *in_ptr_f80[MAX_REGISTERS];
	Value *in_ptr_f128[MAX_REGISTERS];
} cpu_t;

enum {
	JIT_RETURN_NOERR = 0,
	JIT_RETURN_FUNCNOTFOUND,
	JIT_RETURN_SINGLESTEP,
	JIT_RETURN_TRAP
};

//////////////////////////////////////////////////////////////////////
// optimization flags
//////////////////////////////////////////////////////////////////////
#define CPU_OPTIMIZE_NONE 0x0000000000000000LL
#define CPU_OPTIMIZE_ALL  0xFFFFFFFFFFFFFFFFLL

//////////////////////////////////////////////////////////////////////
// debug flags
//////////////////////////////////////////////////////////////////////
#define CPU_DEBUG_NONE 0x00000000
#define CPU_DEBUG_SINGLESTEP			(1<<0)
#define CPU_DEBUG_PRINT_IR				(1<<1)
#define CPU_DEBUG_PRINT_IR_OPTIMIZED	(1<<2)
#define CPU_DEBUG_LOG					(1<<3)
#define CPU_DEBUG_ALL 0xFFFFFFFF

//////////////////////////////////////////////////////////////////////
// hints
//////////////////////////////////////////////////////////////////////
#define CPU_HINT_NONE 0x00000000
#define CPU_HINT_TRAP_RETURNS		(1<<0)
#define CPU_HINT_TRAP_RETURNS_TWICE	(1<<1)

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

#define LOGGING (cpu->flags_debug & CPU_DEBUG_LOG)
#define log(...) do { if (LOGGING) printf(__VA_ARGS__); } while(0)

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

/*
 * type of the debug callback; second parameter is
 * pointer to CPU specific register struct
 */
typedef void (*debug_function_t)(cpu_t*);

//////////////////////////////////////////////////////////////////////

cpu_t *cpu_new(cpu_arch_t arch);
void cpu_set_flags_optimize(cpu_t *cpu, uint64_t f);
void cpu_set_flags_hint(cpu_t *cpu, uint32_t f);
void cpu_set_flags_debug(cpu_t *cpu, uint32_t f);
void cpu_tag(cpu_t *cpu, addr_t pc);
int cpu_run(cpu_t *cpu, debug_function_t debug_function);
void cpu_translate(cpu_t *cpu);
void cpu_set_flags_arch(cpu_t *cpu, uint32_t f);
void cpu_set_ram(cpu_t *cpu, uint8_t *RAM);
void cpu_flush(cpu_t *cpu);
void cpu_init(cpu_t *cpu);

/* runs the interactive debugger */
int cpu_debugger(cpu_t *cpu, debug_function_t debug_function);

//////////////////////////////////////////////////////////////////////
// LLVM Helpers
//////////////////////////////////////////////////////////////////////

#define _CTX() getGlobalContext()
#define getType(x) (Type::get##x(_CTX()))
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
