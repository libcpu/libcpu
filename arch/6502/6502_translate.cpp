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
//#define P 0

/* these are the flags that aren't handled by the generic flag code */
#define ptr_D cpu->ptr_FLAG[D_SHIFT]
#define ptr_I cpu->ptr_FLAG[I_SHIFT]

#define OPCODE cpu->RAM[pc]
#define OPERAND_8 cpu->RAM[(pc+1)&0xFFFF]
#define OPERAND_16 ((cpu->RAM[(pc+1)&0xFFFF] | (cpu->RAM[(pc+2)&0xFFFF]<<8))&0xFFFF)

#define LOPERAND arch_6502_get_operand_lvalue(cpu, pc, bb)
#define OPERAND LOAD(LOPERAND)

#define GEP(a) GetElementPtrInst::Create(cpu->ptr_RAM, a, "", bb)

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

static int
get_addmode(uint8_t opcode) {
	switch (opcode) {
		case 0x0D:case 0x0E:case 0x20:case 0x2C:case 0x2D:case 0x2E:case 0x4C:case 0x4D:
		case 0x4E:case 0x6D:case 0x6E:case 0x8C:case 0x8D:case 0x8E:case 0xAC:case 0xAD:
		case 0xAE:case 0xCC:case 0xCD:case 0xCE:case 0xEC:case 0xED:case 0xEE:
			return ADDMODE_ABS;

		case 0x1D:case 0x1E:case 0x3D:case 0x3E:case 0x5D:case 0x5E:case 0x7D:case 0x7E:
		case 0x9D:case 0xBC:case 0xBD:case 0xDD:case 0xDE:case 0xFD:case 0xFE:
			return ADDMODE_ABSX;

		case 0x19:case 0x39:case 0x59:case 0x79:case 0x99:case 0xB9:case 0xBE:case 0xD9:
		case 0xF9:
			return ADDMODE_ABSY;

		case 0x0A:case 0x2A:case 0x4A:case 0x6A:
			return ADDMODE_ACC;

		case 0x10:case 0x30:case 0x50:case 0x70:case 0x90:case 0xB0:case 0xD0:case 0xF0:
			return ADDMODE_BRA;

		case 0x09:case 0x29:case 0x49:case 0x69:case 0xA0:case 0xA2:case 0xA9:case 0xC0:
		case 0xC9:case 0xE0:case 0xE9:
			return ADDMODE_IMM;

		case 0x6C:
			return ADDMODE_IND;

		case 0x01:case 0x21:case 0x41:case 0x61:case 0x81:case 0xA1:case 0xC1:case 0xE1:
			return ADDMODE_INDX;

		case 0x11:case 0x31:case 0x51:case 0x71:case 0x91:case 0xB1:case 0xD1:case 0xF1:
			return ADDMODE_INDY;

		case 0x05:case 0x06:case 0x24:case 0x25:case 0x26:case 0x45:case 0x46:case 0x65:
		case 0x66:case 0x84:case 0x85:case 0x86:case 0xA4:case 0xA5:case 0xA6:case 0xC4:
		case 0xC5:case 0xC6:case 0xE4:case 0xE5:case 0xE6:
			return ADDMODE_ZP;

		case 0x15:case 0x16:case 0x35:case 0x36:case 0x55:case 0x56:case 0x75:case 0x76:
		case 0x94:case 0x95:case 0xB4:case 0xB5:case 0xD5:case 0xD6:case 0xF5:case 0xF6:
			return ADDMODE_ZPX;

		case 0x96:
		case 0xB6:
			return ADDMODE_ZPY;
	}
	return ADDMODE_IMPL;
}
static int
get_instr(uint8_t opcode) {
	switch (opcode) {
		case 0x00:
			return INSTR_BRK;
		case 0xEA:
			return INSTR_NOP;

		case 0x8A:
			return INSTR_TXA;
		case 0x98:
			return INSTR_TYA;
		case 0xA8:
			return INSTR_TAY;
		case 0xAA:
			return INSTR_TAX;

		case 0x9A:
			return INSTR_TXS;
		case 0xBA:
			return INSTR_TSX;

		case 0xA1:case 0xA5:case 0xA9:case 0xAD:case 0xB1:case 0xB5:case 0xB9:case 0xBD:
			return INSTR_LDA;
		case 0xA2:case 0xA6:case 0xAE:case 0xB6:case 0xBE:
			return INSTR_LDX;
		case 0xA0:case 0xA4:case 0xAC:case 0xB4:case 0xBC:
			return INSTR_LDY;
		case 0x81:case 0x85:case 0x8D:case 0x91:case 0x95:case 0x99:case 0x9D:
			return INSTR_STA;
		case 0x84:case 0x8C:case 0x94:
			return INSTR_STY;
		case 0x86:case 0x8E:case 0x96:
			return INSTR_STX;

		case 0x21:case 0x25:case 0x29:case 0x2D:case 0x31:case 0x35:case 0x39:case 0x3D:
			return INSTR_AND;
		case 0x01:case 0x05:case 0x09:case 0x0D:case 0x11:case 0x15:case 0x19:case 0x1D:
			return INSTR_ORA;
		case 0x41:case 0x45:case 0x49:case 0x4D:case 0x51:case 0x55:case 0x59:case 0x5D:
			return INSTR_EOR;

		case 0x24:case 0x2C:
			return INSTR_BIT;

		case 0x06:case 0x0A:case 0x0E:case 0x16:case 0x1E:
			return INSTR_ASL;
		case 0x46:case 0x4A:case 0x4E:case 0x56:case 0x5E:
			return INSTR_LSR;
		case 0x26:case 0x2A:case 0x2E:case 0x36:case 0x3E:
			return INSTR_ROL;
		case 0x66:case 0x6A:case 0x6E:case 0x76:case 0x7E:
			return INSTR_ROR;

		case 0xE6:case 0xEE:case 0xF6:case 0xFE:
			return INSTR_INC;
		case 0xC6:case 0xCE:case 0xD6:case 0xDE:
			return INSTR_DEC;
		case 0xE8:
			return INSTR_INX;
		case 0xC8:
			return INSTR_INY;
		case 0xCA:
			return INSTR_DEX;
		case 0x88:
			return INSTR_DEY;

		case 0x61:case 0x65:case 0x69:case 0x6D:case 0x71:case 0x75:case 0x79:case 0x7D:
			return INSTR_ADC;
		case 0xE1:case 0xE5:case 0xE9:case 0xED:case 0xF1:case 0xF5:case 0xF9:case 0xFD:
			return INSTR_SBC;
		case 0xC1:case 0xC5:case 0xC9:case 0xCD:case 0xD1:case 0xD5:case 0xD9:case 0xDD:
			return INSTR_CMP;
		case 0xE0:case 0xE4:case 0xEC:
			return INSTR_CPX;
		case 0xC0:case 0xC4:case 0xCC:
			return INSTR_CPY;

		case 0x08:
			return INSTR_PHP;
		case 0x28:
			return INSTR_PLP;
		case 0x48:
			return INSTR_PHA;
		case 0x68:
			return INSTR_PLA;

		case 0x10:
			return INSTR_BPL;
		case 0x30:
			return INSTR_BMI;
		case 0x50:
			return INSTR_BVC;
		case 0x70:
			return INSTR_BVS;
		case 0x90:
			return INSTR_BCC;
		case 0xB0:
			return INSTR_BCS;
		case 0xD0:
			return INSTR_BNE;
		case 0xF0:
			return INSTR_BEQ;

		case 0x18:
			return INSTR_CLC;
		case 0x58:
			return INSTR_CLI;
		case 0xB8:
			return INSTR_CLV;
		case 0xD8:
			return INSTR_CLD;
		case 0x38:
			return INSTR_SEC;
		case 0x78:
			return INSTR_SEI;
		case 0xF8:
			return INSTR_SED;

		case 0x20:
			return INSTR_JSR;
		case 0x4C:
		case 0x6C:
			return INSTR_JMP;

		case 0x40:
			return INSTR_RTI;
		case 0x60:
			return INSTR_RTS;
	}
	return INSTR_XXX;
}

static Value *
arch_6502_get_operand_lvalue(cpu_t *cpu, addr_t pc, BasicBlock* bb) {
	int am = get_addmode(OPCODE);
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
		case INSTR_ADC:	SET_NZ(ADC(ptr_A, ptr_A, OPERAND, LOAD(cpu->ptr_C)));		break;
		case INSTR_SBC:	SET_NZ(ADC(ptr_A, ptr_A, COM(OPERAND), LOAD(cpu->ptr_C)));	break;
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

	return length[get_addmode(opcode)]+1;
}

