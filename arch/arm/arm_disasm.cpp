#include "libcpu.h"
#include "frontend.h"
#include "arm_internal.h"

/**********************************************************************/
/* START dsemu Disassembler - libcpu code below!                      */
/**********************************************************************/
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef int64_t __int64;

/**************************************************************************
* DSemu: ARM Generic register structure and constants (arm.h)             *
* Released under the terms of the BSD Public Licence                      *
* Imran Nazar (tf@oopsilon.com), 2004                                     *
**************************************************************************/

#ifndef __ARM_H_
#define __ARM_H_

//#include "defs.h"
//#include "cache.h"

//---ARM registers structure-----------------------------------------------

typedef struct {
  u32 r[16];
  u32 flags[8];
  u32 cpsr, spsr[7];
  u32 curop; u32 curmode;
  __int64 clock;

  u32 tmp1, tmp2, tmp3, tmp4, tmpc;

  u32 r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15;
  u32 r8fiq, r9fiq, r10fiq, r11fiq, r12fiq, r13fiq, r14fiq;
  u32 r13svc, r14svc, r13abt, r14abt, r13irq, r14irq, r13und, r14und;

//  RAMWORD pipe0, pipe1, pipe2;

  u32 cp15[16];

  // Coprocessor 15 regions
  u32 cp15_regions[8];
  u32 cp15_tcm_base[2];

  // Coprocessor 15 control register
  u32 cp15_control_register;

} ARMREGS;

#define ARMREG_R0    0
#define ARMREG_R1    1
#define ARMREG_R2    2
#define ARMREG_R3    3
#define ARMREG_R4    4
#define ARMREG_R5    5
#define ARMREG_R6    6
#define ARMREG_R7    7
#define ARMREG_R8    8
#define ARMREG_R9    9
#define ARMREG_R10  10
#define ARMREG_R11  11
#define ARMREG_R12  12
#define ARMREG_R13  13
#define ARMREG_R14  14
#define ARMREG_R15  15
#define ARMREG_FN   16
#define ARMREG_FZ   17
#define ARMREG_FC   18
#define ARMREG_FV   19
#define ARMREG_FT   20
#define ARMREG_FQ   21


#define ARMREG_CPSR 24
#define ARMREG_SUSR 25
#define ARMREG_SSYS 26
#define ARMREG_SFIQ 27
#define ARMREG_SIRQ 28
#define ARMREG_SSVC 29
#define ARMREG_SABT 30
#define ARMREG_SUND 31
#define ARMREG_OP   32
#define ARMREG_MODE 33
#define ARMREG_T1   36
#define ARMREG_T2   37
#define ARMREG_T3   38
#define ARMREG_T4   39
#define ARMREG_TC   40
#define ARMREG_USER 41

//---ARM Flags register----------------------------------------------------

#define ARMS_N 0x80000000            // Negative
#define ARMS_Z 0x40000000            // Zero
#define ARMS_C 0x20000000            // Carry
#define ARMS_V 0x10000000            // Overflow
#define ARMS_Q 0x08000000            // DSP Overflow/Saturated
#define ARMS_I 0x00000080            // Interrupts Disabled
#define ARMS_F 0x00000040            // Fast Interrupts Disabled
#define ARMS_T 0x00000020            // Thumb Mode
#define ARMS_M 0x0000001F            // CPSR Mode Mask

#define ARMFLAG_N 0
#define ARMFLAG_Z 1
#define ARMFLAG_C 2
#define ARMFLAG_V 3
#define ARMFLAG_T 4

#define ARMS_M_USR 0x00000010        // User mode
#define ARMS_M_FIQ 0x00000011        // Fast Interrupt mode
#define ARMS_M_IRQ 0x00000012        // Interrupt mode
#define ARMS_M_SVC 0x00000013        // Supervisor mode
#define ARMS_M_ABT 0x00000017        // Abort mode
#define ARMS_M_UND 0x0000001B        // Undefined-instruction mode
#define ARMS_M_SYS 0x0000001F        // System mode

#define ARMMD_USR 0
#define ARMMD_SYS 1
#define ARMMD_FIQ 2
#define ARMMD_IRQ 3
#define ARMMD_SVC 4
#define ARMMD_ABT 5
#define ARMMD_UND 6

//---Exception vectors-----------------------------------------------------

#define ARMX_RST 0x00000000          // Reset vector
#define ARMX_UND 0x00000004          // Undefined-instruction vector
#define ARMX_SWI 0x00000008          // SWI vector
#define ARMX_ABI 0x0000000C          // Instruction abort vector
#define ARMX_ABD 0x00000010          // Data abort vector
#define ARMX_RSV 0x00000014          // [Reserved]
#define ARMX_IRQ 0x00000018          // IRQ vector
#define ARMX_FIQ 0x0000001C          // Fast IRQ vector

//---High-address exception vectors----------------------------------------

#define ARMXHI_RST 0xFFFF0000
#define ARMXHI_UND 0xFFFF0004
#define ARMXHI_SWI 0xFFFF0008
#define ARMXHI_ABI 0xFFFF000C
#define ARMXHI_ABD 0xFFFF0010
#define ARMXHI_RSV 0xFFFF0014
#define ARMXHI_IRQ 0xFFFF0018
#define ARMXHI_FIQ 0xFFFF001C

//---Opcode field masks----------------------------------------------------

#define ARMMSK_COND 0xF0000000        // Condition held in top 4 bits
#define ARMMSK_OP   0x0FF00000

#define ARMMSK_RN   0x000F0000
#define ARMMSK_RD   0x0000F000
#define ARMMSK_RS   0x00000F00
#define ARMMSK_RM   0x0000000F

#define ARMTMSK_RD  0x0007
#define ARMTMSK_RN  0x0038
#define ARMTMSK_RM  0x01C0

//---Condition table (indexed by condition/z/c/n/v)------------------------
/*
u8 ARMcond[16][2][2][2][2]={
    0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1, //EQ
    1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0, //NE
    0,0,0,0,1,1,1,1,0,0,0,0,1,1,1,1, //CS
    1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0, //CC
    0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1, //MI
    1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0, //PL
    0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1, //VS
    1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0, //VC
    0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0, //HI
    1,1,1,1,0,0,0,0,1,1,1,1,1,1,1,1, //LS
    1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1, //GE
    0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0, //LT
    1,0,0,1,1,0,0,1,0,0,0,0,0,0,0,0, //GT
    0,1,1,0,0,1,1,0,1,1,1,1,1,1,1,1, //LE
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //AL
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //NV
};
*/
#endif//__ARM_H_

/*** EOF:arm.h ***********************************************************/
/**************************************************************************
* DSemu: ARM9 disassembler definitions and string table (arm9dasm.h)      *
* Released under the terms of the BSD Public Licence                      *
* Imran Nazar (tf@oopsilon.com), 2004                                     *
**************************************************************************/

#ifndef __ARM9DASM_H_
#define __ARM9DASM_H_

//#include "defs.h"
//#include "arm.h"

u32 arm9dasmtmp1, arm9dasmtmp2, arm9dasmtmp3, arm9dasmtmp4;
char arm9dasmstr[40];

ARMREGS arm9reg;

const char *ARM9DASMcond[]={
    "EQ","NE","CS","CC",
    "MI","PL","VS","VC",
    "HI","LS","GE","LT",
    "GT","LE","","NV",
};

#define ARM9DASM_RO ((op&0x00F00000)>>20)
#define ARM9DASM_RP ((op&0x0000000F0)>>4)

#define ARM9DASM_RN (op&0x000F0000)>>16
#define ARM9DASM_RD (op&0x0000F000)>>12
#define ARM9DASM_RM (op&0x0000000F)
#define ARM9DASM_RS (op&0x00000F00)>>8

#define TMB7DASM_RD (op&0x0007)
#define TMB7DASM_RN (op&0x0038)>>3
#define TMB7DASM_RM (op&0x01C0)>>6
#define TMB7DASM_RS (op&0x0700)>>8
#define TMB7DASM_RDH (op&0x0007)|((op&0x0080)>>4)
#define TMB7DASM_RNH ((op&0x0078)>>3)
#define TMB7DASM_IMM5 (op&0x07C0)>>6
#define TMB7DASM_IMM8 (op&0x00FF)
#define TMB7DASM_IMM7 (op&0x007F)
#define TMB7DASM_IMM11 (op&0x07FF)
char *ARM9DASM(u32);

void ARM9DASMun(u32);

void ARM9DASMb(u32), ARM9DASMbreg(u32);

void ARM9DASMreg(u32), ARM9DASMimm(u32);
void ARM9DASMlli(u32), ARM9DASMllr(u32);
void ARM9DASMlri(u32), ARM9DASMlrr(u32);
void ARM9DASMari(u32), ARM9DASMarr(u32);
void ARM9DASMrri(u32), ARM9DASMrrr(u32);

void ARM9DASMofim(u32), ARM9DASMofip(u32);
void ARM9DASMofrm(u32), ARM9DASMofrp(u32);
void ARM9DASMprim(u32), ARM9DASMprip(u32);
void ARM9DASMprrm(u32), ARM9DASMprrp(u32);
void ARM9DASMptim(u32), ARM9DASMptip(u32);
void ARM9DASMptrm(u32), ARM9DASMptrp(u32);

void ARM9DASMlmofim(u32), ARM9DASMlmofip(u32);
void ARM9DASMlmprim(u32), ARM9DASMlmprip(u32);
void ARM9DASMlmptim(u32), ARM9DASMlmptip(u32);

void ARM9DASMofrmll(u32), ARM9DASMofrpll(u32);
void ARM9DASMofrmlr(u32), ARM9DASMofrplr(u32);
void ARM9DASMofrmar(u32), ARM9DASMofrpar(u32);
void ARM9DASMofrmrr(u32), ARM9DASMofrprr(u32);
void ARM9DASMprrmll(u32), ARM9DASMprrpll(u32);
void ARM9DASMprrmlr(u32), ARM9DASMprrplr(u32);
void ARM9DASMprrmar(u32), ARM9DASMprrpar(u32);
void ARM9DASMprrmrr(u32), ARM9DASMprrprr(u32);
void ARM9DASMptrmll(u32), ARM9DASMptrpll(u32);
void ARM9DASMptrmlr(u32), ARM9DASMptrplr(u32);
void ARM9DASMptrmar(u32), ARM9DASMptrpar(u32);
void ARM9DASMptrmrr(u32), ARM9DASMptrprr(u32);

void ARM9DASMlm(u32);

void ARM9DASMmrsrs(u32), ARM9DASMmrsrc(u32);
void ARM9DASMmsrrs(u32), ARM9DASMmsrrc(u32);
void ARM9DASMmsris(u32), ARM9DASMmsric(u32);

void ARM9DASMswp(u32);
void ARM9DASMswi(u32);

void ARM9DASMmul(u32);
void ARM9DASMmla(u32);
void ARM9DASMmull(u32);

void ARM9DASMcpd(u32), ARM9DASMmcr(u32);

void Thumb9DASMimm5(u32), Thumb9DASMimm5shft(u32);
void Thumb9DASMimm3(u32), Thumb9DASMimm8(u32);
void Thumb9DASMimm7(u32);
void Thumb9DASMbc(u32),   Thumb9DASMb(u32);
void Thumb9DASMbl(u32),   Thumb9DASMbx(u32);
void Thumb9DASMldm(u32),  Thumb9DASMh(u32);
void Thumb9DASMbkpt(u32), Thumb9DASMswi(u32);
void Thumb9DASMpc(u32),   Thumb9DASMsp(u32);
void Thumb9DASMdp1(u32),  Thumb9DASMdp2(u32);
void Thumb9DASMdp3(u32),  Thumb9DASMdp4(u32);
void Thumb9DASMreg(u32),  Thumb9DASMund(u32);

//---And so it begins------------------------------------------------------

typedef struct {
    char op[16];
    void (*addr)(u32);
} ARM9DASMOPDESC;

//---Opcode function pointer table. Indexed by 27-20|7-4-------------------

