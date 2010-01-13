#include <cassert>

#include "cg/generate.h"

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

	expression *e = CAND(CSHR(bse->sub_expr(0),
			bse->sub_expr(1)), CMASKBIT(bse->sub_expr(2)))->simplify();

	generate_libcpu_expression(o, e);
}

#if 0
static void
dump_bit_combine(bit_combine_expression const *e)
{
	expression const *sub;

	printf("( ");
	for (size_t n = 0; (sub = e->sub_expr(n)) != 0; n++) {
		if (n != 0)
			printf(" : ");
		dump_expression(sub);
	}
	printf(" )");
}

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
	if (e->get_type()->get_bits() == 64)
		o << "CONST(";
	else
		o << "CONSTn(" << e->get_type()->get_bits() << ',';

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
		case expression::CAST:
			dump_cast(o, (cast_expression const *)e, flags);
			break;
#endif
		case expression::BIT_SLICE:
			dump_bit_slice(o, (bit_slice_expression const *)e);
			break;
#if 0
		case expression::BIT_COMBINE:
			dump_bit_combine((bit_combine_expression const *)e);
			break;
#endif
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
