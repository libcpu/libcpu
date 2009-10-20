#define OPT_LOCAL_REGISTERS

#include "libcpu.h"
#include "CPUDisassembler.h"
#include "arch/mips/libcpu_mips.h"

using namespace llvm;
extern Function* func_jitmain;
extern Value *ptr_reg;
extern Value* ptr_RAM;
extern PointerType* type_pfunc_callout;
extern Value *ptr_func_debug;

extern Value *get_struct_member_pointer(Value *s, int index, BasicBlock *bb);
extern const BasicBlock *lookup_basicblock(Function* f, addr_t pc);

extern Value* ptr_PC;

#ifdef OPT_LOCAL_REGISTERS
/*
 * These are the arguments to the JITted function;
 * we read them into ptr_r[] on entry and write
 * them back on exit
 */
Value *in_ptr_r[32];
#endif

bool is_64bit;
bool is_little_endian;

#define GetSA	((instr >> 6) & 0x1F)
#define GetRS	((instr >> 21) & 0x1F)
#define GetRT	((instr >> 16) & 0x1F)
#define GetRD	((instr >> 11) & 0x1F)

#define GetFD	((U8)(BITS(instr.All, 6, 5)))
#define GetFS	((U8)(BITS(instr.All, 11, 5)))
#define GetFT	((U8)(BITS(instr.All, 16, 5)))

#define GetOpcode (instr.op)
#define GetFunction (instr & 0x3F)
#define GetSpecialInstruction GetFunction
#define GetRegimmInstruction GetRT
#define GetFMT GetRS
#define GetCacheType (GetRT&BitM2)
#define GetCacheInstr ((GetRT>>2)&BitM3)
#define GetCOP1FloatInstruction GetFunction

#define GetImmediate (instr & 0xFFFF)

#define GetTarget (instr & 0x3FFFFFF)
#define MIPS_BRANCH_TARGET ((uint32_t)(pc + 4 + (uint32_t)(((sint32_t)(sint16_t)GetImmediate<<2))))

#if 0
typedef union {
	uint32_t All;
	struct {
		unsigned immediate : 16;
		unsigned rt : 5;
		unsigned rs : 5;
		unsigned op : 6;
	};
	
	struct {
		unsigned target : 26;
		unsigned op2: 6;
	};

	struct {
		unsigned funct : 6;
		unsigned sa : 5;
		unsigned rd : 5;
		unsigned rt3: 5;
		unsigned rs3: 5;
		unsigned op3: 6;
	};

	struct {
		unsigned offset  : 7;
		unsigned element : 4;
		unsigned wc2op   : 5;
		unsigned vt      : 5;
		unsigned base    : 5;
		unsigned op      : 6;
	} RSPWC2;

	struct {
		unsigned vectop  : 6;
		unsigned vd      : 5;
		unsigned vs      : 5;
		unsigned vt      : 5;
		unsigned e       : 4;
		unsigned gap     : 1;
		unsigned op      : 6;
	} RSPVECTOR;
} TOpcode;
#endif

//globals
Value* ptr_r[32];
//Value* ptr_PC; //XXX overlap with 6502
int arch_mips_recompile_instr(uint8_t* RAM, addr_t pc, BasicBlock *bb_dispatch, BasicBlock *bb);

void
arch_mips_init(cpu_t *cpu)
{
	is_64bit = !!(cpu->flags_arch & CPU_MIPS_IS_64BIT);
	is_little_endian = !!(cpu->flags_arch & CPU_MIPS_IS_LE);

	if (is_64bit) {
		reg_mips64_t *reg;
		reg = (reg_mips64_t*)malloc(sizeof(reg_mips64_t));
		for (int i=0; i<32; i++) 
			reg->r[i] = 0;
		reg->pc = 0;
		cpu->reg = reg;
		cpu->pc_width = 64;
	} else {
		reg_mips32_t *reg;
		reg = (reg_mips32_t*)malloc(sizeof(reg_mips32_t));
		for (int i=0; i<32; i++) 
			reg->r[i] = 0;
		reg->pc = 0;
		cpu->reg = reg;
		cpu->pc_width = 32;
	}

	printf("%d bit MIPS initialized.\n", is_64bit?64:32);
}

StructType *arch_mips_get_struct_reg(void) {
	/* struct reg_mips_t */
	std::vector<const Type*>type_struct_reg_t_fields;

	// 32 GPRs
	for (int i = 0; i < 32; i++)
		type_struct_reg_t_fields.push_back(IntegerType::get(is_64bit? 64:32));
	// PC
	type_struct_reg_t_fields.push_back(IntegerType::get(is_64bit? 64:32));

	return StructType::get(type_struct_reg_t_fields, /*isPacked=*/false);
}

void arch_mips_emit_decode_reg(BasicBlock *bb) {
#ifdef OPT_LOCAL_REGISTERS
	// decode struct reg and copy the registers into local variables
	for (int i=0; i<32; i++) {
		in_ptr_r[i] = get_struct_member_pointer(ptr_reg, i, bb);
		ptr_r[i] = new AllocaInst(IntegerType::get(is_64bit?64:32), "", bb);
		LoadInst* v = new LoadInst(in_ptr_r[i], "", false, bb);
		new StoreInst(v, ptr_r[i], false, bb);
	}
#else
	// decode struct reg
	for (int i=0; i<32; i++) 
		ptr_r[i] = get_struct_member_pointer(ptr_reg, i, bb);
#endif
	ptr_PC = get_struct_member_pointer(ptr_reg, 32, bb);
}

void
arch_mips_spill_reg_state(BasicBlock *bb)
{
#ifdef OPT_LOCAL_REGISTERS
	for (int i=0; i<32; i++) {
		LoadInst* v = new LoadInst(ptr_r[i], "", false, bb);
		new StoreInst(v, in_ptr_r[i], false, bb);
	}
#endif
}

addr_t
arch_mips_get_pc(void *reg)
{
	if (is_64bit)
		return ((reg_mips64_t*)reg)->pc;
	else
		return ((reg_mips32_t*)reg)->pc;
}