const ARM9DASMOPDESC arm9dasmops[]={
    //0x00
    {"AND%s %s",ARM9DASMlli      },
    {"AND%s %s",ARM9DASMllr      },
    {"AND%s %s",ARM9DASMlri      },
    {"AND%s %s",ARM9DASMlrr      },
    {"AND%s %s",ARM9DASMari      },
    {"AND%s %s",ARM9DASMarr      },
    {"AND%s %s",ARM9DASMrri      },
    {"AND%s %s",ARM9DASMrrr      },
    {"AND%s %s",ARM9DASMlli      },
    {"MUL%s %s",ARM9DASMmul      },
    {"AND%s %s",ARM9DASMlri      },
    {"STRH%s %s",ARM9DASMptrm    },
    {"AND%s %s",ARM9DASMari      },
    {"UNP%s %s",ARM9DASMun       },  // Defined in 5E!
    {"AND%s %s",ARM9DASMrri      },
    {"UNP%s %s",ARM9DASMun       },  // Defined in 5E!

    //0x01
    {"AND%sS %s",ARM9DASMlli     },
    {"AND%sS %s",ARM9DASMllr     },
    {"AND%sS %s",ARM9DASMlri     },
    {"AND%sS %s",ARM9DASMlrr     },
    {"AND%sS %s",ARM9DASMari     },
    {"AND%sS %s",ARM9DASMarr     },
    {"AND%sS %s",ARM9DASMrri     },
    {"AND%sS %s",ARM9DASMrrr     },
    {"AND%sS %s",ARM9DASMlli     },
    {"MUL%sS %s",ARM9DASMmul     },
    {"AND%sS %s",ARM9DASMlri     },
    {"LDRH%s %s",ARM9DASMptrm    },
    {"AND%sS %s",ARM9DASMari     },
    {"LDRSB%s %s",ARM9DASMptrm   },
    {"AND%sS %s",ARM9DASMrri     },
    {"LDRSH%s %s",ARM9DASMptrm   },

    //0x02
    {"EOR%s %s",ARM9DASMlli      },
    {"EOR%s %s",ARM9DASMllr      },
    {"EOR%s %s",ARM9DASMlri      },
    {"EOR%s %s",ARM9DASMlrr      },
    {"EOR%s %s",ARM9DASMari      },
    {"EOR%s %s",ARM9DASMarr      },
    {"EOR%s %s",ARM9DASMrri      },
    {"EOR%s %s",ARM9DASMrrr      },
    {"EOR%s %s",ARM9DASMlli      },
    {"MLA%s %s",ARM9DASMmla      },
    {"EOR%s %s",ARM9DASMlri      },
    {"STRH%s %s",ARM9DASMptrm    },  // Post-indexed with W bit
    {"EOR%s %s",ARM9DASMari      },
    {"UNP%s %s",ARM9DASMun       },  // Post-indexed with W bit
    {"EOR%s %s",ARM9DASMrri      },
    {"OPR%s %s",ARM9DASMun       },  // Post-indexed with W bit [reindexed]

    //0x03
    {"EOR%sS %s",ARM9DASMlli     },
    {"EOR%sS %s",ARM9DASMllr     },
    {"EOR%sS %s",ARM9DASMlri     },
    {"EOR%sS %s",ARM9DASMlrr     },
    {"EOR%sS %s",ARM9DASMari     },
    {"EOR%sS %s",ARM9DASMarr     },
    {"EOR%sS %s",ARM9DASMrri     },
    {"EOR%sS %s",ARM9DASMrrr     },
    {"EOR%sS %s",ARM9DASMlli     },
    {"MLA%sS %s",ARM9DASMmla     },
    {"EOR%sS %s",ARM9DASMlri     },
    {"LDRH%s %s",ARM9DASMptrm    },  // Post-indexed with W bit
    {"EOR%sS %s",ARM9DASMari     },
    {"LDRSB%s %s",ARM9DASMptrm   },  // Post-indexed with W bit
    {"EOR%sS %s",ARM9DASMrri     },
    {"LDRSH%s %s",ARM9DASMptrm   },  // Post-indexed with W bit

    //0x04
    {"SUB%s %s",ARM9DASMlli      },
    {"SUB%s %s",ARM9DASMllr      },
    {"SUB%s %s",ARM9DASMlri      },
    {"SUB%s %s",ARM9DASMlrr      },
    {"SUB%s %s",ARM9DASMari      },
    {"SUB%s %s",ARM9DASMarr      },
    {"SUB%s %s",ARM9DASMrri      },
    {"SUB%s %s",ARM9DASMrrr      },
    {"SUB%s %s",ARM9DASMlli      },
    {"UNP%s %s",ARM9DASMun       },  // Unallocated
    {"SUB%s %s",ARM9DASMlri      },
    {"STRH%s %s",ARM9DASMlmptim  },
    {"SUB%s %s",ARM9DASMari      },
    {"UNP%s %s",ARM9DASMun       },  // Defined in 5E!
    {"SUB%s %s",ARM9DASMrri      },
    {"UNP%s %s",ARM9DASMun       },  // Defined in 5E!

    //0x05
    {"SUB%sS %s",ARM9DASMlli     },
    {"SUB%sS %s",ARM9DASMllr     },
    {"SUB%sS %s",ARM9DASMlri     },
    {"SUB%sS %s",ARM9DASMlrr     },
    {"SUB%sS %s",ARM9DASMari     },
    {"SUB%sS %s",ARM9DASMarr     },
    {"SUB%sS %s",ARM9DASMrri     },
    {"SUB%sS %s",ARM9DASMrrr     },
    {"SUB%sS %s",ARM9DASMlli     },
    {"UNP%s %s",ARM9DASMun       },  // Unallocated
    {"SUB%sS %s",ARM9DASMlri     },
    {"LDRH%s %s",ARM9DASMlmptim  },
    {"SUB%sS %s",ARM9DASMari     },
    {"LDRSB%s %s",ARM9DASMlmptim },
    {"SUB%sS %s",ARM9DASMrri     },
    {"LDRSH%s %s",ARM9DASMlmptim },

    //0x06
    {"RSB%s %s",ARM9DASMlli      },
    {"RSB%s %s",ARM9DASMllr      },
    {"RSB%s %s",ARM9DASMlri      },
    {"RSB%s %s",ARM9DASMlrr      },
    {"RSB%s %s",ARM9DASMari      },
    {"RSB%s %s",ARM9DASMarr      },
    {"RSB%s %s",ARM9DASMrri      },
    {"RSB%s %s",ARM9DASMrrr      },
    {"RSB%s %s",ARM9DASMlli      },
    {"UNP%s %s",ARM9DASMun       },  // Unallocated
    {"RSB%s %s",ARM9DASMlri      },
    {"STRH%s %s",ARM9DASMlmptim  },  // Post-indexed with W bit
    {"RSB%s %s",ARM9DASMari      },
    {"UNP%s %s",ARM9DASMun       },  // Post-indexed with W bit
    {"RSB%s %s",ARM9DASMrri      },
    {"UNP%s %s",ARM9DASMun       },  // Post-indexed with W bit

    //0x07
    {"RSB%sS %s",ARM9DASMlli     },
    {"RSB%sS %s",ARM9DASMllr     },
    {"RSB%sS %s",ARM9DASMlri     },
    {"RSB%sS %s",ARM9DASMlrr     },
    {"RSB%sS %s",ARM9DASMari     },
    {"RSB%sS %s",ARM9DASMarr     },
    {"RSB%sS %s",ARM9DASMrri     },
    {"RSB%sS %s",ARM9DASMrrr     },
    {"RSB%sS %s",ARM9DASMlli     },
    {"UNP%s %s",ARM9DASMun       },  // Unallocated
    {"RSB%sS %s",ARM9DASMlri     },
    {"LDRH%s %s",ARM9DASMlmptim  },  // Post-indexed with W bit
    {"RSB%sS %s",ARM9DASMari     },
    {"LDRSB%s %s",ARM9DASMlmptim },  // Post-indexed with W bit
    {"RSB%sS %s",ARM9DASMrri     },
    {"LDRSH%s %s",ARM9DASMlmptim },  // Post-indexed with W bit

    //0x08
    {"ADD%s %s",ARM9DASMlli      },
    {"ADD%s %s",ARM9DASMllr      },
    {"ADD%s %s",ARM9DASMlri      },
    {"ADD%s %s",ARM9DASMlrr      },
    {"ADD%s %s",ARM9DASMari      },
    {"ADD%s %s",ARM9DASMarr      },
    {"ADD%s %s",ARM9DASMrri      },
    {"ADD%s %s",ARM9DASMrrr      },
    {"ADD%s %s",ARM9DASMlli      },
    {"UMULL%s %s",ARM9DASMmull   },
    {"ADD%s %s",ARM9DASMlri      },
    {"STRH%s %s",ARM9DASMptrp    },
    {"ADD%s %s",ARM9DASMari      },
    {"UNP%s %s",ARM9DASMun       },  // Defined in 5E!
    {"ADD%s %s",ARM9DASMrri      },
    {"UNP%s %s",ARM9DASMun       },  // Defined in 5E!

    //0x09
    {"ADD%sS %s",ARM9DASMlli     },
    {"ADD%sS %s",ARM9DASMllr     },
    {"ADD%sS %s",ARM9DASMlri     },
    {"ADD%sS %s",ARM9DASMlrr     },
    {"ADD%sS %s",ARM9DASMari     },
    {"ADD%sS %s",ARM9DASMarr     },
    {"ADD%sS %s",ARM9DASMrri     },
    {"ADD%sS %s",ARM9DASMrrr     },
    {"ADD%sS %s",ARM9DASMlli     },
    {"UMULL%sS %s",ARM9DASMmull  },
    {"ADD%sS %s",ARM9DASMlri     },
    {"LDRH%s %s",ARM9DASMptrp    },
    {"ADD%sS %s",ARM9DASMari     },
    {"LDRSB%s %s",ARM9DASMptrp   },
    {"ADD%sS %s",ARM9DASMrri     },
    {"LDRSH%s %s",ARM9DASMptrp   },

    //0x0A
    {"ADC%s %s",ARM9DASMlli      },
    {"ADC%s %s",ARM9DASMllr      },
    {"ADC%s %s",ARM9DASMlri      },
    {"ADC%s %s",ARM9DASMlrr      },
    {"ADC%s %s",ARM9DASMari      },
    {"ADC%s %s",ARM9DASMarr      },
    {"ADC%s %s",ARM9DASMrri      },
    {"ADC%s %s",ARM9DASMrrr      },
    {"ADC%s %s",ARM9DASMlli      },
    {"UMLAL%s %s",ARM9DASMmull   },
    {"ADC%s %s",ARM9DASMlri      },
    {"STRH%s %s",ARM9DASMptrp    },  // Post-indexed with W bit
    {"ADC%s %s",ARM9DASMari      },
    {"UNP%s %s",ARM9DASMun       },  // Post-indexed with W bit
    {"ADC%s %s",ARM9DASMrri      },
    {"UNP%s %s",ARM9DASMun       },  // Post-indexed with W bit

    //0x0B
    {"ADC%sS %s",ARM9DASMlli     },
    {"ADC%sS %s",ARM9DASMllr     },
    {"ADC%sS %s",ARM9DASMlri     },
    {"ADC%sS %s",ARM9DASMlrr     },
    {"ADC%sS %s",ARM9DASMari     },
    {"ADC%sS %s",ARM9DASMarr     },
    {"ADC%sS %s",ARM9DASMrri     },
    {"ADC%sS %s",ARM9DASMrrr     },
    {"ADC%sS %s",ARM9DASMlli     },
    {"UMLAL%sS %s",ARM9DASMmull  },
    {"ADC%sS %s",ARM9DASMlri     },
    {"LDRH%s %s",ARM9DASMptrm    },  // Post-indexed with W bit
    {"ADC%sS %s",ARM9DASMari     },
    {"LDRSB%s %s",ARM9DASMptrm   },  // Post-indexed with W bit
    {"ADC%sS %s",ARM9DASMrri     },
    {"LDRSH%s %s",ARM9DASMptrm   },  // Post-indexed with W bit

    //0x0C
    {"SBC%s %s",ARM9DASMlli      },
    {"SBC%s %s",ARM9DASMllr      },
    {"SBC%s %s",ARM9DASMlri      },
    {"SBC%s %s",ARM9DASMlrr      },
    {"SBC%s %s",ARM9DASMari      },
    {"SBC%s %s",ARM9DASMarr      },
    {"SBC%s %s",ARM9DASMrri      },
    {"SBC%s %s",ARM9DASMrrr      },
    {"SBC%s %s",ARM9DASMlli      },
    {"SMULL%s %s",ARM9DASMmull   },
    {"SBC%s %s",ARM9DASMlri      },
    {"STRH%s %s",ARM9DASMlmptip  },
    {"SBC%s %s",ARM9DASMari      },
    {"UNP%s %s",ARM9DASMun       },  // Defined in 5E!
    {"SBC%s %s",ARM9DASMrri      },
    {"UNP%s %s",ARM9DASMun       },  // Defined in 5E!

    //0x0D
    {"SBC%sS %s",ARM9DASMlli     },
    {"SBC%sS %s",ARM9DASMllr     },
    {"SBC%sS %s",ARM9DASMlri     },
    {"SBC%sS %s",ARM9DASMlrr     },
    {"SBC%sS %s",ARM9DASMari     },
    {"SBC%sS %s",ARM9DASMarr     },
    {"SBC%sS %s",ARM9DASMrri     },
    {"SBC%sS %s",ARM9DASMrrr     },
    {"SBC%sS %s",ARM9DASMlli     },
    {"SMULL%sS %s",ARM9DASMmull  },
    {"SBC%sS %s",ARM9DASMlri     },
    {"LDRH%s %s",ARM9DASMlmptip  },
    {"SBC%sS %s",ARM9DASMari     },
    {"LDRSB%s %s",ARM9DASMlmptip },
    {"SBC%sS %s",ARM9DASMrri     },
    {"LDRSH%s %s",ARM9DASMlmptip },

    //0x0E
    {"RSC%s %s",ARM9DASMlli      },
    {"RSC%s %s",ARM9DASMllr      },
    {"RSC%s %s",ARM9DASMlri      },
    {"RSC%s %s",ARM9DASMlrr      },
    {"RSC%s %s",ARM9DASMari      },
    {"RSC%s %s",ARM9DASMarr      },
    {"RSC%s %s",ARM9DASMrri      },
    {"RSC%s %s",ARM9DASMrrr      },
    {"RSC%s %s",ARM9DASMlli      },
    {"SMLAL%s %s",ARM9DASMmull   },
    {"RSC%s %s",ARM9DASMlri      },
    {"STRH%s %s",ARM9DASMlmptip  },  // Post-indexed with W bit
    {"RSC%s %s",ARM9DASMari      },
    {"UNP%s %s",ARM9DASMun       },  // Post-indexed with W bit
    {"RSC%s %s",ARM9DASMrri      },
    {"UNP%s %s",ARM9DASMun       },  // Post-indexed with W bit

    //0x0F
    {"RSC%sS %s",ARM9DASMlli     },
    {"RSC%sS %s",ARM9DASMllr     },
    {"RSC%sS %s",ARM9DASMlri     },
    {"RSC%sS %s",ARM9DASMlrr     },
    {"RSC%sS %s",ARM9DASMari     },
    {"RSC%sS %s",ARM9DASMarr     },
    {"RSC%sS %s",ARM9DASMrri     },
    {"RSC%sS %s",ARM9DASMrrr     },
    {"RSC%sS %s",ARM9DASMlli     },
    {"SMLAL%sS %s",ARM9DASMmull  },
    {"RSC%sS %s",ARM9DASMlri     },
    {"LDRH%s %s",ARM9DASMlmptip  },  // Post-indexed with W bit
    {"RSC%sS %s",ARM9DASMari     },
    {"LDRSB%s %s",ARM9DASMlmptip },  // Post-indexed with W bit
    {"RSC%sS %s",ARM9DASMrri     },
    {"LDRSH%s %s",ARM9DASMlmptip },  // Post-indexed with W bit

    //0x10
    {"MRS%s %s",ARM9DASMmrsrc    },
    {"UNP%s %s",ARM9DASMun       },  // MRS with non-zero 7-4
    {"UNP%s %s",ARM9DASMun       },  // MRS with non-zero 7-4
    {"UNP%s %s",ARM9DASMun       },  // MRS with non-zero 7-4
    {"UNP%s %s",ARM9DASMun       },  // MRS with non-zero 7-4
    {"UNP%s %s",ARM9DASMun       },  // Defined in ARM5E!
    {"UNP%s %s",ARM9DASMun       },  // MRS with non-zero 7-4
    {"UNP%s %s",ARM9DASMun       },  // MRS with non-zero 7-4
    {"SMLABB%s %s",ARM9DASMmla   },  // ++ARM5E
    {"SWP%s %s",ARM9DASMswp      },
    {"SMLATB%s %s",ARM9DASMmla   },  // ++ARM5E
    {"STRH%s %s",ARM9DASMofrm    },
    {"SMLABT%s %s",ARM9DASMmla   },  // ++ARM5E
    {"UNP%s %s",ARM9DASMun       },  // Defined in ARM5E!
    {"SMLATT%s %s",ARM9DASMmla   },  // ++ARM5E
    {"UNP%s %s",ARM9DASMun       },  // Defined in ARM5E!

    //0x11
    {"TST%sS %s",ARM9DASMlli     },
    {"TST%sS %s",ARM9DASMllr     },
    {"TST%sS %s",ARM9DASMlri     },
    {"TST%sS %s",ARM9DASMlrr     },
    {"TST%sS %s",ARM9DASMari     },
    {"TST%sS %s",ARM9DASMarr     },
    {"TST%sS %s",ARM9DASMrri     },
    {"TST%sS %s",ARM9DASMrrr     },
    {"TST%sS %s",ARM9DASMlli     },
    {"UNP%s %s",ARM9DASMun       },  // Unallocated
    {"TST%sS %s",ARM9DASMlri     },
    {"LDRH%s %s",ARM9DASMofrm    },
    {"TST%sS %s",ARM9DASMari     },
    {"LDRSB%s %s",ARM9DASMofrm   },
    {"TST%sS %s",ARM9DASMrri     },
    {"LDRSH%s %s",ARM9DASMofrm   },

    //0x12
    {"MSR%s %s",ARM9DASMmsrrc    },
    {"BX%s %s",ARM9DASMbreg      },
    {"UNP%s %s",ARM9DASMun       },  // Unallocated
    {"BLX%s %s",ARM9DASMbreg     },
    {"UNP%s %s",ARM9DASMun       },  // Unallocated
    {"UNP%s %s",ARM9DASMun       },  // Defined in ARM5E!
    {"UNP%s %s",ARM9DASMun       },  // Unallocated
    {"BKPT%s %s",ARM9DASMun      },  // Defined in ARM5E!
    {"UNP%s %s",ARM9DASMun       },  // Defined in ARM5E!
    {"UNP%s %s",ARM9DASMun       },  // Unallocated
    {"UNP%s %s",ARM9DASMun       },  // Defined in ARM5E!
    {"STRH%s %s",ARM9DASMprrm    },
    {"UNP%s %s",ARM9DASMun       },  // Defined in ARM5E!
    {"UNP%s %s",ARM9DASMun       },  // Defined in ARM5E!
    {"UNP%s %s",ARM9DASMun       },  // Defined in ARM5E!
    {"UNP%s %s",ARM9DASMun       },  // Defined in ARM5E!

    //0x13
    {"TEQ%sS %s",ARM9DASMlli     },
    {"TEQ%sS %s",ARM9DASMllr     },
    {"TEQ%sS %s",ARM9DASMlri     },
    {"TEQ%sS %s",ARM9DASMlrr     },
    {"TEQ%sS %s",ARM9DASMari     },
    {"TEQ%sS %s",ARM9DASMarr     },
    {"TEQ%sS %s",ARM9DASMrri     },
    {"TEQ%sS %s",ARM9DASMrrr     },
    {"TEQ%sS %s",ARM9DASMlli     },
    {"UNP%s %s",ARM9DASMun       },  // Unallocated
    {"TEQ%sS %s",ARM9DASMlri     },
    {"LDRH%s %s",ARM9DASMprrm    },
    {"TEQ%sS %s",ARM9DASMari     },
    {"LDRSB%s %s",ARM9DASMprrm   },
    {"TEQ%sS %s",ARM9DASMrri     },
    {"LDRSH%s %s",ARM9DASMprrm   },

    //0x14
    {"MRS%s %s",ARM9DASMmrsrs    },
    {"UNP%s %s",ARM9DASMun       },  // Unallocated
    {"UNP%s %s",ARM9DASMun       },  // Unallocated
    {"UNP%s %s",ARM9DASMun       },  // Unallocated
    {"UNP%s %s",ARM9DASMun       },  // Unallocated
    {"UNP%s %s",ARM9DASMun       },  // Defined in ARM5E!
    {"UNP%s %s",ARM9DASMun       },  // Unallocated
    {"UNP%s %s",ARM9DASMun       },  // Unallocated
    {"UNP%s %s",ARM9DASMun       },  // Defined in ARM5E!
    {"SWPB%s %s",ARM9DASMswp     },
    {"UNP%s %s",ARM9DASMun       },  // Defined in ARM5E!
    {"STRH%s %s",ARM9DASMlmofim  },
    {"UNP%s %s",ARM9DASMun       },  // Defined in ARM5E!
    {"UNP%s %s",ARM9DASMun       },  // Defined in ARM5E!
    {"UNP%s %s",ARM9DASMun       },  // Defined in ARM5E!
    {"UNP%s %s",ARM9DASMun       },  // Defined in ARM5E!

    //0x15
    {"CMP%sS %s",ARM9DASMlli     },
    {"CMP%sS %s",ARM9DASMllr     },
    {"CMP%sS %s",ARM9DASMlri     },
    {"CMP%sS %s",ARM9DASMlrr     },
    {"CMP%sS %s",ARM9DASMari     },
    {"CMP%sS %s",ARM9DASMarr     },
    {"CMP%sS %s",ARM9DASMrri     },
    {"CMP%sS %s",ARM9DASMrrr     },
    {"CMP%sS %s",ARM9DASMlli     },
    {"UNP%s %s",ARM9DASMun       },  // Unallocated
    {"CMP%sS %s",ARM9DASMlri     },
    {"LDRH%s %s",ARM9DASMlmofim  },
    {"CMP%sS %s",ARM9DASMari     },
    {"LDRSB%s %s",ARM9DASMlmofim },
    {"CMP%sS %s",ARM9DASMrri     },
    {"LDRSH%s %s",ARM9DASMlmofim },

    //0x16
    {"MSR%s %s",ARM9DASMmsrrs    },
    {"UNP%s %s",ARM9DASMun       },  // Unallocated
    {"UNP%s %s",ARM9DASMun       },  // Unallocated
    {"UNP%s %s",ARM9DASMun       },  // Unallocated
    {"UNP%s %s",ARM9DASMun       },  // Unallocated
    {"UNP%s %s",ARM9DASMun       },  // Defined in ARM5E!
    {"UNP%s %s",ARM9DASMun       },  // Unallocated
    {"UNP%s %s",ARM9DASMun       },  // Unallocated
    {"SMULBB%s %s",ARM9DASMmul   },  // ++ARM5E
    {"UNP%s %s",ARM9DASMun       },  // Unallocated
    {"SMULTB%s %s",ARM9DASMmul   },  // ++ARM5E
    {"STRH%s %s",ARM9DASMlmprim  },
    {"SMULBT%s %s",ARM9DASMmul   },  // ++ARM5E
    {"UNP%s %s",ARM9DASMun       },  // Defined in ARM5E!
    {"SMULTT%s %s",ARM9DASMmul   },  // ++ARM5E
    {"UNP%s %s",ARM9DASMun       },  // Defined in ARM5E!

    //0x17
    {"CMN%sS %s",ARM9DASMlli     },
    {"CMN%sS %s",ARM9DASMllr     },
    {"CMN%sS %s",ARM9DASMlri     },
    {"CMN%sS %s",ARM9DASMlrr     },
    {"CMN%sS %s",ARM9DASMari     },
    {"CMN%sS %s",ARM9DASMarr     },
    {"CMN%sS %s",ARM9DASMrri     },
    {"CMN%sS %s",ARM9DASMrrr     },
    {"CMN%sS %s",ARM9DASMlli     },
    {"UNP%s %s",ARM9DASMun       },  // Unallocated
    {"CMN%sS %s",ARM9DASMlri     },
    {"LDRH%s %s",ARM9DASMlmprim  },
    {"CMN%sS %s",ARM9DASMari     },
    {"LDRSB%s %s",ARM9DASMlmprim },
    {"CMN%sS %s",ARM9DASMrri     },
    {"LDRSH%s %s",ARM9DASMlmprim },

    //0x18
    {"ORR%s %s",ARM9DASMlli      },
    {"ORR%s %s",ARM9DASMllr      },
    {"ORR%s %s",ARM9DASMlri      },
    {"ORR%s %s",ARM9DASMlrr      },
    {"ORR%s %s",ARM9DASMari      },
    {"ORR%s %s",ARM9DASMarr      },
    {"ORR%s %s",ARM9DASMrri      },
    {"ORR%s %s",ARM9DASMrrr      },
    {"ORR%s %s",ARM9DASMlli      },
    {"UNP%s %s",ARM9DASMun       },  // Unallocated
    {"ORR%s %s",ARM9DASMlri      },
    {"STRH%s %s",ARM9DASMofrp    },
    {"ORR%s %s",ARM9DASMari      },
    {"UNP%s %s",ARM9DASMun       },  // Defined in 5E!
    {"ORR%s %s",ARM9DASMrri      },
    {"UNP%s %s",ARM9DASMun       },  // Defined in 5E!

    //0x19
    {"ORR%sS %s",ARM9DASMlli     },
    {"ORR%sS %s",ARM9DASMllr     },
    {"ORR%sS %s",ARM9DASMlri     },
    {"ORR%sS %s",ARM9DASMlrr     },
    {"ORR%sS %s",ARM9DASMari     },
    {"ORR%sS %s",ARM9DASMarr     },
    {"ORR%sS %s",ARM9DASMrri     },
    {"ORR%sS %s",ARM9DASMrrr     },
    {"ORR%sS %s",ARM9DASMlli     },
    {"UNP%s %s",ARM9DASMun       },  // Unallocated
    {"ORR%sS %s",ARM9DASMlri     },
    {"LDRH%s %s",ARM9DASMofrp    },
    {"ORR%sS %s",ARM9DASMari     },
    {"LDRSB%s %s",ARM9DASMofrp   },
    {"ORR%sS %s",ARM9DASMrri     },
    {"LDRSH%s %s",ARM9DASMofrp   },

    //0x1A
    {"MOV%s %s",ARM9DASMlli      },
    {"MOV%s %s",ARM9DASMllr      },
    {"MOV%s %s",ARM9DASMlri      },
    {"MOV%s %s",ARM9DASMlrr      },
    {"MOV%s %s",ARM9DASMari      },
    {"MOV%s %s",ARM9DASMarr      },
    {"MOV%s %s",ARM9DASMrri      },
    {"MOV%s %s",ARM9DASMrrr      },
    {"MOV%s %s",ARM9DASMlli      },
    {"UNP%s %s",ARM9DASMun       },  // Unallocated
    {"MOV%s %s",ARM9DASMlri      },
    {"STRH%s %s",ARM9DASMprrp    },
    {"MOV%s %s",ARM9DASMari      },
    {"UNP%s %s",ARM9DASMun       },  // Defined in 5E!
    {"MOV%s %s",ARM9DASMrri      },
    {"UNP%s %s",ARM9DASMun       },  // Defined in 5E!

    //0x1B
    {"MOV%sS %s",ARM9DASMlli     },
    {"MOV%sS %s",ARM9DASMllr     },
    {"MOV%sS %s",ARM9DASMlri     },
    {"MOV%sS %s",ARM9DASMlrr     },
    {"MOV%sS %s",ARM9DASMari     },
    {"MOV%sS %s",ARM9DASMarr     },
    {"MOV%sS %s",ARM9DASMrri     },
    {"MOV%sS %s",ARM9DASMrrr     },
    {"MOV%sS %s",ARM9DASMlli     },
    {"UNP%s %s",ARM9DASMun       },  // Unallocated
    {"MOV%sS %s",ARM9DASMlri     },
    {"LDRH%s %s",ARM9DASMprrp    },
    {"MOV%sS %s",ARM9DASMari     },
    {"LDRSB%s %s",ARM9DASMprrp   },
    {"MOV%sS %s",ARM9DASMrri     },
    {"LDRSH%s %s",ARM9DASMprrp   },

    //0x1C
    {"BIC%s %s",ARM9DASMlli      },
    {"BIC%s %s",ARM9DASMllr      },
    {"BIC%s %s",ARM9DASMlri      },
    {"BIC%s %s",ARM9DASMlrr      },
    {"BIC%s %s",ARM9DASMari      },
    {"BIC%s %s",ARM9DASMarr      },
    {"BIC%s %s",ARM9DASMrri      },
    {"BIC%s %s",ARM9DASMrrr      },
    {"BIC%s %s",ARM9DASMlli      },
    {"UNP%s %s",ARM9DASMun       },  // Unallocated
    {"BIC%s %s",ARM9DASMlri      },
    {"STRH%s %s",ARM9DASMlmofip  },
    {"BIC%s %s",ARM9DASMari      },
    {"UNP%s %s",ARM9DASMun       },  // Defined in 5E!
    {"BIC%s %s",ARM9DASMrri      },
    {"UNP%s %s",ARM9DASMun       },  // Defined in 5E!

    //0x1D
    {"BIC%sS %s",ARM9DASMlli     },
    {"BIC%sS %s",ARM9DASMllr     },
    {"BIC%sS %s",ARM9DASMlri     },
    {"BIC%sS %s",ARM9DASMlrr     },
    {"BIC%sS %s",ARM9DASMari     },
    {"BIC%sS %s",ARM9DASMarr     },
    {"BIC%sS %s",ARM9DASMrri     },
    {"BIC%sS %s",ARM9DASMrrr     },
    {"BIC%sS %s",ARM9DASMlli     },
    {"UNP%s %s",ARM9DASMun       },  // Unallocated
    {"BIC%sS %s",ARM9DASMlri     },
    {"LDRH%s %s",ARM9DASMlmofip  },
    {"BIC%sS %s",ARM9DASMari     },
    {"LDRSB%s %s",ARM9DASMlmofip },
    {"BIC%sS %s",ARM9DASMrri     },
    {"LDRSH%s %s",ARM9DASMlmofip },

    //0x1E
    {"MVN%s %s",ARM9DASMlli      },
    {"MVN%s %s",ARM9DASMllr      },
    {"MVN%s %s",ARM9DASMlri      },
    {"MVN%s %s",ARM9DASMlrr      },
    {"MVN%s %s",ARM9DASMari      },
    {"MVN%s %s",ARM9DASMarr      },
    {"MVN%s %s",ARM9DASMrri      },
    {"MVN%s %s",ARM9DASMrrr      },
    {"MVN%s %s",ARM9DASMlli      },
    {"UNP%s %s",ARM9DASMun       },  // Unallocated
    {"MVN%s %s",ARM9DASMlri      },
    {"STRH%s %s",ARM9DASMlmprip  },
    {"MVN%s %s",ARM9DASMari      },
    {"UNP%s %s",ARM9DASMun       },  // Defined in 5E!
    {"MVN%s %s",ARM9DASMrri      },
    {"UNP%s %s",ARM9DASMun       },  // Defined in 5E!

    //0x1F
    {"MVN%sS %s",ARM9DASMlli     },
    {"MVN%sS %s",ARM9DASMllr     },
    {"MVN%sS %s",ARM9DASMlri     },
    {"MVN%sS %s",ARM9DASMlrr     },
    {"MVN%sS %s",ARM9DASMari     },
    {"MVN%sS %s",ARM9DASMarr     },
    {"MVN%sS %s",ARM9DASMrri     },
    {"MVN%sS %s",ARM9DASMrrr     },
    {"MVN%sS %s",ARM9DASMlli     },
    {"UNP%s %s",ARM9DASMun       },  // Unallocated
    {"MVN%sS %s",ARM9DASMlri     },
    {"LDRH%s %s",ARM9DASMlmprip  },
    {"MVN%sS %s",ARM9DASMari     },
    {"LDRSB%s %s",ARM9DASMlmprip },
    {"MVN%sS %s",ARM9DASMrri     },
    {"LDRSH%s %s",ARM9DASMlmprip },

    //0x20
    {"AND%s %s",ARM9DASMimm      },
    {"AND%s %s",ARM9DASMimm      },
    {"AND%s %s",ARM9DASMimm      },
    {"AND%s %s",ARM9DASMimm      },
    {"AND%s %s",ARM9DASMimm      },
    {"AND%s %s",ARM9DASMimm      },
    {"AND%s %s",ARM9DASMimm      },
    {"AND%s %s",ARM9DASMimm      },
    {"AND%s %s",ARM9DASMimm      },
    {"AND%s %s",ARM9DASMimm      },
    {"AND%s %s",ARM9DASMimm      },
    {"AND%s %s",ARM9DASMimm      },
    {"AND%s %s",ARM9DASMimm      },
    {"AND%s %s",ARM9DASMimm      },
    {"AND%s %s",ARM9DASMimm      },
    {"AND%s %s",ARM9DASMimm      },

    //0x21
    {"AND%sS %s",ARM9DASMimm     },
    {"AND%sS %s",ARM9DASMimm     },
    {"AND%sS %s",ARM9DASMimm     },
    {"AND%sS %s",ARM9DASMimm     },
    {"AND%sS %s",ARM9DASMimm     },
    {"AND%sS %s",ARM9DASMimm     },
    {"AND%sS %s",ARM9DASMimm     },
    {"AND%sS %s",ARM9DASMimm     },
    {"AND%sS %s",ARM9DASMimm     },
    {"AND%sS %s",ARM9DASMimm     },
    {"AND%sS %s",ARM9DASMimm     },
    {"AND%sS %s",ARM9DASMimm     },
    {"AND%sS %s",ARM9DASMimm     },
    {"AND%sS %s",ARM9DASMimm     },
    {"AND%sS %s",ARM9DASMimm     },
    {"AND%sS %s",ARM9DASMimm     },

    //0x22
    {"EOR%s %s",ARM9DASMimm      },
    {"EOR%s %s",ARM9DASMimm      },
    {"EOR%s %s",ARM9DASMimm      },
    {"EOR%s %s",ARM9DASMimm      },
    {"EOR%s %s",ARM9DASMimm      },
    {"EOR%s %s",ARM9DASMimm      },
    {"EOR%s %s",ARM9DASMimm      },
    {"EOR%s %s",ARM9DASMimm      },
    {"EOR%s %s",ARM9DASMimm      },
    {"EOR%s %s",ARM9DASMimm      },
    {"EOR%s %s",ARM9DASMimm      },
    {"EOR%s %s",ARM9DASMimm      },
    {"EOR%s %s",ARM9DASMimm      },
    {"EOR%s %s",ARM9DASMimm      },
    {"EOR%s %s",ARM9DASMimm      },
    {"EOR%s %s",ARM9DASMimm      },

    //0x23
    {"EOR%sS %s",ARM9DASMimm     },
    {"EOR%sS %s",ARM9DASMimm     },
    {"EOR%sS %s",ARM9DASMimm     },
    {"EOR%sS %s",ARM9DASMimm     },
    {"EOR%sS %s",ARM9DASMimm     },
    {"EOR%sS %s",ARM9DASMimm     },
    {"EOR%sS %s",ARM9DASMimm     },
    {"EOR%sS %s",ARM9DASMimm     },
    {"EOR%sS %s",ARM9DASMimm     },
    {"EOR%sS %s",ARM9DASMimm     },
    {"EOR%sS %s",ARM9DASMimm     },
    {"EOR%sS %s",ARM9DASMimm     },
    {"EOR%sS %s",ARM9DASMimm     },
    {"EOR%sS %s",ARM9DASMimm     },
    {"EOR%sS %s",ARM9DASMimm     },
    {"EOR%sS %s",ARM9DASMimm     },

    //0x24
    {"SUB%s %s",ARM9DASMimm      },
    {"SUB%s %s",ARM9DASMimm      },
    {"SUB%s %s",ARM9DASMimm      },
    {"SUB%s %s",ARM9DASMimm      },
    {"SUB%s %s",ARM9DASMimm      },
    {"SUB%s %s",ARM9DASMimm      },
    {"SUB%s %s",ARM9DASMimm      },
    {"SUB%s %s",ARM9DASMimm      },
    {"SUB%s %s",ARM9DASMimm      },
    {"SUB%s %s",ARM9DASMimm      },
    {"SUB%s %s",ARM9DASMimm      },
    {"SUB%s %s",ARM9DASMimm      },
    {"SUB%s %s",ARM9DASMimm      },
    {"SUB%s %s",ARM9DASMimm      },
    {"SUB%s %s",ARM9DASMimm      },
    {"SUB%s %s",ARM9DASMimm      },

    //0x25
    {"SUB%sS %s",ARM9DASMimm     },
    {"SUB%sS %s",ARM9DASMimm     },
    {"SUB%sS %s",ARM9DASMimm     },
    {"SUB%sS %s",ARM9DASMimm     },
    {"SUB%sS %s",ARM9DASMimm     },
    {"SUB%sS %s",ARM9DASMimm     },
    {"SUB%sS %s",ARM9DASMimm     },
    {"SUB%sS %s",ARM9DASMimm     },
    {"SUB%sS %s",ARM9DASMimm     },
    {"SUB%sS %s",ARM9DASMimm     },
    {"SUB%sS %s",ARM9DASMimm     },
    {"SUB%sS %s",ARM9DASMimm     },
    {"SUB%sS %s",ARM9DASMimm     },
    {"SUB%sS %s",ARM9DASMimm     },
    {"SUB%sS %s",ARM9DASMimm     },
    {"SUB%sS %s",ARM9DASMimm     },

    //0x26
    {"RSB%s %s",ARM9DASMimm      },
    {"RSB%s %s",ARM9DASMimm      },
    {"RSB%s %s",ARM9DASMimm      },
    {"RSB%s %s",ARM9DASMimm      },
    {"RSB%s %s",ARM9DASMimm      },
    {"RSB%s %s",ARM9DASMimm      },
    {"RSB%s %s",ARM9DASMimm      },
    {"RSB%s %s",ARM9DASMimm      },
    {"RSB%s %s",ARM9DASMimm      },
    {"RSB%s %s",ARM9DASMimm      },
    {"RSB%s %s",ARM9DASMimm      },
    {"RSB%s %s",ARM9DASMimm      },
    {"RSB%s %s",ARM9DASMimm      },
    {"RSB%s %s",ARM9DASMimm      },
    {"RSB%s %s",ARM9DASMimm      },
    {"RSB%s %s",ARM9DASMimm      },

    //0x27
    {"RSB%sS %s",ARM9DASMimm     },
    {"RSB%sS %s",ARM9DASMimm     },
    {"RSB%sS %s",ARM9DASMimm     },
    {"RSB%sS %s",ARM9DASMimm     },
    {"RSB%sS %s",ARM9DASMimm     },
    {"RSB%sS %s",ARM9DASMimm     },
    {"RSB%sS %s",ARM9DASMimm     },
    {"RSB%sS %s",ARM9DASMimm     },
    {"RSB%sS %s",ARM9DASMimm     },
    {"RSB%sS %s",ARM9DASMimm     },
    {"RSB%sS %s",ARM9DASMimm     },
    {"RSB%sS %s",ARM9DASMimm     },
    {"RSB%sS %s",ARM9DASMimm     },
    {"RSB%sS %s",ARM9DASMimm     },
    {"RSB%sS %s",ARM9DASMimm     },
    {"RSB%sS %s",ARM9DASMimm     },

    //0x28
    {"ADD%s %s",ARM9DASMimm      },
    {"ADD%s %s",ARM9DASMimm      },
    {"ADD%s %s",ARM9DASMimm      },
    {"ADD%s %s",ARM9DASMimm      },
    {"ADD%s %s",ARM9DASMimm      },
    {"ADD%s %s",ARM9DASMimm      },
    {"ADD%s %s",ARM9DASMimm      },
    {"ADD%s %s",ARM9DASMimm      },
    {"ADD%s %s",ARM9DASMimm      },
    {"ADD%s %s",ARM9DASMimm      },
    {"ADD%s %s",ARM9DASMimm      },
    {"ADD%s %s",ARM9DASMimm      },
    {"ADD%s %s",ARM9DASMimm      },
    {"ADD%s %s",ARM9DASMimm      },
    {"ADD%s %s",ARM9DASMimm      },
    {"ADD%s %s",ARM9DASMimm      },

    //0x29
    {"ADD%sS %s",ARM9DASMimm     },
    {"ADD%sS %s",ARM9DASMimm     },
    {"ADD%sS %s",ARM9DASMimm     },
    {"ADD%sS %s",ARM9DASMimm     },
    {"ADD%sS %s",ARM9DASMimm     },
    {"ADD%sS %s",ARM9DASMimm     },
    {"ADD%sS %s",ARM9DASMimm     },
    {"ADD%sS %s",ARM9DASMimm     },
    {"ADD%sS %s",ARM9DASMimm     },
    {"ADD%sS %s",ARM9DASMimm     },
    {"ADD%sS %s",ARM9DASMimm     },
    {"ADD%sS %s",ARM9DASMimm     },
    {"ADD%sS %s",ARM9DASMimm     },
    {"ADD%sS %s",ARM9DASMimm     },
    {"ADD%sS %s",ARM9DASMimm     },
    {"ADD%sS %s",ARM9DASMimm     },

    //0x2A
    {"ADC%s %s",ARM9DASMimm      },
    {"ADC%s %s",ARM9DASMimm      },
    {"ADC%s %s",ARM9DASMimm      },
    {"ADC%s %s",ARM9DASMimm      },
    {"ADC%s %s",ARM9DASMimm      },
    {"ADC%s %s",ARM9DASMimm      },
    {"ADC%s %s",ARM9DASMimm      },
    {"ADC%s %s",ARM9DASMimm      },
    {"ADC%s %s",ARM9DASMimm      },
    {"ADC%s %s",ARM9DASMimm      },
    {"ADC%s %s",ARM9DASMimm      },
    {"ADC%s %s",ARM9DASMimm      },
    {"ADC%s %s",ARM9DASMimm      },
    {"ADC%s %s",ARM9DASMimm      },
    {"ADC%s %s",ARM9DASMimm      },
    {"ADC%s %s",ARM9DASMimm      },

    //0x2B
    {"ADC%sS %s",ARM9DASMimm     },
    {"ADC%sS %s",ARM9DASMimm     },
    {"ADC%sS %s",ARM9DASMimm     },
    {"ADC%sS %s",ARM9DASMimm     },
    {"ADC%sS %s",ARM9DASMimm     },
    {"ADC%sS %s",ARM9DASMimm     },
    {"ADC%sS %s",ARM9DASMimm     },
    {"ADC%sS %s",ARM9DASMimm     },
    {"ADC%sS %s",ARM9DASMimm     },
    {"ADC%sS %s",ARM9DASMimm     },
    {"ADC%sS %s",ARM9DASMimm     },
    {"ADC%sS %s",ARM9DASMimm     },
    {"ADC%sS %s",ARM9DASMimm     },
    {"ADC%sS %s",ARM9DASMimm     },
    {"ADC%sS %s",ARM9DASMimm     },
    {"ADC%sS %s",ARM9DASMimm     },

    //0x2C
    {"SBC%s %s",ARM9DASMimm      },
    {"SBC%s %s",ARM9DASMimm      },
    {"SBC%s %s",ARM9DASMimm      },
    {"SBC%s %s",ARM9DASMimm      },
    {"SBC%s %s",ARM9DASMimm      },
    {"SBC%s %s",ARM9DASMimm      },
    {"SBC%s %s",ARM9DASMimm      },
    {"SBC%s %s",ARM9DASMimm      },
    {"SBC%s %s",ARM9DASMimm      },
    {"SBC%s %s",ARM9DASMimm      },
    {"SBC%s %s",ARM9DASMimm      },
    {"SBC%s %s",ARM9DASMimm      },
    {"SBC%s %s",ARM9DASMimm      },
    {"SBC%s %s",ARM9DASMimm      },
    {"SBC%s %s",ARM9DASMimm      },
    {"SBC%s %s",ARM9DASMimm      },

    //0x2D
    {"SBC%sS %s",ARM9DASMimm     },
    {"SBC%sS %s",ARM9DASMimm     },
    {"SBC%sS %s",ARM9DASMimm     },
    {"SBC%sS %s",ARM9DASMimm     },
    {"SBC%sS %s",ARM9DASMimm     },
    {"SBC%sS %s",ARM9DASMimm     },
    {"SBC%sS %s",ARM9DASMimm     },
    {"SBC%sS %s",ARM9DASMimm     },
    {"SBC%sS %s",ARM9DASMimm     },
    {"SBC%sS %s",ARM9DASMimm     },
    {"SBC%sS %s",ARM9DASMimm     },
    {"SBC%sS %s",ARM9DASMimm     },
    {"SBC%sS %s",ARM9DASMimm     },
    {"SBC%sS %s",ARM9DASMimm     },
    {"SBC%sS %s",ARM9DASMimm     },
    {"SBC%sS %s",ARM9DASMimm     },

    //0x2E
    {"RSC%s %s",ARM9DASMimm      },
    {"RSC%s %s",ARM9DASMimm      },
    {"RSC%s %s",ARM9DASMimm      },
    {"RSC%s %s",ARM9DASMimm      },
    {"RSC%s %s",ARM9DASMimm      },
    {"RSC%s %s",ARM9DASMimm      },
    {"RSC%s %s",ARM9DASMimm      },
    {"RSC%s %s",ARM9DASMimm      },
    {"RSC%s %s",ARM9DASMimm      },
    {"RSC%s %s",ARM9DASMimm      },
    {"RSC%s %s",ARM9DASMimm      },
    {"RSC%s %s",ARM9DASMimm      },
    {"RSC%s %s",ARM9DASMimm      },
    {"RSC%s %s",ARM9DASMimm      },
    {"RSC%s %s",ARM9DASMimm      },
    {"RSC%s %s",ARM9DASMimm      },

    //0x2F
    {"RSC%sS %s",ARM9DASMimm     },
    {"RSC%sS %s",ARM9DASMimm     },
    {"RSC%sS %s",ARM9DASMimm     },
    {"RSC%sS %s",ARM9DASMimm     },
    {"RSC%sS %s",ARM9DASMimm     },
    {"RSC%sS %s",ARM9DASMimm     },
    {"RSC%sS %s",ARM9DASMimm     },
    {"RSC%sS %s",ARM9DASMimm     },
    {"RSC%sS %s",ARM9DASMimm     },
    {"RSC%sS %s",ARM9DASMimm     },
    {"RSC%sS %s",ARM9DASMimm     },
    {"RSC%sS %s",ARM9DASMimm     },
    {"RSC%sS %s",ARM9DASMimm     },
    {"RSC%sS %s",ARM9DASMimm     },
    {"RSC%sS %s",ARM9DASMimm     },
    {"RSC%sS %s",ARM9DASMimm     },

    //0x30
    {"UND%s %s",ARM9DASMun       },
    {"UND%s %s",ARM9DASMun       },
    {"UND%s %s",ARM9DASMun       },
    {"UND%s %s",ARM9DASMun       },
    {"UND%s %s",ARM9DASMun       },
    {"UND%s %s",ARM9DASMun       },
    {"UND%s %s",ARM9DASMun       },
    {"UND%s %s",ARM9DASMun       },
    {"UND%s %s",ARM9DASMun       },
    {"UND%s %s",ARM9DASMun       },
    {"UND%s %s",ARM9DASMun       },
    {"UND%s %s",ARM9DASMun       },
    {"UND%s %s",ARM9DASMun       },
    {"UND%s %s",ARM9DASMun       },
    {"UND%s %s",ARM9DASMun       },
    {"UND%s %s",ARM9DASMun       },

    //0x31
    {"TST%sS %s",ARM9DASMimm     },
    {"TST%sS %s",ARM9DASMimm     },
    {"TST%sS %s",ARM9DASMimm     },
    {"TST%sS %s",ARM9DASMimm     },
    {"TST%sS %s",ARM9DASMimm     },
    {"TST%sS %s",ARM9DASMimm     },
    {"TST%sS %s",ARM9DASMimm     },
    {"TST%sS %s",ARM9DASMimm     },
    {"TST%sS %s",ARM9DASMimm     },
    {"TST%sS %s",ARM9DASMimm     },
    {"TST%sS %s",ARM9DASMimm     },
    {"TST%sS %s",ARM9DASMimm     },
    {"TST%sS %s",ARM9DASMimm     },
    {"TST%sS %s",ARM9DASMimm     },
    {"TST%sS %s",ARM9DASMimm     },
    {"TST%sS %s",ARM9DASMimm     },

    //0x32
    {"MSR%s %s",ARM9DASMmsric       },
    {"MSR%s %s",ARM9DASMmsric       },
    {"MSR%s %s",ARM9DASMmsric       },
    {"MSR%s %s",ARM9DASMmsric       },
    {"MSR%s %s",ARM9DASMmsric       },
    {"MSR%s %s",ARM9DASMmsric       },
    {"MSR%s %s",ARM9DASMmsric       },
    {"MSR%s %s",ARM9DASMmsric       },
    {"MSR%s %s",ARM9DASMmsric       },
    {"MSR%s %s",ARM9DASMmsric       },
    {"MSR%s %s",ARM9DASMmsric       },
    {"MSR%s %s",ARM9DASMmsric       },
    {"MSR%s %s",ARM9DASMmsric       },
    {"MSR%s %s",ARM9DASMmsric       },
    {"MSR%s %s",ARM9DASMmsric       },
    {"MSR%s %s",ARM9DASMmsric       },

    //0x33
    {"TEQ%sS %s",ARM9DASMimm     },
    {"TEQ%sS %s",ARM9DASMimm     },
    {"TEQ%sS %s",ARM9DASMimm     },
    {"TEQ%sS %s",ARM9DASMimm     },
    {"TEQ%sS %s",ARM9DASMimm     },
    {"TEQ%sS %s",ARM9DASMimm     },
    {"TEQ%sS %s",ARM9DASMimm     },
    {"TEQ%sS %s",ARM9DASMimm     },
    {"TEQ%sS %s",ARM9DASMimm     },
    {"TEQ%sS %s",ARM9DASMimm     },
    {"TEQ%sS %s",ARM9DASMimm     },
    {"TEQ%sS %s",ARM9DASMimm     },
    {"TEQ%sS %s",ARM9DASMimm     },
    {"TEQ%sS %s",ARM9DASMimm     },
    {"TEQ%sS %s",ARM9DASMimm     },
    {"TEQ%sS %s",ARM9DASMimm     },

    //0x34
    {"UND%s %s",ARM9DASMun       },
    {"UND%s %s",ARM9DASMun       },
    {"UND%s %s",ARM9DASMun       },
    {"UND%s %s",ARM9DASMun       },
    {"UND%s %s",ARM9DASMun       },
    {"UND%s %s",ARM9DASMun       },
    {"UND%s %s",ARM9DASMun       },
    {"UND%s %s",ARM9DASMun       },
    {"UND%s %s",ARM9DASMun       },
    {"UND%s %s",ARM9DASMun       },
    {"UND%s %s",ARM9DASMun       },
    {"UND%s %s",ARM9DASMun       },
    {"UND%s %s",ARM9DASMun       },
    {"UND%s %s",ARM9DASMun       },
    {"UND%s %s",ARM9DASMun       },
    {"UND%s %s",ARM9DASMun       },

    //0x35
    {"CMP%sS %s",ARM9DASMimm     },
    {"CMP%sS %s",ARM9DASMimm     },
    {"CMP%sS %s",ARM9DASMimm     },
    {"CMP%sS %s",ARM9DASMimm     },
    {"CMP%sS %s",ARM9DASMimm     },
    {"CMP%sS %s",ARM9DASMimm     },
    {"CMP%sS %s",ARM9DASMimm     },
    {"CMP%sS %s",ARM9DASMimm     },
    {"CMP%sS %s",ARM9DASMimm     },
    {"CMP%sS %s",ARM9DASMimm     },
    {"CMP%sS %s",ARM9DASMimm     },
    {"CMP%sS %s",ARM9DASMimm     },
    {"CMP%sS %s",ARM9DASMimm     },
    {"CMP%sS %s",ARM9DASMimm     },
    {"CMP%sS %s",ARM9DASMimm     },
    {"CMP%sS %s",ARM9DASMimm     },

    //0x36
    {"MSR%s %s",ARM9DASMmsris       },
    {"MSR%s %s",ARM9DASMmsris       },
    {"MSR%s %s",ARM9DASMmsris       },
    {"MSR%s %s",ARM9DASMmsris       },
    {"MSR%s %s",ARM9DASMmsris       },
    {"MSR%s %s",ARM9DASMmsris       },
    {"MSR%s %s",ARM9DASMmsris       },
    {"MSR%s %s",ARM9DASMmsris       },
    {"MSR%s %s",ARM9DASMmsris       },
    {"MSR%s %s",ARM9DASMmsris       },
    {"MSR%s %s",ARM9DASMmsris       },
    {"MSR%s %s",ARM9DASMmsris       },
    {"MSR%s %s",ARM9DASMmsris       },
    {"MSR%s %s",ARM9DASMmsris       },
    {"MSR%s %s",ARM9DASMmsris       },
    {"MSR%s %s",ARM9DASMmsris       },

    //0x37
    {"CMN%sS %s",ARM9DASMimm     },
    {"CMN%sS %s",ARM9DASMimm     },
    {"CMN%sS %s",ARM9DASMimm     },
    {"CMN%sS %s",ARM9DASMimm     },
    {"CMN%sS %s",ARM9DASMimm     },
    {"CMN%sS %s",ARM9DASMimm     },
    {"CMN%sS %s",ARM9DASMimm     },
    {"CMN%sS %s",ARM9DASMimm     },
    {"CMN%sS %s",ARM9DASMimm     },
    {"CMN%sS %s",ARM9DASMimm     },
    {"CMN%sS %s",ARM9DASMimm     },
    {"CMN%sS %s",ARM9DASMimm     },
    {"CMN%sS %s",ARM9DASMimm     },
    {"CMN%sS %s",ARM9DASMimm     },
    {"CMN%sS %s",ARM9DASMimm     },
    {"CMN%sS %s",ARM9DASMimm     },

    //0x38
    {"ORR%s %s",ARM9DASMimm      },
    {"ORR%s %s",ARM9DASMimm      },
    {"ORR%s %s",ARM9DASMimm      },
    {"ORR%s %s",ARM9DASMimm      },
    {"ORR%s %s",ARM9DASMimm      },
    {"ORR%s %s",ARM9DASMimm      },
    {"ORR%s %s",ARM9DASMimm      },
    {"ORR%s %s",ARM9DASMimm      },
    {"ORR%s %s",ARM9DASMimm      },
    {"ORR%s %s",ARM9DASMimm      },
    {"ORR%s %s",ARM9DASMimm      },
    {"ORR%s %s",ARM9DASMimm      },
    {"ORR%s %s",ARM9DASMimm      },
    {"ORR%s %s",ARM9DASMimm      },
    {"ORR%s %s",ARM9DASMimm      },
    {"ORR%s %s",ARM9DASMimm      },

    //0x39
    {"ORR%sS %s",ARM9DASMimm     },
    {"ORR%sS %s",ARM9DASMimm     },
    {"ORR%sS %s",ARM9DASMimm     },
    {"ORR%sS %s",ARM9DASMimm     },
    {"ORR%sS %s",ARM9DASMimm     },
    {"ORR%sS %s",ARM9DASMimm     },
    {"ORR%sS %s",ARM9DASMimm     },
    {"ORR%sS %s",ARM9DASMimm     },
    {"ORR%sS %s",ARM9DASMimm     },
    {"ORR%sS %s",ARM9DASMimm     },
    {"ORR%sS %s",ARM9DASMimm     },
    {"ORR%sS %s",ARM9DASMimm     },
    {"ORR%sS %s",ARM9DASMimm     },
    {"ORR%sS %s",ARM9DASMimm     },
    {"ORR%sS %s",ARM9DASMimm     },
    {"ORR%sS %s",ARM9DASMimm     },

    //0x3A
    {"MOV%s %s",ARM9DASMimm      },
    {"MOV%s %s",ARM9DASMimm      },
    {"MOV%s %s",ARM9DASMimm      },
    {"MOV%s %s",ARM9DASMimm      },
    {"MOV%s %s",ARM9DASMimm      },
    {"MOV%s %s",ARM9DASMimm      },
    {"MOV%s %s",ARM9DASMimm      },
    {"MOV%s %s",ARM9DASMimm      },
    {"MOV%s %s",ARM9DASMimm      },
    {"MOV%s %s",ARM9DASMimm      },
    {"MOV%s %s",ARM9DASMimm      },
    {"MOV%s %s",ARM9DASMimm      },
    {"MOV%s %s",ARM9DASMimm      },
    {"MOV%s %s",ARM9DASMimm      },
    {"MOV%s %s",ARM9DASMimm      },
    {"MOV%s %s",ARM9DASMimm      },

    //0x3B
    {"MOV%sS %s",ARM9DASMimm     },
    {"MOV%sS %s",ARM9DASMimm     },
    {"MOV%sS %s",ARM9DASMimm     },
    {"MOV%sS %s",ARM9DASMimm     },
    {"MOV%sS %s",ARM9DASMimm     },
    {"MOV%sS %s",ARM9DASMimm     },
    {"MOV%sS %s",ARM9DASMimm     },
    {"MOV%sS %s",ARM9DASMimm     },
    {"MOV%sS %s",ARM9DASMimm     },
    {"MOV%sS %s",ARM9DASMimm     },
    {"MOV%sS %s",ARM9DASMimm     },
    {"MOV%sS %s",ARM9DASMimm     },
    {"MOV%sS %s",ARM9DASMimm     },
    {"MOV%sS %s",ARM9DASMimm     },
    {"MOV%sS %s",ARM9DASMimm     },
    {"MOV%sS %s",ARM9DASMimm     },

    //0x3C
    {"BIC%s %s",ARM9DASMimm      },
    {"BIC%s %s",ARM9DASMimm      },
    {"BIC%s %s",ARM9DASMimm      },
    {"BIC%s %s",ARM9DASMimm      },
    {"BIC%s %s",ARM9DASMimm      },
    {"BIC%s %s",ARM9DASMimm      },
    {"BIC%s %s",ARM9DASMimm      },
    {"BIC%s %s",ARM9DASMimm      },
    {"BIC%s %s",ARM9DASMimm      },
    {"BIC%s %s",ARM9DASMimm      },
    {"BIC%s %s",ARM9DASMimm      },
    {"BIC%s %s",ARM9DASMimm      },
    {"BIC%s %s",ARM9DASMimm      },
    {"BIC%s %s",ARM9DASMimm      },
    {"BIC%s %s",ARM9DASMimm      },
    {"BIC%s %s",ARM9DASMimm      },

    //0x3D
    {"BIC%sS %s",ARM9DASMimm     },
    {"BIC%sS %s",ARM9DASMimm     },
    {"BIC%sS %s",ARM9DASMimm     },
    {"BIC%sS %s",ARM9DASMimm     },
    {"BIC%sS %s",ARM9DASMimm     },
    {"BIC%sS %s",ARM9DASMimm     },
    {"BIC%sS %s",ARM9DASMimm     },
    {"BIC%sS %s",ARM9DASMimm     },
    {"BIC%sS %s",ARM9DASMimm     },
    {"BIC%sS %s",ARM9DASMimm     },
    {"BIC%sS %s",ARM9DASMimm     },
    {"BIC%sS %s",ARM9DASMimm     },
    {"BIC%sS %s",ARM9DASMimm     },
    {"BIC%sS %s",ARM9DASMimm     },
    {"BIC%sS %s",ARM9DASMimm     },
    {"BIC%sS %s",ARM9DASMimm     },

    //0x3E
    {"MVN%s %s",ARM9DASMimm      },
    {"MVN%s %s",ARM9DASMimm      },
    {"MVN%s %s",ARM9DASMimm      },
    {"MVN%s %s",ARM9DASMimm      },
    {"MVN%s %s",ARM9DASMimm      },
    {"MVN%s %s",ARM9DASMimm      },
    {"MVN%s %s",ARM9DASMimm      },
    {"MVN%s %s",ARM9DASMimm      },
    {"MVN%s %s",ARM9DASMimm      },
    {"MVN%s %s",ARM9DASMimm      },
    {"MVN%s %s",ARM9DASMimm      },
    {"MVN%s %s",ARM9DASMimm      },
    {"MVN%s %s",ARM9DASMimm      },
    {"MVN%s %s",ARM9DASMimm      },
    {"MVN%s %s",ARM9DASMimm      },
    {"MVN%s %s",ARM9DASMimm      },

    //0x3F
    {"MVN%sS %s",ARM9DASMimm     },
    {"MVN%sS %s",ARM9DASMimm     },
    {"MVN%sS %s",ARM9DASMimm     },
    {"MVN%sS %s",ARM9DASMimm     },
    {"MVN%sS %s",ARM9DASMimm     },
    {"MVN%sS %s",ARM9DASMimm     },
    {"MVN%sS %s",ARM9DASMimm     },
    {"MVN%sS %s",ARM9DASMimm     },
    {"MVN%sS %s",ARM9DASMimm     },
    {"MVN%sS %s",ARM9DASMimm     },
    {"MVN%sS %s",ARM9DASMimm     },
    {"MVN%sS %s",ARM9DASMimm     },
    {"MVN%sS %s",ARM9DASMimm     },
    {"MVN%sS %s",ARM9DASMimm     },
    {"MVN%sS %s",ARM9DASMimm     },
    {"MVN%sS %s",ARM9DASMimm     },

    //0x40
    {"STR%s %s",ARM9DASMptim     },
    {"STR%s %s",ARM9DASMptim     },
    {"STR%s %s",ARM9DASMptim     },
    {"STR%s %s",ARM9DASMptim     },
    {"STR%s %s",ARM9DASMptim     },
    {"STR%s %s",ARM9DASMptim     },
    {"STR%s %s",ARM9DASMptim     },
    {"STR%s %s",ARM9DASMptim     },
    {"STR%s %s",ARM9DASMptim     },
    {"STR%s %s",ARM9DASMptim     },
    {"STR%s %s",ARM9DASMptim     },
    {"STR%s %s",ARM9DASMptim     },
    {"STR%s %s",ARM9DASMptim     },
    {"STR%s %s",ARM9DASMptim     },
    {"STR%s %s",ARM9DASMptim     },
    {"STR%s %s",ARM9DASMptim     },

    //0x41
    {"LDR%s %s",ARM9DASMptim     },
    {"LDR%s %s",ARM9DASMptim     },
    {"LDR%s %s",ARM9DASMptim     },
    {"LDR%s %s",ARM9DASMptim     },
    {"LDR%s %s",ARM9DASMptim     },
    {"LDR%s %s",ARM9DASMptim     },
    {"LDR%s %s",ARM9DASMptim     },
    {"LDR%s %s",ARM9DASMptim     },
    {"LDR%s %s",ARM9DASMptim     },
    {"LDR%s %s",ARM9DASMptim     },
    {"LDR%s %s",ARM9DASMptim     },
    {"LDR%s %s",ARM9DASMptim     },
    {"LDR%s %s",ARM9DASMptim     },
    {"LDR%s %s",ARM9DASMptim     },
    {"LDR%s %s",ARM9DASMptim     },
    {"LDR%s %s",ARM9DASMptim     },

    //0x42
    {"STRT%s %s",ARM9DASMptim    },
    {"STRT%s %s",ARM9DASMptim    },
    {"STRT%s %s",ARM9DASMptim    },
    {"STRT%s %s",ARM9DASMptim    },
    {"STRT%s %s",ARM9DASMptim    },
    {"STRT%s %s",ARM9DASMptim    },
    {"STRT%s %s",ARM9DASMptim    },
    {"STRT%s %s",ARM9DASMptim    },
    {"STRT%s %s",ARM9DASMptim    },
    {"STRT%s %s",ARM9DASMptim    },
    {"STRT%s %s",ARM9DASMptim    },
    {"STRT%s %s",ARM9DASMptim    },
    {"STRT%s %s",ARM9DASMptim    },
    {"STRT%s %s",ARM9DASMptim    },
    {"STRT%s %s",ARM9DASMptim    },
    {"STRT%s %s",ARM9DASMptim    },

    //0x43
    {"LDRT%s %s",ARM9DASMptim    },
    {"LDRT%s %s",ARM9DASMptim    },
    {"LDRT%s %s",ARM9DASMptim    },
    {"LDRT%s %s",ARM9DASMptim    },
    {"LDRT%s %s",ARM9DASMptim    },
    {"LDRT%s %s",ARM9DASMptim    },
    {"LDRT%s %s",ARM9DASMptim    },
    {"LDRT%s %s",ARM9DASMptim    },
    {"LDRT%s %s",ARM9DASMptim    },
    {"LDRT%s %s",ARM9DASMptim    },
    {"LDRT%s %s",ARM9DASMptim    },
    {"LDRT%s %s",ARM9DASMptim    },
    {"LDRT%s %s",ARM9DASMptim    },
    {"LDRT%s %s",ARM9DASMptim    },
    {"LDRT%s %s",ARM9DASMptim    },
    {"LDRT%s %s",ARM9DASMptim    },

    //0x44
    {"STRB%s %s",ARM9DASMptim    },
    {"STRB%s %s",ARM9DASMptim    },
    {"STRB%s %s",ARM9DASMptim    },
    {"STRB%s %s",ARM9DASMptim    },
    {"STRB%s %s",ARM9DASMptim    },
    {"STRB%s %s",ARM9DASMptim    },
    {"STRB%s %s",ARM9DASMptim    },
    {"STRB%s %s",ARM9DASMptim    },
    {"STRB%s %s",ARM9DASMptim    },
    {"STRB%s %s",ARM9DASMptim    },
    {"STRB%s %s",ARM9DASMptim    },
    {"STRB%s %s",ARM9DASMptim    },
    {"STRB%s %s",ARM9DASMptim    },
    {"STRB%s %s",ARM9DASMptim    },
    {"STRB%s %s",ARM9DASMptim    },
    {"STRB%s %s",ARM9DASMptim    },

    //0x45
    {"LDRB%s %s",ARM9DASMptim    },
    {"LDRB%s %s",ARM9DASMptim    },
    {"LDRB%s %s",ARM9DASMptim    },
    {"LDRB%s %s",ARM9DASMptim    },
    {"LDRB%s %s",ARM9DASMptim    },
    {"LDRB%s %s",ARM9DASMptim    },
    {"LDRB%s %s",ARM9DASMptim    },
    {"LDRB%s %s",ARM9DASMptim    },
    {"LDRB%s %s",ARM9DASMptim    },
    {"LDRB%s %s",ARM9DASMptim    },
    {"LDRB%s %s",ARM9DASMptim    },
    {"LDRB%s %s",ARM9DASMptim    },
    {"LDRB%s %s",ARM9DASMptim    },
    {"LDRB%s %s",ARM9DASMptim    },
    {"LDRB%s %s",ARM9DASMptim    },
    {"LDRB%s %s",ARM9DASMptim    },

    //0x46
    {"STRBT%s %s",ARM9DASMptim   },
    {"STRBT%s %s",ARM9DASMptim   },
    {"STRBT%s %s",ARM9DASMptim   },
    {"STRBT%s %s",ARM9DASMptim   },
    {"STRBT%s %s",ARM9DASMptim   },
    {"STRBT%s %s",ARM9DASMptim   },
    {"STRBT%s %s",ARM9DASMptim   },
    {"STRBT%s %s",ARM9DASMptim   },
    {"STRBT%s %s",ARM9DASMptim   },
    {"STRBT%s %s",ARM9DASMptim   },
    {"STRBT%s %s",ARM9DASMptim   },
    {"STRBT%s %s",ARM9DASMptim   },
    {"STRBT%s %s",ARM9DASMptim   },
    {"STRBT%s %s",ARM9DASMptim   },
    {"STRBT%s %s",ARM9DASMptim   },
    {"STRBT%s %s",ARM9DASMptim   },

    //0x47
    {"LDRBT%s %s",ARM9DASMptim   },
    {"LDRBT%s %s",ARM9DASMptim   },
    {"LDRBT%s %s",ARM9DASMptim   },
    {"LDRBT%s %s",ARM9DASMptim   },
    {"LDRBT%s %s",ARM9DASMptim   },
    {"LDRBT%s %s",ARM9DASMptim   },
    {"LDRBT%s %s",ARM9DASMptim   },
    {"LDRBT%s %s",ARM9DASMptim   },
    {"LDRBT%s %s",ARM9DASMptim   },
    {"LDRBT%s %s",ARM9DASMptim   },
    {"LDRBT%s %s",ARM9DASMptim   },
    {"LDRBT%s %s",ARM9DASMptim   },
    {"LDRBT%s %s",ARM9DASMptim   },
    {"LDRBT%s %s",ARM9DASMptim   },
    {"LDRBT%s %s",ARM9DASMptim   },
    {"LDRBT%s %s",ARM9DASMptim   },

    //0x48
    {"STR%s %s",ARM9DASMptip     },
    {"STR%s %s",ARM9DASMptip     },
    {"STR%s %s",ARM9DASMptip     },
    {"STR%s %s",ARM9DASMptip     },
    {"STR%s %s",ARM9DASMptip     },
    {"STR%s %s",ARM9DASMptip     },
    {"STR%s %s",ARM9DASMptip     },
    {"STR%s %s",ARM9DASMptip     },
    {"STR%s %s",ARM9DASMptip     },
    {"STR%s %s",ARM9DASMptip     },
    {"STR%s %s",ARM9DASMptip     },
    {"STR%s %s",ARM9DASMptip     },
    {"STR%s %s",ARM9DASMptip     },
    {"STR%s %s",ARM9DASMptip     },
    {"STR%s %s",ARM9DASMptip     },
    {"STR%s %s",ARM9DASMptip     },

    //0x49
    {"LDR%s %s",ARM9DASMptip     },
    {"LDR%s %s",ARM9DASMptip     },
    {"LDR%s %s",ARM9DASMptip     },
    {"LDR%s %s",ARM9DASMptip     },
    {"LDR%s %s",ARM9DASMptip     },
    {"LDR%s %s",ARM9DASMptip     },
    {"LDR%s %s",ARM9DASMptip     },
    {"LDR%s %s",ARM9DASMptip     },
    {"LDR%s %s",ARM9DASMptip     },
    {"LDR%s %s",ARM9DASMptip     },
    {"LDR%s %s",ARM9DASMptip     },
    {"LDR%s %s",ARM9DASMptip     },
    {"LDR%s %s",ARM9DASMptip     },
    {"LDR%s %s",ARM9DASMptip     },
    {"LDR%s %s",ARM9DASMptip     },
    {"LDR%s %s",ARM9DASMptip     },

    //0x4A
    {"STRT%s %s",ARM9DASMptip    },
    {"STRT%s %s",ARM9DASMptip    },
    {"STRT%s %s",ARM9DASMptip    },
    {"STRT%s %s",ARM9DASMptip    },
    {"STRT%s %s",ARM9DASMptip    },
    {"STRT%s %s",ARM9DASMptip    },
    {"STRT%s %s",ARM9DASMptip    },
    {"STRT%s %s",ARM9DASMptip    },
    {"STRT%s %s",ARM9DASMptip    },
    {"STRT%s %s",ARM9DASMptip    },
    {"STRT%s %s",ARM9DASMptip    },
    {"STRT%s %s",ARM9DASMptip    },
    {"STRT%s %s",ARM9DASMptip    },
    {"STRT%s %s",ARM9DASMptip    },
    {"STRT%s %s",ARM9DASMptip    },
    {"STRT%s %s",ARM9DASMptip    },

    //0x4B
    {"LDRT%s %s",ARM9DASMptip    },
    {"LDRT%s %s",ARM9DASMptip    },
    {"LDRT%s %s",ARM9DASMptip    },
    {"LDRT%s %s",ARM9DASMptip    },
    {"LDRT%s %s",ARM9DASMptip    },
    {"LDRT%s %s",ARM9DASMptip    },
    {"LDRT%s %s",ARM9DASMptip    },
    {"LDRT%s %s",ARM9DASMptip    },
    {"LDRT%s %s",ARM9DASMptip    },
    {"LDRT%s %s",ARM9DASMptip    },
    {"LDRT%s %s",ARM9DASMptip    },
    {"LDRT%s %s",ARM9DASMptip    },
    {"LDRT%s %s",ARM9DASMptip    },
    {"LDRT%s %s",ARM9DASMptip    },
    {"LDRT%s %s",ARM9DASMptip    },
    {"LDRT%s %s",ARM9DASMptip    },

    //0x4C
    {"STRB%s %s",ARM9DASMptip    },
    {"STRB%s %s",ARM9DASMptip    },
    {"STRB%s %s",ARM9DASMptip    },
    {"STRB%s %s",ARM9DASMptip    },
    {"STRB%s %s",ARM9DASMptip    },
    {"STRB%s %s",ARM9DASMptip    },
    {"STRB%s %s",ARM9DASMptip    },
    {"STRB%s %s",ARM9DASMptip    },
    {"STRB%s %s",ARM9DASMptip    },
    {"STRB%s %s",ARM9DASMptip    },
    {"STRB%s %s",ARM9DASMptip    },
    {"STRB%s %s",ARM9DASMptip    },
    {"STRB%s %s",ARM9DASMptip    },
    {"STRB%s %s",ARM9DASMptip    },
    {"STRB%s %s",ARM9DASMptip    },
    {"STRB%s %s",ARM9DASMptip    },

    //0x4D
    {"LDRB%s %s",ARM9DASMptip    },
    {"LDRB%s %s",ARM9DASMptip    },
    {"LDRB%s %s",ARM9DASMptip    },
    {"LDRB%s %s",ARM9DASMptip    },
    {"LDRB%s %s",ARM9DASMptip    },
    {"LDRB%s %s",ARM9DASMptip    },
    {"LDRB%s %s",ARM9DASMptip    },
    {"LDRB%s %s",ARM9DASMptip    },
    {"LDRB%s %s",ARM9DASMptip    },
    {"LDRB%s %s",ARM9DASMptip    },
    {"LDRB%s %s",ARM9DASMptip    },
    {"LDRB%s %s",ARM9DASMptip    },
    {"LDRB%s %s",ARM9DASMptip    },
    {"LDRB%s %s",ARM9DASMptip    },
    {"LDRB%s %s",ARM9DASMptip    },
    {"LDRB%s %s",ARM9DASMptip    },

    //0x4E
    {"STRBT%s %s",ARM9DASMptip   },
    {"STRBT%s %s",ARM9DASMptip   },
    {"STRBT%s %s",ARM9DASMptip   },
    {"STRBT%s %s",ARM9DASMptip   },
    {"STRBT%s %s",ARM9DASMptip   },
    {"STRBT%s %s",ARM9DASMptip   },
    {"STRBT%s %s",ARM9DASMptip   },
    {"STRBT%s %s",ARM9DASMptip   },
    {"STRBT%s %s",ARM9DASMptip   },
    {"STRBT%s %s",ARM9DASMptip   },
    {"STRBT%s %s",ARM9DASMptip   },
    {"STRBT%s %s",ARM9DASMptip   },
    {"STRBT%s %s",ARM9DASMptip   },
    {"STRBT%s %s",ARM9DASMptip   },
    {"STRBT%s %s",ARM9DASMptip   },
    {"STRBT%s %s",ARM9DASMptip   },

    //0x4F
    {"LDRBT%s %s",ARM9DASMptip   },
    {"LDRBT%s %s",ARM9DASMptip   },
    {"LDRBT%s %s",ARM9DASMptip   },
    {"LDRBT%s %s",ARM9DASMptip   },
    {"LDRBT%s %s",ARM9DASMptip   },
    {"LDRBT%s %s",ARM9DASMptip   },
    {"LDRBT%s %s",ARM9DASMptip   },
    {"LDRBT%s %s",ARM9DASMptip   },
    {"LDRBT%s %s",ARM9DASMptip   },
    {"LDRBT%s %s",ARM9DASMptip   },
    {"LDRBT%s %s",ARM9DASMptip   },
    {"LDRBT%s %s",ARM9DASMptip   },
    {"LDRBT%s %s",ARM9DASMptip   },
    {"LDRBT%s %s",ARM9DASMptip   },
    {"LDRBT%s %s",ARM9DASMptip   },
    {"LDRBT%s %s",ARM9DASMptip   },

    //0x50
    {"STR%s %s",ARM9DASMofim     },
    {"STR%s %s",ARM9DASMofim     },
    {"STR%s %s",ARM9DASMofim     },
    {"STR%s %s",ARM9DASMofim     },
    {"STR%s %s",ARM9DASMofim     },
    {"STR%s %s",ARM9DASMofim     },
    {"STR%s %s",ARM9DASMofim     },
    {"STR%s %s",ARM9DASMofim     },
    {"STR%s %s",ARM9DASMofim     },
    {"STR%s %s",ARM9DASMofim     },
    {"STR%s %s",ARM9DASMofim     },
    {"STR%s %s",ARM9DASMofim     },
    {"STR%s %s",ARM9DASMofim     },
    {"STR%s %s",ARM9DASMofim     },
    {"STR%s %s",ARM9DASMofim     },
    {"STR%s %s",ARM9DASMofim     },

    //0x51
    {"LDR%s %s",ARM9DASMofim     },
    {"LDR%s %s",ARM9DASMofim     },
    {"LDR%s %s",ARM9DASMofim     },
    {"LDR%s %s",ARM9DASMofim     },
    {"LDR%s %s",ARM9DASMofim     },
    {"LDR%s %s",ARM9DASMofim     },
    {"LDR%s %s",ARM9DASMofim     },
    {"LDR%s %s",ARM9DASMofim     },
    {"LDR%s %s",ARM9DASMofim     },
    {"LDR%s %s",ARM9DASMofim     },
    {"LDR%s %s",ARM9DASMofim     },
    {"LDR%s %s",ARM9DASMofim     },
    {"LDR%s %s",ARM9DASMofim     },
    {"LDR%s %s",ARM9DASMofim     },
    {"LDR%s %s",ARM9DASMofim     },
    {"LDR%s %s",ARM9DASMofim     },

    //0x52
    {"STR%s %s",ARM9DASMprim     },
    {"STR%s %s",ARM9DASMprim     },
    {"STR%s %s",ARM9DASMprim     },
    {"STR%s %s",ARM9DASMprim     },
    {"STR%s %s",ARM9DASMprim     },
    {"STR%s %s",ARM9DASMprim     },
    {"STR%s %s",ARM9DASMprim     },
    {"STR%s %s",ARM9DASMprim     },
    {"STR%s %s",ARM9DASMprim     },
    {"STR%s %s",ARM9DASMprim     },
    {"STR%s %s",ARM9DASMprim     },
    {"STR%s %s",ARM9DASMprim     },
    {"STR%s %s",ARM9DASMprim     },
    {"STR%s %s",ARM9DASMprim     },
    {"STR%s %s",ARM9DASMprim     },
    {"STR%s %s",ARM9DASMprim     },

    //0x53
    {"LDR%s %s",ARM9DASMprim     },
    {"LDR%s %s",ARM9DASMprim     },
    {"LDR%s %s",ARM9DASMprim     },
    {"LDR%s %s",ARM9DASMprim     },
    {"LDR%s %s",ARM9DASMprim     },
    {"LDR%s %s",ARM9DASMprim     },
    {"LDR%s %s",ARM9DASMprim     },
    {"LDR%s %s",ARM9DASMprim     },
    {"LDR%s %s",ARM9DASMprim     },
    {"LDR%s %s",ARM9DASMprim     },
    {"LDR%s %s",ARM9DASMprim     },
    {"LDR%s %s",ARM9DASMprim     },
    {"LDR%s %s",ARM9DASMprim     },
    {"LDR%s %s",ARM9DASMprim     },
    {"LDR%s %s",ARM9DASMprim     },
    {"LDR%s %s",ARM9DASMprim     },

    //0x54
    {"STRB%s %s",ARM9DASMofim    },
    {"STRB%s %s",ARM9DASMofim    },
    {"STRB%s %s",ARM9DASMofim    },
    {"STRB%s %s",ARM9DASMofim    },
    {"STRB%s %s",ARM9DASMofim    },
    {"STRB%s %s",ARM9DASMofim    },
    {"STRB%s %s",ARM9DASMofim    },
    {"STRB%s %s",ARM9DASMofim    },
    {"STRB%s %s",ARM9DASMofim    },
    {"STRB%s %s",ARM9DASMofim    },
    {"STRB%s %s",ARM9DASMofim    },
    {"STRB%s %s",ARM9DASMofim    },
    {"STRB%s %s",ARM9DASMofim    },
    {"STRB%s %s",ARM9DASMofim    },
    {"STRB%s %s",ARM9DASMofim    },
    {"STRB%s %s",ARM9DASMofim    },

    //0x55
    {"LDRB%s %s",ARM9DASMofim    },
    {"LDRB%s %s",ARM9DASMofim    },
    {"LDRB%s %s",ARM9DASMofim    },
    {"LDRB%s %s",ARM9DASMofim    },
    {"LDRB%s %s",ARM9DASMofim    },
    {"LDRB%s %s",ARM9DASMofim    },
    {"LDRB%s %s",ARM9DASMofim    },
    {"LDRB%s %s",ARM9DASMofim    },
    {"LDRB%s %s",ARM9DASMofim    },
    {"LDRB%s %s",ARM9DASMofim    },
    {"LDRB%s %s",ARM9DASMofim    },
    {"LDRB%s %s",ARM9DASMofim    },
    {"LDRB%s %s",ARM9DASMofim    },
    {"LDRB%s %s",ARM9DASMofim    },
    {"LDRB%s %s",ARM9DASMofim    },
    {"LDRB%s %s",ARM9DASMofim    },

    //0x56
    {"STRB%s %s",ARM9DASMprim    },
    {"STRB%s %s",ARM9DASMprim    },
    {"STRB%s %s",ARM9DASMprim    },
    {"STRB%s %s",ARM9DASMprim    },
    {"STRB%s %s",ARM9DASMprim    },
    {"STRB%s %s",ARM9DASMprim    },
    {"STRB%s %s",ARM9DASMprim    },
    {"STRB%s %s",ARM9DASMprim    },
    {"STRB%s %s",ARM9DASMprim    },
    {"STRB%s %s",ARM9DASMprim    },
    {"STRB%s %s",ARM9DASMprim    },
    {"STRB%s %s",ARM9DASMprim    },
    {"STRB%s %s",ARM9DASMprim    },
    {"STRB%s %s",ARM9DASMprim    },
    {"STRB%s %s",ARM9DASMprim    },
    {"STRB%s %s",ARM9DASMprim    },

    //0x57
    {"LDRB%s %s",ARM9DASMprim    },
    {"LDRB%s %s",ARM9DASMprim    },
    {"LDRB%s %s",ARM9DASMprim    },
    {"LDRB%s %s",ARM9DASMprim    },
    {"LDRB%s %s",ARM9DASMprim    },
    {"LDRB%s %s",ARM9DASMprim    },
    {"LDRB%s %s",ARM9DASMprim    },
    {"LDRB%s %s",ARM9DASMprim    },
    {"LDRB%s %s",ARM9DASMprim    },
    {"LDRB%s %s",ARM9DASMprim    },
    {"LDRB%s %s",ARM9DASMprim    },
    {"LDRB%s %s",ARM9DASMprim    },
    {"LDRB%s %s",ARM9DASMprim    },
    {"LDRB%s %s",ARM9DASMprim    },
    {"LDRB%s %s",ARM9DASMprim    },
    {"LDRB%s %s",ARM9DASMprim    },

    //0x58
    {"STR%s %s",ARM9DASMofip     },
    {"STR%s %s",ARM9DASMofip     },
    {"STR%s %s",ARM9DASMofip     },
    {"STR%s %s",ARM9DASMofip     },
    {"STR%s %s",ARM9DASMofip     },
    {"STR%s %s",ARM9DASMofip     },
    {"STR%s %s",ARM9DASMofip     },
    {"STR%s %s",ARM9DASMofip     },
    {"STR%s %s",ARM9DASMofip     },
    {"STR%s %s",ARM9DASMofip     },
    {"STR%s %s",ARM9DASMofip     },
    {"STR%s %s",ARM9DASMofip     },
    {"STR%s %s",ARM9DASMofip     },
    {"STR%s %s",ARM9DASMofip     },
    {"STR%s %s",ARM9DASMofip     },
    {"STR%s %s",ARM9DASMofip     },

    //0x59    
    {"LDR%s %s",ARM9DASMofip     },
    {"LDR%s %s",ARM9DASMofip     },
    {"LDR%s %s",ARM9DASMofip     },
    {"LDR%s %s",ARM9DASMofip     },
    {"LDR%s %s",ARM9DASMofip     },
    {"LDR%s %s",ARM9DASMofip     },
    {"LDR%s %s",ARM9DASMofip     },
    {"LDR%s %s",ARM9DASMofip     },
    {"LDR%s %s",ARM9DASMofip     },
    {"LDR%s %s",ARM9DASMofip     },
    {"LDR%s %s",ARM9DASMofip     },
    {"LDR%s %s",ARM9DASMofip     },
    {"LDR%s %s",ARM9DASMofip     },
    {"LDR%s %s",ARM9DASMofip     },
    {"LDR%s %s",ARM9DASMofip     },
    {"LDR%s %s",ARM9DASMofip     },

    //0x5A
    {"STR%s %s",ARM9DASMprip     },
    {"STR%s %s",ARM9DASMprip     },
    {"STR%s %s",ARM9DASMprip     },
    {"STR%s %s",ARM9DASMprip     },
    {"STR%s %s",ARM9DASMprip     },
    {"STR%s %s",ARM9DASMprip     },
    {"STR%s %s",ARM9DASMprip     },
    {"STR%s %s",ARM9DASMprip     },
    {"STR%s %s",ARM9DASMprip     },
    {"STR%s %s",ARM9DASMprip     },
    {"STR%s %s",ARM9DASMprip     },
    {"STR%s %s",ARM9DASMprip     },
    {"STR%s %s",ARM9DASMprip     },
    {"STR%s %s",ARM9DASMprip     },
    {"STR%s %s",ARM9DASMprip     },
    {"STR%s %s",ARM9DASMprip     },

    //0x5B
    {"LDR%s %s",ARM9DASMprip     },
    {"LDR%s %s",ARM9DASMprip     },
    {"LDR%s %s",ARM9DASMprip     },
    {"LDR%s %s",ARM9DASMprip     },
    {"LDR%s %s",ARM9DASMprip     },
    {"LDR%s %s",ARM9DASMprip     },
    {"LDR%s %s",ARM9DASMprip     },
    {"LDR%s %s",ARM9DASMprip     },
    {"LDR%s %s",ARM9DASMprip     },
    {"LDR%s %s",ARM9DASMprip     },
    {"LDR%s %s",ARM9DASMprip     },
    {"LDR%s %s",ARM9DASMprip     },
    {"LDR%s %s",ARM9DASMprip     },
    {"LDR%s %s",ARM9DASMprip     },
    {"LDR%s %s",ARM9DASMprip     },
    {"LDR%s %s",ARM9DASMprip     },

    //0x5C
    {"STRB%s %s",ARM9DASMofip    },
    {"STRB%s %s",ARM9DASMofip    },
    {"STRB%s %s",ARM9DASMofip    },
    {"STRB%s %s",ARM9DASMofip    },
    {"STRB%s %s",ARM9DASMofip    },
    {"STRB%s %s",ARM9DASMofip    },
    {"STRB%s %s",ARM9DASMofip    },
    {"STRB%s %s",ARM9DASMofip    },
    {"STRB%s %s",ARM9DASMofip    },
    {"STRB%s %s",ARM9DASMofip    },
    {"STRB%s %s",ARM9DASMofip    },
    {"STRB%s %s",ARM9DASMofip    },
    {"STRB%s %s",ARM9DASMofip    },
    {"STRB%s %s",ARM9DASMofip    },
    {"STRB%s %s",ARM9DASMofip    },
    {"STRB%s %s",ARM9DASMofip    },

    //0x5D
    {"LDRB%s %s",ARM9DASMofip    },
    {"LDRB%s %s",ARM9DASMofip    },
    {"LDRB%s %s",ARM9DASMofip    },
    {"LDRB%s %s",ARM9DASMofip    },
    {"LDRB%s %s",ARM9DASMofip    },
    {"LDRB%s %s",ARM9DASMofip    },
    {"LDRB%s %s",ARM9DASMofip    },
    {"LDRB%s %s",ARM9DASMofip    },
    {"LDRB%s %s",ARM9DASMofip    },
    {"LDRB%s %s",ARM9DASMofip    },
    {"LDRB%s %s",ARM9DASMofip    },
    {"LDRB%s %s",ARM9DASMofip    },
    {"LDRB%s %s",ARM9DASMofip    },
    {"LDRB%s %s",ARM9DASMofip    },
    {"LDRB%s %s",ARM9DASMofip    },
    {"LDRB%s %s",ARM9DASMofip    },

    //0x5E
    {"STRB%s %s",ARM9DASMprip    },
    {"STRB%s %s",ARM9DASMprip    },
    {"STRB%s %s",ARM9DASMprip    },
    {"STRB%s %s",ARM9DASMprip    },
    {"STRB%s %s",ARM9DASMprip    },
    {"STRB%s %s",ARM9DASMprip    },
    {"STRB%s %s",ARM9DASMprip    },
    {"STRB%s %s",ARM9DASMprip    },
    {"STRB%s %s",ARM9DASMprip    },
    {"STRB%s %s",ARM9DASMprip    },
    {"STRB%s %s",ARM9DASMprip    },
    {"STRB%s %s",ARM9DASMprip    },
    {"STRB%s %s",ARM9DASMprip    },
    {"STRB%s %s",ARM9DASMprip    },
    {"STRB%s %s",ARM9DASMprip    },
    {"STRB%s %s",ARM9DASMprip    },

    //0x5F
    {"LDRB%s %s",ARM9DASMprip    },
    {"LDRB%s %s",ARM9DASMprip    },
    {"LDRB%s %s",ARM9DASMprip    },
    {"LDRB%s %s",ARM9DASMprip    },
    {"LDRB%s %s",ARM9DASMprip    },
    {"LDRB%s %s",ARM9DASMprip    },
    {"LDRB%s %s",ARM9DASMprip    },
    {"LDRB%s %s",ARM9DASMprip    },
    {"LDRB%s %s",ARM9DASMprip    },
    {"LDRB%s %s",ARM9DASMprip    },
    {"LDRB%s %s",ARM9DASMprip    },
    {"LDRB%s %s",ARM9DASMprip    },
    {"LDRB%s %s",ARM9DASMprip    },
    {"LDRB%s %s",ARM9DASMprip    },
    {"LDRB%s %s",ARM9DASMprip    },
    {"LDRB%s %s",ARM9DASMprip    },

    //0x60
    {"STR%s %s",ARM9DASMptrmll   },
    {"UND%s %s",ARM9DASMun       },
    {"STR%s %s",ARM9DASMptrmlr   },
    {"UND%s %s",ARM9DASMun       },
    {"STR%s %s",ARM9DASMptrmar   },
    {"UND%s %s",ARM9DASMun       },
    {"STR%s %s",ARM9DASMptrmrr   },
    {"UND%s %s",ARM9DASMun       },
    {"STR%s %s",ARM9DASMptrmll   },
    {"UND%s %s",ARM9DASMun       },
    {"STR%s %s",ARM9DASMptrmlr   },
    {"UND%s %s",ARM9DASMun       },
    {"STR%s %s",ARM9DASMptrmar   },
    {"UND%s %s",ARM9DASMun       },
    {"STR%s %s",ARM9DASMptrmrr   },
    {"UND%s %s",ARM9DASMun       },

    //0x61
    {"LDR%s %s",ARM9DASMptrmll   },
    {"UND%s %s",ARM9DASMun       },
    {"LDR%s %s",ARM9DASMptrmlr   },
    {"UND%s %s",ARM9DASMun       },
    {"LDR%s %s",ARM9DASMptrmar   },
    {"UND%s %s",ARM9DASMun       },
    {"LDR%s %s",ARM9DASMptrmrr   },
    {"UND%s %s",ARM9DASMun       },
    {"LDR%s %s",ARM9DASMptrmll   },
    {"UND%s %s",ARM9DASMun       },
    {"LDR%s %s",ARM9DASMptrmlr   },
    {"UND%s %s",ARM9DASMun       },
    {"LDR%s %s",ARM9DASMptrmar   },
    {"UND%s %s",ARM9DASMun       },
    {"LDR%s %s",ARM9DASMptrmrr   },
    {"UND%s %s",ARM9DASMun       },

    //0x62
    {"STRT%s %s",ARM9DASMptrmll  },
    {"UND%s %s",ARM9DASMun       },
    {"STRT%s %s",ARM9DASMptrmlr  },
    {"UND%s %s",ARM9DASMun       },
    {"STRT%s %s",ARM9DASMptrmar  },
    {"UND%s %s",ARM9DASMun       },
    {"STRT%s %s",ARM9DASMptrmrr  },
    {"UND%s %s",ARM9DASMun       },
    {"STRT%s %s",ARM9DASMptrmll  },
    {"UND%s %s",ARM9DASMun       },
    {"STRT%s %s",ARM9DASMptrmlr  },
    {"UND%s %s",ARM9DASMun       },
    {"STRT%s %s",ARM9DASMptrmar  },
    {"UND%s %s",ARM9DASMun       },
    {"STRT%s %s",ARM9DASMptrmrr  },
    {"UND%s %s",ARM9DASMun       },

    //0x63
    {"LDRT%s %s",ARM9DASMptrmll  },
    {"UND%s %s",ARM9DASMun       },
    {"LDRT%s %s",ARM9DASMptrmlr  },
    {"UND%s %s",ARM9DASMun       },
    {"LDRT%s %s",ARM9DASMptrmar  },
    {"UND%s %s",ARM9DASMun       },
    {"LDRT%s %s",ARM9DASMptrmrr  },
    {"UND%s %s",ARM9DASMun       },
    {"LDRT%s %s",ARM9DASMptrmll  },
    {"UND%s %s",ARM9DASMun       },
    {"LDRT%s %s",ARM9DASMptrmlr  },
    {"UND%s %s",ARM9DASMun       },
    {"LDRT%s %s",ARM9DASMptrmar  },
    {"UND%s %s",ARM9DASMun       },
    {"LDRT%s %s",ARM9DASMptrmrr  },
    {"UND%s %s",ARM9DASMun       },

    //0x64
    {"STRB%s %s",ARM9DASMptrmll  },
    {"UND%s %s",ARM9DASMun       },
    {"STRB%s %s",ARM9DASMptrmlr  },
    {"UND%s %s",ARM9DASMun       },
    {"STRB%s %s",ARM9DASMptrmar  },
    {"UND%s %s",ARM9DASMun       },
    {"STRB%s %s",ARM9DASMptrmrr  },
    {"UND%s %s",ARM9DASMun       },
    {"STRB%s %s",ARM9DASMptrmll  },
    {"UND%s %s",ARM9DASMun       },
    {"STRB%s %s",ARM9DASMptrmlr  },
    {"UND%s %s",ARM9DASMun       },
    {"STRB%s %s",ARM9DASMptrmar  },
    {"UND%s %s",ARM9DASMun       },
    {"STRB%s %s",ARM9DASMptrmrr  },
    {"UND%s %s",ARM9DASMun       },

    //0x65
    {"LDRB%s %s",ARM9DASMptrmll  },
    {"UND%s %s",ARM9DASMun       },
    {"LDRB%s %s",ARM9DASMptrmlr  },
    {"UND%s %s",ARM9DASMun       },
    {"LDRB%s %s",ARM9DASMptrmar  },
    {"UND%s %s",ARM9DASMun       },
    {"LDRB%s %s",ARM9DASMptrmrr  },
    {"UND%s %s",ARM9DASMun       },
    {"LDRB%s %s",ARM9DASMptrmll  },
    {"UND%s %s",ARM9DASMun       },
    {"LDRB%s %s",ARM9DASMptrmlr  },
    {"UND%s %s",ARM9DASMun       },
    {"LDRB%s %s",ARM9DASMptrmar  },
    {"UND%s %s",ARM9DASMun       },
    {"LDRB%s %s",ARM9DASMptrmrr  },
    {"UND%s %s",ARM9DASMun       },

    //0x66
    {"STRBT%s %s",ARM9DASMptrmll },
    {"UND%s %s",ARM9DASMun       },
    {"STRBT%s %s",ARM9DASMptrmlr },
    {"UND%s %s",ARM9DASMun       },
    {"STRBT%s %s",ARM9DASMptrmar },
    {"UND%s %s",ARM9DASMun       },
    {"STRBT%s %s",ARM9DASMptrmrr },
    {"UND%s %s",ARM9DASMun       },
    {"STRBT%s %s",ARM9DASMptrmll },
    {"UND%s %s",ARM9DASMun       },
    {"STRBT%s %s",ARM9DASMptrmlr },
    {"UND%s %s",ARM9DASMun       },
    {"STRBT%s %s",ARM9DASMptrmar },
    {"UND%s %s",ARM9DASMun       },
    {"STRBT%s %s",ARM9DASMptrmrr },
    {"UND%s %s",ARM9DASMun       },

    //0x67
    {"LDRBT%s %s",ARM9DASMptrmll },
    {"UND%s %s",ARM9DASMun       },
    {"LDRBT%s %s",ARM9DASMptrmlr },
    {"UND%s %s",ARM9DASMun       },
    {"LDRBT%s %s",ARM9DASMptrmar },
    {"UND%s %s",ARM9DASMun       },
    {"LDRBT%s %s",ARM9DASMptrmrr },
    {"UND%s %s",ARM9DASMun       },
    {"LDRBT%s %s",ARM9DASMptrmll },
    {"UND%s %s",ARM9DASMun       },
    {"LDRBT%s %s",ARM9DASMptrmlr },
    {"UND%s %s",ARM9DASMun       },
    {"LDRBT%s %s",ARM9DASMptrmar },
    {"UND%s %s",ARM9DASMun       },
    {"LDRBT%s %s",ARM9DASMptrmrr },
    {"UND%s %s",ARM9DASMun       },

    //0x68
    {"STR%s %s",ARM9DASMptrpll   },
    {"UND%s %s",ARM9DASMun       },
    {"STR%s %s",ARM9DASMptrplr   },
    {"UND%s %s",ARM9DASMun       },
    {"STR%s %s",ARM9DASMptrpar   },
    {"UND%s %s",ARM9DASMun       },
    {"STR%s %s",ARM9DASMptrprr   },
    {"UND%s %s",ARM9DASMun       },
    {"STR%s %s",ARM9DASMptrpll   },
    {"UND%s %s",ARM9DASMun       },
    {"STR%s %s",ARM9DASMptrplr   },
    {"UND%s %s",ARM9DASMun       },
    {"STR%s %s",ARM9DASMptrpar   },
    {"UND%s %s",ARM9DASMun       },
    {"STR%s %s",ARM9DASMptrprr   },
    {"UND%s %s",ARM9DASMun       },

    //0x69
    {"LDR%s %s",ARM9DASMptrpll   },
    {"UND%s %s",ARM9DASMun       },
    {"LDR%s %s",ARM9DASMptrplr   },
    {"UND%s %s",ARM9DASMun       },
    {"LDR%s %s",ARM9DASMptrpar   },
    {"UND%s %s",ARM9DASMun       },
    {"LDR%s %s",ARM9DASMptrprr   },
    {"UND%s %s",ARM9DASMun       },
    {"LDR%s %s",ARM9DASMptrpll   },
    {"UND%s %s",ARM9DASMun       },
    {"LDR%s %s",ARM9DASMptrplr   },
    {"UND%s %s",ARM9DASMun       },
    {"LDR%s %s",ARM9DASMptrpar   },
    {"UND%s %s",ARM9DASMun       },
    {"LDR%s %s",ARM9DASMptrprr   },
    {"UND%s %s",ARM9DASMun       },

    //0x6A
    {"STRT%s %s",ARM9DASMptrpll  },
    {"UND%s %s",ARM9DASMun       },
    {"STRT%s %s",ARM9DASMptrplr  },
    {"UND%s %s",ARM9DASMun       },
    {"STRT%s %s",ARM9DASMptrpar  },
    {"UND%s %s",ARM9DASMun       },
    {"STRT%s %s",ARM9DASMptrprr  },
    {"UND%s %s",ARM9DASMun       },
    {"STRT%s %s",ARM9DASMptrpll  },
    {"UND%s %s",ARM9DASMun       },
    {"STRT%s %s",ARM9DASMptrplr  },
    {"UND%s %s",ARM9DASMun       },
    {"STRT%s %s",ARM9DASMptrpar  },
    {"UND%s %s",ARM9DASMun       },
    {"STRT%s %s",ARM9DASMptrprr  },
    {"UND%s %s",ARM9DASMun       },

    //0x6B
    {"LDRT%s %s",ARM9DASMptrpll  },
    {"UND%s %s",ARM9DASMun       },
    {"LDRT%s %s",ARM9DASMptrplr  },
    {"UND%s %s",ARM9DASMun       },
    {"LDRT%s %s",ARM9DASMptrpar  },
    {"UND%s %s",ARM9DASMun       },
    {"LDRT%s %s",ARM9DASMptrprr  },
    {"UND%s %s",ARM9DASMun       },
    {"LDRT%s %s",ARM9DASMptrpll  },
    {"UND%s %s",ARM9DASMun       },
    {"LDRT%s %s",ARM9DASMptrplr  },
    {"UND%s %s",ARM9DASMun       },
    {"LDRT%s %s",ARM9DASMptrpar  },
    {"UND%s %s",ARM9DASMun       },
    {"LDRT%s %s",ARM9DASMptrprr  },
    {"UND%s %s",ARM9DASMun       },

    //0x6C
    {"STRB%s %s",ARM9DASMptrpll  },
    {"UND%s %s",ARM9DASMun       },
    {"STRB%s %s",ARM9DASMptrplr  },
    {"UND%s %s",ARM9DASMun       },
    {"STRB%s %s",ARM9DASMptrpar  },
    {"UND%s %s",ARM9DASMun       },
    {"STRB%s %s",ARM9DASMptrprr  },
    {"UND%s %s",ARM9DASMun       },
    {"STRB%s %s",ARM9DASMptrpll  },
    {"UND%s %s",ARM9DASMun       },
    {"STRB%s %s",ARM9DASMptrplr  },
    {"UND%s %s",ARM9DASMun       },
    {"STRB%s %s",ARM9DASMptrpar  },
    {"UND%s %s",ARM9DASMun       },
    {"STRB%s %s",ARM9DASMptrprr  },
    {"UND%s %s",ARM9DASMun       },

    //0x6D
    {"LDRB%s %s",ARM9DASMptrpll  },
    {"UND%s %s",ARM9DASMun       },
    {"LDRB%s %s",ARM9DASMptrplr  },
    {"UND%s %s",ARM9DASMun       },
    {"LDRB%s %s",ARM9DASMptrpar  },
    {"UND%s %s",ARM9DASMun       },
    {"LDRB%s %s",ARM9DASMptrprr  },
    {"UND%s %s",ARM9DASMun       },
    {"LDRB%s %s",ARM9DASMptrpll  },
    {"UND%s %s",ARM9DASMun       },
    {"LDRB%s %s",ARM9DASMptrplr  },
    {"UND%s %s",ARM9DASMun       },
    {"LDRB%s %s",ARM9DASMptrpar  },
    {"UND%s %s",ARM9DASMun       },
    {"LDRB%s %s",ARM9DASMptrprr  },
    {"UND%s %s",ARM9DASMun       },

    //0x6E
    {"STRBT%s %s",ARM9DASMptrpll },
    {"UND%s %s",ARM9DASMun       },
    {"STRBT%s %s",ARM9DASMptrplr },
    {"UND%s %s",ARM9DASMun       },
    {"STRBT%s %s",ARM9DASMptrpar },
    {"UND%s %s",ARM9DASMun       },
    {"STRBT%s %s",ARM9DASMptrprr },
    {"UND%s %s",ARM9DASMun       },
    {"STRBT%s %s",ARM9DASMptrpll },
    {"UND%s %s",ARM9DASMun       },
    {"STRBT%s %s",ARM9DASMptrplr },
    {"UND%s %s",ARM9DASMun       },
    {"STRBT%s %s",ARM9DASMptrpar },
    {"UND%s %s",ARM9DASMun       },
    {"STRBT%s %s",ARM9DASMptrprr },
    {"UND%s %s",ARM9DASMun       },

    //0x6F
    {"LDRBT%s %s",ARM9DASMptrpll },
    {"UND%s %s",ARM9DASMun       },
    {"LDRBT%s %s",ARM9DASMptrplr },
    {"UND%s %s",ARM9DASMun       },
    {"LDRBT%s %s",ARM9DASMptrpar },
    {"UND%s %s",ARM9DASMun       },
    {"LDRBT%s %s",ARM9DASMptrprr },
    {"UND%s %s",ARM9DASMun       },
    {"LDRBT%s %s",ARM9DASMptrpll },
    {"UND%s %s",ARM9DASMun       },
    {"LDRBT%s %s",ARM9DASMptrplr },
    {"UND%s %s",ARM9DASMun       },
    {"LDRBT%s %s",ARM9DASMptrpar },
    {"UND%s %s",ARM9DASMun       },
    {"LDRBT%s %s",ARM9DASMptrprr },
    {"UND%s %s",ARM9DASMun       },

    //0x70
    {"STR%s %s",ARM9DASMofrmll   },
    {"UND%s %s",ARM9DASMun       },
    {"STR%s %s",ARM9DASMofrmlr   },
    {"UND%s %s",ARM9DASMun       },
    {"STR%s %s",ARM9DASMofrmar   },
    {"UND%s %s",ARM9DASMun       },
    {"STR%s %s",ARM9DASMofrmrr   },
    {"UND%s %s",ARM9DASMun       },
    {"STR%s %s",ARM9DASMofrmll   },
    {"UND%s %s",ARM9DASMun       },
    {"STR%s %s",ARM9DASMofrmlr   },
    {"UND%s %s",ARM9DASMun       },
    {"STR%s %s",ARM9DASMofrmar   },
    {"UND%s %s",ARM9DASMun       },
    {"STR%s %s",ARM9DASMofrmrr   },
    {"UND%s %s",ARM9DASMun       },

    //0x71
    {"LDR%s %s",ARM9DASMofrmll   },
    {"UND%s %s",ARM9DASMun       },
    {"LDR%s %s",ARM9DASMofrmlr   },
    {"UND%s %s",ARM9DASMun       },
    {"LDR%s %s",ARM9DASMofrmar   },
    {"UND%s %s",ARM9DASMun       },
    {"LDR%s %s",ARM9DASMofrmrr   },
    {"UND%s %s",ARM9DASMun       },
    {"LDR%s %s",ARM9DASMofrmll   },
    {"UND%s %s",ARM9DASMun       },
    {"LDR%s %s",ARM9DASMofrmlr   },
    {"UND%s %s",ARM9DASMun       },
    {"LDR%s %s",ARM9DASMofrmar   },
    {"UND%s %s",ARM9DASMun       },
    {"LDR%s %s",ARM9DASMofrmrr   },
    {"UND%s %s",ARM9DASMun       },

    //0x72
    {"STR%s %s",ARM9DASMprrmll   },
    {"UND%s %s",ARM9DASMun       },
    {"STR%s %s",ARM9DASMprrmlr   },
    {"UND%s %s",ARM9DASMun       },
    {"STR%s %s",ARM9DASMprrmar   },
    {"UND%s %s",ARM9DASMun       },
    {"STR%s %s",ARM9DASMprrmrr   },
    {"UND%s %s",ARM9DASMun       },
    {"STR%s %s",ARM9DASMprrmll   },
    {"UND%s %s",ARM9DASMun       },
    {"STR%s %s",ARM9DASMprrmlr   },
    {"UND%s %s",ARM9DASMun       },
    {"STR%s %s",ARM9DASMprrmar   },
    {"UND%s %s",ARM9DASMun       },
    {"STR%s %s",ARM9DASMprrmrr   },
    {"UND%s %s",ARM9DASMun       },

    //0x73
    {"LDR%s %s",ARM9DASMprrmll   },
    {"UND%s %s",ARM9DASMun       },
    {"LDR%s %s",ARM9DASMprrmlr   },
    {"UND%s %s",ARM9DASMun       },
    {"LDR%s %s",ARM9DASMprrmar   },
    {"UND%s %s",ARM9DASMun       },
    {"LDR%s %s",ARM9DASMprrmrr   },
    {"UND%s %s",ARM9DASMun       },
    {"LDR%s %s",ARM9DASMprrmll   },
    {"UND%s %s",ARM9DASMun       },
    {"LDR%s %s",ARM9DASMprrmlr   },
    {"UND%s %s",ARM9DASMun       },
    {"LDR%s %s",ARM9DASMprrmar   },
    {"UND%s %s",ARM9DASMun       },
    {"LDR%s %s",ARM9DASMprrmrr   },
    {"UND%s %s",ARM9DASMun       },

    //0x74
    {"STRB%s %s",ARM9DASMofrmll  },
    {"UND%s %s",ARM9DASMun       },
    {"STRB%s %s",ARM9DASMofrmlr  },
    {"UND%s %s",ARM9DASMun       },
    {"STRB%s %s",ARM9DASMofrmar  },
    {"UND%s %s",ARM9DASMun       },
    {"STRB%s %s",ARM9DASMofrmrr  },
    {"UND%s %s",ARM9DASMun       },
    {"STRB%s %s",ARM9DASMofrmll  },
    {"UND%s %s",ARM9DASMun       },
    {"STRB%s %s",ARM9DASMofrmlr  },
    {"UND%s %s",ARM9DASMun       },
    {"STRB%s %s",ARM9DASMofrmar  },
    {"UND%s %s",ARM9DASMun       },
    {"STRB%s %s",ARM9DASMofrmrr  },
    {"UND%s %s",ARM9DASMun       },

    //0x75
    {"LDRB%s %s",ARM9DASMofrmll  },
    {"UND%s %s",ARM9DASMun       },
    {"LDRB%s %s",ARM9DASMofrmlr  },
    {"UND%s %s",ARM9DASMun       },
    {"LDRB%s %s",ARM9DASMofrmar  },
    {"UND%s %s",ARM9DASMun       },
    {"LDRB%s %s",ARM9DASMofrmrr  },
    {"UND%s %s",ARM9DASMun       },
    {"LDRB%s %s",ARM9DASMofrmll  },
    {"UND%s %s",ARM9DASMun       },
    {"LDRB%s %s",ARM9DASMofrmlr  },
    {"UND%s %s",ARM9DASMun       },
    {"LDRB%s %s",ARM9DASMofrmar  },
    {"UND%s %s",ARM9DASMun       },
    {"LDRB%s %s",ARM9DASMofrmrr  },
    {"UND%s %s",ARM9DASMun       },

    //0x76
    {"STRB%s %s",ARM9DASMprrmll  },
    {"UND%s %s",ARM9DASMun       },
    {"STRB%s %s",ARM9DASMprrmlr  },
    {"UND%s %s",ARM9DASMun       },
    {"STRB%s %s",ARM9DASMprrmar  },
    {"UND%s %s",ARM9DASMun       },
    {"STRB%s %s",ARM9DASMprrmrr  },
    {"UND%s %s",ARM9DASMun       },
    {"STRB%s %s",ARM9DASMprrmll  },
    {"UND%s %s",ARM9DASMun       },
    {"STRB%s %s",ARM9DASMprrmlr  },
    {"UND%s %s",ARM9DASMun       },
    {"STRB%s %s",ARM9DASMprrmar  },
    {"UND%s %s",ARM9DASMun       },
    {"STRB%s %s",ARM9DASMprrmrr  },
    {"UND%s %s",ARM9DASMun       },

    //0x77
    {"LDRB%s %s",ARM9DASMprrmll  },
    {"UND%s %s",ARM9DASMun       },
    {"LDRB%s %s",ARM9DASMprrmlr  },
    {"UND%s %s",ARM9DASMun       },
    {"LDRB%s %s",ARM9DASMprrmar  },
    {"UND%s %s",ARM9DASMun       },
    {"LDRB%s %s",ARM9DASMprrmrr  },
    {"UND%s %s",ARM9DASMun       },
    {"LDRB%s %s",ARM9DASMprrmll  },
    {"UND%s %s",ARM9DASMun       },
    {"LDRB%s %s",ARM9DASMprrmlr  },
    {"UND%s %s",ARM9DASMun       },
    {"LDRB%s %s",ARM9DASMprrmar  },
    {"UND%s %s",ARM9DASMun       },
    {"LDRB%s %s",ARM9DASMprrmrr  },
    {"UND%s %s",ARM9DASMun       },

    //0x78
    {"STR%s %s",ARM9DASMofrpll   },
    {"UND%s %s",ARM9DASMun       },
    {"STR%s %s",ARM9DASMofrplr   },
    {"UND%s %s",ARM9DASMun       },
    {"STR%s %s",ARM9DASMofrpar   },
    {"UND%s %s",ARM9DASMun       },
    {"STR%s %s",ARM9DASMofrprr   },
    {"UND%s %s",ARM9DASMun       },
    {"STR%s %s",ARM9DASMofrpll   },
    {"UND%s %s",ARM9DASMun       },
    {"STR%s %s",ARM9DASMofrplr   },
    {"UND%s %s",ARM9DASMun       },
    {"STR%s %s",ARM9DASMofrpar   },
    {"UND%s %s",ARM9DASMun       },
    {"STR%s %s",ARM9DASMofrprr   },
    {"UND%s %s",ARM9DASMun       },

    //0x79
    {"LDR%s %s",ARM9DASMofrpll   },
    {"UND%s %s",ARM9DASMun       },
    {"LDR%s %s",ARM9DASMofrplr   },
    {"UND%s %s",ARM9DASMun       },
    {"LDR%s %s",ARM9DASMofrpar   },
    {"UND%s %s",ARM9DASMun       },
    {"LDR%s %s",ARM9DASMofrprr   },
    {"UND%s %s",ARM9DASMun       },
    {"LDR%s %s",ARM9DASMofrpll   },
    {"UND%s %s",ARM9DASMun       },
    {"LDR%s %s",ARM9DASMofrplr   },
    {"UND%s %s",ARM9DASMun       },
    {"LDR%s %s",ARM9DASMofrpar   },
    {"UND%s %s",ARM9DASMun       },
    {"LDR%s %s",ARM9DASMofrprr   },
    {"UND%s %s",ARM9DASMun       },

    //0x7A
    {"STR%s %s",ARM9DASMprrpll   },
    {"UND%s %s",ARM9DASMun       },
    {"STR%s %s",ARM9DASMprrplr   },
    {"UND%s %s",ARM9DASMun       },
    {"STR%s %s",ARM9DASMprrpar   },
    {"UND%s %s",ARM9DASMun       },
    {"STR%s %s",ARM9DASMprrprr   },
    {"UND%s %s",ARM9DASMun       },
    {"STR%s %s",ARM9DASMprrpll   },
    {"UND%s %s",ARM9DASMun       },
    {"STR%s %s",ARM9DASMprrplr   },
    {"UND%s %s",ARM9DASMun       },
    {"STR%s %s",ARM9DASMprrpar   },
    {"UND%s %s",ARM9DASMun       },
    {"STR%s %s",ARM9DASMprrprr   },
    {"UND%s %s",ARM9DASMun       },

    //0x7B
    {"LDR%s %s",ARM9DASMprrpll   },
    {"UND%s %s",ARM9DASMun       },
    {"LDR%s %s",ARM9DASMprrplr   },
    {"UND%s %s",ARM9DASMun       },
    {"LDR%s %s",ARM9DASMprrpar   },
    {"UND%s %s",ARM9DASMun       },
    {"LDR%s %s",ARM9DASMprrprr   },
    {"UND%s %s",ARM9DASMun       },
    {"LDR%s %s",ARM9DASMprrpll   },
    {"UND%s %s",ARM9DASMun       },
    {"LDR%s %s",ARM9DASMprrplr   },
    {"UND%s %s",ARM9DASMun       },
    {"LDR%s %s",ARM9DASMprrpar   },
    {"UND%s %s",ARM9DASMun       },
    {"LDR%s %s",ARM9DASMprrprr   },
    {"UND%s %s",ARM9DASMun       },

    //0x7C
    {"STRB%s %s",ARM9DASMofrpll  },
    {"UND%s %s",ARM9DASMun       },
    {"STRB%s %s",ARM9DASMofrplr  },
    {"UND%s %s",ARM9DASMun       },
    {"STRB%s %s",ARM9DASMofrpar  },
    {"UND%s %s",ARM9DASMun       },
    {"STRB%s %s",ARM9DASMofrprr  },
    {"UND%s %s",ARM9DASMun       },
    {"STRB%s %s",ARM9DASMofrpll  },
    {"UND%s %s",ARM9DASMun       },
    {"STRB%s %s",ARM9DASMofrplr  },
    {"UND%s %s",ARM9DASMun       },
    {"STRB%s %s",ARM9DASMofrpar  },
    {"UND%s %s",ARM9DASMun       },
    {"STRB%s %s",ARM9DASMofrprr  },
    {"UND%s %s",ARM9DASMun       },

    //0x7D
    {"LDRB%s %s",ARM9DASMofrpll  },
    {"UND%s %s",ARM9DASMun       },
    {"LDRB%s %s",ARM9DASMofrplr  },
    {"UND%s %s",ARM9DASMun       },
    {"LDRB%s %s",ARM9DASMofrpar  },
    {"UND%s %s",ARM9DASMun       },
    {"LDRB%s %s",ARM9DASMofrprr  },
    {"UND%s %s",ARM9DASMun       },
    {"LDRB%s %s",ARM9DASMofrpll  },
    {"UND%s %s",ARM9DASMun       },
    {"LDRB%s %s",ARM9DASMofrplr  },
    {"UND%s %s",ARM9DASMun       },
    {"LDRB%s %s",ARM9DASMofrpar  },
    {"UND%s %s",ARM9DASMun       },
    {"LDRB%s %s",ARM9DASMofrprr  },
    {"UND%s %s",ARM9DASMun       },

    //0x7E
    {"STRB%s %s",ARM9DASMprrpll  },
    {"UND%s %s",ARM9DASMun       },
    {"STRB%s %s",ARM9DASMprrplr  },
    {"UND%s %s",ARM9DASMun       },
    {"STRB%s %s",ARM9DASMprrpar  },
    {"UND%s %s",ARM9DASMun       },
    {"STRB%s %s",ARM9DASMprrprr  },
    {"UND%s %s",ARM9DASMun       },
    {"STRB%s %s",ARM9DASMprrpll  },
    {"UND%s %s",ARM9DASMun       },
    {"STRB%s %s",ARM9DASMprrplr  },
    {"UND%s %s",ARM9DASMun       },
    {"STRB%s %s",ARM9DASMprrpar  },
    {"UND%s %s",ARM9DASMun       },
    {"STRB%s %s",ARM9DASMprrprr  },
    {"UND%s %s",ARM9DASMun       },

    //0x7F
    {"LDRB%s %s",ARM9DASMprrpll  },
    {"UND%s %s",ARM9DASMun       },
    {"LDRB%s %s",ARM9DASMprrplr  },
    {"UND%s %s",ARM9DASMun       },
    {"LDRB%s %s",ARM9DASMprrpar  },
    {"UND%s %s",ARM9DASMun       },
    {"LDRB%s %s",ARM9DASMprrprr  },
    {"UND%s %s",ARM9DASMun       },
    {"LDRB%s %s",ARM9DASMprrpll  },
    {"UND%s %s",ARM9DASMun       },
    {"LDRB%s %s",ARM9DASMprrplr  },
    {"UND%s %s",ARM9DASMun       },
    {"LDRB%s %s",ARM9DASMprrpar  },
    {"UND%s %s",ARM9DASMun       },
    {"LDRB%s %s",ARM9DASMprrprr  },
    {"UND%s %s",ARM9DASMun       },

    //0x80
    {"STMDA%s %s",ARM9DASMlm       },
    {"STMDA%s %s",ARM9DASMlm       },
    {"STMDA%s %s",ARM9DASMlm       },
    {"STMDA%s %s",ARM9DASMlm       },
    {"STMDA%s %s",ARM9DASMlm       },
    {"STMDA%s %s",ARM9DASMlm       },
    {"STMDA%s %s",ARM9DASMlm       },
    {"STMDA%s %s",ARM9DASMlm       },
    {"STMDA%s %s",ARM9DASMlm       },
    {"STMDA%s %s",ARM9DASMlm       },
    {"STMDA%s %s",ARM9DASMlm       },
    {"STMDA%s %s",ARM9DASMlm       },
    {"STMDA%s %s",ARM9DASMlm       },
    {"STMDA%s %s",ARM9DASMlm       },
    {"STMDA%s %s",ARM9DASMlm       },
    {"STMDA%s %s",ARM9DASMlm       },

    //0x81
    {"LDMDA%s %s",ARM9DASMlm       },
    {"LDMDA%s %s",ARM9DASMlm       },
    {"LDMDA%s %s",ARM9DASMlm       },
    {"LDMDA%s %s",ARM9DASMlm       },
    {"LDMDA%s %s",ARM9DASMlm       },
    {"LDMDA%s %s",ARM9DASMlm       },
    {"LDMDA%s %s",ARM9DASMlm       },
    {"LDMDA%s %s",ARM9DASMlm       },
    {"LDMDA%s %s",ARM9DASMlm       },
    {"LDMDA%s %s",ARM9DASMlm       },
    {"LDMDA%s %s",ARM9DASMlm       },
    {"LDMDA%s %s",ARM9DASMlm       },
    {"LDMDA%s %s",ARM9DASMlm       },
    {"LDMDA%s %s",ARM9DASMlm       },
    {"LDMDA%s %s",ARM9DASMlm       },
    {"LDMDA%s %s",ARM9DASMlm       },

    //0x82
    {"STMDA%s %s",ARM9DASMlm      },
    {"STMDA%s %s",ARM9DASMlm      },
    {"STMDA%s %s",ARM9DASMlm      },
    {"STMDA%s %s",ARM9DASMlm      },
    {"STMDA%s %s",ARM9DASMlm      },
    {"STMDA%s %s",ARM9DASMlm      },
    {"STMDA%s %s",ARM9DASMlm      },
    {"STMDA%s %s",ARM9DASMlm      },
    {"STMDA%s %s",ARM9DASMlm      },
    {"STMDA%s %s",ARM9DASMlm      },
    {"STMDA%s %s",ARM9DASMlm      },
    {"STMDA%s %s",ARM9DASMlm      },
    {"STMDA%s %s",ARM9DASMlm      },
    {"STMDA%s %s",ARM9DASMlm      },
    {"STMDA%s %s",ARM9DASMlm      },
    {"STMDA%s %s",ARM9DASMlm      },

    //0x83
    {"LDMDA%s %s",ARM9DASMlm      },
    {"LDMDA%s %s",ARM9DASMlm      },
    {"LDMDA%s %s",ARM9DASMlm      },
    {"LDMDA%s %s",ARM9DASMlm      },
    {"LDMDA%s %s",ARM9DASMlm      },
    {"LDMDA%s %s",ARM9DASMlm      },
    {"LDMDA%s %s",ARM9DASMlm      },
    {"LDMDA%s %s",ARM9DASMlm      },
    {"LDMDA%s %s",ARM9DASMlm      },
    {"LDMDA%s %s",ARM9DASMlm      },
    {"LDMDA%s %s",ARM9DASMlm      },
    {"LDMDA%s %s",ARM9DASMlm      },
    {"LDMDA%s %s",ARM9DASMlm      },
    {"LDMDA%s %s",ARM9DASMlm      },
    {"LDMDA%s %s",ARM9DASMlm      },
    {"LDMDA%s %s",ARM9DASMlm      },

    //0x84
    {"STMDA%s %s",ARM9DASMlm      },
    {"STMDA%s %s",ARM9DASMlm      },
    {"STMDA%s %s",ARM9DASMlm      },
    {"STMDA%s %s",ARM9DASMlm      },
    {"STMDA%s %s",ARM9DASMlm      },
    {"STMDA%s %s",ARM9DASMlm      },
    {"STMDA%s %s",ARM9DASMlm      },
    {"STMDA%s %s",ARM9DASMlm      },
    {"STMDA%s %s",ARM9DASMlm      },
    {"STMDA%s %s",ARM9DASMlm      },
    {"STMDA%s %s",ARM9DASMlm      },
    {"STMDA%s %s",ARM9DASMlm      },
    {"STMDA%s %s",ARM9DASMlm      },
    {"STMDA%s %s",ARM9DASMlm      },
    {"STMDA%s %s",ARM9DASMlm      },
    {"STMDA%s %s",ARM9DASMlm      },

    //0x85
    {"LDMDA%s %s",ARM9DASMlm      },
    {"LDMDA%s %s",ARM9DASMlm      },
    {"LDMDA%s %s",ARM9DASMlm      },
    {"LDMDA%s %s",ARM9DASMlm      },
    {"LDMDA%s %s",ARM9DASMlm      },
    {"LDMDA%s %s",ARM9DASMlm      },
    {"LDMDA%s %s",ARM9DASMlm      },
    {"LDMDA%s %s",ARM9DASMlm      },
    {"LDMDA%s %s",ARM9DASMlm      },
    {"LDMDA%s %s",ARM9DASMlm      },
    {"LDMDA%s %s",ARM9DASMlm      },
    {"LDMDA%s %s",ARM9DASMlm      },
    {"LDMDA%s %s",ARM9DASMlm      },
    {"LDMDA%s %s",ARM9DASMlm      },
    {"LDMDA%s %s",ARM9DASMlm      },
    {"LDMDA%s %s",ARM9DASMlm      },

    //0x86
    {"STMDA%s %s",ARM9DASMlm     },
    {"STMDA%s %s",ARM9DASMlm     },
    {"STMDA%s %s",ARM9DASMlm     },
    {"STMDA%s %s",ARM9DASMlm     },
    {"STMDA%s %s",ARM9DASMlm     },
    {"STMDA%s %s",ARM9DASMlm     },
    {"STMDA%s %s",ARM9DASMlm     },
    {"STMDA%s %s",ARM9DASMlm     },
    {"STMDA%s %s",ARM9DASMlm     },
    {"STMDA%s %s",ARM9DASMlm     },
    {"STMDA%s %s",ARM9DASMlm     },
    {"STMDA%s %s",ARM9DASMlm     },
    {"STMDA%s %s",ARM9DASMlm     },
    {"STMDA%s %s",ARM9DASMlm     },
    {"STMDA%s %s",ARM9DASMlm     },
    {"STMDA%s %s",ARM9DASMlm     },

    //0x87
    {"LDMDA%s %s",ARM9DASMlm     },
    {"LDMDA%s %s",ARM9DASMlm     },
    {"LDMDA%s %s",ARM9DASMlm     },
    {"LDMDA%s %s",ARM9DASMlm     },
    {"LDMDA%s %s",ARM9DASMlm     },
    {"LDMDA%s %s",ARM9DASMlm     },
    {"LDMDA%s %s",ARM9DASMlm     },
    {"LDMDA%s %s",ARM9DASMlm     },
    {"LDMDA%s %s",ARM9DASMlm     },
    {"LDMDA%s %s",ARM9DASMlm     },
    {"LDMDA%s %s",ARM9DASMlm     },
    {"LDMDA%s %s",ARM9DASMlm     },
    {"LDMDA%s %s",ARM9DASMlm     },
    {"LDMDA%s %s",ARM9DASMlm     },
    {"LDMDA%s %s",ARM9DASMlm     },
    {"LDMDA%s %s",ARM9DASMlm     },

    //0x88
    {"STMIA%s %s",ARM9DASMlm       },
    {"STMIA%s %s",ARM9DASMlm       },
    {"STMIA%s %s",ARM9DASMlm       },
    {"STMIA%s %s",ARM9DASMlm       },
    {"STMIA%s %s",ARM9DASMlm       },
    {"STMIA%s %s",ARM9DASMlm       },
    {"STMIA%s %s",ARM9DASMlm       },
    {"STMIA%s %s",ARM9DASMlm       },
    {"STMIA%s %s",ARM9DASMlm       },
    {"STMIA%s %s",ARM9DASMlm       },
    {"STMIA%s %s",ARM9DASMlm       },
    {"STMIA%s %s",ARM9DASMlm       },
    {"STMIA%s %s",ARM9DASMlm       },
    {"STMIA%s %s",ARM9DASMlm       },
    {"STMIA%s %s",ARM9DASMlm       },
    {"STMIA%s %s",ARM9DASMlm       },

    //0x89
    {"LDMIA%s %s",ARM9DASMlm       },
    {"LDMIA%s %s",ARM9DASMlm       },
    {"LDMIA%s %s",ARM9DASMlm       },
    {"LDMIA%s %s",ARM9DASMlm       },
    {"LDMIA%s %s",ARM9DASMlm       },
    {"LDMIA%s %s",ARM9DASMlm       },
    {"LDMIA%s %s",ARM9DASMlm       },
    {"LDMIA%s %s",ARM9DASMlm       },
    {"LDMIA%s %s",ARM9DASMlm       },
    {"LDMIA%s %s",ARM9DASMlm       },
    {"LDMIA%s %s",ARM9DASMlm       },
    {"LDMIA%s %s",ARM9DASMlm       },
    {"LDMIA%s %s",ARM9DASMlm       },
    {"LDMIA%s %s",ARM9DASMlm       },
    {"LDMIA%s %s",ARM9DASMlm       },
    {"LDMIA%s %s",ARM9DASMlm       },

    //0x8A
    {"STMIA%s %s",ARM9DASMlm      },
    {"STMIA%s %s",ARM9DASMlm      },
    {"STMIA%s %s",ARM9DASMlm      },
    {"STMIA%s %s",ARM9DASMlm      },
    {"STMIA%s %s",ARM9DASMlm      },
    {"STMIA%s %s",ARM9DASMlm      },
    {"STMIA%s %s",ARM9DASMlm      },
    {"STMIA%s %s",ARM9DASMlm      },
    {"STMIA%s %s",ARM9DASMlm      },
    {"STMIA%s %s",ARM9DASMlm      },
    {"STMIA%s %s",ARM9DASMlm      },
    {"STMIA%s %s",ARM9DASMlm      },
    {"STMIA%s %s",ARM9DASMlm      },
    {"STMIA%s %s",ARM9DASMlm      },
    {"STMIA%s %s",ARM9DASMlm      },
    {"STMIA%s %s",ARM9DASMlm      },

    //0x8B
    {"LDMIA%s %s",ARM9DASMlm      },
    {"LDMIA%s %s",ARM9DASMlm      },
    {"LDMIA%s %s",ARM9DASMlm      },
    {"LDMIA%s %s",ARM9DASMlm      },
    {"LDMIA%s %s",ARM9DASMlm      },
    {"LDMIA%s %s",ARM9DASMlm      },
    {"LDMIA%s %s",ARM9DASMlm      },
    {"LDMIA%s %s",ARM9DASMlm      },
    {"LDMIA%s %s",ARM9DASMlm      },
    {"LDMIA%s %s",ARM9DASMlm      },
    {"LDMIA%s %s",ARM9DASMlm      },
    {"LDMIA%s %s",ARM9DASMlm      },
    {"LDMIA%s %s",ARM9DASMlm      },
    {"LDMIA%s %s",ARM9DASMlm      },
    {"LDMIA%s %s",ARM9DASMlm      },
    {"LDMIA%s %s",ARM9DASMlm      },

    //0x8C
    {"STMIA%s %s",ARM9DASMlm      },
    {"STMIA%s %s",ARM9DASMlm      },
    {"STMIA%s %s",ARM9DASMlm      },
    {"STMIA%s %s",ARM9DASMlm      },
    {"STMIA%s %s",ARM9DASMlm      },
    {"STMIA%s %s",ARM9DASMlm      },
    {"STMIA%s %s",ARM9DASMlm      },
    {"STMIA%s %s",ARM9DASMlm      },
    {"STMIA%s %s",ARM9DASMlm      },
    {"STMIA%s %s",ARM9DASMlm      },
    {"STMIA%s %s",ARM9DASMlm      },
    {"STMIA%s %s",ARM9DASMlm      },
    {"STMIA%s %s",ARM9DASMlm      },
    {"STMIA%s %s",ARM9DASMlm      },
    {"STMIA%s %s",ARM9DASMlm      },
    {"STMIA%s %s",ARM9DASMlm      },

    //0x8D
    {"LDMIA%s %s",ARM9DASMlm      },
    {"LDMIA%s %s",ARM9DASMlm      },
    {"LDMIA%s %s",ARM9DASMlm      },
    {"LDMIA%s %s",ARM9DASMlm      },
    {"LDMIA%s %s",ARM9DASMlm      },
    {"LDMIA%s %s",ARM9DASMlm      },
    {"LDMIA%s %s",ARM9DASMlm      },
    {"LDMIA%s %s",ARM9DASMlm      },
    {"LDMIA%s %s",ARM9DASMlm      },
    {"LDMIA%s %s",ARM9DASMlm      },
    {"LDMIA%s %s",ARM9DASMlm      },
    {"LDMIA%s %s",ARM9DASMlm      },
    {"LDMIA%s %s",ARM9DASMlm      },
    {"LDMIA%s %s",ARM9DASMlm      },
    {"LDMIA%s %s",ARM9DASMlm      },
    {"LDMIA%s %s",ARM9DASMlm      },

    //0x8E
    {"STMIA%s %s",ARM9DASMlm     },
    {"STMIA%s %s",ARM9DASMlm     },
    {"STMIA%s %s",ARM9DASMlm     },
    {"STMIA%s %s",ARM9DASMlm     },
    {"STMIA%s %s",ARM9DASMlm     },
    {"STMIA%s %s",ARM9DASMlm     },
    {"STMIA%s %s",ARM9DASMlm     },
    {"STMIA%s %s",ARM9DASMlm     },
    {"STMIA%s %s",ARM9DASMlm     },
    {"STMIA%s %s",ARM9DASMlm     },
    {"STMIA%s %s",ARM9DASMlm     },
    {"STMIA%s %s",ARM9DASMlm     },
    {"STMIA%s %s",ARM9DASMlm     },
    {"STMIA%s %s",ARM9DASMlm     },
    {"STMIA%s %s",ARM9DASMlm     },
    {"STMIA%s %s",ARM9DASMlm     },

    //0x8F
    {"LDMIA%s %s",ARM9DASMlm     },
    {"LDMIA%s %s",ARM9DASMlm     },
    {"LDMIA%s %s",ARM9DASMlm     },
    {"LDMIA%s %s",ARM9DASMlm     },
    {"LDMIA%s %s",ARM9DASMlm     },
    {"LDMIA%s %s",ARM9DASMlm     },
    {"LDMIA%s %s",ARM9DASMlm     },
    {"LDMIA%s %s",ARM9DASMlm     },
    {"LDMIA%s %s",ARM9DASMlm     },
    {"LDMIA%s %s",ARM9DASMlm     },
    {"LDMIA%s %s",ARM9DASMlm     },
    {"LDMIA%s %s",ARM9DASMlm     },
    {"LDMIA%s %s",ARM9DASMlm     },
    {"LDMIA%s %s",ARM9DASMlm     },
    {"LDMIA%s %s",ARM9DASMlm     },
    {"LDMIA%s %s",ARM9DASMlm     },

    //0x90
    {"STMDB%s %s",ARM9DASMlm       },
    {"STMDB%s %s",ARM9DASMlm       },
    {"STMDB%s %s",ARM9DASMlm       },
    {"STMDB%s %s",ARM9DASMlm       },
    {"STMDB%s %s",ARM9DASMlm       },
    {"STMDB%s %s",ARM9DASMlm       },
    {"STMDB%s %s",ARM9DASMlm       },
    {"STMDB%s %s",ARM9DASMlm       },
    {"STMDB%s %s",ARM9DASMlm       },
    {"STMDB%s %s",ARM9DASMlm       },
    {"STMDB%s %s",ARM9DASMlm       },
    {"STMDB%s %s",ARM9DASMlm       },
    {"STMDB%s %s",ARM9DASMlm       },
    {"STMDB%s %s",ARM9DASMlm       },
    {"STMDB%s %s",ARM9DASMlm       },
    {"STMDB%s %s",ARM9DASMlm       },

    //0x91
    {"LDMDB%s %s",ARM9DASMlm       },
    {"LDMDB%s %s",ARM9DASMlm       },
    {"LDMDB%s %s",ARM9DASMlm       },
    {"LDMDB%s %s",ARM9DASMlm       },
    {"LDMDB%s %s",ARM9DASMlm       },
    {"LDMDB%s %s",ARM9DASMlm       },
    {"LDMDB%s %s",ARM9DASMlm       },
    {"LDMDB%s %s",ARM9DASMlm       },
    {"LDMDB%s %s",ARM9DASMlm       },
    {"LDMDB%s %s",ARM9DASMlm       },
    {"LDMDB%s %s",ARM9DASMlm       },
    {"LDMDB%s %s",ARM9DASMlm       },
    {"LDMDB%s %s",ARM9DASMlm       },
    {"LDMDB%s %s",ARM9DASMlm       },
    {"LDMDB%s %s",ARM9DASMlm       },
    {"LDMDB%s %s",ARM9DASMlm       },

    //0x92
    {"STMDB%s %s",ARM9DASMlm      },
    {"STMDB%s %s",ARM9DASMlm      },
    {"STMDB%s %s",ARM9DASMlm      },
    {"STMDB%s %s",ARM9DASMlm      },
    {"STMDB%s %s",ARM9DASMlm      },
    {"STMDB%s %s",ARM9DASMlm      },
    {"STMDB%s %s",ARM9DASMlm      },
    {"STMDB%s %s",ARM9DASMlm      },
    {"STMDB%s %s",ARM9DASMlm      },
    {"STMDB%s %s",ARM9DASMlm      },
    {"STMDB%s %s",ARM9DASMlm      },
    {"STMDB%s %s",ARM9DASMlm      },
    {"STMDB%s %s",ARM9DASMlm      },
    {"STMDB%s %s",ARM9DASMlm      },
    {"STMDB%s %s",ARM9DASMlm      },
    {"STMDB%s %s",ARM9DASMlm      },

    //0x93
    {"LDMDB%s %s",ARM9DASMlm      },
    {"LDMDB%s %s",ARM9DASMlm      },
    {"LDMDB%s %s",ARM9DASMlm      },
    {"LDMDB%s %s",ARM9DASMlm      },
    {"LDMDB%s %s",ARM9DASMlm      },
    {"LDMDB%s %s",ARM9DASMlm      },
    {"LDMDB%s %s",ARM9DASMlm      },
    {"LDMDB%s %s",ARM9DASMlm      },
    {"LDMDB%s %s",ARM9DASMlm      },
    {"LDMDB%s %s",ARM9DASMlm      },
    {"LDMDB%s %s",ARM9DASMlm      },
    {"LDMDB%s %s",ARM9DASMlm      },
    {"LDMDB%s %s",ARM9DASMlm      },
    {"LDMDB%s %s",ARM9DASMlm      },
    {"LDMDB%s %s",ARM9DASMlm      },
    {"LDMDB%s %s",ARM9DASMlm      },

    //0x94
    {"STMDB%s %s",ARM9DASMlm      },
    {"STMDB%s %s",ARM9DASMlm      },
    {"STMDB%s %s",ARM9DASMlm      },
    {"STMDB%s %s",ARM9DASMlm      },
    {"STMDB%s %s",ARM9DASMlm      },
    {"STMDB%s %s",ARM9DASMlm      },
    {"STMDB%s %s",ARM9DASMlm      },
    {"STMDB%s %s",ARM9DASMlm      },
    {"STMDB%s %s",ARM9DASMlm      },
    {"STMDB%s %s",ARM9DASMlm      },
    {"STMDB%s %s",ARM9DASMlm      },
    {"STMDB%s %s",ARM9DASMlm      },
    {"STMDB%s %s",ARM9DASMlm      },
    {"STMDB%s %s",ARM9DASMlm      },
    {"STMDB%s %s",ARM9DASMlm      },
    {"STMDB%s %s",ARM9DASMlm      },

    //0x95
    {"LDMDB%s %s",ARM9DASMlm      },
    {"LDMDB%s %s",ARM9DASMlm      },
    {"LDMDB%s %s",ARM9DASMlm      },
    {"LDMDB%s %s",ARM9DASMlm      },
    {"LDMDB%s %s",ARM9DASMlm      },
    {"LDMDB%s %s",ARM9DASMlm      },
    {"LDMDB%s %s",ARM9DASMlm      },
    {"LDMDB%s %s",ARM9DASMlm      },
    {"LDMDB%s %s",ARM9DASMlm      },
    {"LDMDB%s %s",ARM9DASMlm      },
    {"LDMDB%s %s",ARM9DASMlm      },
    {"LDMDB%s %s",ARM9DASMlm      },
    {"LDMDB%s %s",ARM9DASMlm      },
    {"LDMDB%s %s",ARM9DASMlm      },
    {"LDMDB%s %s",ARM9DASMlm      },
    {"LDMDB%s %s",ARM9DASMlm      },

    //0x96
    {"STMDB%s %s",ARM9DASMlm     },
    {"STMDB%s %s",ARM9DASMlm     },
    {"STMDB%s %s",ARM9DASMlm     },
    {"STMDB%s %s",ARM9DASMlm     },
    {"STMDB%s %s",ARM9DASMlm     },
    {"STMDB%s %s",ARM9DASMlm     },
    {"STMDB%s %s",ARM9DASMlm     },
    {"STMDB%s %s",ARM9DASMlm     },
    {"STMDB%s %s",ARM9DASMlm     },
    {"STMDB%s %s",ARM9DASMlm     },
    {"STMDB%s %s",ARM9DASMlm     },
    {"STMDB%s %s",ARM9DASMlm     },
    {"STMDB%s %s",ARM9DASMlm     },
    {"STMDB%s %s",ARM9DASMlm     },
    {"STMDB%s %s",ARM9DASMlm     },
    {"STMDB%s %s",ARM9DASMlm     },

    //0x97
    {"LDMDB%s %s",ARM9DASMlm     },
    {"LDMDB%s %s",ARM9DASMlm     },
    {"LDMDB%s %s",ARM9DASMlm     },
    {"LDMDB%s %s",ARM9DASMlm     },
    {"LDMDB%s %s",ARM9DASMlm     },
    {"LDMDB%s %s",ARM9DASMlm     },
    {"LDMDB%s %s",ARM9DASMlm     },
    {"LDMDB%s %s",ARM9DASMlm     },
    {"LDMDB%s %s",ARM9DASMlm     },
    {"LDMDB%s %s",ARM9DASMlm     },
    {"LDMDB%s %s",ARM9DASMlm     },
    {"LDMDB%s %s",ARM9DASMlm     },
    {"LDMDB%s %s",ARM9DASMlm     },
    {"LDMDB%s %s",ARM9DASMlm     },
    {"LDMDB%s %s",ARM9DASMlm     },
    {"LDMDB%s %s",ARM9DASMlm     },

    //0x98
    {"STMIB%s %s",ARM9DASMlm       },
    {"STMIB%s %s",ARM9DASMlm       },
    {"STMIB%s %s",ARM9DASMlm       },
    {"STMIB%s %s",ARM9DASMlm       },
    {"STMIB%s %s",ARM9DASMlm       },
    {"STMIB%s %s",ARM9DASMlm       },
    {"STMIB%s %s",ARM9DASMlm       },
    {"STMIB%s %s",ARM9DASMlm       },
    {"STMIB%s %s",ARM9DASMlm       },
    {"STMIB%s %s",ARM9DASMlm       },
    {"STMIB%s %s",ARM9DASMlm       },
    {"STMIB%s %s",ARM9DASMlm       },
    {"STMIB%s %s",ARM9DASMlm       },
    {"STMIB%s %s",ARM9DASMlm       },
    {"STMIB%s %s",ARM9DASMlm       },
    {"STMIB%s %s",ARM9DASMlm       },

    //0x99
    {"LDMIB%s %s",ARM9DASMlm       },
    {"LDMIB%s %s",ARM9DASMlm       },
    {"LDMIB%s %s",ARM9DASMlm       },
    {"LDMIB%s %s",ARM9DASMlm       },
    {"LDMIB%s %s",ARM9DASMlm       },
    {"LDMIB%s %s",ARM9DASMlm       },
    {"LDMIB%s %s",ARM9DASMlm       },
    {"LDMIB%s %s",ARM9DASMlm       },
    {"LDMIB%s %s",ARM9DASMlm       },
    {"LDMIB%s %s",ARM9DASMlm       },
    {"LDMIB%s %s",ARM9DASMlm       },
    {"LDMIB%s %s",ARM9DASMlm       },
    {"LDMIB%s %s",ARM9DASMlm       },
    {"LDMIB%s %s",ARM9DASMlm       },
    {"LDMIB%s %s",ARM9DASMlm       },
    {"LDMIB%s %s",ARM9DASMlm       },

    //0x9A
    {"STMIB%s %s",ARM9DASMlm      },
    {"STMIB%s %s",ARM9DASMlm      },
    {"STMIB%s %s",ARM9DASMlm      },
    {"STMIB%s %s",ARM9DASMlm      },
    {"STMIB%s %s",ARM9DASMlm      },
    {"STMIB%s %s",ARM9DASMlm      },
    {"STMIB%s %s",ARM9DASMlm      },
    {"STMIB%s %s",ARM9DASMlm      },
    {"STMIB%s %s",ARM9DASMlm      },
    {"STMIB%s %s",ARM9DASMlm      },
    {"STMIB%s %s",ARM9DASMlm      },
    {"STMIB%s %s",ARM9DASMlm      },
    {"STMIB%s %s",ARM9DASMlm      },
    {"STMIB%s %s",ARM9DASMlm      },
    {"STMIB%s %s",ARM9DASMlm      },
    {"STMIB%s %s",ARM9DASMlm      },

    //0x9B
    {"LDMIB%s %s",ARM9DASMlm      },
    {"LDMIB%s %s",ARM9DASMlm      },
    {"LDMIB%s %s",ARM9DASMlm      },
    {"LDMIB%s %s",ARM9DASMlm      },
    {"LDMIB%s %s",ARM9DASMlm      },
    {"LDMIB%s %s",ARM9DASMlm      },
    {"LDMIB%s %s",ARM9DASMlm      },
    {"LDMIB%s %s",ARM9DASMlm      },
    {"LDMIB%s %s",ARM9DASMlm      },
    {"LDMIB%s %s",ARM9DASMlm      },
    {"LDMIB%s %s",ARM9DASMlm      },
    {"LDMIB%s %s",ARM9DASMlm      },
    {"LDMIB%s %s",ARM9DASMlm      },
    {"LDMIB%s %s",ARM9DASMlm      },
    {"LDMIB%s %s",ARM9DASMlm      },
    {"LDMIB%s %s",ARM9DASMlm      },

    //0x9C
    {"STMIB%s %s",ARM9DASMlm      },
    {"STMIB%s %s",ARM9DASMlm      },
    {"STMIB%s %s",ARM9DASMlm      },
    {"STMIB%s %s",ARM9DASMlm      },
    {"STMIB%s %s",ARM9DASMlm      },
    {"STMIB%s %s",ARM9DASMlm      },
    {"STMIB%s %s",ARM9DASMlm      },
    {"STMIB%s %s",ARM9DASMlm      },
    {"STMIB%s %s",ARM9DASMlm      },
    {"STMIB%s %s",ARM9DASMlm      },
    {"STMIB%s %s",ARM9DASMlm      },
    {"STMIB%s %s",ARM9DASMlm      },
    {"STMIB%s %s",ARM9DASMlm      },
    {"STMIB%s %s",ARM9DASMlm      },
    {"STMIB%s %s",ARM9DASMlm      },
    {"STMIB%s %s",ARM9DASMlm      },

    //0x9D
    {"LDMIB%s %s",ARM9DASMlm      },
    {"LDMIB%s %s",ARM9DASMlm      },
    {"LDMIB%s %s",ARM9DASMlm      },
    {"LDMIB%s %s",ARM9DASMlm      },
    {"LDMIB%s %s",ARM9DASMlm      },
    {"LDMIB%s %s",ARM9DASMlm      },
    {"LDMIB%s %s",ARM9DASMlm      },
    {"LDMIB%s %s",ARM9DASMlm      },
    {"LDMIB%s %s",ARM9DASMlm      },
    {"LDMIB%s %s",ARM9DASMlm      },
    {"LDMIB%s %s",ARM9DASMlm      },
    {"LDMIB%s %s",ARM9DASMlm      },
    {"LDMIB%s %s",ARM9DASMlm      },
    {"LDMIB%s %s",ARM9DASMlm      },
    {"LDMIB%s %s",ARM9DASMlm      },
    {"LDMIB%s %s",ARM9DASMlm      },

    //0x9E
    {"STMIB%s %s",ARM9DASMlm     },
    {"STMIB%s %s",ARM9DASMlm     },
    {"STMIB%s %s",ARM9DASMlm     },
    {"STMIB%s %s",ARM9DASMlm     },
    {"STMIB%s %s",ARM9DASMlm     },
    {"STMIB%s %s",ARM9DASMlm     },
    {"STMIB%s %s",ARM9DASMlm     },
    {"STMIB%s %s",ARM9DASMlm     },
    {"STMIB%s %s",ARM9DASMlm     },
    {"STMIB%s %s",ARM9DASMlm     },
    {"STMIB%s %s",ARM9DASMlm     },
    {"STMIB%s %s",ARM9DASMlm     },
    {"STMIB%s %s",ARM9DASMlm     },
    {"STMIB%s %s",ARM9DASMlm     },
    {"STMIB%s %s",ARM9DASMlm     },
    {"STMIB%s %s",ARM9DASMlm     },

    //0x9F
    {"LDMIB%s %s",ARM9DASMlm     },
    {"LDMIB%s %s",ARM9DASMlm     },
    {"LDMIB%s %s",ARM9DASMlm     },
    {"LDMIB%s %s",ARM9DASMlm     },
    {"LDMIB%s %s",ARM9DASMlm     },
    {"LDMIB%s %s",ARM9DASMlm     },
    {"LDMIB%s %s",ARM9DASMlm     },
    {"LDMIB%s %s",ARM9DASMlm     },
    {"LDMIB%s %s",ARM9DASMlm     },
    {"LDMIB%s %s",ARM9DASMlm     },
    {"LDMIB%s %s",ARM9DASMlm     },
    {"LDMIB%s %s",ARM9DASMlm     },
    {"LDMIB%s %s",ARM9DASMlm     },
    {"LDMIB%s %s",ARM9DASMlm     },
    {"LDMIB%s %s",ARM9DASMlm     },
    {"LDMIB%s %s",ARM9DASMlm     },

    //0xA0
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },

    //0xA1
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },

    //0xA2
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },

    //0xA3
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },

    //0xA4
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },

    //0xA5
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },

    //0xA6
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },

    //0xA7
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },

    //0xA8
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },

    //0xA9
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },

    //0xAA
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },

    //0xAB
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },

    //0xAC
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },

    //0xAD
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },

    //0xAE
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },

    //0xAF
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },
    {"B%s %s",ARM9DASMb           },

    //0xB0
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },

    //0xB1
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },

    //0xB2
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },

    //0xB3
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },

    //0xB4
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },

    //0xB5
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },

    //0xB6
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },

    //0xB7
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },

    //0xB8
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },

    //0xB9
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },

    //0xBA
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },

    //0xBB
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },

    //0xBC
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },

    //0xBD
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },

    //0xBE
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },

    //0xBF
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },
    {"BL%s %s",ARM9DASMb          },

    //0xC0 // Coprocessors undefined here!
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },

    //0xC1
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },

    //0xC2
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },

    //0xC3
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },

    //0xC4
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },

    //0xC5
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },

    //0xC6
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },

    //0xC7
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },

    //0xC8
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },

    //0xC9
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },

    //0xCA
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },

    //0xCB
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },

    //0xCC
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },

    //0xCD
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },

    //0xCE
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },

    //0xCF
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },

    //0xD0
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },

    //0xD1
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },

    //0xD2
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },

    //0xD3
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },

    //0xD4
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },

    //0xD5
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },

    //0xD6
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },

    //0xD7
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },

    //0xD8
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },

    //0xD9
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },

    //0xDA
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },

    //0xDB
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },

    //0xDC
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },

    //0xDD
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },

    //0xDE
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },

    //0xDF
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },
    {"UNI%s %s",ARM9DASMun  },

    //0xE0
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },

    //0xE1
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },

    //0xE2
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },

    //0xE3
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },

    //0xE4
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },

    //0xE5
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },

    //0xE6
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },

    //0xE7
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },

    //0xE8
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },

    //0xE9
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },

    //0xEA
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },

    //0xEB
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },

    //0xEC
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },

    //0xED
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },

    //0xEE
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MCR%s %s",ARM9DASMmcr         },

    //0xEF
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },
    {"CDP%s %s",ARM9DASMcpd         },
    {"MRC%s %s",ARM9DASMmcr         },

    //0xF0
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },

    //0xF1
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },

    //0xF2
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },

    //0xF3
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },

    //0xF4
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },

    //0xF5
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },

    //0xF6
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },

    //0xF7
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },

    //0xF8
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },

    //0xF9
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },

    //0xFA
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },

    //0xFB
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },

    //0xFC
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },

    //0xFD
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },

    //0xFE
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },

    //0xFF
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },
    {"SWI%s %s",ARM9DASMswi         },

};

