#include "libcpu.h"
#include "6502_isa.h"
#include "6502_cc.h"
#include "frontend.h"
#include "libcpu_6502.h"

#define A 0
#define X 1
#define Y 2
#define S 3
#define ptr_A cpu->ptr_gpr[A]
#define ptr_X cpu->ptr_gpr[X]
#define ptr_Y cpu->ptr_gpr[Y]
#define ptr_S cpu->ptr_gpr[S]
//#define P 0
#define ptr_P cpu->ptr_xr[0]

#define ptr_C ((cc6502_t*)cpu->feptr)->ptr_C
#define ptr_Z ((cc6502_t*)cpu->feptr)->ptr_Z
#define ptr_I ((cc6502_t*)cpu->feptr)->ptr_I
#define ptr_D ((cc6502_t*)cpu->feptr)->ptr_D
#define ptr_V ((cc6502_t*)cpu->feptr)->ptr_V
#define ptr_N ((cc6502_t*)cpu->feptr)->ptr_N

#define OPCODE cpu->RAM[pc]
#define OPERAND_8 cpu->RAM[(pc+1)&0xFFFF]
#define OPERAND_16 ((cpu->RAM[(pc+1)&0xFFFF] | (cpu->RAM[(pc+2)&0xFFFF]<<8))&0xFFFF)

#define SET_NZ(a) { Value *t = a; LET1(ptr_Z, ICMP_EQ(t, CONST8(0))); LET1(ptr_N, ICMP_SLT(t, CONST8(0))); }

#define LOPERAND arch_6502_get_operand_lvalue(cpu, pc, bb)
#define OPERAND LOAD(LOPERAND)
#define LET1(a,b) new StoreInst(b, a, false, bb)

#define GEP(a) GetElementPtrInst::Create(cpu->ptr_RAM, a, "", bb)
#define LOAD(a) new LoadInst(a, "", false, bb)

#define TOS GEP(OR(ZEXT32(R(S)), CONST32(0x0100)))
#define PUSH(v) { STORE(v, TOS); LET(S,DEC(R(S))); }
#define PULL (LET(S,INC(R(S))), LOAD(TOS))
#define PUSH16(v) { PUSH(CONST8((v) >> 8)); PUSH(CONST8((v) & 0xFF)); }
// Because of a GCC evaluation order problem, the PULL16
// macro needs to be expanded.
#define PULL16 arch_6502_pull16(cpu, bb)

static inline Value *
arch_6502_pull16(cpu_t *cpu, BasicBlock *bb)
{
	Value *lo = PULL;
	Value *hi = PULL;
	return (OR(ZEXT16(lo), SHL(ZEXT16(hi), CONST16(8))));
}

#define LOAD_RAM8(a) LOAD(GEP(a))
/* explicit little endian load of 16 bits */
#define LOAD_RAM16(a) OR(ZEXT16(LOAD_RAM8(a)), SHL(ZEXT16(LOAD_RAM8(ADD(a, CONST32(1)))), CONST16(8)))

static Value *
arch_6502_get_operand_lvalue(cpu_t *cpu, addr_t pc, BasicBlock* bb) {
	int am = instraddmode[OPCODE].addmode;
	Value *index_register_before;
	Value *index_register_after;
	bool is_indirect;
	bool is_8bit_base;

	switch (am) {
		case ADDMODE_ACC:
			return ptr_A;
		case ADDMODE_BRA:
		case ADDMODE_IMPL:
			return NULL;
		case ADDMODE_IMM:
			{
			Value *ptr_temp = new AllocaInst(getIntegerType(8), "temp", bb);
			new StoreInst(CONST8(OPERAND_8), ptr_temp, bb);
			return ptr_temp;
			}
	}

	is_indirect = ((am == ADDMODE_IND) || (am == ADDMODE_INDX) || (am == ADDMODE_INDY));
	is_8bit_base = !((am == ADDMODE_ABS) || (am == ADDMODE_ABSX) || (am == ADDMODE_ABSY));
	index_register_before = NULL;
	if ((am == ADDMODE_ABSX) || (am == ADDMODE_INDX) || (am == ADDMODE_ZPX))
		index_register_before = ptr_X;
	if ((am == ADDMODE_ABSY) || (am == ADDMODE_ZPY))
		index_register_before = ptr_Y;
	index_register_after = (am == ADDMODE_INDY)? ptr_Y : NULL;

#if 0
	log("pc = %x\n", pc);
	log("index_register_before = %x\n", index_register_before);
	log("index_register_after = %x\n", index_register_after);
	log("is_indirect = %x\n", is_indirect);
	log("is_8bit_base = %x\n", is_8bit_base);
#endif

	/* create base constant */
	uint16_t base = is_8bit_base? (OPERAND_8):(OPERAND_16);
	Value *ea = CONST32(base);

	if (index_register_before)
		ea = ADD(ZEXT32(LOAD(index_register_before)), ea);

	/* wrap around in zero page */
	if (is_8bit_base)
		ea = AND(ea, CONST32(0x00FF));
	else if (base >= 0xFF00) /* wrap around in memory */
		ea = AND(ea, CONST32(0xFFFF));

	if (is_indirect)
		ea = ZEXT32(LOAD_RAM16(ea));

	if (index_register_after)
		ea = ADD(ZEXT32(LOAD(index_register_after)), ea);

	return GEP(ea);
}

