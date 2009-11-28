/*
 libcpu
 (C)2007-2009 Michael Steil et al.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* project global headers */
#include "libcpu.h"
#include "tag.h"
#include "disasm.h"
#include "arch.h"
#include "optimize.h"

using namespace llvm;

#include "arch/6502/libcpu_6502.h"
#include "arch/m68k/libcpu_m68k.h"
#include "arch/mips/libcpu_mips.h"
#include "arch/m88k/libcpu_m88k.h"
#include "arch/arm/libcpu_arm.h"

//////////////////////////////////////////////////////////////////////
// cpu_t
//////////////////////////////////////////////////////////////////////
cpu_t *
cpu_new(cpu_arch_t arch)
{
	cpu_t *cpu;

	llvm::InitializeNativeTarget();

	cpu = (cpu_t*)malloc(sizeof(cpu_t));
	cpu->arch = arch;

	switch (arch) {
		case CPU_ARCH_6502:
			cpu->f = arch_func_6502;
			break;
		case CPU_ARCH_M68K:
			cpu->f = arch_func_m68k;
			break;
		case CPU_ARCH_MIPS:
			cpu->f = arch_func_mips;
			break;
		case CPU_ARCH_M88K:
			cpu->f = arch_func_m88k;
			break;
		case CPU_ARCH_ARM:
			cpu->f = arch_func_arm;
			break;
		default:
			printf("illegal arch: %d\n", arch);
			exit(1);
	}

	cpu->name = "noname";
	cpu->code_start = 0;
	cpu->code_end = 0;
	cpu->code_entry = 0;
	cpu->tag = NULL;

	cpu->flags_optimize = CPU_OPTIMIZE_NONE;
	cpu->flags_debug = CPU_DEBUG_NONE;
	cpu->flags_hint = CPU_HINT_NONE;
	cpu->flags = 0;
	cpu->flags_arch = 0;

	cpu->fp = NULL;
	cpu->reg = NULL;
	cpu->mod = new Module(cpu->name, _CTX());
	cpu->exec_engine = ExecutionEngine::create(cpu->mod);

	assert(cpu->exec_engine != NULL);

	//XXX there is a better way to do this?
	std::string data_layout = cpu->exec_engine->getTargetData()->getStringRepresentation();
	//printf("Target Data Layout = %s\n", data_layout.c_str());
	if (data_layout.find("f80") != std::string::npos) {
		log("INFO: FP80 supported.\n");
		cpu->flags |= CPU_FLAG_FP80;
	}
	if (data_layout.find("f128") != std::string::npos) {
		log("INFO: FP128 supported.\n");
		cpu->flags |= CPU_FLAG_FP128;
	}

//	cpu->mod->setDataLayout("e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32");
//	cpu->mod->setTargetTriple("i386-pc-linux-gnu");

	return cpu;
}

void
cpu_set_ram(cpu_t*cpu, uint8_t *r)
{
	cpu->RAM = r;
}

void
cpu_set_flags_optimize(cpu_t *cpu, uint64_t f)
{
	cpu->flags_optimize = f;
}

void
cpu_set_flags_debug(cpu_t *cpu, uint32_t f)
{
	cpu->flags_debug = f;
}

void
cpu_set_flags_hint(cpu_t *cpu, uint32_t f)
{
	cpu->flags_hint = f;
}

void
cpu_set_flags_arch(cpu_t *cpu, uint32_t f)
{
	cpu->flags_arch = f;
}

//////////////////////////////////////////////////////////////////////
// disassemble
//////////////////////////////////////////////////////////////////////
void disasm_instr(cpu_t *cpu, addr_t pc) {
	char disassembly_line1[MAX_DISASSEMBLY_LINE];
	char disassembly_line2[MAX_DISASSEMBLY_LINE];
	int bytes, i;

	bytes = cpu->f.disasm_instr(cpu, pc, disassembly_line1, sizeof(disassembly_line1));

	log(".,%04llx ", (unsigned long long)pc);
	for (i=0; i<bytes; i++) {
		log("%02X ", cpu->RAM[pc+i]);
	}
	log("%*s", (18-3*bytes)+1, ""); /* TODO make this arch neutral */

	/* delay slot */
	tag_t tag;
	addr_t dummy, dummy2;
	cpu->f.tag_instr(cpu, pc, &tag, &dummy, &dummy2);
	if (tag & TAG_DELAY_SLOT)
		bytes = cpu->f.disasm_instr(cpu, pc + bytes, disassembly_line2, sizeof(disassembly_line2));

	if (tag & TAG_DELAY_SLOT)
		log("%-23s [%s]\n", disassembly_line1, disassembly_line2);
	else
		log("%-23s\n", disassembly_line1);

}