const ARM9DASMOPDESC thumb9dasmops[]={

    //0x00
    {"LSL %s",   Thumb9DASMimm5shft},
    {"LSL %s",   Thumb9DASMimm5shft},
    {"LSL %s",   Thumb9DASMimm5shft},
    {"LSL %s",   Thumb9DASMimm5shft},
    {"LSL %s",   Thumb9DASMimm5shft},
    {"LSL %s",   Thumb9DASMimm5shft},
    {"LSL %s",   Thumb9DASMimm5shft},
    {"LSL %s",   Thumb9DASMimm5shft},
    {"LSR %s",   Thumb9DASMimm5shft},
    {"LSR %s",   Thumb9DASMimm5shft},
    {"LSR %s",   Thumb9DASMimm5shft},
    {"LSR %s",   Thumb9DASMimm5shft},
    {"LSR %s",   Thumb9DASMimm5shft},
    {"LSR %s",   Thumb9DASMimm5shft},
    {"LSR %s",   Thumb9DASMimm5shft},
    {"LSR %s",   Thumb9DASMimm5shft},

    //0x10
    {"ASR %s",   Thumb9DASMimm5shft},
    {"ASR %s",   Thumb9DASMimm5shft},
    {"ASR %s",   Thumb9DASMimm5shft},
    {"ASR %s",   Thumb9DASMimm5shft},
    {"ASR %s",   Thumb9DASMimm5shft},
    {"ASR %s",   Thumb9DASMimm5shft},
    {"ASR %s",   Thumb9DASMimm5shft},
    {"ASR %s",   Thumb9DASMimm5shft},
    {"ADD %s",   Thumb9DASMreg     },
    {"ADD %s",   Thumb9DASMreg     },
    {"SUB %s",   Thumb9DASMreg     },
    {"SUB %s",   Thumb9DASMreg     },
    {"ADD %s",   Thumb9DASMimm3    },
    {"ADD %s",   Thumb9DASMimm3    },
    {"SUB %s",   Thumb9DASMimm3    },
    {"SUB %s",   Thumb9DASMimm3    },

    //0x20
    {"MOV r0, %s",Thumb9DASMimm8    },
    {"MOV r1, %s",Thumb9DASMimm8    },
    {"MOV r2, %s",Thumb9DASMimm8    },
    {"MOV r3, %s",Thumb9DASMimm8    },
    {"MOV r4, %s",Thumb9DASMimm8    },
    {"MOV r5, %s",Thumb9DASMimm8    },
    {"MOV r6, %s",Thumb9DASMimm8    },
    {"MOV r7, %s",Thumb9DASMimm8    },
    {"CMP r0, %s",Thumb9DASMimm8    },
    {"CMP r1, %s",Thumb9DASMimm8    },
    {"CMP r2, %s",Thumb9DASMimm8    },
    {"CMP r3, %s",Thumb9DASMimm8    },
    {"CMP r4, %s",Thumb9DASMimm8    },
    {"CMP r5, %s",Thumb9DASMimm8    },
    {"CMP r6, %s",Thumb9DASMimm8    },
    {"CMP r7, %s",Thumb9DASMimm8    },

    //0x30
    {"ADD r0, %s",Thumb9DASMimm8    },
    {"ADD r1, %s",Thumb9DASMimm8    },
    {"ADD r2, %s",Thumb9DASMimm8    },
    {"ADD r3, %s",Thumb9DASMimm8    },
    {"ADD r4, %s",Thumb9DASMimm8    },
    {"ADD r5, %s",Thumb9DASMimm8    },
    {"ADD r6, %s",Thumb9DASMimm8    },
    {"ADD r7, %s",Thumb9DASMimm8    },
    {"SUB r0, %s",Thumb9DASMimm8    },
    {"SUB r1, %s",Thumb9DASMimm8    },
    {"SUB r2, %s",Thumb9DASMimm8    },
    {"SUB r3, %s",Thumb9DASMimm8    },
    {"SUB r4, %s",Thumb9DASMimm8    },
    {"SUB r5, %s",Thumb9DASMimm8    },
    {"SUB r6, %s",Thumb9DASMimm8    },
    {"SUB r7, %s",Thumb9DASMimm8    },

    //0x40
    {"%s",       Thumb9DASMdp1     },
    {"%s",       Thumb9DASMdp2     },
    {"%s",       Thumb9DASMdp3     },
    {"%s",       Thumb9DASMdp4     },
    {"ADD %s",   Thumb9DASMh       },
    {"CMP %s",   Thumb9DASMh       },
    {"MOV %s",   Thumb9DASMh       },
    {"%s",       Thumb9DASMbx      },
    {"LDR r0, %s",Thumb9DASMpc      },
    {"LDR r1, %s",Thumb9DASMpc      },
    {"LDR r2, %s",Thumb9DASMpc      },
    {"LDR r3, %s",Thumb9DASMpc      },
    {"LDR r4, %s",Thumb9DASMpc      },
    {"LDR r5, %s",Thumb9DASMpc      },
    {"LDR r6, %s",Thumb9DASMpc      },
    {"LDR r7, %s",Thumb9DASMpc      },

    //0x50
    {"STR %s",   Thumb9DASMreg     },
    {"STR %s",   Thumb9DASMreg     },
    {"STRH %s",  Thumb9DASMreg     },
    {"STRH %s",  Thumb9DASMreg     },
    {"STRB %s",  Thumb9DASMreg     },
    {"STRB %s",  Thumb9DASMreg     },
    {"LDRSB %s", Thumb9DASMreg     },
    {"LDRSB %s", Thumb9DASMreg     },
    {"LDR %s",   Thumb9DASMreg     },
    {"LDR %s",   Thumb9DASMreg     },
    {"LDRH %s",  Thumb9DASMreg     },
    {"LDRH %s",  Thumb9DASMreg     },
    {"LDRB %s",  Thumb9DASMreg     },
    {"LDRB %s",  Thumb9DASMreg     },
    {"LDRSH %s", Thumb9DASMreg     },
    {"LDRSH %s", Thumb9DASMreg     },

    //0x60
    {"STR %s*4]",Thumb9DASMimm5    },
    {"STR %s*4]",Thumb9DASMimm5    },
    {"STR %s*4]",Thumb9DASMimm5    },
    {"STR %s*4]",Thumb9DASMimm5    },
    {"STR %s*4]",Thumb9DASMimm5    },
    {"STR %s*4]",Thumb9DASMimm5    },
    {"STR %s*4]",Thumb9DASMimm5    },
    {"STR %s*4]",Thumb9DASMimm5    },
    {"LDR %s*4]",Thumb9DASMimm5    },
    {"LDR %s*4]",Thumb9DASMimm5    },
    {"LDR %s*4]",Thumb9DASMimm5    },
    {"LDR %s*4]",Thumb9DASMimm5    },
    {"LDR %s*4]",Thumb9DASMimm5    },
    {"LDR %s*4]",Thumb9DASMimm5    },
    {"LDR %s*4]",Thumb9DASMimm5    },
    {"LDR %s*4]",Thumb9DASMimm5    },

    //0x70
    {"STRB %s]", Thumb9DASMimm5    },
    {"STRB %s]", Thumb9DASMimm5    },
    {"STRB %s]", Thumb9DASMimm5    },
    {"STRB %s]", Thumb9DASMimm5    },
    {"STRB %s]", Thumb9DASMimm5    },
    {"STRB %s]", Thumb9DASMimm5    },
    {"STRB %s]", Thumb9DASMimm5    },
    {"STRB %s]", Thumb9DASMimm5    },
    {"LDRB %s]", Thumb9DASMimm5    },
    {"LDRB %s]", Thumb9DASMimm5    },
    {"LDRB %s]", Thumb9DASMimm5    },
    {"LDRB %s]", Thumb9DASMimm5    },
    {"LDRB %s]", Thumb9DASMimm5    },
    {"LDRB %s]", Thumb9DASMimm5    },
    {"LDRB %s]", Thumb9DASMimm5    },
    {"LDRB %s]", Thumb9DASMimm5    },

    //0x80
    {"STRH %s*2]",Thumb9DASMimm5   },
    {"STRH %s*2]",Thumb9DASMimm5   },
    {"STRH %s*2]",Thumb9DASMimm5   },
    {"STRH %s*2]",Thumb9DASMimm5   },
    {"STRH %s*2]",Thumb9DASMimm5   },
    {"STRH %s*2]",Thumb9DASMimm5   },
    {"STRH %s*2]",Thumb9DASMimm5   },
    {"STRH %s*2]",Thumb9DASMimm5   },
    {"LDRH %s*2]",Thumb9DASMimm5   },
    {"LDRH %s*2]",Thumb9DASMimm5   },
    {"LDRH %s*2]",Thumb9DASMimm5   },
    {"LDRH %s*2]",Thumb9DASMimm5   },
    {"LDRH %s*2]",Thumb9DASMimm5   },
    {"LDRH %s*2]",Thumb9DASMimm5   },
    {"LDRH %s*2]",Thumb9DASMimm5   },
    {"LDRH %s*2]",Thumb9DASMimm5   },

    //0x90
    {"STR r0, %s",Thumb9DASMsp      },
    {"STR r1, %s",Thumb9DASMsp      },
    {"STR r2, %s",Thumb9DASMsp      },
    {"STR r3, %s",Thumb9DASMsp      },
    {"STR r4, %s",Thumb9DASMsp      },
    {"STR r5, %s",Thumb9DASMsp      },
    {"STR r6, %s",Thumb9DASMsp      },
    {"STR r7, %s",Thumb9DASMsp      },
    {"LDR r0, %s",Thumb9DASMsp      },
    {"LDR r1, %s",Thumb9DASMsp      },
    {"LDR r2, %s",Thumb9DASMsp      },
    {"LDR r3, %s",Thumb9DASMsp      },
    {"LDR r4, %s",Thumb9DASMsp      },
    {"LDR r5, %s",Thumb9DASMsp      },
    {"LDR r6, %s",Thumb9DASMsp      },
    {"LDR r7, %s",Thumb9DASMsp      },

    //0xA0
    {"ADD r0, %s",Thumb9DASMpc      },
    {"ADD r1, %s",Thumb9DASMpc      },
    {"ADD r2, %s",Thumb9DASMpc      },
    {"ADD r3, %s",Thumb9DASMpc      },
    {"ADD r4, %s",Thumb9DASMpc      },
    {"ADD r5, %s",Thumb9DASMpc      },
    {"ADD r6, %s",Thumb9DASMpc      },
    {"ADD r7, %s",Thumb9DASMpc      },
    {"ADD r0, %s",Thumb9DASMsp      },
    {"ADD r1, %s",Thumb9DASMsp      },
    {"ADD r2, %s",Thumb9DASMsp      },
    {"ADD r3, %s",Thumb9DASMsp      },
    {"ADD r4, %s",Thumb9DASMsp      },
    {"ADD r5, %s",Thumb9DASMsp      },
    {"ADD r6, %s",Thumb9DASMsp      },
    {"ADD r7, %s",Thumb9DASMsp      },

    //0xB0
    {"%s",       Thumb9DASMimm7    },
    {"UND %s",   Thumb9DASMund     },  // Misc map: unallocated
    {"UND %s",   Thumb9DASMund     },  // Misc map: unallocated
    {"UND %s",   Thumb9DASMund     },  // Misc map: unallocated
    {"PUSH %s",  Thumb9DASMldm     },
    {"PUSH %s, LR",Thumb9DASMldm    },
    {"UND %s",   Thumb9DASMund     },  // Misc map: unallocated
    {"UND %s",   Thumb9DASMund     },  // Misc map: unallocated
    {"UND %s",   Thumb9DASMund     },  // Misc map: unallocated
    {"UND %s",   Thumb9DASMund     },  // Misc map: unallocated
    {"UND %s",   Thumb9DASMund     },  // Misc map: unallocated
    {"UND %s",   Thumb9DASMund     },  // Misc map: unallocated
    {"POP %s",   Thumb9DASMldm     },
    {"POP %s, PC",Thumb9DASMldm     },
    {"BKPT %s",  Thumb9DASMbkpt    },
    {"UND %s",   Thumb9DASMund     },  // Misc map: unallocated

    //0xC0
    {"STMIA r0!, %s", Thumb9DASMldm },
    {"STMIA r1!, %s", Thumb9DASMldm },
    {"STMIA r2!, %s", Thumb9DASMldm },
    {"STMIA r3!, %s", Thumb9DASMldm },
    {"STMIA r4!, %s", Thumb9DASMldm },
    {"STMIA r5!, %s", Thumb9DASMldm },
    {"STMIA r6!, %s", Thumb9DASMldm },
    {"STMIA r7!, %s", Thumb9DASMldm },
    {"LDMIA r0!, %s", Thumb9DASMldm },
    {"LDMIA r1!, %s", Thumb9DASMldm },
    {"LDMIA r2!, %s", Thumb9DASMldm },
    {"LDMIA r3!, %s", Thumb9DASMldm },
    {"LDMIA r4!, %s", Thumb9DASMldm },
    {"LDMIA r5!, %s", Thumb9DASMldm },
    {"LDMIA r6!, %s", Thumb9DASMldm },
    {"LDMIA r7!, %s", Thumb9DASMldm },

    //0xD0
    {"BEQ %s",   Thumb9DASMbc      },
    {"BNE %s",   Thumb9DASMbc      },
    {"BCS %s",   Thumb9DASMbc      },
    {"BCC %s",   Thumb9DASMbc      },
    {"BMI %s",   Thumb9DASMbc      },
    {"BPL %s",   Thumb9DASMbc      },
    {"BVS %s",   Thumb9DASMbc      },
    {"BVC %s",   Thumb9DASMbc      },
    {"BHI %s",   Thumb9DASMbc      },
    {"BLS %s",   Thumb9DASMbc      },
    {"BGE %s",   Thumb9DASMbc      },
    {"BLT %s",   Thumb9DASMbc      },
    {"BGT %s",   Thumb9DASMbc      },
    {"BLE %s",   Thumb9DASMbc      },
    {"UND %s",   Thumb9DASMund     },  // Covered by unconditional
    {"SWI %s",   Thumb9DASMswi     },

    //0xE0
    {"B %s",     Thumb9DASMb       },
    {"B %s",     Thumb9DASMb       },
    {"B %s",     Thumb9DASMb       },
    {"B %s",     Thumb9DASMb       },
    {"B %s",     Thumb9DASMb       },
    {"B %s",     Thumb9DASMb       },
    {"B %s",     Thumb9DASMb       },
    {"B %s",     Thumb9DASMb       },
    {"UND %s",   Thumb9DASMund     },  // Defined in ARM5!
    {"UND %s",   Thumb9DASMund     },  // Defined in ARM5!
    {"UND %s",   Thumb9DASMund     },  // Defined in ARM5!
    {"UND %s",   Thumb9DASMund     },  // Defined in ARM5!
    {"UND %s",   Thumb9DASMund     },  // Defined in ARM5!
    {"UND %s",   Thumb9DASMund     },  // Defined in ARM5!
    {"UND %s",   Thumb9DASMund     },  // Defined in ARM5!
    {"UND %s",   Thumb9DASMund     },  // Defined in ARM5!

    //0xF0
    {"BL %s*2048",Thumb9DASMbl     },
    {"BL %s*2048",Thumb9DASMbl     },
    {"BL %s*2048",Thumb9DASMbl     },
    {"BL %s*2048",Thumb9DASMbl     },
    {"BL %s*2048",Thumb9DASMbl     },
    {"BL %s*2048",Thumb9DASMbl     },
    {"BL %s*2048",Thumb9DASMbl     },
    {"BL %s*2048",Thumb9DASMbl     },
    {"BL +%s",   Thumb9DASMbl      },
    {"BL +%s",   Thumb9DASMbl      },
    {"BL +%s",   Thumb9DASMbl      },
    {"BL +%s",   Thumb9DASMbl      },
    {"BL +%s",   Thumb9DASMbl      },
    {"BL +%s",   Thumb9DASMbl      },
    {"BL +%s",   Thumb9DASMbl      },
    {"BL +%s",   Thumb9DASMbl      },
};

