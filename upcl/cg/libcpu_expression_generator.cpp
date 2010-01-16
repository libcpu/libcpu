#include <cassert>

#include "cg/generate.h"
#include "c/expression_dumper.h"

#include "c/fast_aliases.h"
#include "c/unary_expression.h"
#include "c/binary_expression.h"
#include "c/bit_slice_expression.h"
#include "c/bit_combine_expression.h"
#include "c/integer_expression.h"
#include "c/float_expression.h"
#include "c/register_expression.h"
#include "c/decoder_operand_expression.h"
#include "c/cast_expression.h"

using namespace upcl;
using namespace upcl::c;
using namespace upcl::cg;

enum {
	F_SIGNED = 1
};

static void
generate_libcpu_expression(std::ostream &o, expression const *e,
		unsigned flags);

static void
dump_unary(std::ostream &o, unary_expression const *e, unsigned flags)
{
	switch (e->get_operation()) {
		case unary_expression::COM:
			o << "COM";
			break;
		case unary_expression::NEG:
			o << "NEG";
			break;
		case unary_expression::NOT:
			o << "NOT";
			break;
		default:
			assert(0 && "Not implemented yet.");
			break;
	}
	o << '(';
	generate_libcpu_expression(o, e->sub_expr(0), flags);
	o << ')';
}

static void
dump_binary(std::ostream &o, binary_expression const *e,
		unsigned flags)
{
	switch (e->get_operation()) {
		case binary_expression::ADD:
			o << "ADD";
			break;
		case binary_expression::SUB:
			o << "SUB";
			break;
		case binary_expression::MUL:
			o << "MUL";
			break;
		case binary_expression::DIV:
			o << "DIV";
			break;
		case binary_expression::REM:
			o << "REM";
			break;
		case binary_expression::AND:
			o << "AND";
			break;
		case binary_expression::OR:
			o << "OR";
			break;
		case binary_expression::XOR:
			o << "XOR";
			break;
		case binary_expression::SHL:
			o << "SHL";
			break;
		case binary_expression::SHR:
			o << "SHR";
			break;
		case binary_expression::ROL:
			o << "ROL";
			break;
		case binary_expression::ROR:
			o << "ROR";
			break;
		case binary_expression::EQ:
			o << "ICMP_EQ";
			break;
		case binary_expression::NE:
			o << "ICMP_NE";
			break;
		case binary_expression::LE:
			if (flags & F_SIGNED)
				o << "ICMP_SLE";
			else
				o << "ICMP_ULE";
			break;
		case binary_expression::LT:
			if (flags & F_SIGNED)
				o << "ICMP_SLT";
			else
				o << "ICMP_ULT";
			break;
		case binary_expression::GE:
			if (flags & F_SIGNED)
				o << "ICMP_SGE";
			else
				o << "ICMP_UGE";
			break;
		case binary_expression::GT:
			if (flags & F_SIGNED)
				o << "ICMP_SGT";
			else
				o << "ICMP_UGT";
			break;
		default:
			assert(0 && "Not implemented yet.");
			break;
	}
	o << '(';
	generate_libcpu_expression(o, e->sub_expr(0), 0);
	o << ',';
	generate_libcpu_expression(o, e->sub_expr(1), 0);
	o << ')';
}

static void
dump_bit_slice(std::ostream &o, bit_slice_expression const *bse)
{
	assert(bse->get_type()->get_bits() <= 64 && "Not yet tested.");

	expression *e;
	uint64_t bits;
	bool need_cast = (bse->sub_expr(1)->evaluate_as_integer(bits) &&
			bse->get_type()->get_bits() == bits);

	// is this a cast? bitslice can represent a cast.
	if (bse->sub_expr(0)->is_zero() && need_cast) {
		e = CCAST(bse->get_type(), bse->sub_expr(2));
	} else {
		e = CAND(CSHR(bse->sub_expr(0), bse->sub_expr(1)),
				CMASKBIT(bse->sub_expr(2)))->simplify();

		if (need_cast) 
			e = CCAST(bse->get_type(), e);
	}

	generate_libcpu_expression(o, e);
}

static void
dump_bit_combine(std::ostream &o, bit_combine_expression const *e)
{
	expression const *sub;

	o << '(';
	for (size_t n = 0; (sub = e->sub_expr(n)) != 0; n++) {
		if (n != 0)
			o << " : ";
		generate_libcpu_expression(o, e, 0);
	}
	o << ')';
}