//////////////////////////////////////////////////////////////////////
// generic code
//////////////////////////////////////////////////////////////////////
static bool
is_start_of_basicblock(cpu_t *cpu, addr_t a)
{
	return !!(get_tag(cpu, a) &
		(TAG_BRANCH_TARGET |	/* someone jumps/branches here */
		 TAG_SUBROUTINE |		/* someone calls this */
		 TAG_AFTER_CALL |		/* instruction after a call */
		 TAG_AFTER_COND |		/* instruction after a branch */
		 TAG_AFTER_TRAP |		/* instruction after a trap */
		 TAG_ENTRY));			/* client wants to enter guest code here */
}

static bool
needs_dispatch_entry(cpu_t *cpu, addr_t a)
{
	return !!(get_tag(cpu, a) &
		(TAG_ENTRY |			/* client wants to enter guest code here */
		 TAG_AFTER_CALL |		/* instruction after a call */
		 TAG_AFTER_TRAP));		/* instruction after a call */
}

enum {
	BB_TYPE_NORMAL   = 'L', /* basic block for instructions */
	BB_TYPE_COND     = 'C', /* basic block for "taken" case of cond. execution */
	BB_TYPE_DELAY    = 'D', /* basic block for delay slot in non-taken case of cond. exec. */
	BB_TYPE_EXTERNAL = 'E'  /* basic block for unknown addresses; just traps */
};

BasicBlock *
create_basicblock(cpu_t *cpu, addr_t addr, Function *f, uint8_t bb_type) {
	char label[17];
	snprintf(label, sizeof(label), "%c%08llx", bb_type, (unsigned long long)addr);
log("creating basic block %s\n", label);
	return BasicBlock::Create(_CTX(), label, f, 0);
}

//////////////////////////////////////////////////////////////////////
// translate one instruction
//////////////////////////////////////////////////////////////////////
void
emit_store_pc(cpu_t *cpu, BasicBlock *bb_branch, addr_t new_pc)
{
	Value *v_pc = ConstantInt::get(getIntegerType(cpu->pc_width), new_pc);
	new StoreInst(v_pc, cpu->ptr_PC, bb_branch);
}

void
emit_store_pc_return(cpu_t *cpu, BasicBlock *bb_branch, addr_t new_pc, BasicBlock *bb_ret)
{
	emit_store_pc(cpu, bb_branch, new_pc);
	BranchInst::Create(bb_ret, bb_branch);
}

/*
 * returns the basic block where code execution continues, or
 * NULL if the instruction always branches away
 * (The caller needs this to link the basic block)
 */
static BasicBlock *
translate_instr(cpu_t *cpu, addr_t pc, tag_t tag,
	BasicBlock *bb_target,
	BasicBlock *bb_next,
	BasicBlock *bb_trap,
	BasicBlock *cur_bb)
{
	BasicBlock *bb_cond;
	BasicBlock *bb_delay;

	/* create internal basic blocks if needed */
	if (tag & TAG_CONDITIONAL)
		bb_cond = create_basicblock(cpu, pc, cpu->func_jitmain, BB_TYPE_COND);
	if ((tag & TAG_DELAY_SLOT) && (tag & TAG_CONDITIONAL))
		bb_delay = create_basicblock(cpu, pc, cpu->func_jitmain, BB_TYPE_DELAY);

	/* special case: delay slot */
	if (tag & TAG_DELAY_SLOT) {
		if (tag & TAG_CONDITIONAL) {
			addr_t delay_pc;
			// cur_bb:  if (cond) goto b_cond; else goto bb_delay;
			Value *c = cpu->f.translate_cond(cpu, pc, cur_bb);
			BranchInst::Create(bb_cond, bb_delay, c, cur_bb);
			// bb_cond: instr; delay; goto bb_target;
			pc += cpu->f.translate_instr(cpu, pc, bb_cond);
			delay_pc = pc;
			cpu->f.translate_instr(cpu, pc, bb_cond);
			BranchInst::Create(bb_target, bb_cond);
			// bb_cond: delay; goto bb_next;
			cpu->f.translate_instr(cpu, delay_pc, bb_delay);
			BranchInst::Create(bb_next, bb_delay);
		} else {
			// cur_bb:  instr; delay; goto bb_target;
			pc += cpu->f.translate_instr(cpu, pc, cur_bb);
			cpu->f.translate_instr(cpu, pc, cur_bb);
			BranchInst::Create(bb_target, cur_bb);
		}
		return NULL; /* don't link */
	}

	/* no delay slot */
	if (tag & TAG_CONDITIONAL) {
		// cur_bb:  if (cond) goto b_cond; else goto bb_next;
		addr_t delay_pc;
		Value *c = cpu->f.translate_cond(cpu, pc, cur_bb);
		BranchInst::Create(bb_cond, bb_next, c, cur_bb);
		cur_bb = bb_cond;
	}

	cpu->f.translate_instr(cpu, pc, cur_bb);

	if (tag & (TAG_BRANCH | TAG_CALL | TAG_RET))
		BranchInst::Create(bb_target, cur_bb);
	else if (tag & TAG_TRAP)
		BranchInst::Create(bb_trap, cur_bb);

	if (tag & TAG_CONTINUE)
		return cur_bb;
	else
		return NULL;
}