static Value *
arch_6502_store(Value *v, Value *a, BasicBlock *bb)
{
	new StoreInst(v, a, bb);
	return v;
}

#define STORE(v,a) arch_6502_store(v, a, bb)
//#define STORE(v,a) (new StoreInst(v, a, bb),v) // why does this not work?

static void
arch_6502_trap(cpu_t *cpu, addr_t pc, BasicBlock *bb)
{
	Value* v_pc = CONST16(pc);
	new StoreInst(v_pc, cpu->ptr_PC, bb);
	ReturnInst::Create(_CTX(), CONST32(JIT_RETURN_TRAP), bb);
}

static Value *
arch_6502_shiftrotate(cpu_t *cpu, Value *l, bool left, bool rotate,
	BasicBlock *bb)
{
	Value *c;
	Value *v = LOAD(l);

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
	
	LET1(ptr_C, c);
	return STORE(v, l);
}

#define SHIFTROTATE(l,left,rotate) arch_6502_shiftrotate(cpu,l,left,rotate,bb)

/*
 * XXX TODO: consider changing code to avoid 16 bit arithmetic:
 *     while this works ok for 8 bit, it doesn't scale. M88K and ARM
 *     do it differently already.
 *     we should use llvm.uadd.with.overflow.*
 */
static Value *
arch_6502_adc(cpu_t *cpu, Value *dreg, Value *sreg, Value *v, Value *c,
	BasicBlock *bb)
{
	/* calculate intermediate result */
	Value *v1 = ADD(ADD(ZEXT16(LOAD(sreg)), ZEXT16(v)), ZEXT16(c));

	/* get C */
	STORE(TRUNC1(LSHR(v1, CONST16(8))), ptr_C);

	/* get result */
	v1 = TRUNC8(v1);

	if (dreg)
		STORE(v1, dreg);

	return v1;
}
#define ADC(dreg,sreg,v,c) arch_6502_adc(cpu,dreg,sreg,v,c,bb)

#define N_SHIFT 7
#define V_SHIFT 6
#define D_SHIFT 3
#define I_SHIFT 2
#define Z_SHIFT 1
#define C_SHIFT 0

static Value *
arch_6502_flags_encode(cpu_t *cpu, BasicBlock *bb)
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
arch_6502_flags_decode(cpu_t *cpu, Value *flags, BasicBlock *bb)
{
	arch_decode_bit(flags, ptr_N, N_SHIFT, 8, bb);
	arch_decode_bit(flags, ptr_V, V_SHIFT, 8, bb);
	arch_decode_bit(flags, ptr_D, D_SHIFT, 8, bb);
	arch_decode_bit(flags, ptr_I, I_SHIFT, 8, bb);
	arch_decode_bit(flags, ptr_Z, Z_SHIFT, 8, bb);
	arch_decode_bit(flags, ptr_C, C_SHIFT, 8, bb);
}

