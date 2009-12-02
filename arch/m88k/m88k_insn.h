#ifndef __m88k_insn_h
#define __m88k_insn_h

#include <stddef.h>

#include "m88k_isa.h"

//
//  Motorola 88000 Instruction Encodings
// 
// 31          26        21        16             11          5          0
// +-----------+---------+---------+--------------------------+---------+
// | Triadic   |   Dst   |  Src 1  |         Opcode           |  Src 2  |
// +-----------+---------+---------+--------------+-----------+---------+
// | Bit       |   Dst   |   Src   |    Opcode    |   Width   |  Offset |
// +-----------+---------+---------+--------------+-----------+---------+
// | Immediate |   Dst   |   Src   |              Immediate             |
// +-----------+---------+---------+------------------------------------+
// | Cbr       |   M/B   |   Src   |                Offset              |
// +-----------+---------+---------+------------------------------------+
// | Br/Bsr    |                         Offset                         |
// +-----------+--------------------------------------------------------+
//

#define M88K_REG_MASK          0x1f  // masks off regs 0-31

#define M88K_MAJOPC_SHIFT      26
#define M88K_MAJOPC_MASK       0x3f  // masks off 6 bit major opcode

#define M88K_TFMT_MINOPC_SHIFT 5     // Triadic
#define M88K_TFMT_MINOPC_MASK  0x7ff // masks off 11 bit minor opcode
#define M88K_TFMT_DST_SHIFT    21
#define M88K_TFMT_SRC1_SHIFT   16
#define M88K_TFMT_SRC2_SHIFT   0

#define M88K_BFMT_MINOPC_SHIFT 10    // Bit Format
#define M88K_BFMT_MINOPC_MASK  0x3f  // masks off 6 bit minor opcode
#define M88K_BFMT_DST_SHIFT    21
#define M88K_BFMT_SRC_SHIFT    16
#define M88K_BFMT_W_SHIFT      5 
#define M88K_BFMT_W_MASK       0x1f  // masks off 5 bit width
#define M88K_BFMT_OFF_SHIFT    0 
#define M88K_BFMT_OFF_MASK     0x1f  // masks off 5 bit width

#define M88K_IFMT_DST_SHIFT    21    // Immediate
#define M88K_IFMT_SRC_SHIFT    16
#define M88K_IFMT_IMM_SHIFT    0
#define M88K_IFMT_IMM_MASK     0xffff

#define M88K_CBFMT_MB_SHIFT    21    // Cbr
#define M88K_CBFMT_MB_MASK     0x1f
#define M88K_CBFMT_SRC_SHIFT   16
#define M88K_CBFMT_OFF_SHIFT   0
#define M88K_CBFMT_OFF_MASK    0xffff

#define M88K_BRFMT_OFF_SHIFT   0    // Br/Bsr
#define M88K_BRFMT_OFF_MASK    0x3ffffff

#define M88K_CFMT_CR_SHIFT     5
#define M88K_CFMT_CR_MASK      0x1f

class m88k_insn {
	uint32_t m_insn;

private:
	struct insn_desc {
		m88k_insnfmt_t format;
		m88k_opcode_t  opcode;
	};

	static insn_desc const desc_major[64];
	static insn_desc const desc_cfmt[64];
	static insn_desc const desc_ext1[64];
	static insn_desc const desc_ext2[8][64];
	static insn_desc const desc_sfu1[64];
	static insn_desc const desc_sfu2[64];

public:
	m88k_insn (uint32_t insn)
		: m_insn (insn)
	{
	}

public:
	inline m88k_opcode_t opcode() const
	{
		insn_desc const *d = desc();
		if (d != NULL) {
			if (d->opcode == M88K_OPC_DIVU && d->format == M88K_TFMT_REG) {
				if (m_insn & 0x100)
					return M88K_OPC_DIVU_D;
			} else if (d->opcode == M88K_OPC_MULU && d->format == M88K_TFMT_REG) {
				if (m_insn & 0x200)
					return M88K_OPC_MULS;
				if (m_insn & 0x100)
					return M88K_OPC_MULU_D;
			} else if(d->opcode == M88K_OPC_RTE) {
				switch(m_insn & 3) {
					case 0: return M88K_OPC_RTE;
					case 1: return M88K_OPC_ILLOP1;
					case 2: return M88K_OPC_ILLOP2;
					case 3: return M88K_OPC_ILLOP3;
				}
			} else if(d->opcode == M88K_OPC_FCMP) {
				if(m_insn & 0x20)
					return M88K_OPC_FCMPU;
			}
			return d->opcode;
		} else
			return M88K_OPC_ILLEGAL;
	}

