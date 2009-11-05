#include "libcpu.h"
#include "cpu_generic.h"

#include "m88k_internal.h"
#include "m88k_insn.h"

#include "strbuf.h"

//
// Disasm Format:
//
//   $xd - xfr dst register
//   $rd - gpr dst register
//   $x1 - xfr src1 register
//   $c1 - cr src1 register
//   $r1 - gpr src1 register
//   $x2 - xfr src2 register
//   $r2 - gpr src2 register
//   $i  - unsigned immediate
//   $s  - small unsigned immediate
//   $p  - pc
//   $b  - branch bits
//   $m  - branch comparison code
//   $w  - width
//

static char const * const m88k_insn_formats[] = {
	NULL,              // M88K_FMT_NONE
	"$rd, $r1, $i",    // M88K_IFMT_REG
	"$rd, $r1, $i",    // M88K_IFMT_MEM
	"$xd, $r1, $i",    // M88K_IFMT_XMEM
	"$p",              // M88K_BRFMT_OFF
	"$C, $r1, $p",     // M88K_BRFMT_COND
	"$b, $r1, $p",     // M88K_BRFMT_BIT
	"$rd, $r1, $r2",   // M88K_TFMT_REG
	"$xd, $x1, $x2",   // M88K_TFMT_XREG
	"$rd, $r1, $r2",   // M88K_TFMT_REGS
	"$rd, $r1[$r2]",   // M88K_TFMT_REGX
	"$rd, $r1, $w<$s>",// M88K_BFMT_REG
	"$rd, $r1, $i",    // M88K_BFMT_TST
	"$rd, $c1",        // M88K_CFMT_REG
	"$cd, $r1"         // M88K_CFMT_GER
};

static char const * const m88k_bcmp_values[] = {
	NULL, NULL, "eq", "ne", "gt", "le", "lt", "ge",
	"hi", "ls", "lo", "hs", "be", "nb", "he", "nh",
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

static char const * const m88k_bcnd_values[] = {
	NULL, "gt0", "eq0", "ge0", NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, "lt0", "ne0", "le0", NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

static char const * const m88k_insn_mnemonics[] = {
	"<ILLEGAL>",

	"add",
	"addu",

	"sub",
	"subu",

	"muls",
	"mulu",

	"divs",
	"divu",

	"mask",
	"mask.u",
	"and",
	"and.c",
	"and.u",
	"or",
	"or.c",
	"or.u",
	"xor",
	"xor.c",
	"xor.u",
	"rot",
	"mak",
	"set",
	"clr",
	"ext",
	"extu",
	"ff0",
	"ff1",

	"cmp",

	"lda",
	"lda.h",
	"lda.d",
	"lda.x",

	"ld",
	"ld.b",
	"ld.bu",
	"ld.h",
	"ld.hu",
	"ld.d",
	"ld.x",

	"st",
	"st.b",
	"st.h",
	"st.d",
	"st.x",

	"xmem",
	"xmem.bu",

	"jmp",
	"jmp.n",
	"jsr",
	"jsr.n",
	"br",
	"br.n",
	"bsr",
	"bsr.n",
	"bb0",
	"bb0.n",
	"bb1",
	"bb1.n",
	"bcnd",
	"bcnd.n",

	"tbnd",
	"tb0",
	"tb1",

	"ldcr",
	"stcr",

	"rte",

	/* FP Opcodes (SFU1) */

	"mov",
	"fcmp",
	"fcmpu",
	"flt",
	"fcnv",
	"fcvt",
	"int",
	"nint",
	"trnc",
	"fadd",
	"fsub",
	"fmul",
	"fdiv",
	"fsqrt",

	"fldcr",
	"fstcr"
};


static int m88k_disassemble (strbuf_t *strbuf, m88k_address_t pc,
	m88k_insn const &insn)
{
	char        reg_char;
	char const *tmp;
	char const *format;

	format = m88k_insn_formats[insn.format()];

	if (strbuf_append(strbuf, m88k_insn_mnemonics[insn.opcode()]))
		return (-1);

	if (insn.has_carry() && insn.carry() != 0) {
		if (strbuf_append(strbuf, ".c"))
			return (-1);

		switch (insn.carry()) {
			case M88K_CARRY_IN:
				if (strbuf_append_char(strbuf, 'i'))
					return (-1);
				break;

			case M88K_CARRY_OUT:
				if (strbuf_append_char(strbuf, 'o'))
					return (-1);
				break;

			case M88K_CARRY_IN | M88K_CARRY_OUT:
				if (strbuf_append(strbuf, "io"))
					return (-1);
				break;
		}
	}

	if (format == NULL)
		return (0);

	if (strbuf_append_char(strbuf, ' '))
		return (-1);

	while (*format != 0) {
		if (*format == '$') {
			format++;
			switch (*format++) {
				case 'c':
				case 'x':
				case 'r':
					reg_char = format[-1];
					switch (*format++) {
						case 'd':
							if (strbuf_append_format(strbuf, "%c%u", reg_char, insn.rd()))
								return (-1);
							break;

						case '1':
							if (strbuf_append_format(strbuf, "%c%u", reg_char, insn.rs1()))
								return (-1);
							break;

						case '2':
							if (strbuf_append_format(strbuf, "%c%u", reg_char, insn.rs2()))
								return (-1);
							break;
					}
					break;

				case 'w':
					if (strbuf_append_format(strbuf, "%u", insn.width()))
						return (-1);
					break;

				case 'i':
					if (strbuf_append_format(strbuf, "%#x", insn.immediate()))
						return (-1);
					break;

				case 'p':
					if (insn.format () != M88K_BRFMT_OFF)
						pc += insn.branch16_offset () << 2;
					else
						pc += insn.branch26_offset () << 2;

					if (strbuf_append_format(strbuf, "0x%08x", pc))
					  return (-1);
					break;

				case 's':
					if (strbuf_append_format(strbuf, "%u", insn.offset()))
						return (-1);
					break;

				case 'b':
					tmp = m88k_bcmp_values[insn.mb()];
					if (tmp == NULL) {
						if (strbuf_append_format(strbuf, "%#x", insn.mb()))
							return (-1);
					} else {
						if (strbuf_append(strbuf, tmp))
							return (-1);
					}
					break;

				case 'C':
					tmp = m88k_bcnd_values[insn.mb()];
					if (tmp == NULL) {
						if (strbuf_append_format(strbuf, "%#x", insn.mb()))
							return (-1);
					} else {
						if (strbuf_append(strbuf, tmp))
							return (-1);
					}
					break;
			}
		} else {
			if (strbuf_append_char(strbuf, *format++))
				return (-1);
		}
	}

	return (0);
}

int
arch_m88k_disasm_instr(uint8_t* RAM, addr_t pc, char *line, unsigned int max_line)
{
	strbuf_t strbuf;
	int      dummy1;
	addr_t   dummy2;
	int      bytes;
	
	bytes = arch_m88k_tag_instr(RAM, pc, &dummy1, &dummy2);

	strbuf_init(line, max_line, &strbuf);

	if (m88k_disassemble(&strbuf, pc, INSTR(pc))) {
		strbuf_terminate(&strbuf);
		return (strbuf_length(&strbuf));
	}

	if (m88k_insn(INSTR(pc)).is_delaying ()) {
		pc += 4;

		strbuf_append(&strbuf, " [");
		m88k_disassemble(&strbuf, pc, INSTR(pc));
		strbuf_append_char(&strbuf, ']');
	}
	strbuf_terminate(&strbuf);

	return (strbuf_length(&strbuf));
}