static uint32_t
RAM32BE(uint8_t *RAM, addr_t a) {
	uint32_t v;
	v  = RAM[a+0] << 24;
	v |= RAM[a+1] << 16;
	v |= RAM[a+2] << 8;
	v |= RAM[a+3] << 0;
	return v;
}

#include "tag_generic.h"
int arch_mips_tag_instr(uint8_t* RAM, addr_t pc, int *flow_type, addr_t *new_pc) {
	int bytes = 4;

	uint32_t instr = RAM32BE(RAM, pc);

	switch(instr >> 26) {
		case 0x00: //INCPU_SPECIAL
			switch(instr & 0x3F) {
				case 0x08: //INCPUS_JR
					bytes = 8;
					*flow_type = FLOW_TYPE_RET;
					break;
				case 0x01: //IN_invalid
				case 0x05: //IN_invalid
				case 0x0A: //IN_invalid
				case 0x0B: //IN_invalid
				case 0x0C: //INCPUS_SYSCALL
				case 0x0D: //INCPUS_BREAK
				case 0x0E: //IN_invalid
				case 0x15: //IN_invalid
				case 0x28: //IN_invalid
				case 0x29: //IN_invalid
				case 0x35: //IN_invalid
				case 0x37: //IN_invalid
				case 0x39: //IN_invalid
				case 0x3D: //IN_invalid
					*flow_type = FLOW_TYPE_ERR;
					break;
				default:
					*flow_type = FLOW_TYPE_CONTINUE;
					break;
			}
			break;
		case 0x01: //INCPU_REGIMM
			switch (GetRegimmInstruction) {
				case 0x00: //INCPUR_BLTZ
				case 0x01: //INCPUR_BGEZ
					*new_pc = MIPS_BRANCH_TARGET;
					bytes = 8; // !likely
					*flow_type = FLOW_TYPE_BRANCH;
					break;
				case 0x10: //INCPUR_BLTZAL
				case 0x11: //INCPUR_BGEZAL
					*new_pc = MIPS_BRANCH_TARGET;
					bytes = 8; // !likely
					*flow_type = FLOW_TYPE_CALL;
					break;
				case 0x02: //INCPUR_BLTZL
				case 0x03: //INCPUR_BGEZL
					*new_pc = MIPS_BRANCH_TARGET;
					bytes = 8; // likely
					*flow_type = FLOW_TYPE_BRANCH;
					break;
				case 0x12: //INCPUR_BLTZALL
				case 0x13: //INCPUR_BGEZALL
					*new_pc = MIPS_BRANCH_TARGET;
					bytes = 8; // likely
					*flow_type = FLOW_TYPE_CALL;
					break;
				case 0x04: //IN_invalid
				case 0x05: //IN_invalid
				case 0x06: //IN_invalid
				case 0x07: //IN_invalid
				case 0x0D: //IN_invalid
				case 0x0F: //IN_invalid
				case 0x14: //IN_invalid
				case 0x15: //IN_invalid
				case 0x16: //IN_invalid
				case 0x17: //IN_invalid
				case 0x18: //IN_invalid
				case 0x19: //IN_invalid
				case 0x1A: //IN_invalid
				case 0x1B: //IN_invalid
				case 0x1C: //IN_invalid
				case 0x1D: //IN_invalid
				case 0x1E: //IN_invalid
				case 0x1F: //IN_invalid
					*flow_type = FLOW_TYPE_ERR;
					break;
				default:
					*flow_type = FLOW_TYPE_CONTINUE;
					break;
			}
		case 0x02: //INCPU_J
			*new_pc = (pc & 0xF0000000) | (GetTarget << 2);
			bytes = 8;
			*flow_type = FLOW_TYPE_BRANCH;
		case 0x03: //INCPU_JAL
			*new_pc = (pc & 0xF0000000) | (GetTarget << 2);
			bytes = 8;
			*flow_type = FLOW_TYPE_CALL;
			break;
		case 0x04: //INCPU_BEQ
			if (!GetRS && !GetRT) { // special case: B
				*new_pc = MIPS_BRANCH_TARGET;
				bytes = 8; // !likely
				*flow_type = FLOW_TYPE_JUMP;
			} else {
				*new_pc = MIPS_BRANCH_TARGET;
				bytes = 8; // !likely
				*flow_type = FLOW_TYPE_BRANCH;
			}
			break;
		case 0x05: //INCPU_BNE
		case 0x06: //INCPU_BLEZ
		case 0x07: //INCPU_BGTZ
			*new_pc = MIPS_BRANCH_TARGET;
			bytes = 8; // !likely
			*flow_type = FLOW_TYPE_BRANCH;
			break;
		case 0x10: //INCPU_COP0
			// we don't translate any of the INCPU_COP0 branch
			*flow_type = FLOW_TYPE_ERR;
			break;
		case 0x11: //INCPU_COP1
			switch(GetFMT) {
				case 0x03: //IN_invalid
				case 0x07: //IN_invalid
				case 0x09: //IN_invalid
				case 0x0A: //IN_invalid
				case 0x0B: //IN_invalid
				case 0x0C: //IN_invalid
				case 0x0D: //IN_invalid
				case 0x0E: //IN_invalid
				case 0x0F: //IN_invalid
					*flow_type = FLOW_TYPE_ERR;
					break;
				case 0x10: //INCOP1_S
					switch(GetCOP1FloatInstruction) {
						case 0x00: //INCOP1_ADD
								case 0x10: //IN_invalid
								case 0x11: //IN_invalid
								case 0x12: //IN_invalid
								case 0x13: //IN_invalid
								case 0x14: //IN_invalid
								case 0x15: //IN_invalid
								case 0x16: //IN_invalid
								case 0x17: //IN_invalid
								case 0x18: //IN_invalid
								case 0x19: //IN_invalid
								case 0x1A: //IN_invalid
								case 0x1B: //IN_invalid
								case 0x1C: //IN_invalid
								case 0x1D: //IN_invalid
								case 0x1E: //IN_invalid
								case 0x1F: //IN_invalid
								case 0x22: //IN_invalid
								case 0x23: //IN_invalid
								case 0x26: //IN_invalid
								case 0x27: //IN_invalid
								case 0x28: //IN_invalid
								case 0x29: //IN_invalid
								case 0x2A: //IN_invalid
								case 0x2B: //IN_invalid
								case 0x2C: //IN_invalid
								case 0x2D: //IN_invalid
								case 0x2E: //IN_invalid
								case 0x2F: //IN_invalid
									*flow_type = FLOW_TYPE_ERR;
									break;
								default:
									*flow_type = FLOW_TYPE_CONTINUE;
									break;
							}
				case 0x12: //IN_invalid
				case 0x13: //IN_invalid
				case 0x16: //IN_invalid
				case 0x17: //IN_invalid
				case 0x18: //IN_invalid
				case 0x19: //IN_invalid
				case 0x1A: //IN_invalid
				case 0x1B: //IN_invalid
				case 0x1C: //IN_invalid
				case 0x1D: //IN_invalid
				case 0x1E: //IN_invalid
				case 0x1F: //IN_invalid
					*flow_type = FLOW_TYPE_ERR;
					break;
				default:
					*flow_type = FLOW_TYPE_CONTINUE;
					break;
			}
		case 0x14: //INCPU_BEQL
		case 0x15: //INCPU_BNEL
		case 0x16: //INCPU_BLEZL
		case 0x17: //INCPU_BGTZL
			*new_pc = MIPS_BRANCH_TARGET;
			bytes = 8; // likely
			*flow_type = FLOW_TYPE_BRANCH;
			break;
		case 0x12: //IN_invalid
		case 0x13: //IN_invalid
		case 0x1C: //IN_invalid
		case 0x1D: //IN_invalid
		case 0x1E: //IN_invalid
		case 0x1F: //IN_invalid
		case 0x32: //IN_invalid
		case 0x33: //IN_invalid
		case 0x36: //IN_invalid
		case 0x3A: //IN_invalid
		case 0x3B: //IN_invalid
		case 0x3E: //IN_invalid
			*flow_type = FLOW_TYPE_ERR;
			break;
		default:
			*flow_type = FLOW_TYPE_CONTINUE;
			break;
	}

//printf("%s:%d bytes=%d\n", __func__, __LINE__, bytes);
	return bytes;
}