//////////////////////////////////////////////////////////////////////
// buik translate
//////////////////////////////////////////////////////////////////////
static const BasicBlock *
lookup_basicblock(cpu_t *cpu, Function* f, addr_t pc, uint8_t bb_type) {
	Function::const_iterator it;
	for (it = f->getBasicBlockList().begin(); it != f->getBasicBlockList().end(); it++) {
		const char *cstr = (*it).getNameStr().c_str();
		if (cstr[0] == bb_type) {
			addr_t pc2 = strtol(cstr + 1, (char **)NULL, 16);
			if (pc == pc2)
				return it;
		}
	}
	log("error: basic block %c%08llx not found!\n", bb_type, pc);
	return NULL;
}

static BasicBlock *
cpu_translate(cpu_t *cpu, BasicBlock *bb_ret, BasicBlock *bb_trap)
{
	// find all instructions that need labels and create basic blocks for them
	int bbs = 0;
	addr_t pc;
	pc = cpu->code_start;
	while (pc < cpu->code_end) {
		//log("%04X: %d\n", pc, get_tag(cpu, pc));
		if (is_start_of_basicblock(cpu, pc)) {
			create_basicblock(cpu, pc, cpu->func_jitmain, BB_TYPE_NORMAL);
			bbs++;
		}
		pc++;
	}
	log("bbs: %d\n", bbs);

	// create dispatch basicblock
	BasicBlock* bb_dispatch = BasicBlock::Create(_CTX(), "dispatch", cpu->func_jitmain, 0);
	Value *v_pc = new LoadInst(cpu->ptr_PC, "", false, bb_dispatch);
	SwitchInst* sw = SwitchInst::Create(v_pc, bb_ret, bbs /*XXX upper bound, not accurate count!*/, bb_dispatch);

	for (pc = cpu->code_start; pc<cpu->code_end; pc++) {
		if (needs_dispatch_entry(cpu, pc)) {
			log("info: adding case: %llx\n", pc);
			ConstantInt* c = ConstantInt::get(getIntegerType(cpu->pc_width), pc);
			BasicBlock *target = (BasicBlock*)lookup_basicblock(cpu, cpu->func_jitmain, pc, BB_TYPE_NORMAL);
			if (!target) {
				log("error: unknown rts target $%04llx!\n", (unsigned long long)pc);
				exit(1);
			} else {
				sw->addCase(c, target);
			}
		}
	}

// translate basic blocks
    Function::const_iterator it;
    for (it = cpu->func_jitmain->getBasicBlockList().begin(); it != cpu->func_jitmain->getBasicBlockList().end(); it++) {
		const BasicBlock *hack = it;
		BasicBlock *cur_bb = (BasicBlock*)hack;	/* cast away const */
		const char *cstr = (*it).getNameStr().c_str();
		if (cstr[0] != BB_TYPE_NORMAL)
			continue; // skip special blocks like entry, dispatch...
		pc = strtol(cstr+1, (char **)NULL, 16);
log("basicblock: L%08llx\n", (unsigned long long)pc);
		tag_t tag;
		BasicBlock *bb_target = NULL, *bb_next = NULL, *bb_delay = NULL, *bb_cont = NULL;
		do {
			tag_t dummy1;
			addr_t dummy2;

			if (LOGGING)
				disasm_instr(cpu, pc);

			tag = get_tag(cpu, pc);

			/* get address of the following instruction */
			addr_t new_pc, next_pc;
			cpu->f.tag_instr(cpu, pc, &dummy1, &new_pc, &next_pc);

			/* get target basic block */
			if (tag & TAG_RET)
				bb_target = bb_dispatch;
			if (tag & (TAG_CALL|TAG_BRANCH)) {
				if (new_pc == NEW_PC_NONE) { /* translate_instr() will set PC */
					bb_target = bb_dispatch;
				} else {
					bb_target = (BasicBlock*)lookup_basicblock(cpu, cpu->func_jitmain, new_pc, BB_TYPE_NORMAL);
					if (!bb_target) { /* outside of code segment */
						bb_target = create_basicblock(cpu, new_pc, cpu->func_jitmain, BB_TYPE_EXTERNAL);
						emit_store_pc_return(cpu, bb_target, new_pc, bb_ret);
					}
				}
			}
			/* get not-taken basic block */
			if (tag & TAG_CONDITIONAL)
 				bb_next = (BasicBlock*)lookup_basicblock(cpu, cpu->func_jitmain, next_pc, BB_TYPE_NORMAL);

			bb_cont = translate_instr(cpu, pc, tag, bb_target, bb_next, bb_trap, cur_bb);

			pc = next_pc;
			
		} while (!(is_start_of_basicblock(cpu, pc)) && /* new basic block starts here */
				is_code(cpu, pc) && /* end of code section */
				bb_cont); /* last intruction jumped away */

		/* link with next basic block if there isn't a control flow instr. already */
		if (bb_cont) {
			BasicBlock *target = (BasicBlock*)lookup_basicblock(cpu, cpu->func_jitmain, pc, BB_TYPE_NORMAL);
			if (!target) {
				printf("error: unknown continue $%04llx!\n", (unsigned long long)pc);
				exit(1);
			}
			log("info: linking continue $%04llx!\n", (unsigned long long)pc);
			BranchInst::Create(target, bb_cont);
		}
    }

	return bb_dispatch;
}

