#include "libcpu.h"
#include "types.h"
#include "6502_isa.h"
#include "cpu_generic.h"
#include "libcpu_6502.h"

using namespace llvm;

//XXX put into cpu_t
Value* ptr_C;
Value* ptr_Z;
Value* ptr_I;
Value* ptr_D;
Value* ptr_V;
Value* ptr_N;

#define A 0
#define X 1
#define Y 2
#define S 3
#define P 4
#define ptr_A cpu->ptr_r8[A]
#define ptr_X cpu->ptr_r8[X]
#define ptr_Y cpu->ptr_r8[Y]
#define ptr_S cpu->ptr_r8[S]
#define ptr_P cpu->ptr_r8[P]

#define OPCODE cpu->RAM[pc]
#define OPERAND_8 cpu->RAM[(pc+1)&0xFFFF]
#define OPERAND_16 ((cpu->RAM[(pc+1)&0xFFFF] | (cpu->RAM[(pc+2)&0xFFFF]<<8))&0xFFFF)

#define SET_NZ(a) { Value *t = a; LET1(ptr_Z, ICMP_EQ(t, CONST8(0))); LET1(ptr_N, ICMP_SLT(t, CONST8(0))); }

#define OPERAND arch_6502_get_operand_rvalue(cpu, pc, bb)
#define LOPERAND arch_6502_get_operand_lvalue(cpu, pc, bb)
#define LET1(a,b) new StoreInst(b, a, false, bb)

#define GEP(a) GetElementPtrInst::Create(cpu->ptr_RAM, a, "", bb)
#define LOAD(a) new LoadInst(a, "", false, bb)

#define TOS GEP(OR(ZEXT32(R(S)), CONST32(0x0100)))
#define PUSH(v) { STORE(v, TOS); LET(S,DEC(R(S))); }
#define PULL (LET(S,INC(R(S))), LOAD(TOS))
#define PUSH16(v) { PUSH(CONST8(v >> 8)); PUSH(CONST8(v & 0xFF)); }
// Because of a GCC evaluation order problem, the PULL16
// macro needs to be expanded.
#define PULL16 arch_6502_pull16(cpu, bb)

static inline Value *
arch_6502_pull16(cpu_t *cpu, BasicBlock *bb)
{
	Value *lo = PULL;
	Value *hi = PULL;
	return (ADD(ZEXT16(lo), SHL(ZEXT16(hi), CONST16(8))));
}

static inline Value *
arch_6502_load_ram_8(cpu_t *cpu, Value *addr, BasicBlock *bb) {
	Value* ptr = GetElementPtrInst::Create(cpu->ptr_RAM, addr, "", bb);
	return new LoadInst(ptr, "", false, bb);
}

static Value *
arch_6502_load_ram_16(cpu_t *cpu, int is_32, Value *addr, BasicBlock *bb) {
	ConstantInt* const_int32_0001 = ConstantInt::get(getType(Int32Ty), 0x0001);
	ConstantInt* const_int32_0008 = ConstantInt::get(is_32? getType(Int32Ty) : getType(Int16Ty), 0x0008);

	/* get lo */
	Value *lo = arch_6502_load_ram_8(cpu, addr, bb);
	Value *lo32 = new ZExtInst(lo, getIntegerType(is_32? 32:16), "", bb);

	/* get hi */
	addr = BinaryOperator::Create(Instruction::Add, addr, const_int32_0001, "", bb);
	Value *hi = arch_6502_load_ram_8(cpu, addr, bb);
	Value *hi32 = new ZExtInst(hi, getIntegerType(is_32? 32:16), "", bb);

	/* combine */
	BinaryOperator* hi32shifted = BinaryOperator::Create(Instruction::Shl, hi32, const_int32_0008, "", bb);
	return BinaryOperator::Create(Instruction::Add, lo32, hi32shifted, "", bb);
}

static Value *
arch_6502_add_index(Value *ea, Value *index_register, BasicBlock *bb) {
	return ADD(ZEXT32(LOAD(index_register)), ea);
}