#endif//__ARM9DASM_H_

/*** EOF:arm9dasm.h ******************************************************/

/**************************************************************************
* DSemu: ARM9 per-instruction disassembler (arm9dasm.c)                   *
* Released under the terms of the BSD Public Licence                      *
* Imran Nazar (tf@oopsilon.com), 2004                                     *
**************************************************************************/

//#include <stdio.h>
//#include <string.h>
//#include "arm9dasm.h"
//#include "vtbl.h"

char *ARM9DASM(u32 op)
{
    static char str[100];
    u16 idx=((op&0x0FF00000)>>16)+((op&0x000000F0)>>4);
    arm9dasmops[idx].addr(op);
    arm9dasmstr[32]=0;
    sprintf(str,arm9dasmops[idx].op,
        ARM9DASMcond[((op&0xF0000000)>>28)],arm9dasmstr);
    return str;
}

char *Thumb9DASM(u32 op)
{
    static char str[100];
    u8 idx=((op&0xFF00)>>8);
    thumb9dasmops[idx].addr(op);
    arm9dasmstr[32]=0;
    sprintf(str,thumb9dasmops[idx].op,arm9dasmstr);
    return str;
}

void ARM9DASMun(u32 op)
{
    sprintf(arm9dasmstr,"");
}

//---Branching-------------------------------------------------------------

