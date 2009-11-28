/*
 * libcpu: idbg.cpp
 *
 * This is a small interactive debugger for libcpu.
 *
 * The possible commands are:
 *
 * ? - help
 * x[/Ntf] <address>|$pc|$rX - examine N <t> from address, PC or register X
 *                             in format <f>
 *                             where t = i,s,b,h,w,d,q,o,f,F,l,L
 *                             and f = x,o,t,d,u,c (valid only for integers)
 *   i = instructions
 *   s = string
 *   b = 8bit
 *   h = 16bit words
 *   w = 32bit words
 *   d = 64bit words
 *   q = 128bit words
 *   f = 32bit float
 *   F = 64bit float
 *   l = 80bit float
 *   L = 128bit float
 * p[/f] $pc|$psr|$[rfv]X - print PC, PSR or register X
 *   f = x,o,t,d,u,c
 * s - step
 * . - step and display modified registers.
 * > - non interactive step and display modified registers.
 * c/q - continue execution (detach)
 * i - toggle IR dumping
 *
 * --NYI--
 * I - dump LLVM IR.
 * r [rfv]X <value> - set or get register
 * j <address> - jump to address
 * b <address> - add breakpoint
 * w <address> - add watchpoint
 * lb - list breakpoint
 * lw - list watchpoint
 * be N - enable breakpoint N
 * bd N - disable breakpoint N
 * we N - enable watchpoint N
 * wd N - disable watchpoint N
 * bt - call stack trace
 */

#include <stdio.h>
#include <assert.h>

#define USE_READLINE 1

#ifdef USE_READLINE
#define Function FunctionX // XXX clash in readline.
#include <readline/readline.h>
#include <readline/history.h>
#undef Function
#endif

#include "libcpu.h"

#define ALIGN8(x) (((x) + 7) & -8)

typedef struct _idbg {
	cpu_t *cpu;

	unsigned old_flags;
	unsigned old_optimize_flags;
	unsigned old_debug_flags;

	unsigned reg_format;
	unsigned reg_count;

	unsigned fp_reg_format;
	unsigned fp_reg_count;

	uint64_t saved_pc;
	uint64_t saved_psr;
	uint64_t *saved_regs;
	fp128_reg_t *saved_fp_regs;

	debug_function_t debug_function;
} idbg_t;

enum {
	F_INVALID = -1,
	F_INSTRUCTION,
	F_STRING,
	F_BYTE,
	F_HWORD,
	F_WORD,
	F_DWORD,
	F_QWORD,
	F_FLOAT32,
	F_FLOAT64,
	F_FLOAT80,//TWORD (intel colloqualism)
	F_FLOAT128
};

enum {
	M_HEX,
	M_OCT,
	M_BIN,
	M_UNSIGNED,
	M_SIGNED,
	M_CHAR
};

enum {
	R_GPR,
	R_FPR,
	R_VR
};

//////////////////////////////////////////////////////////////////////////////
// PRINT FORMATTER HELPERS
//////////////////////////////////////////////////////////////////////////////

static void
idbg_print_char(char c);

static void
idbg_print_bits(uint64_t v, unsigned bits)
{
	while (bits-- != 0) {
		fputc('0' + (v & 1), stdout);
		v >>= 1;
	}
}

static void
idbg_print_int_value(uint64_t v, unsigned bits, int align, unsigned mode)
{
	switch (mode) {
		case M_CHAR:
			fputc('\'', stdout);
			idbg_print_char(v);
			fputc('\'', stdout);
			break;

		case M_SIGNED:
			if (bits != 64 && (v & (1ULL << (bits - 1))) != 0)
				v |= -1ULL << bits;
			fprintf(stdout, "%lld", (int64_t)v);
			break;

		case M_UNSIGNED:
			fprintf(stdout, "%llu", v);
			break;

		case M_BIN:
			idbg_print_bits(v, bits);
			break;

		case M_OCT:
			if (align < 0)
				align = ALIGN8(bits) / 3;
			if (align == 0)
				fprintf(stdout, "0%llo", v);
			else
				fprintf(stdout, "0%.*llo", align, v);
			break;

		case M_HEX:
			if (align < 0)
				align = ALIGN8(bits) >> 2;
			if (align == 0)
				fprintf(stdout, "0x%llx", v);
			else
				fprintf(stdout, "0x%.*llx", align, v);
			break;
	} 
}