static Value *
arch_6502_get_operand_lvalue(cpu_t *cpu, addr_t pc, BasicBlock* bb) {
	int am = instraddmode[OPCODE].addmode;
	Value *index_register_before;
	Value *index_register_after;
	bool is_indirect;
	bool is_8bit;

	switch (am) {
		case ADDMODE_ACC:
			return ptr_A;
		case ADDMODE_BRA:
		case ADDMODE_IMM:
		case ADDMODE_IMPL:
			return NULL;
	}

	is_indirect = ((am == ADDMODE_IND) || (am == ADDMODE_INDX) || (am == ADDMODE_INDY));
	is_8bit = !((am == ADDMODE_ABS) || (am == ADDMODE_ABSX) || (am == ADDMODE_ABSY));
	index_register_before = NULL;
	if ((am == ADDMODE_ABSX) || (am == ADDMODE_INDX) || (am == ADDMODE_ZPX))
		index_register_before = ptr_X;
	if ((am == ADDMODE_ABSY) || (am == ADDMODE_ZPY))
		index_register_before = ptr_Y;
	index_register_after = (am == ADDMODE_INDY)? ptr_Y : NULL;

#if 0
	printf("pc = %x\n", pc);
	printf("index_register_before = %x\n", index_register_before);
	printf("index_register_after = %x\n", index_register_after);
	printf("is_indirect = %x\n", is_indirect);
	printf("is_8bit = %x\n", is_8bit);
#endif

	/* create base constant */
	uint16_t base = is_8bit? (OPERAND_8):(OPERAND_16);
	Value *ea = CONST32(base);

	if (index_register_before)
		ea = arch_6502_add_index(ea, index_register_before, bb);

	/* wrap around in zero page */
	if (is_8bit)
		ea = BinaryOperator::Create(Instruction::And, ea, CONST32(0x00FF), "", bb);
	else if (base >= 0xFF00) /* wrap around in memory */
		ea = BinaryOperator::Create(Instruction::And, ea, CONST32(0xFFFF), "", bb);

	if (is_indirect)
		ea = arch_6502_load_ram_16(cpu, true, ea, bb);

	if (index_register_after)
		ea = arch_6502_add_index(ea, index_register_after, bb);

	return GetElementPtrInst::Create(cpu->ptr_RAM, ea, "", bb);
}

static Value *
arch_6502_get_operand_rvalue(cpu_t *cpu, addr_t pc, BasicBlock* bb)
{
	switch (instraddmode[OPCODE].addmode) {
		case ADDMODE_IMM:
			return CONST8(OPERAND_8);
		default:
			Value *lvalue = arch_6502_get_operand_lvalue(cpu, pc, bb);
			return new LoadInst(lvalue, "", false, bb);
	}
}

static Value *
arch_6502_store(Value *v, Value *a, BasicBlock *bb)
{
	new StoreInst(v, a, bb);
	return v;
}

#define STORE(v,a) arch_6502_store(v, a, bb)

static void
arch_6502_trap(cpu_t *cpu, addr_t pc, BasicBlock *bb)
{
	Value* v_pc = CONST16(pc);
	new StoreInst(v_pc, cpu->ptr_PC, bb);
	ReturnInst::Create(_CTX(), CONST32(JIT_RETURN_TRAP), bb);//XXX needs #define
}

static void
arch_6502_shiftrotate(cpu_t *cpu, uint16_t pc, bool left, bool rotate, BasicBlock *bb)
{
	/* load operand */
	Value *v = OPERAND;
	Value *c;

	/* shift */
	if (left) {
		c = ICMP_SLT(v, CONST8(0));	/* old MSB to carry */
		v = SHL(v, CONST8(1));
		if (rotate)
			v = OR(v,ZEXT8(LOAD(ptr_C)));
	} else {
		c = TRUNC1(v);		/* old LSB to carry */
		v = LSHR(v, CONST8(1));
		if (rotate)
			v = OR(v,SHL(ZEXT8(LOAD(ptr_C)), CONST8(7)));
	}
	
	/* store */	
	SET_NZ(STORE(v, LOPERAND));
	LET1(ptr_C, c);
}

/*
 * This is used for ADC, SBC and CMP
 * - for ADC, we add A + B + C
 * - for SBC, we add A + ~B + C
 * - for CMP, we add A + ~B + 1
 * XXX TODO: consider changing code to avoid 16 bit arithmetic:
 *     while this works ok for 8 bit, it doesn't scale. M88K and ARM
 *     do it differently already.
 */
