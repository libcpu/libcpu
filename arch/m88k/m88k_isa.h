#ifndef __m88k_isa_h
#define __m88k_isa_h

typedef uint32_t m88k_address_t;

typedef uint32_t m88k_reg_t;

typedef enum _m88k_opcode {
	M88K_OPC_ILLEGAL = 0,

	M88K_OPC_ADD,
	M88K_OPC_ADDU,

	M88K_OPC_SUB,
	M88K_OPC_SUBU,

	M88K_OPC_MULS,
	M88K_OPC_MULU,
	M88K_OPC_MULU_D,

	M88K_OPC_DIVS,
	M88K_OPC_DIVU,
	M88K_OPC_DIVU_D,

	M88K_OPC_MASK,
	M88K_OPC_MASK_U,
	M88K_OPC_AND,
	M88K_OPC_AND_C,
	M88K_OPC_AND_U,
	M88K_OPC_OR,
	M88K_OPC_OR_C,
	M88K_OPC_OR_U,
	M88K_OPC_XOR,
	M88K_OPC_XOR_C,
	M88K_OPC_XOR_U,
	M88K_OPC_ROT,
	M88K_OPC_MAK,
	M88K_OPC_SET,
	M88K_OPC_CLR,
	M88K_OPC_EXT,
	M88K_OPC_EXTU,
	M88K_OPC_FF0,
	M88K_OPC_FF1,

	M88K_OPC_CMP,

	M88K_OPC_LDA,
	M88K_OPC_LDA_H,
	M88K_OPC_LDA_D,
	M88K_OPC_LDA_X,

	M88K_OPC_LD,
	M88K_OPC_LD_B,
	M88K_OPC_LD_BU,
	M88K_OPC_LD_H,
	M88K_OPC_LD_HU,
	M88K_OPC_LD_D,
	M88K_OPC_LD_X,

	M88K_OPC_ST,
	M88K_OPC_ST_B,
	M88K_OPC_ST_H,
	M88K_OPC_ST_D,
	M88K_OPC_ST_X,

	M88K_OPC_XMEM,
	M88K_OPC_XMEM_BU,

	M88K_OPC_JMP,
	M88K_OPC_JMP_N,
	M88K_OPC_JSR,
	M88K_OPC_JSR_N,
	M88K_OPC_BR,
	M88K_OPC_BR_N,
	M88K_OPC_BSR,
	M88K_OPC_BSR_N,
	M88K_OPC_BB0,
	M88K_OPC_BB0_N,
	M88K_OPC_BB1,
	M88K_OPC_BB1_N,
	M88K_OPC_BCND,
	M88K_OPC_BCND_N,

	M88K_OPC_TB0,
	M88K_OPC_TB1,
	M88K_OPC_TBND,
	M88K_OPC_TCND,

	M88K_OPC_LDCR,
	M88K_OPC_STCR,
	M88K_OPC_XCR,

	M88K_OPC_ILLOP1,
	M88K_OPC_ILLOP2,
	M88K_OPC_ILLOP3,
	M88K_OPC_RTE,

	/* FP Opcodes (SFU1) */

	M88K_OPC_MOV,
	M88K_OPC_FCMP,
	M88K_OPC_FCMPU,
	M88K_OPC_FLT,
	M88K_OPC_FCVT,
	M88K_OPC_INT,
	M88K_OPC_NINT,
	M88K_OPC_TRNC,
	M88K_OPC_FADD,
	M88K_OPC_FSUB,
	M88K_OPC_FMUL,
	M88K_OPC_FDIV,
	M88K_OPC_FSQRT,

	M88K_OPC_FLDCR,
	M88K_OPC_FSTCR,
	M88K_OPC_FXCR,

	/* Vectorial Opcodes */
	M88K_OPC_PADD,
	M88K_OPC_PCMP,
	M88K_OPC_PMUL,
	M88K_OPC_PPACK,
	M88K_OPC_PROT,
	M88K_OPC_PSUB,
	M88K_OPC_PUNPK,

	/* Pseudo Opcodes */
	M88K_POPC_BFMT, /* Bit Format */
	M88K_POPC_SFU1, /* Floating Point Unit */
	M88K_POPC_SFU2, /* Vectorial Unit */
	M88K_POPC_EXT1, /* More Opcodes ! */
	M88K_POPC_EXT2  /* More Opcodes ! */
} m88k_opcode_t;

typedef enum _m88k_bcmp {
	M88K_BCMP_00 = 0,
	M88K_BCMP_01,

	M88K_BCMP_EQ, M88K_BCMP_NE,
	M88K_BCMP_GT, M88K_BCMP_LE,
	M88K_BCMP_LT, M88K_BCMP_GE,
	M88K_BCMP_HI, M88K_BCMP_LS,
	M88K_BCMP_LO, M88K_BCMP_HS,
	M88K_BCMP_BE, M88K_BCMP_NB,
	M88K_BCMP_HE, M88K_BCMP_NH
} m88k_bcmp_t;
 