//////////////////////////////////////////////////////////////////////
// single stepping
//////////////////////////////////////////////////////////////////////
BasicBlock *
create_singlestep_return_basicblock(cpu_t *cpu, addr_t new_pc, BasicBlock *bb_ret)
{
	BasicBlock *bb_branch = create_basicblock(cpu, new_pc, cpu->func_jitmain, BB_TYPE_NORMAL);
	emit_store_pc_return(cpu, bb_branch, new_pc, bb_ret);
	return bb_branch;
}

static BasicBlock *
cpu_translate_singlestep(cpu_t *cpu, BasicBlock *bb_ret, BasicBlock *bb_trap)
{
	addr_t new_pc;
	tag_t tag;
	BasicBlock *cur_bb = NULL, *bb_target = NULL, *bb_next = NULL, *bb_cont = NULL;
	addr_t next_pc, pc = cpu->f.get_pc(cpu, cpu->reg);
	tag_t dummy1;
	addr_t dummy2;

	cur_bb = BasicBlock::Create(_CTX(), "instruction", cpu->func_jitmain, 0);

	if (LOGGING)
		disasm_instr(cpu, pc);

	cpu->f.tag_instr(cpu, pc, &tag, &new_pc, &next_pc);

	/* get target basic block */
	if ((tag & TAG_RET) || (new_pc == NEW_PC_NONE)) /* translate_instr() will set PC */
		bb_target = bb_ret;
	else if (tag & (TAG_CALL|TAG_BRANCH))
		bb_target = create_singlestep_return_basicblock(cpu, new_pc, bb_ret);
	/* get not-taken & conditional basic block */
	if (tag & TAG_CONDITIONAL)
		bb_next = create_singlestep_return_basicblock(cpu, next_pc, bb_ret);

	bb_cont = translate_instr(cpu, pc, tag, bb_target, bb_next, bb_trap, cur_bb);

	/* If it's not a branch, append "store PC & return" to basic block */
	if (bb_cont)
		emit_store_pc_return(cpu, bb_cont, next_pc, bb_ret);

	return cur_bb;
}

//////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////
static StructType *
get_struct_reg(cpu_t *cpu) {
	std::vector<const Type*>type_struct_reg_t_fields;

	for (uint32_t i = 0; i < cpu->count_regs_i8; i++) /* 8 bit registers */
		type_struct_reg_t_fields.push_back(getIntegerType(8));
	for (uint32_t i = 0; i < cpu->count_regs_i16; i++) /* 16 bit registers */
		type_struct_reg_t_fields.push_back(getIntegerType(16));
	for (uint32_t i = 0; i < cpu->count_regs_i32; i++) /* 32 bit registers */
		type_struct_reg_t_fields.push_back(getIntegerType(32));
	for (uint32_t i = 0; i < cpu->count_regs_i64; i++) /* 64 bit registers */
		type_struct_reg_t_fields.push_back(getIntegerType(64));

	type_struct_reg_t_fields.push_back(getIntegerType(cpu->pc_width)); /* PC */

	return getStructType(type_struct_reg_t_fields, /*isPacked=*/true);
}