#define idbg_print_byte(v, m, a)  idbg_print_int_value(v, 8, a, m)
#define idbg_print_hword(v, m, a) idbg_print_int_value(v, 16, a, m)
#define idbg_print_word(v, m, a)  idbg_print_int_value(v, 32, a, m)
#define idbg_print_dword(v, m, a) idbg_print_int_value(v, 64, a, m)

static inline void
idbg_print_qword(uint64_t const *v, unsigned mode, int)
{
	if (mode == M_BIN) {
		idbg_print_bits(v[0], 64);
		idbg_print_bits(v[1], 64);
	} else
		fprintf(stdout, "0x%016llx%016llx", v[0], v[1]);
}

static void
idbg_print_char(char c)
{
	switch (c) {
		case '\a':
		  fprintf(stdout, "\\a");
		  break;
		case '\b':
		  fprintf(stdout, "\\b");
		  break;
		case '\f':
		  fprintf(stdout, "\\f");
		  break;
		case '\n':
		  fprintf(stdout, "\\n");
		  break;
		case '\r':
		  fprintf(stdout, "\\r");
		  break;
		case '\t':
		  fprintf(stdout, "\\t");
		  break;
		case '\v':
		  fprintf(stdout, "\\v");
		  break;
		case '\\':
		case '\'':
		case '\"':
		  fprintf(stdout, "\\%c", c);
		  break;
		default:
			if (iscntrl(c) || !isprint(c))
				fprintf(stdout, "\\%04o", (uint8_t)c);
			else
				fprintf(stdout, "%c", c);
			break;
	}
}

static inline ssize_t
idbg_print_string(char const *string)
{
	char const *p = string;

	while (*p != '\0')
		idbg_print_char(*p++);

	return ((p - string) + 1);
}

static inline void
idbg_print_float32(uint32_t v, bool hex)
{
	if (hex)
		fprintf(stdout, "%f [%08lx]", *(float *)&v, (unsigned long)v);
	else
		fprintf(stdout, "%f", *(float *)&v);
}

static inline void
idbg_print_float64(uint64_t v, bool hex)
{
	if (hex)
		fprintf(stdout, "%f [%016llx]", *(double *)&v, v);
	else
		fprintf(stdout, "%f", *(double *)&v);
}

static inline void
idbg_print_float80(uint64_t const *v, bool hex)
{
	fprintf(stdout, "[%04llx%016llx]", v[0], v[1]);
}

static inline void
idbg_print_float128(uint64_t const *v, bool hex)
{
	fprintf(stdout, "[%016llx%016llx]", v[0], v[1]);
}

//////////////////////////////////////////////////////////////////////////////
// MEMORY HELPERS
//////////////////////////////////////////////////////////////////////////////

static inline int
idbg_read_hword(idbg_t *ctx, addr_t address, uint16_t *half)
{
	uint8_t b[2];
	cpu_t *cpu = ctx->cpu;

	if (cpu->is_little_endian) {
		b[1] = cpu->RAM[address+0];
		b[0] = cpu->RAM[address+1];
	} else {
		b[0] = cpu->RAM[address+1];
		b[1] = cpu->RAM[address+0];
	}
	*half = (b[1] << 8) | b[0];
	return (0);
}

static inline int
idbg_read_word(idbg_t *ctx, addr_t address, uint32_t *word)
{
	uint8_t b[2];
	cpu_t *cpu = ctx->cpu;

	if (cpu->is_little_endian) {
		b[3] = cpu->RAM[address+0];
		b[2] = cpu->RAM[address+1];
		b[1] = cpu->RAM[address+2];
		b[0] = cpu->RAM[address+3];
	} else {
		b[0] = cpu->RAM[address+3];
		b[1] = cpu->RAM[address+2];
		b[2] = cpu->RAM[address+1];
		b[3] = cpu->RAM[address+0];
	}
	*word = (b[3] << 24) | (b[2] << 16) | (b[1] << 8) | b[0];
	return (0);
}

static inline int
idbg_read_dword(idbg_t *ctx, addr_t address, uint64_t *dword)
{
  	uint32_t w[2];

	if (idbg_read_word(ctx, address, w + 0) ||
		idbg_read_word(ctx, address + 4, w + 1))
		return (-1);

	if (ctx->cpu->is_little_endian)
		*dword = ((uint64_t)w[1] << 32) | w[0];
	else
		*dword = ((uint64_t)w[0] << 32) | w[1];

	return (0);
}

