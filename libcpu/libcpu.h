#ifndef _LIBCPU_H_
#define _LIBCPU_H_

#include "config.h"
#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "llvm/LLVMContext.h"
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
#ifdef LIBCPU_BUILD_CORE
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/LinkAllPasses.h"
#else
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#endif
#include "llvm/Config/config.h"
#include "llvm/Target/TargetSelect.h"


#include "types.h"
#include "fp_types.h"

using namespace llvm;

struct cpu;
typedef void        (*fp_init)(struct cpu *cpu, struct cpu_archinfo *info, struct cpu_archrf *rf);
typedef void        (*fp_done)(struct cpu *cpu);
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
	fp_done done;
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
	CPU_ARCH_ARM,
	CPU_ARCH_8086,
} cpu_arch_t;

enum {
	CPU_FLAG_ENDIAN_MASK   = (1 << 1),
	CPU_FLAG_ENDIAN_BIG    = (0 << 1),
	CPU_FLAG_ENDIAN_LITTLE = (1 << 1),

	CPU_FLAG_HARDWIRE_GPR0 = (1 << 2),
	CPU_FLAG_HARDWIRE_FPR0 = (1 << 3),
	CPU_FLAG_DELAY_SLOT    = (1 << 4),
	CPU_FLAG_DELAY_NULLIFY = (1 << 5),

	// internal flags.
	CPU_FLAG_FP80          = (1 << 15), // FP80 is natively supported.
	CPU_FLAG_FP128         = (1 << 16), // FP128 is natively supported.
	CPU_FLAG_SWAPMEM       = (1 << 17), // Swap load/store
};

// Four register classes
enum {
	CPU_REG_GPR, // General Purpose
	CPU_REG_FPR, // Floating Point
	CPU_REG_VR,  // Vector
	CPU_REG_XR   // Extra Registers, the core expects these to follow
	 			 // GPRs in the memory layout, they are kept separate
				 // to avoid confusing the client about the number of
				 // registers available.
};

#define TIMER_COUNT	4
#define TIMER_TAG	0
#define TIMER_FE	1
#define TIMER_BE	2
#define TIMER_RUN	3

typedef struct cpu_archinfo {
	cpu_arch_t type;
	
	char const *name;
	char const *full_name;

	uint32_t common_flags;
	uint32_t arch_flags;

	uint32_t delay_slots;

	uint32_t byte_size;
	uint32_t word_size;
	uint32_t float_size;
	uint32_t vector_size;
	uint32_t address_size;

	uint32_t min_page_size;
	uint32_t max_page_size;
	uint32_t default_page_size;

	uint32_t register_count[4];
	uint32_t register_size[4];
} cpu_archinfo_t;

typedef struct cpu_archrf {
	void *pc;  // Program Counter
	void *grf; // GP register file
	void *frf; // FP register file
	void *vrf; // Vector register file
} cpu_archrf_t;

typedef std::map<addr_t, BasicBlock *> bbaddr_map;
typedef std::map<Function *, bbaddr_map> funcbb_map;

typedef struct cpu {
	cpu_archinfo_t info;
	cpu_archrf_t rf;
	arch_func_t f;

	funcbb_map func_bb; // faster bb lookup

	uint16_t pc_offset;
	addr_t code_start;
	addr_t code_end;
	addr_t code_entry;
	uint32_t flags_codegen;
	uint32_t flags_debug;
	uint32_t flags_hint;
	uint32_t flags;
	uint8_t code_digest[20];
	FILE *file_entries;
	tag_t *tag;
	bool tags_dirty;
	Module *mod;
	ExistingModuleProvider *mp;
	void *fp[1024];
	Function *func[1024];
	Function *cur_func;
	uint32_t functions;
	ExecutionEngine *exec_engine;
	uint8_t *RAM;
	Value *ptr_PC;
	Value *ptr_RAM;
	PointerType *type_pfunc_callout;
	Value *ptr_func_debug;

	Value *ptr_grf; // gpr register file
	Value **ptr_gpr; // GPRs
	Value **in_ptr_gpr;
	Value **ptr_xr; // XRs
	Value **in_ptr_xr;

	Value *ptr_frf; // fp register file
	Value **ptr_fpr; // FPRs
	Value **in_ptr_fpr;

	uint64_t timer_total[TIMER_COUNT];
	uint64_t timer_start[TIMER_COUNT];

	void *feptr; /* This pointer can be used freely by the frontend. */
} cpu_t;

enum {
	JIT_RETURN_NOERR = 0,
	JIT_RETURN_FUNCNOTFOUND,
	JIT_RETURN_SINGLESTEP,
	JIT_RETURN_TRAP
};

//////////////////////////////////////////////////////////////////////
// codegen flags
//////////////////////////////////////////////////////////////////////
#define CPU_CODEGEN_NONE 0

// Run optimization passes on generated IR (disable for debugging)
#define CPU_CODEGEN_OPTIMIZE  (1<<1)

// Limits the DFS when tagging code, so that we don't
// translate all reachable code at a time, but only a
// certain amount of code in advance, and translate more
// on demand.
// If this is turned off, we do "entry caching", i.e. we
// create a file in /tmp that holds all entries to the code
// (i.e. all start addresses that can't be found automatically),
// and we start tagging at these addresses on load if the
// cache exists.
#define CPU_CODEGEN_TAG_LIMIT (1<<2)

//////////////////////////////////////////////////////////////////////
// debug flags
//////////////////////////////////////////////////////////////////////
#define CPU_DEBUG_NONE 0x00000000
#define CPU_DEBUG_SINGLESTEP			(1<<0)
#define CPU_DEBUG_SINGLESTEP_BB			(1<<1)
#define CPU_DEBUG_PRINT_IR				(1<<2)
#define CPU_DEBUG_PRINT_IR_OPTIMIZED	(1<<3)
#define CPU_DEBUG_LOG					(1<<4)
#define CPU_DEBUG_PROFILE				(1<<5)
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

API_FUNC cpu_t *cpu_new(cpu_arch_t arch, uint32_t flags, uint32_t arch_flags);
API_FUNC void cpu_free(cpu_t *cpu);
API_FUNC void cpu_set_flags_codegen(cpu_t *cpu, uint32_t f);
API_FUNC void cpu_set_flags_hint(cpu_t *cpu, uint32_t f);
API_FUNC void cpu_set_flags_debug(cpu_t *cpu, uint32_t f);
API_FUNC void cpu_tag(cpu_t *cpu, addr_t pc);
API_FUNC int cpu_run(cpu_t *cpu, debug_function_t debug_function);
API_FUNC void cpu_translate(cpu_t *cpu);
API_FUNC void cpu_set_ram(cpu_t *cpu, uint8_t *RAM);
API_FUNC void cpu_flush(cpu_t *cpu);
API_FUNC void cpu_print_statistics(cpu_t *cpu);

/* runs the interactive debugger */
API_FUNC int cpu_debugger(cpu_t *cpu, debug_function_t debug_function);

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