static StructType *
get_struct_fp_reg(cpu_t *cpu) {
	std::vector<const Type*>type_struct_fp_reg_t_fields;

	for (uint32_t i = 0; i < cpu->count_regs_f32; i++) /* 32 bit registers */
		type_struct_fp_reg_t_fields.push_back(getFloatType(32));
	for (uint32_t i = 0; i < cpu->count_regs_f64; i++) /* 64 bit registers */
		type_struct_fp_reg_t_fields.push_back(getFloatType(64));
	for (uint32_t i = 0; i < cpu->count_regs_f80; i++) { /* 80 bit registers */
		if (cpu->flags & CPU_FLAG_FP80)
			type_struct_fp_reg_t_fields.push_back(getFloatType(80));
		else {
			/* two 64bits words hold the data */
			type_struct_fp_reg_t_fields.push_back(getIntegerType(64));
			type_struct_fp_reg_t_fields.push_back(getIntegerType(64));
		}
	}
	for (uint32_t i = 0; i < cpu->count_regs_f128; i++) { /* 128 bit registers */
		if (cpu->flags & CPU_FLAG_FP128)
			type_struct_fp_reg_t_fields.push_back(getFloatType(128));
		else {
			/* two 64bits words hold the data */
			type_struct_fp_reg_t_fields.push_back(getIntegerType(64));
			type_struct_fp_reg_t_fields.push_back(getIntegerType(64));
		}
	}

	return getStructType(type_struct_fp_reg_t_fields, /*isPacked=*/true);
}

static Function*
cpu_create_function(cpu_t *cpu, const char *name)
{
	Function *func;

	// Type Definitions
	// - struct reg
	StructType *type_struct_reg_t = get_struct_reg(cpu);
	cpu->mod->addTypeName("struct.reg_t", type_struct_reg_t);
	// - struct reg *
	PointerType *type_pstruct_reg_t = PointerType::get(type_struct_reg_t, 0);
	// - struct fp_reg
	StructType *type_struct_fp_reg_t = get_struct_fp_reg(cpu);
	cpu->mod->addTypeName("struct.fp_reg_t", type_struct_fp_reg_t);
	// - struct fp_reg *
	PointerType *type_pstruct_fp_reg_t = PointerType::get(type_struct_fp_reg_t, 0);
	// - uint8_t *
	PointerType *type_pi8 = PointerType::get(getIntegerType(8), 0);
	// - intptr *
	PointerType *type_intptr = PointerType::get(cpu->exec_engine->getTargetData()->getIntPtrType(_CTX()), 0);
	// - (*f)(cpu_t *) [debug_function() function pointer]
	std::vector<const Type*>type_func_callout_args;
	type_func_callout_args.push_back(type_intptr);	/* intptr *cpu */
	FunctionType *type_func_callout = FunctionType::get(
		getType(VoidTy),	/* Result */
		type_func_callout_args,	/* Params */
		false);		      	/* isVarArg */
	cpu->type_pfunc_callout = PointerType::get(type_func_callout, 0);

	// - (*f)(uint8_t *, reg_t *, fp_reg_t *, (*)(...)) [jitmain() function pointer)
	std::vector<const Type*>type_func_args;
	type_func_args.push_back(type_pi8);				/* uint8_t *RAM */
	type_func_args.push_back(type_pstruct_reg_t);	/* reg_t *reg */
	type_func_args.push_back(type_pstruct_fp_reg_t);	/* fp_reg_t *fp_reg */
	type_func_args.push_back(cpu->type_pfunc_callout);	/* (*debug)(...) */
	FunctionType* type_func = FunctionType::get(
		getIntegerType(32),		/* Result */
		type_func_args,		/* Params */
		false);						/* isVarArg */

	// Function Declarations
	func = Function::Create(
		type_func,				/* Type */
		GlobalValue::ExternalLinkage,	/* Linkage */
		name, cpu->mod);				/* Name */
	func->setCallingConv(CallingConv::C);
	AttrListPtr func_PAL;
	{
		SmallVector<AttributeWithIndex, 4> Attrs;
		AttributeWithIndex PAWI;
		PAWI.Index = 1U; PAWI.Attrs = 0  | Attribute::NoCapture;
		Attrs.push_back(PAWI);
		PAWI.Index = 4294967295U; PAWI.Attrs = 0  | Attribute::NoUnwind;
		Attrs.push_back(PAWI);
		func_PAL = AttrListPtr::get(Attrs.begin(), Attrs.end());
	}
	func->setAttributes(func_PAL);

	return func;
}