int
arch_mips_disasm_instr(uint8_t* RAM, addr_t pc, char *line, unsigned int max_line) {

	int dummy1;
	addr_t dummy2;
	int bytes = arch_mips_tag_instr(RAM, pc, &dummy1, &dummy2);

	uint32_t instr = RAM32BE(RAM, pc);
	TOpcode op;
	op.all = instr;

	CPUDisassembler *disassembler = new CPUDisassembler();
	snprintf(line, max_line, "%s", disassembler->Disassemble(pc, op, false).c_str());

	if (bytes == 8) {// delay slot
		instr = RAM32BE(RAM, pc+4);
		op.all = instr;
		snprintf(line+strlen(line), max_line-strlen(line), " [%s]", disassembler->Disassemble(pc+4, op, false).c_str());
	}

	return bytes;
}

//////////////////////////////////////////////////////////////////////

#define CONSTs(s,v) ConstantInt::get(IntegerType::get(s), v)
#define CONST8(v) CONSTs(8,v)
#define CONST16(v) CONSTs(16,v)
#define CONST32(v) CONSTs(32,v)
#define CONST64(v) CONSTs(64,v)

#define CONST(v) CONSTs(is_64bit?64:32,v)

#define TRUNC(s,v) new TruncInst(v, IntegerType::get(s), "", bb)
#define TRUNC8(v) TRUNC(8,v)
#define TRUNC16(v) TRUNC(16,v)
#define TRUNC32(v) TRUNC(32,v)

#define ZEXT(s,v) new ZExtInst(v, IntegerType::get(s), "", bb)
#define ZEXT8(v) ZEXT(8,v)
#define ZEXT16(v) ZEXT(16,v)
#define ZEXT32(v) ZEXT(32,v)

#define SEXT(s,v) new SExtInst(v, IntegerType::get(s), "", bb)
#define SEXT8(v) SEXT(8,v)
#define SEXT16(v) SEXT(16,v)
#define SEXT32(v) SEXT(32,v)

#define ADD(a,b) BinaryOperator::Create(Instruction::Add, a, b, "", bb)
#define SUB(a,b) BinaryOperator::Create(Instruction::Sub, a, b, "", bb)
#define AND(a,b) BinaryOperator::Create(Instruction::And, a, b, "", bb)
#define OR(a,b) BinaryOperator::Create(Instruction::Or, a, b, "", bb)
#define XOR(a,b) BinaryOperator::Create(Instruction::Xor, a, b, "", bb)
#define SHL(a,b) BinaryOperator::Create(Instruction::Shl, a, b, "", bb)
#define LSHR(a,b) BinaryOperator::Create(Instruction::LShr, a, b, "", bb)
#define ASHR(a,b) BinaryOperator::Create(Instruction::AShr, a, b, "", bb)
#define ICMP_EQ(a,b) new ICmpInst(ICmpInst::ICMP_EQ, a, b, "", bb)
#define ICMP_NE(a,b) new ICmpInst(ICmpInst::ICMP_NE, a, b, "", bb)
#define ICMP_ULT(a,b) new ICmpInst(ICmpInst::ICMP_ULT, a, b, "", bb)
#define ICMP_SLT(a,b) new ICmpInst(ICmpInst::ICMP_SLT, a, b, "", bb)
#define ICMP_SGT(a,b) new ICmpInst(ICmpInst::ICMP_SGT, a, b, "", bb)
#define ICMP_SGE(a,b) new ICmpInst(ICmpInst::ICMP_SGE, a, b, "", bb)
#define ICMP_SLE(a,b) new ICmpInst(ICmpInst::ICMP_SLE, a, b, "", bb)

//////////////////////////////////////////////////////////////////////

// the *32 functions work on the lower 32 bits of registers
// the *   functions work on the complete register
// if it's a 32 bit MIPS, these two are identical

//////////////////////////////////////////////////////////////////////

