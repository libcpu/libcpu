#include "libcpu.h"
#include "6502_isa.h"
#include "frontend.h"

#define A 0
#define X 1
#define Y 2
#define S 3
#define ptr_A cpu->ptr_gpr[A]
#define ptr_X cpu->ptr_gpr[X]
#define ptr_Y cpu->ptr_gpr[Y]
#define ptr_S cpu->ptr_gpr[S]

/* these are the flags that aren't handled by the generic flag code */
#define ptr_D cpu->ptr_FLAG[D_SHIFT]
#define ptr_I cpu->ptr_FLAG[I_SHIFT]

#define GEP(a) GetElementPtrInst::Create(cpu->ptr_RAM, a, "", bb)

#define LOAD_RAM8(a) LOAD(GEP(a))
/* explicit little endian load of 16 bits */
#define LOAD_RAM16(a) OR(ZEXT16(LOAD_RAM8(a)), SHL(ZEXT16(LOAD_RAM8(ADD(a, CONST32(1)))), CONST16(8)))

#define OPERAND_8 cpu->RAM[(pc+1)&0xFFFF]
#define OPERAND_16 ((cpu->RAM[(pc+1)&0xFFFF] | (cpu->RAM[(pc+2)&0xFFFF]<<8))&0xFFFF)

static Value *
arch_6502_get_operand_lvalue(cpu_t *cpu, addr_t pc, BasicBlock* bb) {
	int am = get_addmode(cpu->RAM[pc]);
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

static void
arch_6502_trap(cpu_t *cpu, addr_t pc, BasicBlock *bb)
{
	Value* v_pc = CONST16(pc);
	new StoreInst(v_pc, cpu->ptr_PC, bb);
	ReturnInst::Create(_CTX(), CONST32(JIT_RETURN_TRAP), bb);
}

Value *
arch_6502_translate_cond(cpu_t *cpu, addr_t pc, BasicBlock *bb) {
	uint8_t opcode = cpu->RAM[pc];
log("%s:%d pc=%llx opcode=%x\n", __func__, __LINE__, pc, opcode);

	switch (get_instr(opcode)) {
		case INSTR_BEQ: /* Z */		return CC_EQ;
		case INSTR_BNE: /* !Z */	return CC_NE;
		case INSTR_BCS: /* C */		return CC_CS;
		case INSTR_BCC: /* !C */	return CC_CC;
		case INSTR_BMI: /* N */		return CC_MI;
		case INSTR_BPL: /* !N */	return CC_PL;
		case INSTR_BVS: /* V */		return CC_VS;
		case INSTR_BVC: /* !V */	return CC_VC;
		default:					return NULL; /* no condition; should not be reached */
	}
}

#define LOPERAND arch_6502_get_operand_lvalue(cpu, pc, bb)
#define OPERAND LOAD(LOPERAND)

/* stack operations */
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

int
arch_6502_translate_instr(cpu_t *cpu, addr_t pc, BasicBlock *bb) {
	uint8_t opcode = cpu->RAM[pc];

//log("%s:%d PC=$%04X\n", __func__, __LINE__, pc);

	switch (get_instr(opcode)) {
		/* flags */
		case INSTR_CLC:	LET1(cpu->ptr_C, FALSE);				break;
		case INSTR_CLD:	LET1(ptr_D, FALSE);				break;
		case INSTR_CLI:	LET1(ptr_I, FALSE);				break;
		case INSTR_CLV:	LET1(cpu->ptr_V, FALSE);				break;
		case INSTR_SEC:	LET1(cpu->ptr_C, TRUE);				break;
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
		case INSTR_PHP:	PUSH(arch_flags_encode(cpu, bb));	break;
		case INSTR_PLA:	SET_NZ(LET(A,PULL));			break;
		case INSTR_PLP:	arch_flags_decode(cpu, PULL, bb);	break;

		/* shift */
		case INSTR_ASL:	SET_NZ(SHIFTROTATE(LOPERAND, LOPERAND, true, false));	break;
		case INSTR_LSR:	SET_NZ(SHIFTROTATE(LOPERAND, LOPERAND, false, false));	break;
		case INSTR_ROL:	SET_NZ(SHIFTROTATE(LOPERAND, LOPERAND, true, true));	break;
		case INSTR_ROR:	SET_NZ(SHIFTROTATE(LOPERAND, LOPERAND, false, true));	break;

		/* bit logic */
		case INSTR_AND:	SET_NZ(LET(A,AND(R(A),OPERAND)));			break;
		case INSTR_ORA:	SET_NZ(LET(A,OR(R(A),OPERAND)));			break;
		case INSTR_EOR:	SET_NZ(LET(A,XOR(R(A),OPERAND)));			break;
		case INSTR_BIT:	SET_NZ(OPERAND);							break;

		/* arithmetic */
		case INSTR_ADC:	SET_NZ(ADC(ptr_A, ptr_A, OPERAND, true, false));		break;
		case INSTR_SBC:	SET_NZ(ADC(ptr_A, ptr_A, COM(OPERAND), true, false));	break;
		case INSTR_CMP:	SET_NZ(ADC(NULL, ptr_A, COM(OPERAND), false, true));		break;
		case INSTR_CPX:	SET_NZ(ADC(NULL, ptr_X, COM(OPERAND), false, true));		break;
		case INSTR_CPY:	SET_NZ(ADC(NULL, ptr_Y, COM(OPERAND), false, true));		break;

		/* increment/decrement */
		case INSTR_INX:	SET_NZ(LET(X,INC(R(X))));			break;
		case INSTR_INY:	SET_NZ(LET(Y,INC(R(Y))));			break;
		case INSTR_DEX:	SET_NZ(LET(X,DEC(R(X))));			break;
		case INSTR_DEY:	SET_NZ(LET(Y,DEC(R(Y))));			break;

		case INSTR_INC:	SET_NZ(STORE(INC(OPERAND),LOPERAND));			break;
		case INSTR_DEC:	SET_NZ(STORE(DEC(OPERAND),LOPERAND));			break;
		
		/* control flow */
		case INSTR_JMP:
			if (get_addmode(opcode) == ADDMODE_IND) {
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

	return get_length(get_addmode(opcode));
}