void ARM9DASMb(u32 op)
{
    signed int b = (op&0x00800000)?(0xFF000000|(op&0x00FFFFFF))
                                  :(op&0x00FFFFFF);
    sprintf(arm9dasmstr,"$%08X",arm9reg.tmp1+8+(b*4));
}

void ARM9DASMbreg(u32 op)
{
    sprintf(arm9dasmstr,"r%d",ARM9DASM_RM);
}

//---Data Processing addressing modes, opcodes-----------------------------

void ARM9DASMreg(u32 op)
{
    sprintf(arm9dasmstr,"r%d, r%d, r%d", ARM9DASM_RD, ARM9DASM_RN, ARM9DASM_RM);
}

void ARM9DASMimm(u32 op)
{
    arm9dasmtmp3 = op&255;
    arm9dasmtmp4 = (op&0x00000F00)>>7;
    arm9dasmtmp1 = (arm9dasmtmp3>>arm9dasmtmp4)|((arm9dasmtmp3&((1<<arm9dasmtmp4)-1))<<(32-arm9dasmtmp4));
    sprintf(arm9dasmstr,"r%d, r%d, #$%08X", ARM9DASM_RD, ARM9DASM_RN, arm9dasmtmp1);
}

void ARM9DASMlli(u32 op)
{
    arm9dasmtmp4=(op&0x00000F80)>>7;
    (arm9dasmtmp4)?
       sprintf(arm9dasmstr,"r%d, r%d, r%d, LSL #%d", ARM9DASM_RD, ARM9DASM_RN, ARM9DASM_RM, arm9dasmtmp4):
       sprintf(arm9dasmstr,"r%d, r%d, r%d", ARM9DASM_RD, ARM9DASM_RN, ARM9DASM_RM);
}