// GET REGISTER, register sized version
Value *
arch_mips_get_reg(uint32_t index, BasicBlock *bb) {
	if (!index)
		return ConstantInt::get(IntegerType::get(is_64bit? 64:32), 0);
	else
		return new LoadInst(ptr_r[index], "", false, bb);
}

// GET RS, register sized version
Value *
arch_mips_get_rs(uint32_t instr, BasicBlock *bb) {
	return arch_mips_get_reg(GetRS, bb);
}

// GET RT, register sized version
Value *
arch_mips_get_rt(uint32_t instr, BasicBlock *bb) {
	return arch_mips_get_reg(GetRT, bb);
}

// GET RS, 32 bit version
Value *
arch_mips_get_rs32(uint32_t instr, BasicBlock *bb) {
	Value *v = arch_mips_get_rs(instr, bb);
	if (is_64bit)
		return new TruncInst(v, Type::Int32Ty, "", bb);
	else
		return v;
}

// GET RT, 32 bit version
Value *
arch_mips_get_rt32(uint32_t instr, BasicBlock *bb) {
	Value *v = arch_mips_get_rt(instr, bb);
	if (is_64bit)
		return new TruncInst(v, Type::Int32Ty, "", bb);
	else
		return v;
}

//////////////////////////////////////////////////////////////////////

// PUT REGISTER, register sized version
void
arc_mips_put_reg(uint32_t index, Value *v, BasicBlock *bb) {
	if (index)
		new StoreInst(v, ptr_r[index], bb);
}

// PUT RS, register sized version
void
arch_mips_put_rs(Value *v, uint32_t instr, BasicBlock *bb) {
	arc_mips_put_reg(GetRS, v, bb);
}

// PUT RT, register sized version
void
arch_mips_put_rt(Value *v, uint32_t instr, BasicBlock *bb) {
	arc_mips_put_reg(GetRT, v, bb);
}

// PUT RD, register sized version
void
arch_mips_put_rd(Value *v, uint32_t instr, BasicBlock *bb) {
	arc_mips_put_reg(GetRD, v, bb);
}

// PUT RS, 32 bit version
void
arch_mips_put_rs32(Value *v, uint32_t instr, BasicBlock *bb) {
	if (is_64bit)
		v = new SExtInst(v, IntegerType::get(64), "", bb);
	arch_mips_put_rs(v, instr, bb);
}

// PUT RT, 32 bit version
void
arch_mips_put_rt32(Value *v, uint32_t instr, BasicBlock *bb) {
	if (is_64bit)
		v = new SExtInst(v, IntegerType::get(64), "", bb);
	arch_mips_put_rt(v, instr, bb);
}

// PUT RD, 32 bit version
void
arch_mips_put_rd32(Value *v, uint32_t instr, BasicBlock *bb) {
	if (is_64bit)
		v = new SExtInst(v, IntegerType::get(64), "", bb);
	arch_mips_put_rd(v, instr, bb);
}

//////////////////////////////////////////////////////////////////////

// zero extends an i1 to register size and stores it in RT
void
arch_mips_put_rt_zext(Value *v, uint32_t instr, BasicBlock *bb) {
	v = new ZExtInst(v, IntegerType::get(is_64bit? 64:32), "", bb);
	arch_mips_put_rt(v, instr, bb);
}

// zero extends an i1 to register size and stores it in RD
void
arch_mips_put_rd_zext(Value *v, uint32_t instr, BasicBlock *bb) {
	v = new ZExtInst(v, IntegerType::get(is_64bit? 64:32), "", bb);
	arch_mips_put_rd(v, instr, bb);
}

//////////////////////////////////////////////////////////////////////

// GET IMMEDIATE, register sized version
Value *
arch_mips_get_imm(uint32_t instr, BasicBlock *bb) {
	return ConstantInt::get(IntegerType::get(is_64bit? 64:32), (uint64_t)(sint16_t)GetImmediate);
}

// GET IMMEDIATE, register sized version, unsigned
Value *
arch_mips_get_immu(uint32_t instr, BasicBlock *bb) {
	return ConstantInt::get(IntegerType::get(is_64bit? 64:32), (uint64_t)GetImmediate);
}

// GET IMMEDIATE, 32 bit version
Value *
arch_mips_get_imm32(uint32_t instr, BasicBlock *bb) {
	return ConstantInt::get(Type::Int32Ty, (uint32_t)(sint16_t)GetImmediate);
}

//////////////////////////////////////////////////////////////////////

// GET SA, register sized version
Value *
arch_mips_get_sa(uint32_t instr, BasicBlock *bb) {
	return ConstantInt::get(IntegerType::get(is_64bit? 64:32), GetSA);
}

// GET SA, 32 bit version
Value *
arch_mips_get_sa32(uint32_t instr, BasicBlock *bb) {
	return ConstantInt::get(Type::Int32Ty, GetSA);
}

//////////////////////////////////////////////////////////////////////

// get a RAM pointer to a 32 bit value
Value *
arch_mips_gep32(Value *a, BasicBlock *bb) {
	a = GetElementPtrInst::Create(ptr_RAM, a, "", bb);
	return new BitCastInst(a, PointerType::get(Type::Int32Ty, 0), "", bb);
}

//////////////////////////////////////////////////////////////////////

// load RAM
Value *
arch_mips_load32(Value *a, BasicBlock *bb) {
	a = arch_mips_gep32(a, bb);
	return new LoadInst(a, "", false, bb);
}

// store RAM
void
arch_mips_store32(Value *v, Value *a, BasicBlock *bb) {
	a = arch_mips_gep32(a, bb);
	new StoreInst(v, a, bb);
}

//////////////////////////////////////////////////////////////////////
// endianness
//////////////////////////////////////////////////////////////////////

Value *
arch_mips_get_shift8(Value *addr, BasicBlock *bb)
{
	Value *shift = AND(addr,CONST(3));
	if (!is_little_endian)
		shift = XOR(shift, CONST(3));
	return SHL(CONST(3), shift);
}

Value *
arch_mips_get_shift16(Value *addr, BasicBlock *bb)
{
	Value *shift = AND(addr,CONST(1));
	if (!is_little_endian)
		shift = XOR(shift, CONST(1));
	return SHL(CONST(4), shift);
}