typedef enum _m88k_bcnd_t {
	M88K_BCND_NEVER		= 0,
	M88K_BCND_GT0		= 1,
	M88K_BCND_EQ0		= 2,
	M88K_BCND_GE0		= 3,
	M88K_BCND_4			= 4,  /* rs != 0x80000000 && rs < 0 */
	M88K_BCND_5			= 5,  /* (rs & 0x7fffffff) != 0 */
	M88K_BCND_6			= 6,  /* rs != 0x80000000 && rs <= 0 */
	M88K_BCND_7			= 7,  /* rs != 0x80000000 */
	M88K_BCND_8			= 8,  /* rs == 0x80000000 */
	M88K_BCND_9			= 9,  /* rs > 0 || rs == 0x80000000 */
	M88K_BCND_10		= 10, /* (rs & 0x7fffffff) == 0 */
	M88K_BCND_11		= 11, /* rs >= 0 || rs == 0x80000000 */
	M88K_BCND_LT0		= 12,
	M88K_BCND_NE0		= 13,
	M88K_BCND_LE0		= 14,
	M88K_BCND_ALWAYS	= 15
} m88k_bcnd_t;

typedef enum _m88k_insnfmt {
	M88K_FMT_NONE = 0,

	M88K_IFMT_REG,
	M88K_IFMT_MEM,
	M88K_IFMT_XMEM, /* fpu register destination */
	M88K_BRFMT_OFF,
	M88K_BRFMT_COND,
	M88K_BRFMT_BIT,
	M88K_TFMT_REG,
	M88K_TFMT_XREG,
	M88K_TFMT_REGS, /* shifted form */
	M88K_TFMT_XREGS, /* shifted form */
	M88K_TFMT_XFR,
	M88K_TFMT_REGX,
	M88K_BFMT_REG,
	M88K_BFMT_TST,
	M88K_CFMT_REG,
	M88K_CFMT_GER,
	M88K_CFMT_REG2
} m88k_insnfmt_t;

typedef enum _m88k_carry {
	M88K_CARRY_NONE	= 0,
	M88K_CARRY_OUT	= 1,
	M88K_CARRY_IN	= 2
} m88k_carry_t;

typedef struct _m88k_grf {
	union {
		struct {
			uint32_t r0;
			uint32_t r1;
			uint32_t r2;
			uint32_t r3;
			uint32_t r4;
			uint32_t r5;
			uint32_t r6;
			uint32_t r7;
			uint32_t r8;
			uint32_t r9;
			uint32_t r10;
			uint32_t r11;
			uint32_t r12;
			uint32_t r13;
			uint32_t r14;
			uint32_t r15;
			uint32_t r16;
			uint32_t r17;
			uint32_t r18;
			uint32_t r19;
			uint32_t r20;
			uint32_t r21;
			uint32_t r22;
			uint32_t r23;
			uint32_t r24;
			uint32_t r25;
			uint32_t r26;
			uint32_t r27;
			uint32_t r28;
			uint32_t r29;
			uint32_t r30;
			uint32_t r31;
		};
		uint32_t r[32];
	};
	uint32_t psr;
	uint32_t sxip; /* Execution IP */
} m88k_grf_t;

typedef union _m88k_xrf_t {
	struct {
		fp80_reg_t x0;
		fp80_reg_t x1;
		fp80_reg_t x2;
		fp80_reg_t x3;
		fp80_reg_t x4;
		fp80_reg_t x5;
		fp80_reg_t x6;
		fp80_reg_t x7;
		fp80_reg_t x8;
		fp80_reg_t x9;
		fp80_reg_t x10;
		fp80_reg_t x11;
		fp80_reg_t x12;
		fp80_reg_t x13;
		fp80_reg_t x14;
		fp80_reg_t x15;
		fp80_reg_t x16;
		fp80_reg_t x17;
		fp80_reg_t x18;
		fp80_reg_t x19;
		fp80_reg_t x20;
		fp80_reg_t x21;
		fp80_reg_t x22;
		fp80_reg_t x23;
		fp80_reg_t x24;
		fp80_reg_t x25;
		fp80_reg_t x26;
		fp80_reg_t x27;
		fp80_reg_t x28;
		fp80_reg_t x29;
		fp80_reg_t x30;
		fp80_reg_t x31;
	};
	fp80_reg_t x[32];
} m88k_xrf_t;

#endif  /* !__m88k_isa_h */
