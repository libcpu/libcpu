#include "c/unary_expression.h"
#include "c/binary_expression.h"
#include "c/bit_slice_expression.h"
#include "c/bit_combine_expression.h"
#include "c/integer_expression.h"
#include "c/float_expression.h"
#include "c/register_expression.h"
#include "c/cast_expression.h"

#include <cassert>

using namespace upcl;
using namespace upcl::c;

void dump_expr(expression const *e);

void
dump_unary(unary_expression const *e)
{
	switch (e->get_operation()) {
		case unary_expression::COM:
			printf("~");
			break;
		case unary_expression::NEG:
			printf("-");
			break;
		case unary_expression::NOT:
			printf("!");
			break;
		default:
			assert(0 && "Not implemented yet.");
			break;
	}
	dump_expr(e->sub_expr(0));
}

void
dump_binary(binary_expression const *e)
{
	printf("( ");
	dump_expr(e->sub_expr(0));
	switch (e->get_operation()) {
		case binary_expression::ADD:
			printf(" + ");
			break;
		case binary_expression::SUB:
			printf(" - ");
			break;
		case binary_expression::MUL:
			printf(" * ");
			break;
		case binary_expression::DIV:
			printf(" / ");
			break;
		case binary_expression::REM:
			printf(" %% ");
			break;
		case binary_expression::AND:
			printf(" & ");
			break;
		case binary_expression::OR:
			printf(" | ");
			break;
		case binary_expression::XOR:
			printf(" ^ ");
			break;
		case binary_expression::SHL:
			printf(" << ");
			break;
		case binary_expression::SHR:
			printf(" >> ");
			break;
		case binary_expression::ROL:
			printf(" <<> ");
			break;
		case binary_expression::ROR:
			printf(" >>< ");
			break;
		case binary_expression::EQ:
			printf(" == ");
			break;
		case binary_expression::NE:
			printf(" != ");
			break;
		case binary_expression::LE:
			printf(" <= ");
			break;
		case binary_expression::LT:
			printf(" < ");
			break;
		case binary_expression::GE:
			printf(" >= ");
			break;
		case binary_expression::GT:
			printf(" > ");
			break;
		default:
			assert(0 && "Not implemented yet.");
			break;
	}
	dump_expr(e->sub_expr(1));
	printf(" )");
}

void
dump_bit_slice(bit_slice_expression const *e)
{
	dump_expr(e->sub_expr(0));
	printf("[ ");
	dump_expr(e->sub_expr(1));
	printf(" : ");
	dump_expr(e->sub_expr(2));
	printf(" ]");
}

void
dump_bit_combine(bit_combine_expression const *e)
{
	expression const *sub;

	printf("( ");
	for (size_t n = 0; (sub = e->sub_expr(n)) != 0; n++) {
		if (n != 0)
			printf(" : ");
		dump_expr(sub);
	}
	printf(" )");
}

void
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

void
dump_cast(cast_expression const *e)
{
	printf("[ ");
	dump_type(e->get_type());
	printf(" ");
	dump_expr(e->sub_expr(0));
	printf(" ]");
}

void
dump_expr(expression const *e)
{
	switch (e->get_expression_operation()) {
		case expression::UNARY:
			dump_unary((unary_expression const *)e);
			break;
		case expression::BINARY:
			dump_binary((binary_expression const *)e);
			break;
		case expression::REGISTER:
			printf("%s", ((register_expression const *)e)->get_register()->get_name().c_str());
			dump_type(e->get_type());
			break;
		case expression::INTEGER:
			printf("%#llx", ((integer_expression const *)e)->get_value(false));
			dump_type(e->get_type());
			break;
		case expression::FLOAT:
			printf("%g", ((float_expression const *)e)->get_value());
			dump_type(e->get_type());
			break;
		case expression::CAST:
			dump_cast((cast_expression const *)e);
			break;
		case expression::BIT_SLICE:
			dump_bit_slice((bit_slice_expression const *)e);
			break;
		case expression::BIT_COMBINE:
			dump_bit_combine((bit_combine_expression const *)e);
			break;
		case expression::SIGNED:
			printf("%%S ( ");
			dump_expr(e->sub_expr(0));
			printf(" )");
			break;
		default:
			assert(0 && "Not implemented yet.");
			break;
	}
}

void
print_expr(expression const *expr)
{
	dump_expr(expr); 
	printf(" [%zu]\n", expr->get_type()->get_bits());
}

#if 0
void
test_expr(expression *m)
{
	printf("m = ");
	print_expr(m);

	for (;;) {
		int fail = 0;
		expression *ms = m->simplify();

		if (!ms->is_equal(m)) {
			printf("ms = ");
			print_expr(ms);
			m = ms;
		} else {
			fail++;
		}

		expression *msf = fold_constants(m);
		if (!msf->is_equal(m)) {
			printf("msf = ");
			print_expr(msf);
			m = msf;
		} else {
			fail++;
		}

		if (fail == 2)
			break;
	}
}
#endif