//////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////
static Value *
get_struct_member_pointer(Value *s, int index, BasicBlock *bb) {
	ConstantInt* const_0 = ConstantInt::get(getType(Int32Ty), 0);
	ConstantInt* const_index = ConstantInt::get(getType(Int32Ty), index);

	SmallVector<Value*, 2> ptr_11_indices;
	ptr_11_indices.push_back(const_0);
	ptr_11_indices.push_back(const_index);
	return (Value*) GetElementPtrInst::Create(s, ptr_11_indices.begin(), ptr_11_indices.end(), "", bb);
}

static void
emit_decode_reg_helper(cpu_t *cpu, int count, int width, Value **in_ptr_r, Value **ptr_r, BasicBlock *bb) {
#ifdef OPT_LOCAL_REGISTERS
	// decode struct reg and copy the registers into local variables
	for (int i = 0; i < count; i++) {
		char reg_name[16];
		snprintf(reg_name, sizeof(reg_name), "gpr_%u", i);
		in_ptr_r[i] = get_struct_member_pointer(cpu->ptr_reg, i, bb);
		ptr_r[i] = new AllocaInst(getIntegerType(width), reg_name, bb);
		LoadInst* v = new LoadInst(in_ptr_r[i], "", false, bb);
		new StoreInst(v, ptr_r[i], false, bb);
	}
#else
	// just decode struct reg
	for (int i = 0; i < count; i++) 
		ptr_r[i] = get_struct_member_pointer(cpu->ptr_reg, i, bb);
#endif
}

static inline unsigned
fp_alignment(unsigned width) {
	return ((width == 80 ? 128 : width) >> 3);
}

static void
emit_decode_fp_reg_helper(cpu_t *cpu, int count, int width, Value **in_ptr_r,
	Value **ptr_r, BasicBlock *bb)
{
#ifdef OPT_LOCAL_REGISTERS
	// decode struct reg and copy the registers into local variables
	for (int i = 0; i < count; i++) {
		char reg_name[16];
		if ((width == 80 && (cpu->flags & CPU_FLAG_FP80) == 0) ||
			(width == 128 && (cpu->flags & CPU_FLAG_FP128) == 0)) {
			snprintf(reg_name, sizeof(reg_name), "fpr_%u_0", i);

			in_ptr_r[i*2+0] = get_struct_member_pointer(cpu->ptr_fp_reg, i*2+0, bb);
			ptr_r[i*2+0] = new AllocaInst(getIntegerType(64), 0, 0, reg_name, bb);
			LoadInst* v = new LoadInst(in_ptr_r[i*2+0], "", false, 0, bb);
			new StoreInst(v, ptr_r[i*2+0], false, 0, bb);

			snprintf(reg_name, sizeof(reg_name), "fpr_%u_1", i);

			in_ptr_r[i*2+1] = get_struct_member_pointer(cpu->ptr_fp_reg, i*2+1, bb);
			ptr_r[i*2+1] = new AllocaInst(getIntegerType(64), 0, 0, reg_name, bb);
			v = new LoadInst(in_ptr_r[i*2+1], "", false, 0, bb);
			new StoreInst(v, ptr_r[i*2+1], false, 0, bb);
		} else {
			snprintf(reg_name, sizeof(reg_name), "fpr_%u", i);
			in_ptr_r[i] = get_struct_member_pointer(cpu->ptr_fp_reg, i, bb);
			ptr_r[i] = new AllocaInst(getFloatType(width), 0, fp_alignment(width), reg_name, bb);
			LoadInst* v = new LoadInst(in_ptr_r[i], "", false, fp_alignment(width), bb);
			new StoreInst(v, ptr_r[i], false, fp_alignment(width), bb);
		}
	}
#else
	// just decode struct reg
	for (int i = 0; i < count; i++) 
		ptr_r[i] = get_struct_member_pointer(cpu->ptr_fp_reg, i, bb);
#endif
}