static inline int
idbg_read_qword(idbg_t *ctx, addr_t address, uint64_t *qword)
{
  	uint64_t w[2];
	cpu_t *cpu = ctx->cpu;

	if (idbg_read_dword(ctx, address, w + 0) ||
		idbg_read_dword(ctx, address + 8, w + 1))
		return (-1);

	if (ctx->cpu->is_little_endian) {
		qword[0] = w[1];
		qword[1] = w[0];
	} else {
		qword[0] = w[0];
		qword[1] = w[1];
	}
	return (0);
}

static inline int
idbg_read_tword(idbg_t *ctx, addr_t address, uint64_t *tword)
{
  	uint64_t lo;
	uint16_t hi;

	if (ctx->cpu->is_little_endian) {
		if (idbg_read_dword(ctx, address, &lo) ||
			idbg_read_hword(ctx, address + 8, &hi))
			return (-1);
	} else {
		if (idbg_read_hword(ctx, address, &hi) ||
			idbg_read_dword(ctx, address + 2, &lo))
			return (-1);
	}
	tword[0] = lo;
	tword[1] = hi;
	return (0);
}

//////////////////////////////////////////////////////////////////////////////
// EXAMINE HELPERS
//////////////////////////////////////////////////////////////////////////////

static inline ssize_t
idbg_examine_byte(idbg_t *ctx, addr_t address, unsigned mode)
{
	idbg_print_byte(ctx->cpu->RAM[address], mode, -1);
	return (1);
}

static inline ssize_t
idbg_examine_char(idbg_t *ctx, addr_t address)
{
	fputc('\'', stdout);
	idbg_print_char(ctx->cpu->RAM[address]);
	fputc('\'', stdout);
	return (1);
}

static inline ssize_t
idbg_examine_hword(idbg_t *ctx, addr_t address, unsigned mode)
{
	uint16_t w;
	if (idbg_read_hword(ctx, address, &w))
		return (-1);
	idbg_print_hword(w, mode, -1);
	return (2);
}

static inline ssize_t
idbg_examine_word(idbg_t *ctx, addr_t address, unsigned mode)
{
	uint32_t w;
	if (idbg_read_word(ctx, address, &w))
		return (-1);
	idbg_print_word(w, mode, -1);
	return (4);
}

static inline ssize_t
idbg_examine_dword(idbg_t *ctx, addr_t address, unsigned mode)
{
	uint64_t w;
	if (idbg_read_dword(ctx, address, &w))
		return (-1);
	idbg_print_dword(w, mode, -1);
	return (8);
}

static inline ssize_t
idbg_examine_qword(idbg_t *ctx, addr_t address, unsigned mode)
{
	uint64_t w[2];
	if (idbg_read_qword(ctx, address, w))
		return (-1);
	idbg_print_qword(w, mode, -1);
	return (16);
}

static inline ssize_t
idbg_examine_float32(idbg_t *ctx, addr_t address)
{
	uint32_t w;
	if (idbg_read_word(ctx, address, &w))
		return (-1);
	idbg_print_float32(w, false);
	return (4);
}

static inline ssize_t
idbg_examine_float64(idbg_t *ctx, addr_t address)
{
	uint64_t w;
	if (idbg_read_dword(ctx, address, &w))
		return (-1);
	idbg_print_float64(w, false);
	return (8);
}

static inline ssize_t
idbg_examine_float80(idbg_t *ctx, addr_t address)
{
	uint64_t w[2];
	if (idbg_read_tword(ctx, address, w))
		return (-1);
	idbg_print_float80(w, false);
	return (10);
}

static inline ssize_t
idbg_examine_float128(idbg_t *ctx, addr_t address)
{
	uint64_t w[2];
	if (idbg_read_qword(ctx, address, w))
		return (-1);
	idbg_print_float128(w, false);
	return (16);
}

static inline ssize_t
idbg_examine_string(idbg_t *ctx, addr_t address)
{
	ssize_t length;

	fputc('"', stdout);
	length = idbg_print_string((char const *)ctx->cpu->RAM + address);
	fputc('"', stdout);
	return length;
}

static inline ssize_t
idbg_examine_instruction(idbg_t *ctx, addr_t address)
{
	char buf[256];
	int nb = ctx->cpu->f.disasm_instr(ctx->cpu, address, buf, sizeof(buf));
	fprintf(stdout, "%s", buf);
	return nb;
}