static void
arch_6502_addsub(cpu_t *cpu, uint16_t pc, Value *reg, Value *reg2, int is_sub, int with_carry, BasicBlock *bb) {
	Value *old_c = NULL; //XXX GCC

	/* load operand, A and C */
	Value *v1 = new LoadInst(reg, "", false, bb);
	Value *v2 = arch_6502_get_operand_rvalue(cpu, pc, bb);

	/* NOT operand (if subtraction) */
	if (is_sub)
		v2 = BinaryOperator::Create(Instruction::Xor, v2, CONST8(0xFF), "", bb);

	/* convert to 16 bits */
	v1 = new ZExtInst(v1, getIntegerType(16), "", bb);
	v2 = new ZExtInst(v2, getIntegerType(16), "", bb);

	/* add them together */
	v1 = BinaryOperator::Create(Instruction::Add, v1, v2, "", bb);

	/* add C or 1 */
	if (with_carry) {
		old_c = new LoadInst(ptr_C, "", false, bb);
		old_c = new ZExtInst(old_c, getIntegerType(16), "", bb);
		v1 = BinaryOperator::Create(Instruction::Add, v1, old_c, "", bb);
	} else {
		v1 = BinaryOperator::Create(Instruction::Add, v1, CONST16(1), "", bb);
	}

	/* get C */
	Value *c = BinaryOperator::Create(Instruction::LShr, v1, CONST16(8), "", bb);
	c = new TruncInst(c, getIntegerType(1), "", bb);
	new StoreInst(c, ptr_C, bb);

	/* get result */
	v1 = new TruncInst(v1, getIntegerType(8), "", bb);
	if (reg2)
		new StoreInst(v1, reg2, bb);

	/* set flags */
	SET_NZ(v1);
}


#define N_SHIFT 7
#define V_SHIFT 6
#define D_SHIFT 3
#define I_SHIFT 2
#define Z_SHIFT 1
#define C_SHIFT 0

static Value *
arch_6502_flags_encode(BasicBlock *bb)
{
	Value *flags = CONST8(0);

	flags = arch_encode_bit(flags, ptr_N, N_SHIFT, 8, bb);
	flags = arch_encode_bit(flags, ptr_V, V_SHIFT, 8, bb);
	flags = arch_encode_bit(flags, ptr_D, D_SHIFT, 8, bb);
	flags = arch_encode_bit(flags, ptr_I, I_SHIFT, 8, bb);
	flags = arch_encode_bit(flags, ptr_Z, Z_SHIFT, 8, bb);
	flags = arch_encode_bit(flags, ptr_C, C_SHIFT, 8, bb);

	return flags;
}

static void
arch_6502_flags_decode(Value *flags, BasicBlock *bb)
{
	arch_decode_bit(flags, ptr_N, N_SHIFT, 8, bb);
	arch_decode_bit(flags, ptr_V, V_SHIFT, 8, bb);
	arch_decode_bit(flags, ptr_D, D_SHIFT, 8, bb);
	arch_decode_bit(flags, ptr_I, I_SHIFT, 8, bb);
	arch_decode_bit(flags, ptr_Z, Z_SHIFT, 8, bb);
	arch_decode_bit(flags, ptr_C, C_SHIFT, 8, bb);
}

Value *
arch_6502_recompile_cond(cpu_t *cpu, addr_t pc, BasicBlock *bb) {
	uint8_t opcode = cpu->RAM[pc];
printf("%s:%d pc=%llx opcode=%x\n", __func__, __LINE__, pc, opcode);

	switch (instraddmode[opcode].instr) {
		case INSTR_BEQ: /* Z */		return LOAD(ptr_Z);
		case INSTR_BNE: /* !Z */	return NOT(LOAD(ptr_Z));
		case INSTR_BCS: /* C */		return LOAD(ptr_C);
		case INSTR_BCC: /* !C */	return NOT(LOAD(ptr_C));
		case INSTR_BMI: /* N */		return LOAD(ptr_N);
		case INSTR_BPL: /* !N */	return NOT(LOAD(ptr_N));
		case INSTR_BVS: /* V */		return LOAD(ptr_V);
		case INSTR_BVC: /* !V */	return NOT(LOAD(ptr_V));
		default:					return NULL; /* no condition; should not be reached */
	}
}