static void
emit_decode_reg(cpu_t *cpu, BasicBlock *bb)
{
	emit_decode_reg_helper(cpu, cpu->count_regs_i8,   8, cpu->in_ptr_r8,  cpu->ptr_r8,  bb);
	emit_decode_reg_helper(cpu, cpu->count_regs_i16, 16, cpu->in_ptr_r16, cpu->ptr_r16, bb);
	emit_decode_reg_helper(cpu, cpu->count_regs_i32, 32, cpu->in_ptr_r32, cpu->ptr_r32, bb);
	emit_decode_reg_helper(cpu, cpu->count_regs_i64, 64, cpu->in_ptr_r64, cpu->ptr_r64, bb);

	emit_decode_fp_reg_helper(cpu, cpu->count_regs_f32, 32, cpu->in_ptr_f32, cpu->ptr_f32, bb);
	emit_decode_fp_reg_helper(cpu, cpu->count_regs_f64, 64, cpu->in_ptr_f64, cpu->ptr_f64, bb);
	emit_decode_fp_reg_helper(cpu, cpu->count_regs_f80, 80, cpu->in_ptr_f80, cpu->ptr_f80, bb);
	emit_decode_fp_reg_helper(cpu, cpu->count_regs_f128, 128, cpu->in_ptr_f128, cpu->ptr_f128, bb);

	uint32_t pc_index = 
		cpu->count_regs_i8 +
		cpu->count_regs_i16+
		cpu->count_regs_i32+
		cpu->count_regs_i64;
	cpu->ptr_PC = get_struct_member_pointer(cpu->ptr_reg, pc_index, bb);

	if (cpu->f.emit_decode_reg) /* cpu specific part */
		cpu->f.emit_decode_reg(cpu, bb);
}

static void
spill_reg_state_helper(int count, Value **in_ptr_r, Value **ptr_r, BasicBlock *bb)
{
#ifdef OPT_LOCAL_REGISTERS
	for (int i=0; i<count; i++) {
		LoadInst* v = new LoadInst(ptr_r[i], "", false, bb);
		new StoreInst(v, in_ptr_r[i], false, bb);
	}
#endif
}

static void
spill_fp_reg_state_helper(cpu_t *cpu, int count, int width, Value **in_ptr_r,
	Value **ptr_r, BasicBlock *bb)
{
#ifdef OPT_LOCAL_REGISTERS
	for (int i=0; i<count; i++) {
		if ((width == 80 && (cpu->flags & CPU_FLAG_FP80) == 0) ||
			(width == 128 && (cpu->flags & CPU_FLAG_FP128) == 0)) {
			LoadInst* v = new LoadInst(ptr_r[i*2+0], "", false, 0, bb);
			new StoreInst(v, in_ptr_r[i*2+0], false, 0, bb);

			v = new LoadInst(ptr_r[i*2+1], "", false, 0, bb);
			new StoreInst(v, in_ptr_r[i*2+1], false, 0, bb);
		} else {
			LoadInst* v = new LoadInst(ptr_r[i], "", false, fp_alignment(width), bb);
			new StoreInst(v, in_ptr_r[i], false, fp_alignment(width), bb);
		}
	}
#endif
}

static void
spill_reg_state(cpu_t *cpu, BasicBlock *bb)
{
	if (cpu->f.spill_reg_state) /* cpu specific part */
		cpu->f.spill_reg_state(cpu, bb);

	spill_reg_state_helper(cpu->count_regs_i8,  cpu->in_ptr_r8,  cpu->ptr_r8,  bb);
	spill_reg_state_helper(cpu->count_regs_i16, cpu->in_ptr_r16, cpu->ptr_r16, bb);
	spill_reg_state_helper(cpu->count_regs_i32, cpu->in_ptr_r32, cpu->ptr_r32, bb);
	spill_reg_state_helper(cpu->count_regs_i64, cpu->in_ptr_r64, cpu->ptr_r64, bb);

	spill_fp_reg_state_helper(cpu, cpu->count_regs_f32, 32, cpu->in_ptr_f32, cpu->ptr_f32, bb);
	spill_fp_reg_state_helper(cpu, cpu->count_regs_f64, 64, cpu->in_ptr_f64, cpu->ptr_f64, bb);
	spill_fp_reg_state_helper(cpu, cpu->count_regs_f80, 80, cpu->in_ptr_f80, cpu->ptr_f80, bb);
	spill_fp_reg_state_helper(cpu, cpu->count_regs_f128, 128, cpu->in_ptr_f128, cpu->ptr_f128, bb);
}