static ssize_t
idbg_examine_one(idbg_t *ctx, addr_t address, unsigned format,
	unsigned mode)
{
	switch (format) {
		case F_INSTRUCTION:
			return idbg_examine_instruction(ctx, address);
		case F_STRING:
			return idbg_examine_string(ctx, address);
		case F_BYTE:
			return idbg_examine_byte(ctx, address, mode);
		case F_HWORD:
			return idbg_examine_hword(ctx, address, mode);
		case F_WORD:
			return idbg_examine_word(ctx, address, mode);
		case F_DWORD:
			return idbg_examine_dword(ctx, address, mode);
		case F_QWORD:
			return idbg_examine_qword(ctx, address, mode);
		case F_FLOAT32:
			return idbg_examine_float32(ctx, address);
		case F_FLOAT64:
			return idbg_examine_float64(ctx, address);
		case F_FLOAT80:
			return idbg_examine_float64(ctx, address);
		case F_FLOAT128:
			return idbg_examine_float128(ctx, address);
		default:
			assert(0 && "Shouldn't happen");
			return (-1);
	}
}

//////////////////////////////////////////////////////////////////////////////

static void
idbg_print_address(idbg_t *ctx, addr_t address)
{
	idbg_print_int_value(address, ctx->cpu->reg_size, -1, M_HEX);
}