static int
arch_6502_recompile_instr(cpu_t *cpu, addr_t pc, BasicBlock *bb) {
	uint8_t opcode = cpu->RAM[pc];

//printf("%s:%d PC=$%04X\n", __func__, __LINE__, pc);

#if 0 //XXX this must move into generic code
	// add a call to debug_function()
	ConstantInt* v_pc = ConstantInt::get(Type::Int16Ty, pc);
	new StoreInst(v_pc, cpu->ptr_PC, bb);
	// serialize flags
	Value *flags = arch_6502_flags_encode(bb);
	new StoreInst(flags, ptr_P, false, bb);

	create_call(cpu, ptr_func_debug, bb);

	flags = new LoadInst(ptr_P, "", false, bb);
	arch_6502_flags_decode(flags, bb);
#endif

//	printf("\naddmode = %i\n", instraddmode[opcode].addmode);
	switch (instraddmode[opcode].instr) {
		/* flags */
		case INSTR_CLC:	LET1(ptr_C, FALSE);				break;
		case INSTR_CLD:	LET1(ptr_D, FALSE);				break;
		case INSTR_CLI:	LET1(ptr_I, FALSE);				break;
		case INSTR_CLV:	LET1(ptr_V, FALSE);				break;
		case INSTR_SEC:	LET1(ptr_C, TRUE);				break;
		case INSTR_SED:	LET1(ptr_D, TRUE);				break;
		case INSTR_SEI:	LET1(ptr_I, TRUE);				break;

		/* register transfer */
		case INSTR_TAX:	SET_NZ(LET(X,R(A)));			break;
		case INSTR_TAY:	SET_NZ(LET(Y,R(A)));			break;
		case INSTR_TXA:	SET_NZ(LET(A,R(X)));			break;
		case INSTR_TYA:	SET_NZ(LET(A,R(Y)));			break;
		case INSTR_TSX:	SET_NZ(LET(X,R(S)));			break;
		case INSTR_TXS:	SET_NZ(LET(S,R(X)));			break;

		/* load */
		case INSTR_LDA:	SET_NZ(LET(A,OPERAND));			break;
		case INSTR_LDX:	SET_NZ(LET(X,OPERAND));			break;
		case INSTR_LDY:	SET_NZ(LET(Y,OPERAND));			break;

		/* store */
		case INSTR_STA:	STORE(R(A),LOPERAND);			break;
		case INSTR_STX:	STORE(R(X),LOPERAND);			break;
		case INSTR_STY:	STORE(R(Y),LOPERAND);			break;

		/* stack */
		case INSTR_PHA:	PUSH(R(A));						break;
		case INSTR_PHP:	PUSH(arch_6502_flags_encode(bb));	break;
		case INSTR_PLA:	SET_NZ(LET(A,PULL));			break;
		case INSTR_PLP:	arch_6502_flags_decode(PULL, bb);	break;

		/* shift */
		case INSTR_ASL:
			arch_6502_shiftrotate(cpu, pc, true, false, bb);
			break;
		case INSTR_LSR:
			arch_6502_shiftrotate(cpu, pc, false, false, bb);
			break;
		case INSTR_ROL:
			arch_6502_shiftrotate(cpu, pc, true, true, bb);
			break;
		case INSTR_ROR:
			arch_6502_shiftrotate(cpu, pc, false, true, bb);
			break;

		/* bit logic */
		case INSTR_AND:	SET_NZ(LET(A,AND(R(A),OPERAND)));			break;
		case INSTR_ORA:	SET_NZ(LET(A,OR(R(A),OPERAND)));			break;
		case INSTR_EOR:	SET_NZ(LET(A,XOR(R(A),OPERAND)));			break;
		case INSTR_BIT:	SET_NZ(OPERAND);							break;

		/* arithmetic */
		case INSTR_ADC:
			arch_6502_addsub(cpu, pc, ptr_A, ptr_A, false, true, bb);
			break;
		case INSTR_SBC:
			arch_6502_addsub(cpu, pc, ptr_A, ptr_A, true, true, bb);
			break;
		case INSTR_CMP:
			arch_6502_addsub(cpu, pc, ptr_A, NULL, true, false, bb);
			break;
		case INSTR_CPX:
			arch_6502_addsub(cpu, pc, ptr_X, NULL, true, false, bb);
			break;
		case INSTR_CPY:
			arch_6502_addsub(cpu, pc, ptr_Y, NULL, true, false, bb);
			break;

		/* increment/decrement */
		case INSTR_INX:	SET_NZ(LET(X,INC(R(X))));			break;
		case INSTR_INY:	SET_NZ(LET(Y,INC(R(Y))));			break;
		case INSTR_DEX:	SET_NZ(LET(X,DEC(R(X))));			break;
		case INSTR_DEY:	SET_NZ(LET(Y,DEC(R(Y))));			break;

		case INSTR_INC:	SET_NZ(STORE(INC(OPERAND),LOPERAND));			break;
		case INSTR_DEC:	SET_NZ(STORE(DEC(OPERAND),LOPERAND));			break;
		
		/* control flow */
		case INSTR_JMP:
			if (instraddmode[opcode].addmode == ADDMODE_IND) {
				Value *v = arch_6502_load_ram_16(cpu, false, CONST32(OPERAND_16), bb);
				new StoreInst(v, cpu->ptr_PC, bb);
			}
			break;
		case INSTR_JSR:	PUSH16(pc+2);						break;
		case INSTR_RTS:	STORE(ADD(PULL16, CONST16(1)), cpu->ptr_PC);	break;

		/* branch */
		case INSTR_BEQ:
		case INSTR_BNE:
		case INSTR_BCS:
		case INSTR_BCC:
		case INSTR_BMI:
		case INSTR_BPL:
		case INSTR_BVS:
		case INSTR_BVC:
			break;

		/* other */
		case INSTR_NOP:											break;
		case INSTR_BRK:	arch_6502_trap(cpu, pc, bb);			break;
		case INSTR_RTI:	arch_6502_trap(cpu, pc, bb);			break;
		case INSTR_XXX:	arch_6502_trap(cpu, pc, bb);			break;
	}

//printf("%s:%d opcode=%02X, addmode=%d, length=%d\n", __func__, __LINE__, opcode, instraddmode[opcode].addmode, length[instraddmode[opcode].addmode]);
	return length[instraddmode[opcode].addmode]+1;
}