#if 0
static void
dump_type(type const *ty)
{
	printf("#");
	switch (ty->get_type_id()) {
		case type::INTEGER:
			printf("i");
			break;
		case type::FLOAT:
			printf("f");
			break;
		case type::VECTOR:
			printf("v");
			break;
		default:
			assert(0 && "Shouldn't happen.");
			break;
	}
	printf("%zu", ty->get_bits());
}

static void
dump_cast(cast_expression const *e)
{
	printf("[ ");
	dump_type(e->get_type());
	printf(" ");
	dump_expression(e->sub_expr(0));
	printf(" ]");
}
#endif

static void
dump_sub_register(std::ostream &o, register_expression const *super,
		sub_register_def const *sub)
{
	expression *e = 0;
	if (sub->is_hardwired()) {
		e = sub->get_expression()->simplify();
	} else {
		e = CBITSLICE(CREG(sub->get_master_register()),
				sub->get_first_bit(), sub->get_bit_count());
		if (e != 0)
			e = e->simplify();
	}
	if (e == 0) {
		assert(0 && "Failed converting bitfield!");
	}

	generate_libcpu_expression(o, e, 0);
}

static void
dump_register(std::ostream &o, register_expression const *e)
{
	register_def const *reg = e->get_register();

	if (reg->is_sub_register()) {
		dump_sub_register(o, e, (sub_register_def const *)reg);
		return;
	}

	if (reg->is_hardwired())
		generate_libcpu_expression(o, reg->get_expression()->simplify(), 0);
	else if (!reg->is_virtual())
		o << "REG(" << reg->get_name() << ")";
	else
		assert(0 && "Virtual registers not yet implmented.");
}

static void
dump_decoder_operand(std::ostream &o, decoder_operand_expression const *e)
{
	decoder_operand_def const *opr = e->get_operand();

	o << "DECOPR(" << opr->get_name() << ")";
}

static void
dump_integer(std::ostream &o, integer_expression const *e, unsigned flags)
{
	uint64_t value = e->get_value((flags & F_SIGNED) != 0);
	size_t   nbits = e->get_type()->get_bits();
	if (nbits == 64)
		o << "CONST(";
	else if (nbits < 64)
		o << "CONSTn(" << e->get_type()->get_bits() << ',';
	else {
		o << "ERROR:INTEGER_TOO_LARGE";
		fprintf(stderr, "error: cannot write integers larger than 64bits, "
				"requested %zubits.\n", nbits);
		return;
	}

	if (value == 0)
		o << 0;
	else {
		o << "0x" << std::hex << value << std::dec;
		if (e->get_type()->get_bits() > 32) {
			if ((int32_t)value != (int64_t)value)
				o << "ULL";
		}
	}
	o << ")";
}

/*
 * Casts:
 *
 */
static inline bool
is_regular_integer(size_t size)
{ return (size == 1 || size == 8 || size == 16 || size == 32 || size == 64); }

static inline uint32_t
xec_ilog2(uint32_t n)
{ 
  register int i = (n & 0xffff0000) ? 16 : 0; 
  if ((n >>= i) & 0xff00) i |= 8, n >>= 8; 
  if (n & 0xf0)           i |= 4, n >>= 4; 
  if (n & 0xc)            i |= 2, n >>= 2; 
  return (i | (n >> 1)); 
} 

static inline size_t
align_size(size_t n, bool ignore1bit = false)
{
	if (!ignore1bit && n <= 1)
		return n;
	else if (n <= 8)
		return 8;
	else if (n <= 16)
		return 16;
	else if (n <= 32)
		return 32;
	else if (n <= 64)
		return 64;
	else {
		abort();
		return 0;
	}
}