static void
cpu_translate_function(cpu_t *cpu)
{
	cpu->func_jitmain = cpu_create_function(cpu, "jitmain");

	// args
	Function::arg_iterator args = cpu->func_jitmain->arg_begin();
	cpu->ptr_RAM = args++;
	cpu->ptr_RAM->setName("RAM");
	cpu->ptr_reg = args++;
	cpu->ptr_reg->setName("reg");	
	cpu->ptr_fp_reg = args++;
	cpu->ptr_fp_reg->setName("fp_reg");	
	cpu->ptr_func_debug = args++;
	cpu->ptr_func_debug->setName("debug");

	// entry basicblock
	BasicBlock *label_entry = BasicBlock::Create(_CTX(), "entry", cpu->func_jitmain, 0);
	emit_decode_reg(cpu, label_entry);

	// create exit code
	Value *exit_code = new AllocaInst(getIntegerType(32), "exit_code", label_entry);
	// assume JIT_RETURN_FUNCNOTFOUND or JIT_RETURN_SINGLESTEP if in in single step.
	new StoreInst(ConstantInt::get(getType(Int32Ty),
					(cpu->flags_debug & CPU_DEBUG_SINGLESTEP) ? JIT_RETURN_SINGLESTEP :
					JIT_RETURN_FUNCNOTFOUND), exit_code, false, 0, label_entry);

#if 0 // bad for debugging, minimal speedup
	/* make the RAM pointer a constant */
	PointerType* type_pi8 = PointerType::get(IntegerType::get(8), 0);
	cpu->ptr_RAM = ConstantExpr::getCast(Instruction::IntToPtr, ConstantInt::get(Type::Int64Ty, (uint64_t)(long)cpu->RAM), type_pi8);
#endif

	// create ret basicblock
	BasicBlock *bb_ret = BasicBlock::Create(_CTX(), "ret", cpu->func_jitmain, 0);  
	spill_reg_state(cpu, bb_ret);
	ReturnInst::Create(_CTX(), new LoadInst(exit_code, "", false, 0, bb_ret), bb_ret);
	// create trap return basicblock
	BasicBlock *bb_trap = BasicBlock::Create(_CTX(), "trap", cpu->func_jitmain, 0);  
	new StoreInst(ConstantInt::get(getType(Int32Ty), JIT_RETURN_TRAP), exit_code, false, 0, bb_trap);
	// return
	BranchInst::Create(bb_ret, bb_trap);

	BasicBlock *bb_start;
	if (cpu->flags_debug & CPU_DEBUG_SINGLESTEP) {
		bb_start = cpu_translate_singlestep(cpu, bb_ret, bb_trap);
	} else {
		bb_start = cpu_translate(cpu, bb_ret, bb_trap);
	}

	// entry basicblock
	BranchInst::Create(bb_start, label_entry);

	// make sure everything is OK
	verifyModule(*cpu->mod, PrintMessageAction);

	if (cpu->flags_debug & CPU_DEBUG_PRINT_IR)
		cpu->mod->dump();

	if (cpu->flags_optimize != CPU_OPTIMIZE_NONE) {
		log("*** Optimizing...");
		optimize(cpu);
		log("done.\n");
		if (cpu->flags_debug & CPU_DEBUG_PRINT_IR_OPTIMIZED)
			cpu->mod->dump();
	}

	log("*** Recompiling...");
	cpu->fp = cpu->exec_engine->getPointerToFunction(cpu->func_jitmain);
	log("done.\n");
}

void
cpu_init(cpu_t *cpu)
{
	cpu->f.init(cpu);
}

/* forces ahead of time translation (e.g. for benchmarking the run) */
void
cpu_translate(cpu_t *cpu)
{
	/* lazy init of frontend */
	if (!cpu->reg)
		cpu_init(cpu);

	/* on demand recompilation */
	if (!cpu->fp)
		cpu_translate_function(cpu);
}

int
cpu_run(cpu_t *cpu, debug_function_t debug_function)
{
	cpu_translate(cpu);

	/* run it ! */
	typedef int (*fp_t)(uint8_t *RAM, void *reg, void *fp_reg, debug_function_t fp);
	fp_t FP = (fp_t)cpu->fp;

	return FP(cpu->RAM, cpu->reg, cpu->fp_reg, debug_function);
}

void
cpu_flush(cpu_t *cpu)
{
	cpu->exec_engine->freeMachineCodeForFunction(cpu->func_jitmain);
	cpu->func_jitmain->eraseFromParent();

	cpu->fp = 0;

//	delete cpu->mod;
//	cpu->mod = NULL;
}
//printf("%s:%d\n", __func__, __LINE__);