Value *
arch_mips_load8(Value *addr, BasicBlock *bb) {
	Value *shift = arch_mips_get_shift8(addr, bb);
	Value *val = arch_mips_load32(AND(addr, CONST(~3ULL)), bb);
	return TRUNC8(LSHR(val, shift));
}

Value *
arch_mips_load16(Value *addr, BasicBlock *bb) {
	Value *shift = arch_mips_get_shift16(addr, bb);
	Value *val = arch_mips_load32(AND(addr, CONST(~3ULL)), bb);
	return TRUNC16(LSHR(val, shift));
}

void
arch_mips_store8(Value *val, Value *addr, BasicBlock *bb) {
	Value *shift = arch_mips_get_shift8(addr, bb);
	addr = AND(addr, CONST(~3ULL));
	Value *mask = XOR(SHL(CONST(255), shift),CONST(-1ULL));
	Value *old = AND(arch_mips_load32(addr, bb), mask);
	val = OR(old, SHL(AND(val, CONST(255)), shift));
	arch_mips_store32(val, addr, bb);
}

//////////////////////////////////////////////////////////////////////
// memory read/write sign extension
//////////////////////////////////////////////////////////////////////

// big endian 8 bit read, unsigned
Value *
arch_mips_load8_32u(Value *a, BasicBlock *bb) {
	return ZEXT32(arch_mips_load8(a,bb));
}

// big endian 8 bit read, signed
Value *
arch_mips_load8_32s(Value *a, BasicBlock *bb) {
	return SEXT32(arch_mips_load8(a,bb));
}

// big endian 16 bit read, unsigned
Value *
arch_mips_load16_32u(Value *a, BasicBlock *bb) {
	return ZEXT32(arch_mips_load16(a,bb));
}

// big endian 16 bit read, signed
Value *
arch_mips_load16_32s(Value *a, BasicBlock *bb) {
	return SEXT32(arch_mips_load16(a,bb));
}

//////////////////////////////////////////////////////////////////////

void
arch_mips_branch(uint8_t* RAM, addr_t pc, Value *v, bool likely, BasicBlock *bb) {
	uint32_t instr = RAM32BE(RAM, pc);

	if (likely) {
		printf("error: \"likely\" is broken!\n");
		exit(1);
	}

	BasicBlock *target1 = (BasicBlock*)lookup_basicblock(func_jitmain, MIPS_BRANCH_TARGET);
	BasicBlock *target2 = (BasicBlock*)lookup_basicblock(func_jitmain, pc+8);

	if (!target1) {
		printf("error: unknown branch target $%08llx!\n", (unsigned long long)MIPS_BRANCH_TARGET);
		exit(1);
	}
	if (!target2) {
		printf("error: unknown branch continue $%08llx!\n", (unsigned long long)pc+8);
		exit(1);
	}

	BranchInst::Create(target1, target2, v, bb);
}

void
arch_mips_jump(addr_t new_pc, BasicBlock *bb) {
	BasicBlock *target = (BasicBlock*)lookup_basicblock(func_jitmain, new_pc);
	if (!target) {
		printf("error: unknown jump target $%08llx!\n", (unsigned long long)new_pc);
		exit(1);
	}
	BranchInst::Create(target, bb);
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

#define RS arch_mips_get_rs(instr, bb)
#define RT arch_mips_get_rt(instr, bb)
#define SA arch_mips_get_sa(instr, bb)
#define IMM arch_mips_get_imm(instr, bb)
#define IMMU arch_mips_get_immu(instr, bb)
#define RS32 arch_mips_get_rs32(instr, bb)
#define RT32 arch_mips_get_rt32(instr, bb)
#define SA32 arch_mips_get_sa32(instr, bb)
#define IMM32 arch_mips_get_imm32(instr, bb)

#define LET_RT(v) arch_mips_put_rt(v, instr, bb)
#define LET_RD(v) arch_mips_put_rd(v, instr, bb)
#define LET_RT32(v) arch_mips_put_rt32(v, instr, bb)
#define LET_RD32(v) arch_mips_put_rd32(v, instr, bb)
#define LET_RT_ZEXT(v) arch_mips_put_rt_zext(v, instr, bb)
#define LET_RD_ZEXT(v) arch_mips_put_rd_zext(v, instr, bb)
#define LET_PC(v) new StoreInst(v, ptr_PC, bb)
#define LET_Ri(i, v) new StoreInst(v, ptr_r[i], bb)

#define LOAD_RT8(v) arch_mips_put_rt32(arch_mips_load8_32u(v, bb), instr, bb)
#define LOAD_RT16(v) arch_mips_put_rt32(arch_mips_load16_32u(v, bb), instr, bb)
#define LOAD_RT32(v) arch_mips_put_rt32(arch_mips_load32(v, bb), instr, bb)
#define LOAD_RT8S(v) arch_mips_put_rt32(arch_mips_load8_32s(v, bb), instr, bb)
#define LOAD_RT16S(v) arch_mips_put_rt32(arch_mips_load16_32s(v, bb), instr, bb)

#define STORE_RT8(v) arch_mips_store8(RT32, v, bb)
#define STORE_RT32(v) arch_mips_store32(RT32, v, bb)

#define DELAY_SLOT arch_mips_recompile_instr(RAM, pc+4, bb_dispatch, bb)
#define JMP_BB(b) BranchInst::Create(b, bb)
#define JMP_ADDR(a) arch_mips_jump(a, bb)

#define BRANCH_TRUE(tag)  arch_mips_branch(RAM, pc, tag, false, bb);
#define BRANCH_TRUE_LIKELY(tag)  arch_mips_branch(RAM, pc, tag, true, bb);

#define BRANCH_DELAY_TRUE(test) {	\
	Value *f = test;				\
	DELAY_SLOT;						\
	BRANCH_TRUE(f);					\
}
#define BRANCH_DELAY_TRUE_LIKELY(test) {	\
	Value *f = test;				\
	DELAY_SLOT;						\
	BRANCH_TRUE_LIKELY(f);					\
}