static void
dump_cast(std::ostream &o, cast_expression const *e, unsigned flags)
{
	c::type const *type_in  = e->sub_expr(0)->get_type();
	c::type const *type_out = e->get_type();

	if (type_out->get_type_id () == c::type::INTEGER && 
			type_in->get_type_id() == c::type::INTEGER) {
		size_t shift     = 0;
		size_t master_size = 0;
		size_t size_in   = type_in->get_bits();
		size_t size_out  = type_out->get_bits();
		size_t asize_in  = xec_ilog2(size_in);
		size_t asize_out = xec_ilog2(size_out);
		bool   reg_in    = is_regular_integer(size_in);
		bool   reg_out   = is_regular_integer(size_out);

		if (reg_in && (flags & F_SIGNED) != 0) {
			// regular sizes for subregisters do not apply when
			// handling signed casts, take the size of the parent.
			c::expression const *sube = e->sub_expr(0);
			if (sube->get_expression_operation() == c::expression::REGISTER) {
				c::register_expression const *re = (c::register_expression const *)sube;
				c::sub_register_def const *sr = (c::sub_register_def const *)re->get_register();

				master_size = sr->get_master_register()->get_type()->get_bits();
				reg_in = false;
			}
		}


		while ((1U << asize_in) < size_in)
			asize_in++;

		while ((1U << asize_out) < size_out)
			asize_out++;

		asize_in  = align_size(1U << asize_in, master_size != 0);
		asize_out = align_size(1U << asize_out);

		if (size_in == size_out) {
			// XXX what about non-regular ints?
			generate_libcpu_expression(o, e->sub_expr(0), flags);
			return;
		}

		// if value isn't regular on output, we need to and.
		if (!reg_out)
			o << "AND(";

		// if value isn't regular on input and output type
		// is bigger, we'll need to shift right the result.
		if (!reg_in && size_out > size_in) {
			shift = asize_in - size_in;

			if (flags & F_SIGNED)
				o << "SRA(";
			else
				o << "SRL(";
		}

		// extend or shift appropriately
		if (size_in < size_out) {
			if (asize_out != asize_in) {
				if (flags & F_SIGNED)
					o << "SEXT" << asize_out << '(';
				else
					o << "ZEXT" << asize_out << '(';
			}
		} else if (asize_in != asize_out) {
			o << "TRUNC" << asize_out << '(';
		}

		// if we will shift right, shift left now.
		if (shift != 0)
			o << "SLL(";

		// intermediate truncation
		if (master_size != 0 && master_size > asize_in)
			o << "TRUNC" << asize_out << "(";

		// generate the expression
		generate_libcpu_expression(o, e->sub_expr(0), flags);

		// close intermediate truncation
		if (master_size != 0 && master_size > asize_in)
			o << ")";

		// close shift left
		if (shift != 0)
			o << ',' << "CONST(" << shift << ')' << ')';

		// close truncation/extension
		if (asize_in != asize_out)
			o << ')';

		// close shift right
		if (shift != 0)
			o << ',' << "CONST(" << shift << ')' << ')';

		// close and
		if (!reg_out)
			o << ',' << "CONST(" << "0x" << std::hex << ((1 << size_out) - 1)
			 << std::dec << ')' << ')';
	} else {
		fprintf(stderr, "error: casts to non-integer not yet supported.\n");
		exit(-1);
	}
}

static void
generate_libcpu_expression(std::ostream &o, expression const *e,
		unsigned flags)
{
	switch (e->get_expression_operation()) {
		case expression::UNARY:
			dump_unary(o, (unary_expression const *)e, flags);
			break;

		case expression::BINARY:
			dump_binary(o, (binary_expression const *)e, flags);
			break;

		case expression::REGISTER:
			dump_register(o, (register_expression const *)e);
			break;

		case expression::DECOPR:
			dump_decoder_operand(o, (decoder_operand_expression const *)e);
			break;

		case expression::INTEGER:
			dump_integer(o, (integer_expression const *)e, flags);
			break;

#if 0
		case expression::FLOAT:
			printf("%g", ((float_expression const *)e)->get_value());
			dump_type(e->get_type());
			break;
#endif

		case expression::CAST:
			dump_cast(o, (cast_expression const *)e, flags);
			break;

		case expression::BIT_SLICE:
			dump_bit_slice(o, (bit_slice_expression const *)e);
			break;

		case expression::BIT_COMBINE:
			dump_bit_combine(o, (bit_combine_expression const *)e);
			break;

		case expression::SIGNED:
			generate_libcpu_expression(o, e->sub_expr(0), flags | F_SIGNED);
			break;

		default:
			assert(0 && "Not implemented yet.");
			break;
	}
}

void
upcl::cg::generate_libcpu_expression(std::ostream &o, expression const *e)
{
	::generate_libcpu_expression(o, e, 0);
}