static void
idbg_print_register(idbg_t *ctx, unsigned type, void const *value,
	unsigned mode)
{
	if (type == R_GPR) {
		idbg_print_int_value(*(uint64_t *)value, ctx->cpu->reg_size,
			0, mode);
	} else if (type == R_FPR) {
		switch (ctx->fp_reg_format) {
		  	case F_FLOAT32:
				idbg_print_float32(*(uint32_t *)value, true);
				break;
		  	case F_FLOAT64:
				idbg_print_float64(*(uint64_t *)value, true);
				break;
		  	case F_FLOAT80:
				idbg_print_float80((uint64_t const *)value, true);
				break;
			case F_FLOAT128:
				idbg_print_float128((uint64_t const *)value, true);
				break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////

static inline unsigned
reg_size_to_type(cpu_t *cpu)
{
	unsigned format = F_INVALID;

	if (cpu->reg_size <= 8)
		format = F_BYTE;
	else if (cpu->reg_size <= 16)
		format = F_HWORD;
	else if (cpu->reg_size <= 32)
		format = F_WORD;
	else if (cpu->reg_size <= 64)
		format = F_DWORD;
	else if (cpu->reg_size <= 128)
		format = F_QWORD;

	return format;
}

static inline unsigned
fp_reg_size_to_type(cpu_t *cpu)
{
	unsigned format = F_INVALID;

	if (cpu->fp_reg_size == 32)
		format = F_FLOAT32;
	else if (cpu->fp_reg_size == 64)
		format = F_FLOAT64;
	else if (cpu->fp_reg_size == 80)
		format = F_FLOAT80;
	else if (cpu->fp_reg_size == 128)
		format = F_FLOAT128;

	return format;
}

static void
idbg_init(cpu_t *cpu, debug_function_t debug_func, idbg_t *ctx)
{
	ctx->cpu = cpu;
	ctx->debug_function = debug_func;

	ctx->reg_format = reg_size_to_type(cpu);
	ctx->fp_reg_format = fp_reg_size_to_type(cpu);
	
	//
	// Allocate memory to compute register differences.
	//

	// XXX We need to count EFFECTIVE registers, cpu_t
	// internally stores more register than the target
	// architecture does because of practicity, so we
	// need to pass through this loop until there's
	// a nice structure (ie arch_info_t) which describe
	// the architecture.
	for (size_t n = 0; ; n++) {
		uint64_t v;
		if (cpu->f.get_reg(cpu, cpu->reg, n, &v)) {
			ctx->reg_count = n;
			break;
		}
	}

	for (size_t n = 0; ; n++) {
		fp128_reg_t v;
		if (cpu->f.get_fp_reg(cpu, cpu->fp_reg, n, &v)) {
			ctx->fp_reg_count = n;
			break;
		}
	}

	ctx->saved_pc = 0;
	ctx->saved_psr = 0;

	if (ctx->reg_count != 0) {
		ctx->saved_regs = (uint64_t *)malloc(sizeof(uint64_t) * ctx->reg_count);
		assert(ctx->saved_regs != NULL);
	}
	if (ctx->fp_reg_count != 0) {
		ctx->saved_fp_regs = (fp128_reg_t *)malloc(sizeof(fp128_reg_t) * ctx->fp_reg_count);
		assert(ctx->saved_fp_regs != NULL);
	}

	// Turn on single step
	ctx->old_flags = cpu->flags;
	ctx->old_optimize_flags = cpu->flags_optimize;
	ctx->old_debug_flags = cpu->flags_debug;

	cpu->flags = ctx->old_flags;
	cpu_set_flags_optimize(cpu, CPU_OPTIMIZE_NONE);
	cpu_set_flags_debug(cpu, ctx->old_debug_flags | CPU_DEBUG_SINGLESTEP);
}

static void
idbg_done(idbg_t *ctx)
{
	cpu_set_flags_debug(ctx->cpu, ctx->old_debug_flags);
	cpu_set_flags_optimize(ctx->cpu, ctx->old_optimize_flags);
	ctx->cpu->flags = ctx->old_flags;
	if (ctx->saved_fp_regs != NULL)
		free(ctx->saved_fp_regs);
	if (ctx->saved_regs != NULL)
		free(ctx->saved_regs);
}


//////////////////////////////////////////////////////////////////////////////

static void
idbg_save_registers(idbg_t *ctx)
{
  	size_t n;
  	cpu_t *cpu = ctx->cpu;

	// save pc & psr
  	ctx->saved_pc = cpu->f.get_pc(cpu, cpu->reg);
	ctx->saved_psr = cpu->f.get_psr(cpu, cpu->reg);

	// save gpr
	for (n = 0; n < ctx->reg_count; n++)
		cpu->f.get_reg(cpu, cpu->reg, n, ctx->saved_regs + n);

	// save fpr
	for (n = 0; n < ctx->fp_reg_count; n++)
		cpu->f.get_fp_reg(cpu, cpu->fp_reg, n, ctx->saved_fp_regs + n);
}

static void
idbg_diff_registers(idbg_t *ctx)
{
  	size_t n;
	cpu_t *cpu = ctx->cpu;

	uint64_t pc = cpu->f.get_pc(cpu, cpu->reg);
	if (pc != ctx->saved_pc) {
		fprintf(stdout, "pc:\t");
		idbg_print_address(ctx, ctx->saved_pc);
		fprintf(stdout, " -> ");
		idbg_print_address(ctx, pc);
		fprintf(stdout, "\n");
	}

	uint64_t psr = cpu->f.get_psr(cpu, cpu->reg);
	if (psr != ctx->saved_psr) {
		fprintf(stdout, "psr:\t");
		idbg_print_address(ctx, ctx->saved_psr);
		fprintf(stdout, " -> ");
		idbg_print_address(ctx, psr);
		fprintf(stdout, "\n");
	}

	for (n = 0; n < ctx->reg_count; n++) {
		uint64_t v;
		cpu->f.get_reg(cpu, cpu->reg, n, &v);
		if (v != ctx->saved_regs[n]) {
			fprintf(stdout, "r%u:\t", (unsigned)n);
			idbg_print_address(ctx, ctx->saved_regs[n]);
			fprintf(stdout, " -> ");
			idbg_print_address(ctx, v);
			fprintf(stdout, " (delta %lld)\n",
				v - ctx->saved_regs[n]);
		}
	}

#if 0
	for (n = 0; n < ctx->fp_reg_count; n++) {
		fp128_reg_t v;

		cpu->f.get_fp_reg(cpu, cpu->fp_reg, n, &v);
		if (memcmp(&v, cpu->saved_fp_regs + n, sizeof(v)) != 0) {
			fprintf(stdout, "f%u:\t", n);
			idbg_print_float(ctx, &ctx->saved_fp_regs[n]);
			fprintf(stdout, " -> ");
			idbg_print_float(ctx, &v);
			fprintf(stdout, "\n");
		}
	}
#endif

}

//////////////////////////////////////////////////////////////////////////////

static int
idbg_step(idbg_t *ctx)
{
	int rc = cpu_run(ctx->cpu, ctx->debug_function);
	cpu_flush(ctx->cpu);
	return rc;
}

static int
idbg_step_diff(idbg_t *ctx)
{
	int rc;

	idbg_save_registers(ctx);
	rc = idbg_step(ctx);
	idbg_diff_registers(ctx);

	return rc;
}

//////////////////////////////////////////////////////////////////////////////

static inline size_t
elements_per_line(unsigned format)
{
	switch (format) {
		case F_INSTRUCTION:
		case F_STRING:
			return 1;

		case F_BYTE:
			return 8;

		case F_HWORD:
			return 8;

		case F_WORD:
			return 4;

		case F_DWORD:
			return 2;

		case F_QWORD:
			return 1;

		case F_FLOAT32:
		case F_FLOAT64:
		case F_FLOAT80:
		case F_FLOAT128:
			return 2;
	}
}

static inline size_t
elements_per_line(unsigned format, unsigned mode)
{
	size_t n, nelem = elements_per_line(format);
	switch (format) {
		case F_BYTE:
		case F_HWORD:
		case F_WORD:
		case F_DWORD:
		case F_QWORD:
			if (mode == M_BIN && nelem > 1)
				nelem >>= 1;
			break;
	}
	return nelem;
}

static ssize_t
idbg_examine(idbg_t *ctx, addr_t address, size_t count,
	unsigned format, unsigned mode)
{
	addr_t base = address;
	size_t n, nelem = elements_per_line(format, mode);
	for (n = 0; n < count; n++) {
		if ((n % nelem) == 0) {
			if (n != 0)
				fprintf(stdout, "\n");
			idbg_print_address(ctx, address);
			fprintf(stdout, ":");
		}
		fprintf(stdout, "\t");

		ssize_t bytes = idbg_examine_one(ctx, address, format, mode);
		if (bytes < 0) {
			fprintf(stderr, "\nIDBG ERROR: Cannot access address %llx.\n",
				address);
			break;
		}

		address += bytes;
	}
	fprintf(stdout, "\n");
	return (address - base);
}

//////////////////////////////////////////////////////////////////////////////

static void
idbg_input_init(void)
{
#ifdef USE_READLINE
	rl_initialize();
	using_history();
#endif
}

static char *
idbg_input_read(char const *prompt)
{
#ifdef USE_READLINE
	char *p = readline(prompt);
	if (p != NULL) {
		while (isspace (*p))
			p++;
		if (*p != '\0')
			add_history(p);
	}
	return (p);
#else
	static char buf[256];
	fprintf(stdout, "%s", prompt); fflush(stdout);
	if (fgets(buf, sizeof(buf), stdin) == NULL)
		return (NULL);
	return (buf);
#endif
}

static int
idbg_address_operand(idbg_t *ctx, char const *token, addr_t *address)
{
	cpu_t *cpu = ctx->cpu;

	if (token == NULL || *token == '\0')
		return (-1);

	if (*token == '$') {
		uint64_t value;

		token++;
		if (*token == '\0')
			return (-1);
		if (strncmp(token, "pc", 2) == 0)
			value = cpu->f.get_pc((cpu_t *)cpu, cpu->reg);
		else if (*token == 'r' || isdigit(*token)) {
			if (!isdigit(*token))
				token++;
			if (!isdigit(*token))
				return (-1);
			if (cpu->f.get_reg((cpu_t *)cpu, cpu->reg, atoi(token), &value))
				return (-1);
		} else
			return (-1);

		*address = value;
	} else if (*token == '0' && (*(token + 1) == 'x' || *(token + 1) == 'X')) {
		*address = strtoull(token + 2, NULL, 16);
	} else {
		*address = strtoull(token, NULL, 10);
	}

	return (0);
}

static int
idbg_register_operand(idbg_t *ctx, char const *token, unsigned *type, int *index, void *value)
{
	cpu_t *cpu = ctx->cpu;

	if (token == NULL || *token == '\0')
		return (-1);

	if (*token == '$') {
		token++;
		if (*token == '\0')
			return (-1);
		if (strncmp(token, "pc", 2) == 0) {
			*index = -1;
			*(uint64_t *)value = cpu->f.get_pc((cpu_t *)cpu, cpu->reg);
			*type = R_GPR;
		} else if (strncmp(token, "psr", 2) == 0) {
			*index = -2;
			*(uint64_t *)value = cpu->f.get_psr((cpu_t *)cpu, cpu->reg);
			*type = R_GPR;
		} else if (*token == 'r' || isdigit(*token)) {
			if (!isdigit(*token))
				token++;
			if (!isdigit(*token))
				return (-1);
			*index = atoi(token);
			if (cpu->f.get_reg(cpu, cpu->reg, *index, (uint64_t *)value))
				return (-1);
			*type = R_GPR;
		} else if (*token == 'f') {
			token++;
			if (!isdigit(*token))
				return (-1);
			*index = atoi(token);
			if (cpu->f.get_fp_reg(cpu, cpu->fp_reg, *index, (fp128_reg_t *)value))
				return (-1);
			*type = R_FPR;
		}
	} else {
		return (-1);
	}

	return (0);
}

static void
idbg_help(char const *cmd)
{
	fprintf(stderr, "libcpu - interactive debugger\n");

	while (isspace(*cmd))
		cmd++;

	switch (*cmd) {
		case 'x':
			fprintf(stderr, "Syntax:   x[/N[f[m]]] <address> | $reg\n"
			                "Synopsis: Examine memory at the specified address, or at\n"
					        "          the value specified by register $reg.\n"
							"Desc:     The optional parameter 'N' specifies the number\n"
							"          of items to be dumped.\n"
							"          The optional parameter 'f' specifies the format of\n"
							"          the data, valid values are:\n"
							"            i - instruction, s - string, b - 8bit word,\n"
							"            h - 16bit word, w - 32bit word, d - 64bit word,\n"
							"            q - 128bit word, f - 32bit float, F - 64bit float,\n"
							"            l - 80bit float, L - 128bit float\n"
							"          The optional parameter 'm' specifies how the integer\n"
							"          data should be displayed, valid values are:\n"
							"            x - hexadecimal, o - octal, d - decimal (signed)\n"
							"            u - decimal (unsigned), c - characater, t - binary\n"
							"Examples: To disassemble 10 instructions at the current pc:\n\n"
							"            x/10i $pc\n\n"
							"          To display the first 5 strings at address 0x1000:\n\n"
							"            x/5s 0x1000\n\n"
							"          To display the first 20 32bit words at address contained\n"
							"          in register r10 in binary:\n\n"
							"            x/20wt $r10\n");
			break;

		case 'p':
			fprintf(stderr, "Syntax:   p[/m] $reg\n"
			                "Synopsis: Print the value contained in the specified register.\n"
							"Desc:     The optional parameter 'm' specifies how the integer\n"
							"          data should be displayed, valid values are:\n"
							"            x - hexadecimal, o - octal, d - decimal (signed)\n"
							"            u - decimal (unsigned), c - character, t - binary\n"
							"Examples: To print the Processor Status Register in binary:\n"
							"            p/t $psr\n\n");
			break;

		case '.':
			fprintf(stderr, "Syntax:   .\n"
			                "Synopsis: Single step with register changes tracking.\n"
							"Desc:     The register file is saved prior to the single\n"
							"          step and changes are printed as a result.\n");
			break;

		case '>':
			fprintf(stderr, "Syntax:   >\n"
			                "Synopsis: Non-interactive single step with register\n"
							"          changes tracking.\n"
							"Desc:     The execution continues single stepping automatically.\n"
							"          The register file is saved prior to the single\n"
							"          step and changes are printed as a result.\n");
			break;

		case 's':
			fprintf(stderr, "Syntax:   s\n"
			                "Synopsis: Single step.\n");
			break;

		case 'c':
		case 'q':
			fprintf(stderr, "Syntax:   c/q\n"
			                "Synopsis: Continue, detach the debugger.\n");
			break;

		default:
			fprintf(stderr, "Syntax:   ? <command>\n");
			fprintf(stderr, "Synopsis: Shows the help for specified command.\n\n");
			fprintf(stderr, "Available commands: c, p, q, s, x, . (dot), >\n");
			return;
	}
}

int
cpu_debugger(cpu_t *cpu, debug_function_t debug_function)
{
	idbg_t   ctx;
	unsigned format;
	unsigned mode;
	unsigned output = 1;
	int      last_command = 0;
	addr_t   last_address = 0;
	ssize_t  last_count = 0;
	int      rc = 0;
	bool     interactive = true;

	idbg_init(cpu, debug_function, &ctx);

	idbg_input_init();
	format = ctx.reg_format;
	mode = M_HEX;
	for (;;) {
		char const *p;
		
		if (interactive)
			p = idbg_input_read("idbg% ");
		else
			last_count = 2;

		if (p == NULL || ((*p == 'c' || *p == 'q') && *(p + 1) == 0)) {
			rc = -1;
			break;
		}

		if (*p != '\0')
			last_command = 0;

		if (*p == '?') {
			idbg_help(p + 1);
		} else if (last_command == 1 || *p == 's') {
			rc = idbg_step(&ctx);
			if (rc != JIT_RETURN_SINGLESTEP)
				return rc;
			last_command = 1;
		} else if (last_command == 2 || *p == '.') {
			rc = idbg_step_diff(&ctx);
			if (rc != JIT_RETURN_SINGLESTEP)
				return rc;
			last_command = 2;
		} else if (*p == 'i') {
			cpu_set_flags_debug(cpu, cpu->flags_debug ^ CPU_DEBUG_PRINT_IR);
			fprintf(stderr, "LLVM IR dumping is now %s.\n",
				(cpu->flags_debug & CPU_DEBUG_PRINT_IR) ? "ENABLED" : "DISABLED");
		} else if (*p == '>') {
			last_command = 2;
			interactive = false;
			p = "";
			continue;
		} else if (*p == 'p') {
			union {
				uint64_t value;
				fp128_reg_t fp_value;
			} rv;
			static char reg_type[] = { 'r', 'f', 'v', '\0' };
			unsigned type;
			unsigned mode = M_HEX;
			int index;

			p++;
			if (*p == '/') {
				p++;
				switch (*p++) {
					case 'x':
						mode = M_HEX;
						break;
					case 'o':
						mode = M_OCT;
						break;
					case 't':
						mode = M_BIN;
						break;
					case 'd':
						mode = M_SIGNED;
						break;
					case 'u':
						mode = M_UNSIGNED;
						break;
					case 'c':
						mode = M_CHAR;
						break;
					default:
						p--;
						if (!isspace (*p)) {
							fprintf(stderr, "IDBG ERROR: "
								"Unrecognized modifier '%c'.\n", *p);
							continue;
						}
						break;
				}
			}

			if (!isspace(*p)) {
				fprintf(stderr, "IDBG ERROR: Invalid syntax.\n", *p);
				continue;
			}

			// skip space
			while (isspace (*p))
			  p++;

			if (idbg_register_operand(&ctx, p, &type, &index, &rv)) {
				fprintf(stderr, "IDBG ERROR: Invalid syntax.\n");
				continue;
			}

			if (index == -1)
				fprintf(stdout, "$pc = ");
			else if (index == -2)
				fprintf(stdout, "$psr = ");
			else
				fprintf(stdout, "$%c%u = ", reg_type[type], index);

			idbg_print_register(&ctx, type, &rv, mode);
			fprintf(stdout, "\n");
		} else if (last_command == 3 || *p == 'x') {
			addr_t address;
			size_t count = 1;
			if (last_command == 3) {
				address = last_address;
				count = last_count;
			} else {
				p++;
				if (*p == '/') {
					p++;
					if (isdigit(*p)) {
						char *e;
						count = strtol(p, &e, 10);
						p = e;
					}
					switch (*p++) {
						case 'i':
							format = F_INSTRUCTION;
							break;
						case 's':
							format = F_STRING;
							break;
						case 'b':
							format = F_BYTE;
							break;
						case 'h':
							format = F_HWORD;
							break;
						case 'w':
							format = F_WORD;
							break;
						case 'd':
							format = F_DWORD;
							break;
						case 'q':
							format = F_QWORD;
							break;
						case 'f':
							format = F_FLOAT32;
							break;
						case 'F':
							format = F_FLOAT64;
							break;
						case 'l':
							format = F_FLOAT80;
							break;
						case 'L':
							format = F_FLOAT128;
							break;
						default:
							p--; // fallback
							break;
					}

					switch (*p++) {
						case 'x':
							mode = M_HEX;
							break;
						case 'o':
							mode = M_OCT;
							break;
						case 't':
							mode = M_BIN;
							break;
						case 's':
							mode = M_SIGNED;
							break;
						case 'u':
							mode = M_UNSIGNED;
							break;
						case 'c':
							mode = M_CHAR;
							break;
						default:
							p--;
							if (!isspace (*p)) {
								fprintf(stderr, "IDBG ERROR: "
									"Unrecognized modifier '%c'.\n", *p);
								continue;
							}
							break;
					}
				}
				if (!isspace(*p)) {
					fprintf(stderr, "IDBG ERROR: Invalid syntax.\n", *p);
					continue;
				}

				// skip space
				while (isspace (*p))
					p++;

				if (idbg_address_operand(&ctx, p, &address)) {
					fprintf(stderr, "IDBG ERROR: Invalid syntax.\n");
					continue;
				}
			}

			if (count > 0) {
				ssize_t bytes = idbg_examine(&ctx, address, count, format, mode);
				if (bytes > 0) {
					last_count = count;
					last_address = address + bytes;
					last_command = 3;
				}
			}
		} else if (*p == ';' || *p == '#')
			continue;
		else if (*p != '\0')
			fprintf(stderr, "IDBG ERROR: Unrecognized command.\n");
	}

	idbg_done(&ctx);

	return rc;
}