	inline m88k_insnfmt_t format() const
	{
		insn_desc const *d = desc();
		if (d != NULL) {
			m88k_insnfmt_t fmt = d->format;
			if (is_load_or_store(d->opcode) && (m_insn & 0x200) != 0) {
				switch (fmt) {
					case M88K_TFMT_REG:
						fmt = M88K_TFMT_REGS;
						break;
					case M88K_TFMT_XREG:
						fmt = M88K_TFMT_XREGS;
						break;
					default:
						break;
				}
			}
			return fmt;
		} else {
			return M88K_FMT_NONE;
		}
	}

	inline bool has_usr() const
	{
		if (is_load_or_store(opcode())) {
			switch(format()) {
				case M88K_IFMT_MEM:
				case M88K_IFMT_XMEM:
					return (false);
				default:
					return (true);
			}
		}
		return (false);
	}

	inline bool has_wt() const
	{
		if (is_store(opcode())) {
			switch(format()) {
				case M88K_IFMT_MEM:
				case M88K_IFMT_XMEM:
					return (false);
				default:
					return (true);
			}
		}
		return (false);
	}
	inline bool is_branch() const
	{
		switch (opcode()) {
			case M88K_OPC_BR:
			case M88K_OPC_BR_N:
			case M88K_OPC_BSR:
			case M88K_OPC_BSR_N:
			case M88K_OPC_BCND:
			case M88K_OPC_BCND_N:
			case M88K_OPC_BB0:
			case M88K_OPC_BB0_N:
			case M88K_OPC_BB1:
			case M88K_OPC_BB1_N:
			case M88K_OPC_JMP:
			case M88K_OPC_JMP_N:
			case M88K_OPC_JSR:
			case M88K_OPC_JSR_N:
				return (true);
			default:
				return (false);
		}
	}

	inline bool is_delaying() const
	{
		switch (opcode()) {
			case M88K_OPC_BR_N:
			case M88K_OPC_BSR_N:
			case M88K_OPC_BCND_N:
			case M88K_OPC_BB0_N:
			case M88K_OPC_BB1_N:
			case M88K_OPC_JMP_N:
			case M88K_OPC_JSR_N:
				return (true);
			default:
				return (false);
		}
	}

	inline bool has_carry() const
	{
		switch (opcode()) {
			case M88K_OPC_ADD:
			case M88K_OPC_ADDU:
			case M88K_OPC_SUB:
			case M88K_OPC_SUBU:
				return (format() != M88K_IFMT_REG);
			default:
				return (false);
		}
	}

public: // ALU
	inline uint32_t carry() const
	{
		return ((m_insn >> 8) & 3);
	}

public: // Triadic/Bit/Immediate
	inline m88k_reg_t rd() const
	{
		return ((m_insn >> M88K_TFMT_DST_SHIFT) & M88K_REG_MASK);
	}

public: // Triadic/Bit/Cbr
	inline m88k_reg_t rs1() const
	{
		return ((m_insn >> M88K_TFMT_SRC1_SHIFT) & M88K_REG_MASK);
	}

public: // Triadic
	inline m88k_reg_t rs2() const
	{
		return ((m_insn >> M88K_TFMT_SRC2_SHIFT) & M88K_REG_MASK);
	}

public: // Bit
	inline uint32_t bit_width() const
	{ 
		return ((m_insn >> M88K_BFMT_W_SHIFT) & M88K_BFMT_W_MASK);
	}

	inline int32_t bit_offset() const
	{
		return ((m_insn >> M88K_BFMT_OFF_SHIFT) & M88K_BFMT_OFF_MASK);
	}

public: // Immediate
	inline int16_t immediate() const
	{
		return (m_insn & M88K_IFMT_IMM_MASK);
	}

public: // Cbr
	inline uint32_t mb() const
	{
		return ((m_insn >> M88K_CBFMT_MB_SHIFT) & M88K_CBFMT_MB_MASK);
	}

public: // Branch Offset
	inline int32_t branch26() const
	{
		// Sign extend 26bits.
		return (static_cast <int32_t> (((m_insn >> M88K_BRFMT_OFF_SHIFT) &
			M88K_BRFMT_OFF_MASK) << 6) >> 6);
	}

	inline int32_t branch16() const
	{
		// Sign extend 16bits.
		return (static_cast <int16_t> ((m_insn >> M88K_CBFMT_OFF_SHIFT) &
			M88K_CBFMT_OFF_MASK));
	}

public: // Floating
	// SDD/SDS/DSS/DSD/DDS
	inline uint32_t float_format() const
	{
		return (td() | (t1() << 2) | (t2() << 4));
	}