void ARM9DASMllr(u32 op)
{
    sprintf(arm9dasmstr,"r%d, r%d, r%d, LSL r%d", ARM9DASM_RD, ARM9DASM_RN, ARM9DASM_RM, ARM9DASM_RS);
}

void ARM9DASMlri(u32 op)
{
    arm9dasmtmp4=(op&0x00000F80)>>7;
    sprintf(arm9dasmstr,"r%d, r%d, r%d, LSR #%d", ARM9DASM_RD, ARM9DASM_RN, ARM9DASM_RM, arm9dasmtmp4);
}

void ARM9DASMlrr(u32 op)
{
    sprintf(arm9dasmstr,"r%d, r%d, r%d, LSR r%d", ARM9DASM_RD, ARM9DASM_RN, ARM9DASM_RM, ARM9DASM_RS);
}

void ARM9DASMari(u32 op) 
{
    arm9dasmtmp4=(op&0x00000F80)>>7; 
    sprintf(arm9dasmstr,"r%d, r%d, r%d, ASR #%d", ARM9DASM_RD, ARM9DASM_RN, ARM9DASM_RM, arm9dasmtmp4);
}

void ARM9DASMarr(u32 op)
{
    sprintf(arm9dasmstr,"r%d, r%d, r%d, ASR r%d", ARM9DASM_RD, ARM9DASM_RN, ARM9DASM_RM, ARM9DASM_RS);
}

