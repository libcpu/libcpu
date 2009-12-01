#include "libcpu.h"
#include "m88k_insn.h"

m88k_insn::insn_desc const m88k_insn::desc_major[64] =
  {
    { M88K_IFMT_XMEM,  M88K_OPC_LD_D    }, // 00 ld.d   xD, rS, nnnn
    { M88K_IFMT_XMEM,  M88K_OPC_LD      }, // 01 ld     xD, rS, nnnn xD <- rS + nnnn
    { M88K_IFMT_MEM,   M88K_OPC_LD_HU   }, // 02 ld.hu  rD, rS, nnnn
    { M88K_IFMT_MEM,   M88K_OPC_LD_BU   }, // 03 ld.bu  rD, rS, nnnn
    { M88K_IFMT_MEM,   M88K_OPC_LD_D    }, // 04 ld.d   rD, rS, nnnn
    { M88K_IFMT_MEM,   M88K_OPC_LD      }, // 05 ld     rD, rS, nnnn rD <- rs + nnnn
    { M88K_IFMT_MEM,   M88K_OPC_LD_H    }, // 06 ld.h   rD, rS, nnnn
    { M88K_IFMT_MEM,   M88K_OPC_LD_B    }, // 07 ld.b   rD, rS, nnnn
    { M88K_IFMT_MEM,   M88K_OPC_ST_D    }, // 08 st.d   rD, rS, nnnn
    { M88K_IFMT_MEM,   M88K_OPC_ST      }, // 09 st     rD, rS, nnnn rD -> rS + nnnn
    { M88K_IFMT_MEM,   M88K_OPC_ST_H    }, // 0a st.h   rD, rS, nnnn
    { M88K_IFMT_MEM,   M88K_OPC_ST_B    }, // 0b st.b   rD, rS, nnnn
    { M88K_IFMT_XMEM,  M88K_OPC_ST_D    }, // 0c st.d   xD, rS, nnnn
    { M88K_IFMT_XMEM,  M88K_OPC_ST      }, // 0d st     xD, rS, nnnn xD -> rS + nnnn
    { M88K_IFMT_XMEM,  M88K_OPC_ST_X    }, // 0e st.x   xD, rS, nnnn
    { M88K_IFMT_XMEM,  M88K_OPC_LD_X    }, // 0f ld.x   xD, rS, nnnn
    { M88K_IFMT_REG,   M88K_OPC_AND     }, // 10 and    rD, rS, nnnn
    { M88K_IFMT_REG,   M88K_OPC_AND_U   }, // 11 and.u  rD, rS, nnnn
    { M88K_IFMT_REG,   M88K_OPC_MASK    }, // 12 mask   rD, rS, nnnn
    { M88K_IFMT_REG,   M88K_OPC_MASK_U  }, // 13 mask.u rD, rS, nnnn
    { M88K_IFMT_REG,   M88K_OPC_XOR     }, // 14 xor    rD, rS, nnnn
    { M88K_IFMT_REG,   M88K_OPC_XOR_U   }, // 15 xor.u  rD, rS, nnnn
    { M88K_IFMT_REG,   M88K_OPC_OR      }, // 16 or     rD, rS, nnnn
    { M88K_IFMT_REG,   M88K_OPC_OR_U    }, // 17 or.u   rD, rS, nnnn
    { M88K_IFMT_REG,   M88K_OPC_ADDU    }, // 18 addu   rD, rS, nnnn
    { M88K_IFMT_REG,   M88K_OPC_SUBU    }, // 19 subu   rD, rS, nnnn
    { M88K_IFMT_REG,   M88K_OPC_DIVU    }, // 1a divu   rD, rS, nnnn
    { M88K_IFMT_REG,   M88K_OPC_MULU    }, // 1b mulu   rD, rS, nnnn
    { M88K_IFMT_REG,   M88K_OPC_ADD     }, // 1c add    rD, rS, nnnn
    { M88K_IFMT_REG,   M88K_OPC_SUB     }, // 1d sub    rD, rS, nnnn
    { M88K_IFMT_REG,   M88K_OPC_DIVS    }, // 1e divs   rD, rS, nnnn
    { M88K_IFMT_REG,   M88K_OPC_CMP     }, // 1f cmp    rD, rS, nnnn
    { M88K_FMT_NONE,   M88K_POPC_BFMT   }, // 20 <cr opcodes>
    { M88K_FMT_NONE,   M88K_POPC_SFU1   }, // 21 <fpu opcodes>
    { M88K_FMT_NONE,   M88K_POPC_SFU2   }, // 22 <vec opcodes>
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 23 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 24 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 25 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 26 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 27 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 28 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 29 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2a [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2b [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2c [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2d [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2e [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2f [illegal]
    { M88K_BRFMT_OFF,  M88K_OPC_BR      }, // 30 br     <26-bit pcrel>
    { M88K_BRFMT_OFF,  M88K_OPC_BR_N    }, // 31 br.n   <26-bit pcrel>
    { M88K_BRFMT_OFF,  M88K_OPC_BSR     }, // 32 bsr    <26-bit pcrel>
    { M88K_BRFMT_OFF,  M88K_OPC_BSR_N   }, // 33 bsr.n  <26-bit pcrel>
    { M88K_BRFMT_BIT,  M88K_OPC_BB0     }, // 34 bb0    <cc>,rS,<16-bit pcrel>
    { M88K_BRFMT_BIT,  M88K_OPC_BB0_N   }, // 35 bb0.n  <cc>,rS,<16-bit pcrel>
    { M88K_BRFMT_BIT,  M88K_OPC_BB1     }, // 36 bb1    <cc>,rS,<16-bit pcrel>
    { M88K_BRFMT_BIT,  M88K_OPC_BB1_N   }, // 37 bb1.n  <cc>,rS,<16-bit pcrel>
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 38 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 39 [illegal]
    { M88K_BRFMT_COND, M88K_OPC_BCND    }, // 3a bcnd   <cc>,rS,<16-bit pcrel>
    { M88K_BRFMT_COND, M88K_OPC_BCND_N  }, // 3b bcnd.n <cc>,rS,<16-bit pcrel>
    { M88K_FMT_NONE,   M88K_POPC_EXT1   }, // 3c <insn table 1>
    { M88K_FMT_NONE,   M88K_POPC_EXT2   }, // 3d <insn table 2>
    { M88K_IFMT_REG,   M88K_OPC_TBND    }, // 3e tbnd   rS, nnnn
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }  // 3f [illegal]
  };

m88k_insn::insn_desc const m88k_insn::desc_cfmt[64] =
  {
    { M88K_IFMT_XMEM,  M88K_OPC_LD_D    }, // 00 ld.d   xD, rS, nnnn
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 01 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 02 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 03 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 04 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 05 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 06 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 07 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 08 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 09 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0a [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0b [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0c [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0d [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0e [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0f [illegal]
    { M88K_CFMT_REG,   M88K_OPC_LDCR    }, // 10 ldcr
    { M88K_CFMT_REG,   M88K_OPC_LDCR    }, // 11 ldcr
    { M88K_CFMT_REG,   M88K_OPC_FLDCR   }, // 12 fldcr
    { M88K_CFMT_REG,   M88K_OPC_FLDCR   }, // 13 fldcr
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 14 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 15 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 16 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 17 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 18 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 19 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1a [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1b [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1c [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1d [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1e [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1f [illegal]
    { M88K_CFMT_GER,   M88K_OPC_STCR    }, // 20 stcr
    { M88K_CFMT_GER,   M88K_OPC_STCR    }, // 21 stcr
    { M88K_CFMT_GER,   M88K_OPC_FSTCR   }, // 22 fstcr
    { M88K_CFMT_GER,   M88K_OPC_FSTCR   }, // 23 fstcr
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 24 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 25 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 26 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 27 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 28 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 29 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2a [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2b [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2c [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2d [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2e [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2f [illegal]
    { M88K_CFMT_REG2,  M88K_OPC_XCR     }, // 30 xcr
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 31 [illegal]
    { M88K_CFMT_REG2,  M88K_OPC_FXCR    }, // 32 fxcr      
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 33 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 34 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 35 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 36 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 37 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 38 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 39 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 3a [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 3b [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 3c [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 3d [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 3e [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }  // 3f [illegal]
  };

m88k_insn::insn_desc const m88k_insn::desc_ext1[64] =
  {
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 00 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 01 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 02 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 03 [illegal]
    { M88K_TFMT_XREG,  M88K_OPC_LD_D    }, // 04 ld.d   xD, rS1, rS2 / rS1[rS2]
    { M88K_TFMT_XREG,  M88K_OPC_LD      }, // 05 ld     xD, rS1, rS2 / rS1[rS2]
    { M88K_TFMT_XREG,  M88K_OPC_LD_X    }, // 06 ld.x   xD, rS1, rS2 / rS1[rS2]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 07 [illegal]
    { M88K_TFMT_XREG,  M88K_OPC_ST_D    }, // 08 st.d   xD, rS1, rS2 / rS1[rS2]
    { M88K_TFMT_XREG,  M88K_OPC_ST      }, // 09 st     xD, rS1, rS2 / rS1[rS2]
    { M88K_TFMT_XREG,  M88K_OPC_ST_X    }, // 0a st.x   xD, rS1, rS2 / rS1[rS2]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0b [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0c [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0d [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0e [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0f [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 10 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 11 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 12 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 13 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 14 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 15 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 16 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 17 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 18 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 19 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1a [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1b [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1c [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1d [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1e [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1f [illegal]
    { M88K_BFMT_REG,   M88K_OPC_CLR     }, // 20 clr    rD, rS, nn<w>
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 21 [illegal]
    { M88K_BFMT_REG,   M88K_OPC_SET     }, // 22 set    rD, rS, nn<w>
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 23 [illegal]
    { M88K_BFMT_REG,   M88K_OPC_EXT     }, // 24 ext    rD, rS, nn<w>
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 25 [illegal]
    { M88K_BFMT_REG,   M88K_OPC_EXTU    }, // 26 extu   rD, rS, nn<w>
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 27 [illegal]
    { M88K_BFMT_REG,   M88K_OPC_MAK     }, // 28 mak    rD, rS, nn<w>
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 29 [illegal]
    { M88K_BFMT_REG,   M88K_OPC_ROT     }, // 2a rot    rD, rS, <w>
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2b [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2c [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2d [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2e [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2f [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 30 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 31 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 32 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 33 [illegal]
    { M88K_BRFMT_BIT,  M88K_OPC_TB0     }, // 34 tb0    D, rS, nnnn
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 35 [illegal]
    { M88K_BRFMT_BIT,  M88K_OPC_TB1     }, // 36 tb1    D, rS, nnnn
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 37 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 38 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 39 [illegal]
    { M88K_BRFMT_COND, M88K_OPC_TCND    }, // 3a tcnd   <cc>,rS,<16-bit>
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 3b [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 3c [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 3d [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 3e [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }  // 3f [illegal]
  };

m88k_insn::insn_desc const m88k_insn::desc_ext2[8][64] =
  {
    { // 0
      { M88K_TFMT_REG,   M88K_OPC_XMEM_BU }, // 00 xmem.bu rD, rS1, rS2
      { M88K_TFMT_REG,   M88K_OPC_XMEM    }, // 01 xmem    rD, rS1, rS2
      { M88K_TFMT_REG,   M88K_OPC_LD_HU   }, // 02 ld.hu   rD, rS1, rS2
      { M88K_TFMT_REG,   M88K_OPC_LD_BU   }, // 03 ld.bu   rD, rS1, rS2
      { M88K_TFMT_REG,   M88K_OPC_LD_D    }, // 04 ld.d    rD, rS1, rS2
      { M88K_TFMT_REG,   M88K_OPC_LD      }, // 05 ld      rD, rS1, rS2
      { M88K_TFMT_REG,   M88K_OPC_LD_H    }, // 06 ld.h    rD, rS1, rS2
      { M88K_TFMT_REG,   M88K_OPC_LD_B    }, // 07 ld.b    rD, rS1, rS2
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 08 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 09 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0a [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0b [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0c [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0d [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0e [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0f [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 10 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 11 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 12 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 13 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 14 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 15 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 16 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 17 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 18 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 19 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1a [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1b [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1c [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1d [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1e [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1f [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 20 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 21 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 22 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 23 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 24 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 25 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 26 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 27 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 28 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 29 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2a [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2b [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2c [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2d [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2e [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2f [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 30 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 31 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 32 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 33 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 34 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 35 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 36 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 37 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 38 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 39 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 3a [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 3b [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 3c [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 3d [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 3e [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }  // 3f [illegal]
    },
    { // 1
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 00 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 01 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 02 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 03 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 04 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 05 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 06 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 07 [illegal]
      { M88K_TFMT_REG,   M88K_OPC_ST_D    }, // 08 st.d   rD, rS1, rS2
      { M88K_TFMT_REG,   M88K_OPC_ST      }, // 09 st     rD, rS1, rS2
      { M88K_TFMT_REG,   M88K_OPC_ST_H    }, // 0a st.h   rD, rS1, rS2
      { M88K_TFMT_REG,   M88K_OPC_ST_B    }, // 0b st.b   rD, rS1, rS2
      { M88K_TFMT_REGS,  M88K_OPC_LDA_D   }, // 0c lda.d  rD, rS1[rS2]
      { M88K_TFMT_REGS,  M88K_OPC_LDA     }, // 0d lda    rD, rS1[rS2]
      { M88K_TFMT_REGS,  M88K_OPC_LDA_H   }, // 0e lda.h  rD, rS1[rS2]
      { M88K_TFMT_REGS,  M88K_OPC_LDA_X   }, // 0f lda.x  rD, rS1[rS2]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 10 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 11 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 12 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 13 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 14 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 15 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 16 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 17 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 18 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 19 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1a [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1b [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1c [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1d [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1e [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1f [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 20 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 21 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 22 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 23 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 24 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 25 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 26 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 27 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 28 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 29 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2a [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2b [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2c [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2d [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2e [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2f [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 30 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 31 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 32 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 33 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 34 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 35 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 36 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 37 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 38 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 39 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 3a [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 3b [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 3c [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 3d [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 3e [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }  // 3f [illegal]
    },
    { // 2
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 00 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 01 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 02 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 03 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 04 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 05 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 06 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 07 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 08 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 09 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0a [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0b [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0c [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0d [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0e [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0f [illegal]
      { M88K_TFMT_REG,   M88K_OPC_AND     }, // 10 and    rD, rS1, rS2
      { M88K_TFMT_REG,   M88K_OPC_AND_C   }, // 11 and.c  rD, rS1, rS2
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 12 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 13 [illegal]
      { M88K_TFMT_REG,   M88K_OPC_XOR     }, // 14 xor    rD, rS1, rS2
      { M88K_TFMT_REG,   M88K_OPC_XOR_C   }, // 15 xor.c  rD, rS1, rS2
      { M88K_TFMT_REG,   M88K_OPC_OR      }, // 16 or     rD, rS1, rS2
      { M88K_TFMT_REG,   M88K_OPC_OR_C    }, // 17 or.c   rD, rS1, rS2
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 18 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 19 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1a [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1b [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1c [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1d [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1e [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1f [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 20 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 21 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 22 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 23 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 24 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 25 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 26 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 27 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 28 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 29 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2a [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2b [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2c [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2d [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2e [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2f [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 30 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 31 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 32 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 33 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 34 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 35 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 36 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 37 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 38 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 39 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 3a [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 3b [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 3c [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 3d [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 3e [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }  // 3f [illegal]
    },
    { // 3
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 00 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 01 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 02 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 03 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 04 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 05 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 06 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 07 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 08 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 09 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0a [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0b [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0c [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0d [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0e [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0f [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 10 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 11 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 12 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 13 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 14 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 15 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 16 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 17 [illegal]
      { M88K_TFMT_REG,   M88K_OPC_ADDU    }, // 18 addu   rD, rS1, rS2
      { M88K_TFMT_REG,   M88K_OPC_SUBU    }, // 19 subu   rD, rS1, rS2
      { M88K_TFMT_REG,   M88K_OPC_DIVU    }, // 1a divu   rD, rS1, rS2
      { M88K_TFMT_REG,   M88K_OPC_MULU    }, // 1b mulu   rD, rS1, rS2
      { M88K_TFMT_REG,   M88K_OPC_ADD     }, // 1c add    rD, rS1, rS2
      { M88K_TFMT_REG,   M88K_OPC_SUB     }, // 1d sub    rD, rS1, rS2
      { M88K_TFMT_REG,   M88K_OPC_DIVS    }, // 1e divs   rD, rS1, rS2
      { M88K_TFMT_REG,   M88K_OPC_CMP     }, // 1f cmp    rD, rS1, rS2
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 20 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 21 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 22 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 23 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 24 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 25 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 26 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 27 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 28 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 29 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2a [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2b [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2c [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2d [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2e [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2f [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 30 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 31 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 32 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 33 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 34 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 35 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 36 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 37 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 38 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 39 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 3a [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 3b [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 3c [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 3d [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 3e [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }  // 3f [illegal]
    },
    { // 4
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 00 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 01 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 02 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 03 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 04 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 05 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 06 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 07 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 08 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 09 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0a [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0b [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0c [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0d [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0e [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0f [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 10 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 11 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 12 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 13 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 14 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 15 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 16 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 17 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 18 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 19 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1a [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1b [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1c [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1d [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1e [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1f [illegal]
      { M88K_TFMT_REG,   M88K_OPC_CLR     }, // 20 clr    rD, rS1, rS2
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 21 [illegal]
      { M88K_TFMT_REG,   M88K_OPC_SET     }, // 22 set    rD, rS1, rS2
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 23 [illegal]
      { M88K_TFMT_REG,   M88K_OPC_EXT     }, // 24 ext    rD, rS1, rS2
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 25 [illegal]
      { M88K_TFMT_REG,   M88K_OPC_EXTU    }, // 26 extu   rD, rS1, rS2
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 27 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 28 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 29 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2a [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2b [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2c [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2d [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2e [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2f [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 30 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 31 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 32 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 33 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 34 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 35 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 36 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 37 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 38 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 39 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 3a [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 3b [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 3c [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 3d [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 3e [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }  // 3f [illegal]
    },
    { // 5
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 00 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 01 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 02 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 03 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 04 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 05 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 06 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 07 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 08 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 09 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0a [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0b [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0c [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0d [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0e [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0f [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 10 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 11 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 12 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 13 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 14 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 15 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 16 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 17 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 18 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 19 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1a [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1b [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1c [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1d [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1e [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1f [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 20 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 21 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 22 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 23 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 24 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 25 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 26 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 27 [illegal]
      { M88K_TFMT_REG,   M88K_OPC_MAK     }, // 28 mak    rD, rS1, rS2
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 29 [illegal]
      { M88K_TFMT_REG,   M88K_OPC_ROT     }, // 2a rot    rD, rS1, rS2
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2b [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2c [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2d [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2e [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2f [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 30 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 31 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 32 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 33 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 34 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 35 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 36 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 37 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 38 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 39 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 3a [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 3b [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 3c [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 3d [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 3e [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }  // 3f [illegal]
    },
    { // 6
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 00 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 01 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 02 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 03 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 04 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 05 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 06 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 07 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 08 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 09 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0a [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0b [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0c [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0d [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0e [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0f [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 10 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 11 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 12 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 13 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 14 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 15 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 16 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 17 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 18 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 19 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1a [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1b [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1c [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1d [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1e [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1f [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 20 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 21 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 22 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 23 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 24 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 25 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 26 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 27 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 28 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 29 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2a [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2b [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2c [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2d [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2e [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2f [illegal]
      { M88K_TFMT_REG,   M88K_OPC_JMP     }, // 30 jmp    rS2
      { M88K_TFMT_REG,   M88K_OPC_JMP_N   }, // 31 jmp.n  rS2
      { M88K_TFMT_REG,   M88K_OPC_JSR     }, // 32 jsr    rS2
      { M88K_TFMT_REG,   M88K_OPC_JSR_N   }, // 33 jsr.n  rS2
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 34 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 35 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 36 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 37 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 38 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 39 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 3a [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 3b [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 3c [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 3d [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 3e [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }  // 3f [illegal]
    },
    { // 7
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 00 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 01 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 02 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 03 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 04 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 05 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 06 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 07 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 08 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 09 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0a [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0b [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0c [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0d [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0e [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0f [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 10 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 11 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 12 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 13 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 14 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 15 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 16 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 17 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 18 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 19 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1a [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1b [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1c [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1d [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1e [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1f [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 20 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 21 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 22 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 23 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 24 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 25 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 26 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 27 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 28 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 29 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2a [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2b [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2c [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2d [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2e [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2f [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 30 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 31 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 32 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 33 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 34 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 35 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 36 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 37 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 38 [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 39 [illegal]
      { M88K_TFMT_REG,   M88K_OPC_FF1     }, // 3a ff1    rD, rS1
      { M88K_TFMT_REG,   M88K_OPC_FF0     }, // 3b ff0    rD, rS1
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 3c [illegal]
      { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 3d [illegal]
      { M88K_TFMT_REG,   M88K_OPC_TBND    }, // 3e tbnd   rS1,rS2
      { M88K_FMT_NONE,   M88K_OPC_RTE     }  // 3f rte
    },
  };

m88k_insn::insn_desc const m88k_insn::desc_sfu1[] =
  {
    { M88K_TFMT_REG,   M88K_OPC_FMUL    }, // 00 fmul   rD,rS1,rS2
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 01 [illegal]
    { M88K_TFMT_REG,   M88K_OPC_FCVT    }, // 02 fcvt   rD,rS
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 03 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 04 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 05 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 06 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 07 [illegal]
    { M88K_TFMT_REG,   M88K_OPC_FLT     }, // 08 flt    rD,rS
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 09 [illegal]
    { M88K_TFMT_REG,   M88K_OPC_FADD    }, // 0a fadd   rD,rS1,rS2
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0b [illegal]
    { M88K_TFMT_REG,   M88K_OPC_FSUB    }, // 0c fsub   rD,rS1,rS2
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0d [illegal]
    { M88K_TFMT_REG,   M88K_OPC_FCMP    }, // 0e fcmp   rD,rS1,rS2 (fcmpu)
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0f [illegal]
    { M88K_TFMT_XREG,  M88K_OPC_MOV     }, // 10 mov    xD,rD
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 11 [illegal]
    { M88K_TFMT_REG,   M88K_OPC_INT     }, // 12 int    rD,rS
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 13 [illegal]
    { M88K_TFMT_REG,   M88K_OPC_NINT    }, // 14 nint   rD,rS
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 15 [illegal]
    { M88K_TFMT_REG,   M88K_OPC_TRNC    }, // 16 trnc   rD,rS
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 17 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 18 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 19 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1a [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1b [illegal]
    { M88K_TFMT_REG,   M88K_OPC_FDIV    }, // 1c fdiv   rD,rS1,rS2
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1d [illegal]
    { M88K_TFMT_REG,   M88K_OPC_FSQRT   }, // 1e fsqrt  rD,rS
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1f [illegal]
    { M88K_TFMT_XFR,   M88K_OPC_FMUL    }, // 20 fmul   xD,xS1,xS2
    { M88K_TFMT_XFR,   M88K_OPC_FMUL    }, // 21 fmul   xD,xS1,xS2
    { M88K_TFMT_XFR,   M88K_OPC_FCVT    }, // 22 fcvt   xD,xS
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 23 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 24 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 25 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 26 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 27 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 28 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 29 [illegal]
    { M88K_TFMT_XFR,   M88K_OPC_FADD    }, // 2a fadd   xD,xS1,xS2
    { M88K_TFMT_XFR,   M88K_OPC_FADD    }, // 2b fadd   xD,xS1,xS2
    { M88K_TFMT_XFR,   M88K_OPC_FSUB    }, // 2c fsub   xD,xS1,xS2
    { M88K_TFMT_XFR,   M88K_OPC_FSUB    }, // 2d fsub   xD,xS1,xS2
    { M88K_TFMT_XFR,   M88K_OPC_FCMP    }, // 2e fcmp   xD,xS1,xS2 (fcmpu)
    { M88K_TFMT_XFR,   M88K_OPC_FCMP    }, // 2f fcmp   xD,xS1,xS2 (fcmpu)
    { M88K_TFMT_REGX,  M88K_OPC_MOV     }, // 30 mov    rD,xD
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 31 [illegal]
    { M88K_TFMT_REGX,  M88K_OPC_INT     }, // 32 int    rD,xS
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 33 [illegal]
    { M88K_TFMT_REGX,  M88K_OPC_NINT    }, // 34 int    rD,xS
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 35 [illegal]
    { M88K_TFMT_REGX,  M88K_OPC_TRNC    }, // 36 trnc   rD,xS
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 37 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 38 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 39 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 3a [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 3b [illegal]
    { M88K_TFMT_XFR,   M88K_OPC_FDIV    }, // 3c fdiv   xD,xS1,xS2
    { M88K_TFMT_XFR,   M88K_OPC_FDIV    }, // 3d fdiv   xD,xS1,xS2
    { M88K_TFMT_XFR,   M88K_OPC_FSQRT   }, // 3e fsqrt  xD,xS
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }  // 3f [illegal]
  };

m88k_insn::insn_desc const m88k_insn::desc_sfu2[] =
  {
    { M88K_TFMT_REG,   M88K_OPC_PMUL    }, // 00 pmul   rD,rS1,rS2
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 01 [illegal]
    { M88K_TFMT_REG,   M88K_OPC_FCVT    }, // 02 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 03 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 04 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 05 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 06 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 07 [illegal]
    { M88K_TFMT_REG,   M88K_OPC_PADD    }, // 08 padd   rD,rS1,rS2
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 09 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0a [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0b [illegal]
    { M88K_TFMT_REG,   M88K_OPC_PSUB    }, // 0c psub   rD,rS1,rS2
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0d [illegal]
    { M88K_TFMT_REG,   M88K_OPC_PCMP    }, // 0e pcmp   rD,rS1,rS2
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 0f [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 10 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 11 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 12 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 13 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 14 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 15 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 16 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 17 [illegal]
    { M88K_TFMT_REG,   M88K_OPC_PPACK   }, // 18 ppack   rD,rS1,rS2
    { M88K_TFMT_REG,   M88K_OPC_PPACK   }, // 18 ppack   rD,rS1,rS2
    { M88K_TFMT_REG,   M88K_OPC_PUNPK   }, // 1a punpk   rD,rS1,rS2
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1b [illegal]
    { M88K_BFMT_REG,   M88K_OPC_PROT    }, // 1c prot rD,rS1,<w>
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1d [illegal]
    { M88K_TFMT_REG,   M88K_OPC_PROT    }, // 1e prot rD,rS1,rS2
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 1f [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 20 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 21 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 22 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 23 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 24 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 25 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 26 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 27 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 28 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 29 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2a [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2b [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2c [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2d [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2e [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 2f [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 30 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 31 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 32 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 33 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 34 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 35 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 36 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 37 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 38 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 39 [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 3a [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 3b [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 3c [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 3d [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }, // 3e [illegal]
    { M88K_FMT_NONE,   M88K_OPC_ILLEGAL }  // 3f [illegal]
  };