Value *
arch_6502_translate_cond(cpu_t *cpu, addr_t pc, BasicBlock *bb) {
	uint8_t opcode = cpu->RAM[pc];
log("%s:%d pc=%llx opcode=%x\n", __func__, __LINE__, pc, opcode);

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
arch_6502_translate_instr(cpu_t *cpu, addr_t pc, BasicBlock *bb) {
	uint8_t opcode = cpu->RAM[pc];

//log("%s:%d PC=$%04X\n", __func__, __LINE__, pc);
//	log("\naddmode = %i\n", instraddmode[opcode].addmode);

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
		case INSTR_PHP:	PUSH(arch_6502_flags_encode(cpu, bb));	break;
		case INSTR_PLA:	SET_NZ(LET(A,PULL));			break;
		case INSTR_PLP:	arch_6502_flags_decode(cpu, PULL, bb);	break;

		/* shift */
		case INSTR_ASL:	SET_NZ(SHIFTROTATE(LOPERAND, true, false));			break;
		case INSTR_LSR:	SET_NZ(SHIFTROTATE(LOPERAND, false, false));		break;
		case INSTR_ROL:	SET_NZ(SHIFTROTATE(LOPERAND, true, true));			break;
		case INSTR_ROR:	SET_NZ(SHIFTROTATE(LOPERAND, false, true));			break;

		/* bit logic */
		case INSTR_AND:	SET_NZ(LET(A,AND(R(A),OPERAND)));			break;
		case INSTR_ORA:	SET_NZ(LET(A,OR(R(A),OPERAND)));			break;
		case INSTR_EOR:	SET_NZ(LET(A,XOR(R(A),OPERAND)));			break;
		case INSTR_BIT:	SET_NZ(OPERAND);							break;

		/* arithmetic */
		case INSTR_ADC:	SET_NZ(ADC(ptr_A, ptr_A, OPERAND, LOAD(ptr_C)));		break;
		case INSTR_SBC:	SET_NZ(ADC(ptr_A, ptr_A, COM(OPERAND), LOAD(ptr_C)));	break;
		case INSTR_CMP:	SET_NZ(ADC(NULL, ptr_A, COM(OPERAND), CONST1(1)));		break;
		case INSTR_CPX:	SET_NZ(ADC(NULL, ptr_X, COM(OPERAND), CONST1(1)));		break;
		case INSTR_CPY:	SET_NZ(ADC(NULL, ptr_Y, COM(OPERAND), CONST1(1)));		break;

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
				Value *v = LOAD_RAM16(CONST32(OPERAND_16));
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

//log("%s:%d opcode=%02X, addmode=%d, length=%d\n", __func__, __LINE__, opcode, instraddmode[opcode].addmode, length[instraddmode[opcode].addmode]);
	return length[instraddmode[opcode].addmode]+1;
}

static void
arch_6502_init(cpu_t *cpu, cpu_archinfo_t *info, cpu_archrf_t *rf)
{
	assert(offsetof(reg_6502_t, pc) == 5);

	// Basic Information
	info->name = "6502";
	info->full_name = "MOS 6502";

	// This architecture is little endian, override any user flag.
	info->common_flags = CPU_FLAG_ENDIAN_LITTLE;
	// The byte and word size are both 8bits.
	// The address size is 16bits.
	info->byte_size = 8;
	info->word_size = 8;
	info->address_size = 16;
	// There are 4 8-bit GPRs
	info->register_count[CPU_REG_GPR] = 4;
	info->register_size[CPU_REG_GPR] = info->word_size;
	// There is also 1 extra register to handle PSR.
	info->register_count[CPU_REG_XR] = 1;
	info->register_size[CPU_REG_XR] = 8;

	reg_6502_t *reg;
	reg = (reg_6502_t*)malloc(sizeof(reg_6502_t));
	reg->pc = 0;
	reg->a = 0;
	reg->x = 0;
	reg->y = 0;
	reg->s = 0xFF;
	reg->p = 0;

	rf->pc = &reg->pc;
	rf->grf = reg;

	// allocate space for CC flags.
	cpu->feptr = malloc(sizeof(cc6502_t));
	assert(cpu->feptr != NULL);
}

static void
arch_6502_done(cpu_t *cpu)
{
	free(cpu->feptr);
	free(cpu->rf.grf);
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
	arch_6502_flags_decode(cpu, flags, bb);
}

static void
arch_6502_spill_reg_state(cpu_t *cpu, BasicBlock *bb)
{
	Value *flags = arch_6502_flags_encode(cpu, bb);
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
	arch_6502_done,
	arch_6502_get_pc,
	arch_6502_emit_decode_reg,
	arch_6502_spill_reg_state,
	arch_6502_tag_instr,
	arch_6502_disasm_instr,
	arch_6502_translate_cond,
	arch_6502_translate_instr,
	// idbg support
	arch_6502_get_psr,
	arch_6502_get_reg,
	NULL
};