void ARM9DASMrri(u32 op) 
{
    arm9dasmtmp4=(op&0x00000F80)>>7;
    if(arm9dasmtmp4)
        sprintf(arm9dasmstr,"r%d, r%d, r%d, ROR #%d", ARM9DASM_RD, ARM9DASM_RN, ARM9DASM_RM, arm9dasmtmp4);
    else
        sprintf(arm9dasmstr,"r%d, r%d, r%d, RRX", ARM9DASM_RD, ARM9DASM_RN, ARM9DASM_RM);
}

void ARM9DASMrrr(u32 op) 
{
    sprintf(arm9dasmstr,"r%d, r%d, r%d, ROR r%d", ARM9DASM_RD, ARM9DASM_RN, ARM9DASM_RM, ARM9DASM_RS);
}

//---Load/Store addressing modes, opcodes----------------------------------

void ARM9DASMofim(u32 op) 
{
    sprintf(arm9dasmstr,"r%d, [r%d, #-$%03X]", ARM9DASM_RD, ARM9DASM_RN, op&0x00000FFF);
}

void ARM9DASMofip(u32 op)
{
    sprintf(arm9dasmstr,"r%d, [r%d, #+$%03X]", ARM9DASM_RD, ARM9DASM_RN, op&0x00000FFF);
}

