#ifndef __m88k_insn_h
#define __m88k_insn_h

#include <stddef.h>
#include <stdint.h>

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
    //static insn_desc const desc_sfu2[64]; Vectorial Engine nyi

public:
	m88k_insn (uint32_t insn)
		: m_insn (insn)
	{
	}

public:
	inline m88k_opcode_t opcode () const
	{
		insn_desc const *d = desc ();
		if (d != NULL)
			return d->opcode;
		else
			return M88K_OPC_ILLEGAL;
	}

	inline m88k_insnfmt_t format () const
	{
		insn_desc const *d = desc ();
		if (d != NULL)
			return d->format;
		else
			return M88K_FMT_NONE;
	}

	inline insn_desc const *desc () const
	{
		insn_desc const *desc = &desc_major[major_opcode ()];

		switch (desc->opcode) {
			case M88K_POPC_BFMT:
				desc = &desc_cfmt[sub_opcode_1 ()];
				break;

			case M88K_POPC_EXT1:
				desc = &desc_ext1[sub_opcode_1 ()];
				break;

			case M88K_POPC_EXT2:
				desc = &desc_ext2[sub_opcode_0 () >> 8][sub_opcode_1 ()];
				break;

			case M88K_POPC_SFU1:
				desc = &desc_sfu1[sub_opcode_1 ()];
				break;

			case M88K_POPC_SFU2:
				desc = NULL;
				break;

      default:
        break;
		}

		if (desc != NULL && desc->opcode == M88K_OPC_ILLEGAL)
			desc = NULL;

		return (desc);
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
				return false;
		}
	}

	inline bool has_carry() const
	{
		switch (opcode()) {
			case M88K_OPC_ADD:
			case M88K_OPC_ADDU:
			case M88K_OPC_SUB:
			case M88K_OPC_SUBU:
				return (true);
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
	inline uint32_t width() const
	{ 
		return ((m_insn >> M88K_BFMT_W_SHIFT) & M88K_BFMT_W_MASK);
	}

	inline int32_t offset() const
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
	inline int32_t branch26_offset() const
	{
		return ((m_insn >> M88K_BRFMT_OFF_SHIFT) & M88K_BRFMT_OFF_MASK);
	}

	inline int32_t branch16_offset() const
	{
		return ((m_insn >> M88K_CBFMT_OFF_SHIFT) & M88K_CBFMT_OFF_MASK);
	}

public: // Floating
	// SDD/SDS/DSS/DSD/DDS
	inline uint32_t float_format() const
	{
		return (t2() | (t1() << 1) | (td () << 2));
	}

	// Is using XFR register?
	inline bool is_xfr() const
	{
		return ((m_insn & 0x8000) != 0);
	}

private:
	inline uint32_t td() const
	{
		return ((m_insn >> 5) & 1);
	}

	inline uint32_t t1() const
	{
		return ((m_insn >> 9) & 1);
	}

	inline uint32_t t2() const
	{
		return ((m_insn >> 5) & 1);
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
};

#endif  // !__m88k_insn_h