#define PC_PLUS_8 CONST((uint64_t)(sint64_t)(sint32_t)pc+8)

#define LINKr(i) LET_Ri(i, PC_PLUS_8)

#define LINK LINKr(31)

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

int arch_mips_recompile_instr(uint8_t* RAM, addr_t pc, BasicBlock *bb_dispatch, BasicBlock *bb) {
#define BAD printf("%s:%d\n", __func__, __LINE__); exit(1);
#define LOG printf("%s:%d\n", __func__, __LINE__);

//printf("%s:%d %p, %p\n", __func__, __LINE__, bb_dispatch, bb);

	uint32_t instr = RAM32BE(RAM, pc);

	printf("translating (%08llx) %08x\n", pc, instr);

	switch(instr >> 26) {
	case 0x00: /* INCPU_SPECIAL */
		switch(instr & 0x3F) {
			case 0x00: /* INCPUS_SLL */		LET_RD32(SHL(RT32,SA32));	break; /* XXX special case NOP? */
			case 0x02: /* INCPUS_SRL */		LET_RD32(LSHR(RT32,SA32));	break;
			case 0x03: /* INCPUS_SRA */		LET_RD32(ASHR(RT32,SA32));	break;
			case 0x04: /* INCPUS_SLLV */	LET_RD32(SHL(RT32,RS32));	break;
			case 0x06: /* INCPUS_SRLV */	LET_RD32(LSHR(RT32,RS32));	break;
			case 0x07: /* INCPUS_SRAV */	LET_RD32(ASHR(RT32,RS32));	break;
			case 0x08: /* INCPUS_JR */
			{
				LET_PC(RS);
				DELAY_SLOT;
				JMP_BB(bb_dispatch);
				break;
			}
			case 0x09: /* INCPUS_JALR */
			{
				LET_PC(SUB(RS,CONST(4)));
				LINKr(GetRD);
				DELAY_SLOT;
				JMP_BB(bb_dispatch);
				break;
			}
			case 0x0C: /* INCPUS_SYSCALL */	BAD;
			case 0x0D: /* INCPUS_BREAK */	BAD;
			case 0x0F: /* INCPUS_SYNC */	BAD;
			case 0x10: /* INCPUS_MFHI */	BAD;
			case 0x11: /* INCPUS_MTHI */	BAD;
			case 0x12: /* INCPUS_MFLO */	BAD;
			case 0x13: /* INCPUS_MTLO */	BAD;
			case 0x14: /* INCPUS_DSLLV */	BAD;
			case 0x16: /* INCPUS_DSRLV */	BAD;
			case 0x17: /* INCPUS_DSRAV */	BAD;
			case 0x18: /* INCPUS_MULT */	BAD;
			case 0x19: /* INCPUS_MULTU */	BAD;
			case 0x1A: /* INCPUS_DIV */		BAD;
			case 0x1B: /* INCPUS_DIVU */	BAD;
			case 0x1C: /* INCPUS_DMULT */	BAD;
			case 0x1D: /* INCPUS_DMULTU */	BAD;
			case 0x1E: /* INCPUS_DDIV */	BAD;
			case 0x1F: /* INCPUS_DDIVU */	BAD;
			case 0x20: /* INCPUS_ADD */		LET_RD32(ADD(RS32, RT32));		break; //XXX same??
			case 0x21: /* INCPUS_ADDU */	LET_RD32(ADD(RS32, RT32));		break; //XXX same??
			case 0x22: /* INCPUS_SUB */		LET_RD32(SUB(RS32, RT32));		break; //XXX same??
			case 0x23: /* INCPUS_SUBU */	LET_RD32(SUB(RS32, RT32));		break; //XXX same??
			case 0x24: /* INCPUS_AND */		LET_RD32(AND(RS32, RT32));		break;
			case 0x25: /* INCPUS_OR */		LET_RD32(OR(RS32, RT32));		break;
			case 0x26: /* INCPUS_XOR */		LET_RD32(XOR(RS32, RT32));		break;
			case 0x27: /* INCPUS_NOR */		LET_RD32(XOR(OR(RS32, RT32),CONST32(-1)));	break;
			case 0x2A: /* INCPUS_SLT */		LET_RD_ZEXT(ICMP_SLT(RS,RT));	break;
			case 0x2B: /* INCPUS_SLTU */	LET_RD_ZEXT(ICMP_ULT(RS,RT));	break;
			case 0x2C: /* INCPUS_DADD */	LET_RD(ADD(RS, RT));			break; //XXX same??
			case 0x2D: /* INCPUS_DADDU */	LET_RD(ADD(RS, RT));			break; //XXX same??
			case 0x2E: /* INCPUS_DSUB */	LET_RD(SUB(RS, RT));			break; //XXX same??
			case 0x2F: /* INCPUS_DSUBU */	LET_RD(SUB(RS, RT));			break; //XXX same??
			case 0x30: /* INCPUS_TGE */		BAD;
			case 0x31: /* INCPUS_TGEU */	BAD;
			case 0x32: /* INCPUS_TLT */		BAD;
			case 0x33: /* INCPUS_TLTU */	BAD;
			case 0x34: /* INCPUS_TEQ */		BAD;
			case 0x36: /* INCPUS_TNE */		BAD;
			case 0x38: /* INCPUS_DSLL */	LET_RD(SHL(RT,SA));				break;
			case 0x3A: /* INCPUS_DSRL */	LET_RD(LSHR(RT,SA));			break;
			case 0x3B: /* INCPUS_DSRA */	LET_RD(ASHR(RT,SA));			break;
			case 0x3C: /* INCPUS_DSLL32 */	LET_RD(SHL(RT,ADD(SA,CONST(32))));		break;
			case 0x3E: /* INCPUS_DSRL32 */	LET_RD(LSHR(RT,ADD(SA,CONST(32))));		break;
			case 0x3F: /* INCPUS_DSRA32 */	LET_RD(ASHR(RT,ADD(SA,CONST(32))));		break;
			default:
				printf("INVALID %s:%d\n", __func__, __LINE__); exit(1);
		}
		break;
	case 0x01: /* INCPU_REGIMM */
		switch (GetRegimmInstruction) {
			case 0x00: /* INCPUR_BLTZ */	BRANCH_DELAY_TRUE(ICMP_SLT(RS,CONST(0)));	break;
			case 0x01: /* INCPUR_BGEZ */	BRANCH_DELAY_TRUE(ICMP_SGE(RS,CONST(0)));	break;
			case 0x02: /* INCPUR_BLTZL */	BRANCH_DELAY_TRUE_LIKELY(ICMP_SLT(RS,CONST(0)));	break;
			case 0x03: /* INCPUR_BGEZL */	BRANCH_DELAY_TRUE_LIKELY(ICMP_SGE(RS,CONST(0)));	break;
			case 0x08: /* INCPUR_TGEI */	BAD;
			case 0x09: /* INCPUR_TGEIU */	BAD;
			case 0x0A: /* INCPUR_TLTI */	BAD;
			case 0x0B: /* INCPUR_TLTIU */	BAD;
			case 0x0C: /* INCPUR_TEQI */	BAD;
			case 0x0E: /* INCPUR_TNEI */	BAD;
			case 0x10: /* INCPUR_BLTZAL */	LINK; BRANCH_DELAY_TRUE(ICMP_SLT(RS,CONST(0)));	break;
			case 0x11: /* INCPUR_BGEZAL */	LINK; BRANCH_DELAY_TRUE(ICMP_SGE(RS,CONST(0)));	break;
			case 0x12: /* INCPUR_BLTZALL */	LINK; BRANCH_DELAY_TRUE_LIKELY(ICMP_SLT(RS,CONST(0)));	break;
			case 0x13: /* INCPUR_BGEZALL */	LINK; BRANCH_DELAY_TRUE_LIKELY(ICMP_SGE(RS,CONST(0)));	break;
			default:
				printf("INVALID %s:%d\n", __func__, __LINE__); exit(1);
		}
	case 0x02: /* INCPU_J */
		{
		addr_t new_pc = (pc & 0xF0000000) | (GetTarget << 2);
		DELAY_SLOT;
		JMP_ADDR(new_pc);
		break;
		}
	case 0x03: /* INCPU_JAL */
		{
		addr_t new_pc = (pc & 0xF0000000) | (GetTarget << 2);
		LINK;
		DELAY_SLOT;
		JMP_ADDR(new_pc);
		break;
		}
	case 0x04: /* INCPU_BEQ */		
		if (!GetRS && !GetRT) { // special case: B
			DELAY_SLOT;
			JMP_ADDR(MIPS_BRANCH_TARGET);
		} else {
			BRANCH_DELAY_TRUE(ICMP_EQ(RS, RT));
		}
		break;
	case 0x05: /* INCPU_BNE */		BRANCH_DELAY_TRUE(ICMP_NE(RS, RT));			break;
	case 0x06: /* INCPU_BLEZ */		BRANCH_DELAY_TRUE(ICMP_SLE(RS,CONST(0)));	break;
	case 0x07: /* INCPU_BGTZ */		BRANCH_DELAY_TRUE(ICMP_SGT(RS,CONST(0)));	break;
	case 0x08: /* INCPU_ADDI */		LET_RT32(ADD(RS32, IMM32));					break; //XXX same??
	case 0x09: /* INCPU_ADDIU */	LET_RT32(ADD(RS32, IMM32));					break; //XXX same??
	case 0x0A: /* INCPU_SLTI */		LET_RT_ZEXT(ICMP_ULT(RS,IMM));				break; //XXX same??
	case 0x0B: /* INCPU_SLTIU */	LET_RT_ZEXT(ICMP_ULT(RS,IMM));				break; //XXX same??
	case 0x0C: /* INCPU_ANDI */		LET_RT(AND(RS, IMMU));						break;
	case 0x0D: /* INCPU_ORI */		LET_RT(OR(RS, IMMU));						break;
	case 0x0E: /* INCPU_XORI */		LET_RT(XOR(RS, IMMU));						break;
	case 0x0F: /* INCPU_LUI */		LET_RT(SHL(IMMU,CONST(16)));				break;
	case 0x10: /* INCPU_COP0 */
		switch (GetFMT) {
			case 0x00: /* INCOP0_MFC0 */	BAD;
			case 0x04: /* INCOP0_MTC0 */	BAD;
			case 0x10: /* INCOP0_TLB */
				switch(GetFunction) {
					case 0x01: /* INCOP0TLB_TLBR */	BAD;
					case 0x02: /* INCOP0TLB_TLBWI */	BAD;
					case 0x06: /* INCOP0TLB_TLBWR */	BAD;
					case 0x08: /* INCOP0TLB_TLBP */	BAD;
					case 0x18: /* INCOP0TLB_ERET */	BAD;
					default:
						printf("INVALID %s:%d\n", __func__, __LINE__); exit(1);
				}
			default:
				printf("INVALID %s:%d\n", __func__, __LINE__); exit(1);
		}
	case 0x11: /* INCPU_COP1 */
		switch(GetFMT) {
			case 0x00: /* INCOP1_MFC1 */	BAD;
			case 0x01: /* INCOP1_DMFC1 */	BAD;
			case 0x02: /* INCOP1_CFC1 */	BAD;
			case 0x04: /* INCOP1_MTC1 */	BAD;
			case 0x05: /* INCOP1_DMTC1 */	BAD;
			case 0x06: /* INCOP1_CTC1 */	BAD;
			case 0x08: /* INCOP1_BC */	BAD;
			case 0x10: /* INCOP1_S */
				switch(GetCOP1FloatInstruction) {
					case 0x00: /* INCOP1_ADD */	BAD;
					case 0x01: /* INCOP1_SUB */	BAD;
					case 0x02: /* INCOP1_MUL */	BAD;
					case 0x03: /* INCOP1_DIV */	BAD;
					case 0x04: /* INCOP1_SQRT */	BAD;
					case 0x05: /* INCOP1_ABS */	BAD;
					case 0x06: /* INCOP1_MOV */	BAD;
					case 0x07: /* INCOP1_NEG */	BAD;
					case 0x08: /* INCOP1_ROUND_L */	BAD;
					case 0x09: /* INCOP1_TRUNC_L */	BAD;
					case 0x0A: /* INCOP1_CEIL_L */	BAD;
					case 0x0B: /* INCOP1_FLOOR_L */	BAD;
					case 0x0C: /* INCOP1_ROUND_W */	BAD;
					case 0x0D: /* INCOP1_TRUNC_W */	BAD;
					case 0x0E: /* INCOP1_CEIL_W */	BAD;
					case 0x0F: /* INCOP1_FLOOR_W */	BAD;
					case 0x20: /* INCOP1_CVT_S */	BAD;
					case 0x21: /* INCOP1_CVT_D */	BAD;
					case 0x24: /* INCOP1_CVT_W */	BAD;
					case 0x25: /* INCOP1_CVT_L */	BAD;
					case 0x30: /* INCOP1_C_F */	BAD;
					case 0x31: /* INCOP1_C_UN */	BAD;
					case 0x32: /* INCOP1_C_EQ */	BAD;
					case 0x33: /* INCOP1_C_UEQ */	BAD;
					case 0x34: /* INCOP1_C_OLT */	BAD;
					case 0x35: /* INCOP1_C_ULT */	BAD;
					case 0x36: /* INCOP1_C_OLE */	BAD;
					case 0x37: /* INCOP1_C_ULE */	BAD;
					case 0x38: /* INCOP1_C_SF */	BAD;
					case 0x39: /* INCOP1_C_NGLE */	BAD;
					case 0x3A: /* INCOP1_C_SEQ */	BAD;
					case 0x3B: /* INCOP1_C_NGL */	BAD;
					case 0x3C: /* INCOP1_C_LT */	BAD;
					case 0x3D: /* INCOP1_C_NGE */	BAD;
					case 0x3E: /* INCOP1_C_LE */	BAD;
					case 0x3F: /* INCOP1_C_NGT */	BAD;
					default:
						printf("INVALID %s:%d\n", __func__, __LINE__); exit(1);
				}
			case 0x11: /* INCOP1_D */	BAD;
			case 0x14: /* INCOP1_W */	BAD;
			case 0x15: /* INCOP1_L */	BAD;
			default:
				printf("INVALID %s:%d\n", __func__, __LINE__); exit(1);
		}
	case 0x14: /* INCPU_BEQL */		BRANCH_DELAY_TRUE_LIKELY(ICMP_EQ(RS, RT));			break;
	case 0x15: /* INCPU_BNEL */		BRANCH_DELAY_TRUE_LIKELY(ICMP_NE(RS, RT));			break;
	case 0x16: /* INCPU_BLEZL */	BRANCH_DELAY_TRUE_LIKELY(ICMP_SLE(RS, CONST(0)));	break;
	case 0x17: /* INCPU_BGTZL */	BRANCH_DELAY_TRUE_LIKELY(ICMP_SGT(RS, CONST(0)));	break;
	case 0x18: /* INCPU_DADDI */	LET_RT(ADD(RS,IMM));								break; //XXX same??
	case 0x19: /* INCPU_DADDIU */	LET_RT(ADD(RS,IMM));								break; //XXX same??
	case 0x1A: /* INCPU_LDL */		BAD;
	case 0x1B: /* INCPU_LDR */		BAD;
	case 0x20: /* INCPU_LB */		LOAD_RT8S(ADD(RS32,IMM32));							break;
	case 0x21: /* INCPU_LH */		LOAD_RT16S(ADD(RS32,IMM32));						break;
	case 0x22: /* INCPU_LWL */		BAD;
	case 0x23: /* INCPU_LW */		LOAD_RT32(ADD(RS32,IMM32));							break; //XXX ignores misalignment
	case 0x24: /* INCPU_LBU */		LOAD_RT8(ADD(RS32,IMM32));							break;
	case 0x25: /* INCPU_LHU */		LOAD_RT16(ADD(RS32,IMM32));							break;
	case 0x26: /* INCPU_LWR */		BAD;
	case 0x27: /* INCPU_LWU */		BAD;
	case 0x28: /* INCPU_SB */		STORE_RT8(ADD(RS32,IMM32));							break;
	case 0x29: /* INCPU_SH */		BAD;
	case 0x2A: /* INCPU_SWL */		BAD;
	case 0x2B: /* INCPU_SW */		STORE_RT32(ADD(RS32,IMM32));						break;
	case 0x2C: /* INCPU_SDL */	BAD;
	case 0x2D: /* INCPU_SDR */	BAD;
	case 0x2E: /* INCPU_SWR */	BAD;
	case 0x2F: /* INCPU_CACHE */	break; /* no-op */
	case 0x30: /* INCPU_LL */		break; /* no-op */
	case 0x31: /* INCPU_LWC1 */	BAD;
	case 0x34: /* INCPU_LLD */		break; /* no-op */
	case 0x35: /* INCPU_LDC1 */	BAD;
	case 0x37: /* INCPU_LD */	BAD;
	case 0x38: /* INCPU_SC */		break; /* no-op */
	case 0x39: /* INCPU_SWC1 */	BAD;
	case 0x3C: /* INCPU_SCD */	BAD;
	case 0x3D: /* INCPU_SDC1 */	BAD;
	case 0x3F: /* INCPU_SD */	BAD;
	default:
		printf("INVALID %s:%d\n", __func__, __LINE__); exit(1);
	}

	int dummy1;
	addr_t dummy2;
	return arch_mips_tag_instr(RAM, pc, &dummy1, &dummy2);
}



#include "arch/mips/libcpu_mips.h"

arch_func_t arch_func_mips = {
	arch_mips_init,
	arch_mips_get_struct_reg,
	arch_mips_get_pc,
	arch_mips_emit_decode_reg,
	arch_mips_spill_reg_state,
	arch_mips_tag_instr,
	arch_mips_disasm_instr,
	arch_mips_recompile_instr
};

//printf("%s:%d PC=$%04X\n", __func__, __LINE__, pc);
//printf("%s:%d\n", __func__, __LINE__);