void ARM9DASMofrm(u32 op) 
{
    sprintf(arm9dasmstr,"r%d, [r%d, -r%d]", ARM9DASM_RD, ARM9DASM_RN, ARM9DASM_RM);
}

void ARM9DASMofrmll(u32 op)
{
    arm9dasmtmp4=(op&0x00000F80)>>7; 
    sprintf(arm9dasmstr,"r%d, [r%d, -r%d, LSL #%d]", ARM9DASM_RD, ARM9DASM_RN, ARM9DASM_RM, arm9dasmtmp4);
}

void ARM9DASMofrmlr(u32 op) 
{
    arm9dasmtmp4=(op&0x00000F80)>>7;
    sprintf(arm9dasmstr,"r%d, [r%d, -r%d, LSR #%d]", ARM9DASM_RD, ARM9DASM_RN, ARM9DASM_RM, arm9dasmtmp4);
}

void ARM9DASMofrmar(u32 op)
{
    arm9dasmtmp4=(op&0x00000F80)>>7; 
    sprintf(arm9dasmstr,"r%d, [r%d, -r%d, ASR #%d]", ARM9DASM_RD, ARM9DASM_RN, ARM9DASM_RM, arm9dasmtmp4);
}

void ARM9DASMofrmrr(u32 op) 
{
    arm9dasmtmp4=(op&0x00000F80)>>7;
    if(arm9dasmtmp4)
        sprintf(arm9dasmstr,"r%d, [r%d, -r%d, ROR #%d]", ARM9DASM_RD, ARM9DASM_RN, ARM9DASM_RM, arm9dasmtmp4);
    else
        sprintf(arm9dasmstr,"r%d, [r%d, -r%d, RRX]", ARM9DASM_RD, ARM9DASM_RN, ARM9DASM_RM);
}

void ARM9DASMofrp(u32 op) 
{
    sprintf(arm9dasmstr,"r%d, [r%d, +r%d]", ARM9DASM_RD, ARM9DASM_RN, ARM9DASM_RM);
}    

void ARM9DASMofrpll(u32 op) 
{
    arm9dasmtmp4=(op&0x00000F80)>>7;
    sprintf(arm9dasmstr,"r%d, [r%d, +r%d, LSL #%d]", ARM9DASM_RD, ARM9DASM_RN, ARM9DASM_RM, arm9dasmtmp4);
}

void ARM9DASMofrplr(u32 op) 
{
    arm9dasmtmp4=(op&0x00000F80)>>7; 
    sprintf(arm9dasmstr,"r%d, [r%d, +r%d, LSR #%d]", ARM9DASM_RD, ARM9DASM_RN, ARM9DASM_RM, arm9dasmtmp4);
}

void ARM9DASMofrpar(u32 op)
{
    arm9dasmtmp4=(op&0x00000F80)>>7; 
    sprintf(arm9dasmstr,"r%d, [r%d, +r%d, ASR #%d]", ARM9DASM_RD, ARM9DASM_RN, ARM9DASM_RM, arm9dasmtmp4);
}

void ARM9DASMofrprr(u32 op) 
{
    arm9dasmtmp4=(op&0x00000F80)>>7;
    if(arm9dasmtmp4)
        sprintf(arm9dasmstr,"r%d, [r%d, +r%d, ROR #%d]", ARM9DASM_RD, ARM9DASM_RN, ARM9DASM_RM, arm9dasmtmp4);
    else
        sprintf(arm9dasmstr,"r%d, [r%d, +r%d, RRX]", ARM9DASM_RD, ARM9DASM_RN, ARM9DASM_RM);
}

void ARM9DASMprim(u32 op) 
{
    sprintf(arm9dasmstr,"r%d, [r%d, #-$%03X]!", ARM9DASM_RD, ARM9DASM_RN, op&0x00000FFF);
}

void ARM9DASMprip(u32 op) 
{
    sprintf(arm9dasmstr,"r%d, [r%d, #+$%03X]!", ARM9DASM_RD, ARM9DASM_RN, op&0x00000FFF);
}

void ARM9DASMprrm(u32 op)
{
    sprintf(arm9dasmstr,"r%d, [r%d, -r%d]!", ARM9DASM_RD, ARM9DASM_RN, ARM9DASM_RM);
}

void ARM9DASMprrmll(u32 op)
{
    arm9dasmtmp4=(op&0x00000F80)>>7; 
    sprintf(arm9dasmstr,"r%d, [r%d, -r%d, LSL #%d]!", ARM9DASM_RD, ARM9DASM_RN, ARM9DASM_RM, arm9dasmtmp4);
}

void ARM9DASMprrmlr(u32 op) 
{
    arm9dasmtmp4=(op&0x00000F80)>>7; 
    sprintf(arm9dasmstr,"r%d, [r%d, -r%d, LSR #%d]!", ARM9DASM_RD, ARM9DASM_RN, ARM9DASM_RM, arm9dasmtmp4);
}

void ARM9DASMprrmar(u32 op)
{
    arm9dasmtmp4=(op&0x00000F80)>>7;
    sprintf(arm9dasmstr,"r%d, [r%d, -r%d, ASR #%d]!", ARM9DASM_RD, ARM9DASM_RN, ARM9DASM_RM, arm9dasmtmp4);
}

void ARM9DASMprrmrr(u32 op) 
{
    arm9dasmtmp4=(op&0x00000F80)>>7;
    if(arm9dasmtmp4)
        sprintf(arm9dasmstr,"r%d, [r%d, -r%d, ROR #%d]!", ARM9DASM_RD, ARM9DASM_RN, ARM9DASM_RM, arm9dasmtmp4);
    else
        sprintf(arm9dasmstr,"r%d, [r%d, -r%d, RRX]!", ARM9DASM_RD, ARM9DASM_RN, ARM9DASM_RM);
}

void ARM9DASMprrp(u32 op)
{
    sprintf(arm9dasmstr,"r%d, [r%d, +r%d]!", ARM9DASM_RD, ARM9DASM_RN, ARM9DASM_RM);
}

void ARM9DASMprrpll(u32 op)
{
    arm9dasmtmp4=(op&0x00000F80)>>7; 
    sprintf(arm9dasmstr,"r%d, [r%d, +r%d, LSL #%d]!", ARM9DASM_RD, ARM9DASM_RN, ARM9DASM_RM, arm9dasmtmp4);
}

void ARM9DASMprrplr(u32 op)
{
    arm9dasmtmp4=(op&0x00000F80)>>7; 
    sprintf(arm9dasmstr,"r%d, [r%d, +r%d, LSR #%d]!", ARM9DASM_RD, ARM9DASM_RN, ARM9DASM_RM, arm9dasmtmp4);
}

void ARM9DASMprrpar(u32 op)
{
    arm9dasmtmp4=(op&0x00000F80)>>7;
    sprintf(arm9dasmstr,"r%d, [r%d, +r%d, ASR #%d]!", ARM9DASM_RD, ARM9DASM_RN, ARM9DASM_RM, arm9dasmtmp4);
}

void ARM9DASMprrprr(u32 op)
{
    arm9dasmtmp4=(op&0x00000F80)>>7;
    if(arm9dasmtmp4)
        sprintf(arm9dasmstr,"r%d, [r%d, +r%d, ROR #%d]!", ARM9DASM_RD, ARM9DASM_RN, ARM9DASM_RM, arm9dasmtmp4);
    else
        sprintf(arm9dasmstr,"r%d, [r%d, +r%d, RRX]!", ARM9DASM_RD, ARM9DASM_RN, ARM9DASM_RM);
}

void ARM9DASMptim(u32 op)
{
    sprintf(arm9dasmstr,"r%d, [r%d], #-$%03X", ARM9DASM_RD, ARM9DASM_RN, op&0x00000FFF);
}

void ARM9DASMptip(u32 op) 
{
    sprintf(arm9dasmstr,"r%d, [r%d], #+$%03X", ARM9DASM_RD, ARM9DASM_RN, op&0x00000FFF);
}

void ARM9DASMptrm(u32 op) 
{
    sprintf(arm9dasmstr,"r%d, [r%d], -r%d", ARM9DASM_RD, ARM9DASM_RN, ARM9DASM_RM);
}

void ARM9DASMptrmll(u32 op)
{
    arm9dasmtmp4=(op&0x00000F80)>>7; 
    sprintf(arm9dasmstr,"r%d, [r%d], -r%d, LSL #%d", ARM9DASM_RD, ARM9DASM_RN, ARM9DASM_RM, arm9dasmtmp4);
}

void ARM9DASMptrmlr(u32 op) 
{
    arm9dasmtmp4=(op&0x00000F80)>>7;
    sprintf(arm9dasmstr,"r%d, [r%d], -r%d, LSR #%d", ARM9DASM_RD, ARM9DASM_RN, ARM9DASM_RM, arm9dasmtmp4);
}

void ARM9DASMptrmar(u32 op)
{
    arm9dasmtmp4=(op&0x00000F80)>>7; 
    sprintf(arm9dasmstr,"r%d, [r%d], -r%d, ASR #%d", ARM9DASM_RD, ARM9DASM_RN, ARM9DASM_RM, arm9dasmtmp4);
}

void ARM9DASMptrmrr(u32 op)
{
    arm9dasmtmp4=(op&0x00000F80)>>7; 
    if(arm9dasmtmp4)
        sprintf(arm9dasmstr,"r%d, [r%d], -r%d, ROR #%d", ARM9DASM_RD, ARM9DASM_RN, ARM9DASM_RM, arm9dasmtmp4);
    else
        sprintf(arm9dasmstr,"r%d, [r%d], -r%d, RRX", ARM9DASM_RD, ARM9DASM_RN, ARM9DASM_RM);
}

void ARM9DASMptrp(u32 op)
{
    sprintf(arm9dasmstr,"r%d, [r%d], +r%d", ARM9DASM_RD, ARM9DASM_RN, ARM9DASM_RM);
}

void ARM9DASMptrpll(u32 op)
{
    arm9dasmtmp4=(op&0x00000F80)>>7;
    sprintf(arm9dasmstr,"r%d, [r%d], +r%d, LSL #%d", ARM9DASM_RD, ARM9DASM_RN, ARM9DASM_RM, arm9dasmtmp4);
}

void ARM9DASMptrplr(u32 op) 
{
    arm9dasmtmp4=(op&0x00000F80)>>7; 
    sprintf(arm9dasmstr,"r%d, [r%d], +r%d, LSR #%d", ARM9DASM_RD, ARM9DASM_RN, ARM9DASM_RM, arm9dasmtmp4);
}

void ARM9DASMptrpar(u32 op)
{
    arm9dasmtmp4=(op&0x00000F80)>>7; 
    sprintf(arm9dasmstr,"r%d, [r%d], +r%d, ASR #%d", ARM9DASM_RD, ARM9DASM_RN, ARM9DASM_RM, arm9dasmtmp4);
}

void ARM9DASMptrprr(u32 op)
{
    arm9dasmtmp4=(op&0x00000F80)>>7; 
    if(arm9dasmtmp4)
        sprintf(arm9dasmstr,"r%d, [r%d], +r%d, ROR #%d", ARM9DASM_RD, ARM9DASM_RN, ARM9DASM_RM, arm9dasmtmp4);
    else
        sprintf(arm9dasmstr,"r%d, [r%d], +r%d, RRX", ARM9DASM_RD, ARM9DASM_RN, ARM9DASM_RM);
}

void ARM9DASMlmofim(u32 op)
{
    sprintf(arm9dasmstr,"r%d, [r%d, #-$%1X%1X]", ARM9DASM_RD, ARM9DASM_RN, ARM9DASM_RS, ARM9DASM_RM);
}

void ARM9DASMlmofip(u32 op)
{
    sprintf(arm9dasmstr,"r%d, [r%d, #+$%1X%1X]", ARM9DASM_RD, ARM9DASM_RN, ARM9DASM_RS, ARM9DASM_RM);
}

void ARM9DASMlmprim(u32 op)
{
    sprintf(arm9dasmstr,"r%d, [r%d, #-$%1X%1X]!", ARM9DASM_RD, ARM9DASM_RN, ARM9DASM_RS, ARM9DASM_RM);
}

void ARM9DASMlmprip(u32 op)
{
    sprintf(arm9dasmstr,"r%d, [r%d, #+$%1X%1X]!", ARM9DASM_RD, ARM9DASM_RN, ARM9DASM_RS, ARM9DASM_RM);
}

void ARM9DASMlmptim(u32 op)
{
    sprintf(arm9dasmstr,"r%d, [r%d], #-$%1X%1X", ARM9DASM_RD, ARM9DASM_RN, ARM9DASM_RS, ARM9DASM_RM);
}

void ARM9DASMlmptip(u32 op)
{
    sprintf(arm9dasmstr,"r%d, [r%d], #+$%1X%1X", ARM9DASM_RD, ARM9DASM_RN, ARM9DASM_RS, ARM9DASM_RM);
}

//-------------------------------------------------------------------------

//-------------------------------------------------------------------------

// Credit: Costis
void ARM9DASMlm(u32 op)
{
    int i, b_start, b_end, inWord=op;
    char str[520],regstr[512];
    sprintf(str,"r%d",ARM9DASM_RN);
    if(op&0x00200000) sprintf(str,"%s!",str);
    sprintf(regstr,",{");
    b_start = b_end = -1;
    for (i = 0; i < 16; i++)
    {
        if ((inWord & 1) && (b_start < 0)) b_start = i;
	else if (!(inWord & 1))
	{
	    b_end = i - 1;
	    if (b_start >= 0)
	    {
 	        if(strlen(regstr)==2)
 	        {
		    if(b_start!=b_end) sprintf(regstr, "%sr%d-r%d", regstr, b_start, b_end);
		    else sprintf(regstr, "%sr%d", regstr, b_start);
		} else {
		    if(b_start!=b_end) sprintf(regstr, "%s,r%d-r%d", regstr, b_start, b_end);
		    else sprintf(regstr, "%s,r%d", regstr, b_start);
		}
	    }
  	    b_start = -1;
	}
	inWord >>= 1;
    }
    sprintf(str,"%s%s}",str,regstr);
    if(op&0x00400000) sprintf(str,"%s^",str);
    sprintf(arm9dasmstr,"%s",str);
}

//-------------------------------------------------------------------------

void ARM9DASMmrsrs(u32 op)
{
    sprintf(arm9dasmstr,"r%d, cpsr",ARM9DASM_RD);
}

void ARM9DASMmrsrc(u32 op)
{
    sprintf(arm9dasmstr,"r%d, spsr",ARM9DASM_RD);
}

void ARM9DASMmsric(u32 op)
{
    arm9dasmtmp3 = op&255; 
    arm9dasmtmp4 = (op&0x00000F00)>>7; 
    arm9dasmtmp1 = (arm9dasmtmp3>>arm9dasmtmp4)|((arm9dasmtmp3&((1<<arm9dasmtmp4)-1))<<(32-arm9dasmtmp4)); 
    sprintf(arm9dasmstr,"cpsr_%c%c%c%c, #%08X",
        ((op&0x00010000)?'c':'_'), ((op&0x00020000)?'x':'_'),
        ((op&0x00040000)?'s':'_'), ((op&0x00080000)?'f':'_'), arm9dasmtmp1);
	
}

void ARM9DASMmsris(u32 op)
{
    arm9dasmtmp3 = op&255; 
    arm9dasmtmp4 = (op&0x00000F00)>>7;
    arm9dasmtmp1 = (arm9dasmtmp3>>arm9dasmtmp4)|((arm9dasmtmp3&((1<<arm9dasmtmp4)-1))<<(32-arm9dasmtmp4));
    sprintf(arm9dasmstr,"spsr_%c%c%c%c, #%08X",
        ((op&0x00010000)?'c':'_'), ((op&0x00020000)?'x':'_'),
        ((op&0x00040000)?'s':'_'), ((op&0x00080000)?'f':'_'), arm9dasmtmp1);
}

void ARM9DASMmsrrc(u32 op)
{
    sprintf(arm9dasmstr,"cpsr_%c%c%c%c, r%d",
        ((op&0x00010000)?'c':'_'), ((op&0x00020000)?'x':'_'),
        ((op&0x00040000)?'s':'_'), ((op&0x00080000)?'f':'_'), ARM9DASM_RM);
}

void ARM9DASMmsrrs(u32 op)
{
    sprintf(arm9dasmstr,"spsr_%c%c%c%c, r%d",
        ((op&0x00010000)?'c':'_'), ((op&0x00020000)?'x':'_'),
        ((op&0x00040000)?'s':'_'), ((op&0x00080000)?'f':'_'), ARM9DASM_RM);
}

//-------------------------------------------------------------------------

void ARM9DASMswp(u32 op)
{
    sprintf(arm9dasmstr,"r%d, r%d, [r%d]",ARM9DASM_RD,ARM9DASM_RM,ARM9DASM_RN);
}

void ARM9DASMswi(u32 op)
{
    sprintf(arm9dasmstr,"$%06X",op&0x00FFFFFF);
}

//-------------------------------------------------------------------------

void ARM9DASMmul(u32 op)
{
    sprintf(arm9dasmstr,"r%d, r%d, r%d",ARM9DASM_RN,ARM9DASM_RM,ARM9DASM_RS);
}

void ARM9DASMmla(u32 op)
{
    sprintf(arm9dasmstr,"r%d, r%d, r%d, r%d",ARM9DASM_RN,ARM9DASM_RM,ARM9DASM_RS,ARM9DASM_RD);
}

void ARM9DASMmull(u32 op)
{
}

//-------------------------------------------------------------------------

void ARM9DASMmcr(u32 op)
{
    sprintf(arm9dasmstr,"p%1X, %1X, r%d, c%d, c%d, %1X",ARM9DASM_RS,ARM9DASM_RO>>1,ARM9DASM_RD,ARM9DASM_RN,ARM9DASM_RM,ARM9DASM_RP>>1);
}

void ARM9DASMcpd(u32 op)
{
    sprintf(arm9dasmstr,"p%1X, %1X, c%d, c%d, c%d, %1X",ARM9DASM_RS,ARM9DASM_RO>>1,ARM9DASM_RD,ARM9DASM_RN,ARM9DASM_RM,ARM9DASM_RP>>1);
}

//-------------------------------------------------------------------------

void Thumb9DASMimm5(u32 op)
{
    sprintf(arm9dasmstr,"r%d, [r%d, #$%02X",TMB7DASM_RD,TMB7DASM_RN,TMB7DASM_IMM5);
}

void Thumb9DASMimm5shft(u32 op)
{
    sprintf(arm9dasmstr,"r%d, r%d, #$%02X",TMB7DASM_RD,TMB7DASM_RN,TMB7DASM_IMM5);
}

void Thumb9DASMimm7(u32 op)
{
    if(op&0x0080) sprintf(arm9dasmstr,"SUB sp, sp, #$%02X",TMB7DASM_IMM7);
    else          sprintf(arm9dasmstr,"ADD sp, sp, #$%02X",TMB7DASM_IMM7);
}

void Thumb9DASMimm8(u32 op)
{
    sprintf(arm9dasmstr,"r%d, #$%02X",TMB7DASM_RS, TMB7DASM_IMM8);
}

void Thumb9DASMimm3(u32 op)
{
    sprintf(arm9dasmstr,"r%d, r%d, #$%01X",TMB7DASM_RD,TMB7DASM_RN,TMB7DASM_RM);
}

void Thumb9DASMb(u32 op)
{
    signed int b = (op&0x0400)?(0xFFFFFC00|(op&0x03FF))
                                          :(op&0x03FF);
    sprintf(arm9dasmstr,"$%08X",arm9reg.r[15]+4+(b*2));
}

void Thumb9DASMbx(u32 op)
{
    if(op&0x0080)
        sprintf(arm9dasmstr,"BLX r%d",TMB7DASM_RNH);
    else
        sprintf(arm9dasmstr,"BX r%d",TMB7DASM_RNH);
}

void Thumb9DASMbl(u32 op)
{
    sprintf(arm9dasmstr,"$%03X",TMB7DASM_IMM11);
}

void Thumb9DASMbc(u32 op)
{
    signed int b = (op&0x00FF)?(0xFFFFFF00|(op&0x00FF))
                                          :(op&0x00FF);
    sprintf(arm9dasmstr,"$%08X",arm9reg.r[15]+4+(b*2));
}

void Thumb9DASMh(u32 op)
{
    sprintf(arm9dasmstr,"r%d, r%d",TMB7DASM_RDH,TMB7DASM_RNH);
}

void Thumb9DASMldm(u32 op)
{
    int i, b_start, b_end, inWord=op;
    char str[512];
    sprintf(str,"{");
    b_start = b_end = -1;
    for (i = 0; i < 8; i++)
    {
        if ((inWord & 1) && (b_start < 0)) b_start = i;
	else if (!(inWord & 1))
	{
	    b_end = i - 1;
	    if (b_start >= 0)
	    {
 	        if(strlen(str)==1)
 	        {
		    if (b_start != b_end) sprintf (str, "%sr%d-r%d", str, b_start, b_end);
		    else sprintf (str, "%sr%d", str, b_start);
		} else {
		    if (b_start != b_end) sprintf (str, "%s,r%d-r%d", str, b_start, b_end);
		    else sprintf (str, "%s,r%d", str, b_start);
		}
	    }
  	    b_start = -1;
	}
	inWord >>= 1;
    }
    sprintf(arm9dasmstr,"%s}",str);
}

void Thumb9DASMdp1(u32 op)
{
    switch((op&0x00C0)>>6)
    {
        case 0: sprintf(arm9dasmstr,"AND r%d, r%d",TMB7DASM_RD,TMB7DASM_RN); break;
        case 1: sprintf(arm9dasmstr,"EOR r%d, r%d",TMB7DASM_RD,TMB7DASM_RN); break;
        case 2: sprintf(arm9dasmstr,"LSL r%d, r%d",TMB7DASM_RD,TMB7DASM_RN); break;
        case 3: sprintf(arm9dasmstr,"LSR r%d, r%d",TMB7DASM_RD,TMB7DASM_RN); break;
    }
}

void Thumb9DASMdp2(u32 op)
{
    switch((op&0x00C0)>>6)
    {
        case 0: sprintf(arm9dasmstr,"ASR r%d, r%d",TMB7DASM_RD,TMB7DASM_RN); break;
        case 1: sprintf(arm9dasmstr,"ADC r%d, r%d",TMB7DASM_RD,TMB7DASM_RN); break;
        case 2: sprintf(arm9dasmstr,"SBC r%d, r%d",TMB7DASM_RD,TMB7DASM_RN); break;
        case 3: sprintf(arm9dasmstr,"ROR r%d, r%d",TMB7DASM_RD,TMB7DASM_RN); break;
    }
}

void Thumb9DASMdp3(u32 op)
{
    switch((op&0x00C0)>>6)
    {
        case 0: sprintf(arm9dasmstr,"TST r%d, r%d",TMB7DASM_RD,TMB7DASM_RN); break;
        case 1: sprintf(arm9dasmstr,"NEG r%d"     ,TMB7DASM_RD); break;
        case 2: sprintf(arm9dasmstr,"CMP r%d, r%d",TMB7DASM_RD,TMB7DASM_RN); break;
        case 3: sprintf(arm9dasmstr,"CMN r%d, r%d",TMB7DASM_RD,TMB7DASM_RN); break;
    }
}

void Thumb9DASMdp4(u32 op)
{
    switch((op&0x00C0)>>6)
    {
        case 0: sprintf(arm9dasmstr,"ORR r%d, r%d",TMB7DASM_RD,TMB7DASM_RN); break;
        case 1: sprintf(arm9dasmstr,"MUL r%d, r%d",TMB7DASM_RD,TMB7DASM_RN); break;
        case 2: sprintf(arm9dasmstr,"BIC r%d, r%d",TMB7DASM_RD,TMB7DASM_RN); break;
        case 3: sprintf(arm9dasmstr,"MVN r%d, r%d",TMB7DASM_RD,TMB7DASM_RN); break;
    }
}

void Thumb9DASMreg(u32 op)
{
    sprintf(arm9dasmstr,"r%d, r%d",TMB7DASM_RD,TMB7DASM_RN);
}

void Thumb9DASMund(u32 op)
{
    sprintf(arm9dasmstr,"");
}

void Thumb9DASMpc(u32 op)
{
    sprintf(arm9dasmstr,"[pc, #$%02X*4]",TMB7DASM_IMM8);
}

void Thumb9DASMsp(u32 op)
{
    sprintf(arm9dasmstr,"[sp, #$%02X*4]",TMB7DASM_IMM8);
}

void Thumb9DASMbkpt(u32 op)
{
    sprintf(arm9dasmstr,"");
}

void Thumb9DASMswi(u32 op)
{
    sprintf(arm9dasmstr,"$%02X",op&0x00FF);
}

/*** EOF:arm9dasm.c ******************************************************/

/**********************************************************************/
/* END dsemu Disassembler                                             */
/**********************************************************************/


int
arch_arm_disasm_instr(cpu_t *cpu, addr_t pc, char *line, unsigned int max_line) {

	tag_t dummy1;
	addr_t dummy2, dummy3;
	int bytes = arch_arm_tag_instr(cpu, pc, &dummy1, &dummy2, &dummy3);

	uint32_t instr = *(uint32_t*)&cpu->RAM[pc];

	snprintf(line, max_line, "%s", ARM9DASM(instr));

	return bytes;
}