	// using XFR register?
	inline bool use_xfr() const
	{
		return ((m_insn & 0x8200) != 0);
	}

	inline uint32_t is_float_triadic() const
	{
		switch(opcode()) {
			case M88K_OPC_FCVT:
			case M88K_OPC_FLT:
			case M88K_OPC_FSQRT:
			case M88K_OPC_INT:
			case M88K_OPC_NINT:
			case M88K_OPC_TRNC:
				return false;
			default:
				return true;
		}
	}

public: // Control Register
    inline uint32_t cr() const
    {
        return ((m_insn >> M88K_CFMT_CR_SHIFT) & M88K_CFMT_CR_MASK);
    }

	inline bool is_float_cr() const
	{
		return ((sub_opcode_1() & 2) != 0);
	}

public: // Load/Store
	inline bool usr() const
	{
		return ((m_insn & 0x100) != 0);
	}

	inline bool wt() const
	{
		return ((m_insn & 0x80) != 0);
	}

public: // Traps
	inline uint32_t vec9() const
	{
		return (m_insn & 0x1ff);
	}

public: // Floating Point Format
	inline uint32_t td() const
	{
		switch(opcode()) {
			case M88K_OPC_FCMP:
			case M88K_OPC_FCMPU:
				return 0;
			default:
				return ((m_insn >> 5) & 3);
		}
	}

	inline uint32_t t1() const
	{
		return ((m_insn >> 9) & 3);
	}

	inline uint32_t t2() const
	{
		return ((m_insn >> 7) & 3);
	}

private:
	inline uint32_t major_opcode() const
	{
		return ((m_insn >> M88K_MAJOPC_SHIFT) & M88K_MAJOPC_MASK);
	}

	inline uint32_t sub_opcode_0() const
	{
		return ((m_insn >> M88K_TFMT_MINOPC_SHIFT) & M88K_TFMT_MINOPC_MASK);
	}

	inline uint32_t sub_opcode_1() const
	{
		return ((m_insn >> M88K_BFMT_MINOPC_SHIFT) & M88K_BFMT_MINOPC_MASK);
	}

public:
	static inline bool is_load(m88k_opcode_t opc)
	{
		switch(opc) {
			case M88K_OPC_LD_B:
			case M88K_OPC_LD_BU:
			case M88K_OPC_LD_H:
			case M88K_OPC_LD_HU:
			case M88K_OPC_LD:
			case M88K_OPC_LD_D:
			case M88K_OPC_LD_X:
				return true;
			default:
				return false;
		}
	}

	static inline bool is_store(m88k_opcode_t opc)
	{
		switch(opc) {
			case M88K_OPC_ST_B:
			case M88K_OPC_ST_H:
			case M88K_OPC_ST:
			case M88K_OPC_ST_D:
			case M88K_OPC_ST_X:
				return true;
			default:
				return false;
		}
	}

	static inline bool is_xmem(m88k_opcode_t opc)
	{
		switch(opc) {
			case M88K_OPC_XMEM:
			case M88K_OPC_XMEM_BU:
				return true;
			default:
				return false;
		}
	}

	static inline bool is_load_or_store(m88k_opcode_t opc)
	{
		return (is_load(opc) || is_store(opc) || is_xmem(opc));
	}

public:
	inline bool is_sfu1() const
	{
		insn_desc const *desc = &desc_major[major_opcode()];
		return (desc->opcode == M88K_POPC_SFU1);
	}

	inline bool is_sfu2() const
	{
		insn_desc const *desc = &desc_major[major_opcode()];
		return (desc->opcode == M88K_POPC_SFU1);
	}

private:
	inline insn_desc const *desc() const
	{
		insn_desc const *desc = &desc_major[major_opcode()];

		switch (desc->opcode) {
			case M88K_POPC_BFMT:
				desc = &desc_cfmt[sub_opcode_1()];
				break;

			case M88K_POPC_EXT1:
				desc = &desc_ext1[sub_opcode_1()];
				break;

			case M88K_POPC_EXT2:
				desc = &desc_ext2[sub_opcode_0() >> 8][sub_opcode_1()];
				break;

			case M88K_POPC_SFU1:
				desc = &desc_sfu1[sub_opcode_1()];
				break;

			case M88K_POPC_SFU2:
				desc = &desc_sfu2[sub_opcode_1()];
				break;

			default:
				break;
		}

		if (desc != NULL && desc->opcode == M88K_OPC_ILLEGAL)
			desc = NULL;

		return (desc);
	}

public:
	inline operator uint32_t() const { return m_insn; }
};

#endif  // !__m88k_insn_h