static void
arch_6502_init(cpu_t *cpu)
{
	reg_6502_t *reg;
	reg = (reg_6502_t*)malloc(sizeof(reg_6502_t));
	reg->pc = 0;
	reg->a = 0;
	reg->x = 0;
	reg->y = 0;
	reg->s = 0xFF;
	reg->p = 0;
	cpu->reg = reg;

	cpu->pc_width = 16;
	cpu->count_regs_i8 = 5;
	cpu->count_regs_i16 = 0;
	cpu->count_regs_i32 = 0;
	cpu->count_regs_i64 = 0;
	cpu->reg_size = 8;

	cpu->is_little_endian = true;
	cpu->fp_reg = NULL;
	cpu->count_regs_f32 = 0;
	cpu->count_regs_f64 = 0;
	cpu->count_regs_f80 = 0;
	cpu->count_regs_f128 = 0;

	assert(offsetof(reg_6502_t, pc) == 5);
}

static void
arch_6502_emit_decode_reg(cpu_t *cpu, BasicBlock *bb)
{
	// declare flags
	ptr_N = new AllocaInst(getIntegerType(1), "N", bb);
	ptr_V = new AllocaInst(getIntegerType(1), "V", bb);
	ptr_D = new AllocaInst(getIntegerType(1), "D", bb);
	ptr_I = new AllocaInst(getIntegerType(1), "I", bb);
	ptr_Z = new AllocaInst(getIntegerType(1), "Z", bb);
	ptr_C = new AllocaInst(getIntegerType(1), "C", bb);

	// decode P
	Value *flags = new LoadInst(ptr_P, "", false, bb);
	arch_6502_flags_decode(flags, bb);
}

static void
arch_6502_spill_reg_state(cpu_t *cpu, BasicBlock *bb)
{
	Value *flags = arch_6502_flags_encode(bb);
	new StoreInst(flags, ptr_P, false, bb);
}

static addr_t
arch_6502_get_pc(cpu_t *, void *reg)
{
	return ((reg_6502_t*)reg)->pc;
}

static uint64_t
arch_6502_get_psr(cpu_t *, void *reg)
{
	return ((reg_6502_t*)reg)->p;
}

static int
arch_6502_get_reg(cpu_t *cpu, void *reg, unsigned reg_no, uint64_t *value)
{
	switch (reg_no) {
		case 0: *value = ((reg_6502_t *)reg)->a; break;
		case 1: *value = ((reg_6502_t *)reg)->x; break;
		case 2: *value = ((reg_6502_t *)reg)->y; break;
		case 3: *value = ((reg_6502_t *)reg)->s; break;
		default: return (-1);
	}
	return (0);
}

arch_func_t arch_func_6502 = {
	arch_6502_init,
	arch_6502_get_pc,
	arch_6502_emit_decode_reg,
	arch_6502_spill_reg_state,
	arch_6502_tag_instr,
	arch_6502_disasm_instr,
	arch_6502_recompile_cond,
	arch_6502_recompile_instr,
	// idbg support
	arch_6502_get_psr,
	arch_6502_get_reg,
	NULL
};
