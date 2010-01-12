#include "c/decoder_operand_expression.h"

using namespace upcl;
using namespace upcl::c;

decoder_operand_expression::decoder_operand_expression(decoder_operand_def *decopr)
	: expression(DECOPR), m_decopr(decopr)
{
}

bool
decoder_operand_expression::is_constant() const
{
	return false;
}

type *
decoder_operand_expression::get_type() const
{
	return m_decopr->get_type();
}

expression *
decoder_operand_expression::sub_expr(size_t) const
{
	return 0;
}

bool
decoder_operand_expression::evaluate_as_integer(uint64_t &, bool) const
{
	return false;
}

bool
decoder_operand_expression::evaluate_as_float(double &) const
{
	return false;
}

bool
decoder_operand_expression::is_equal(expression const *expr) const
{
	if (expression::is_equal(expr)) {
		decoder_operand_expression const *e = (decoder_operand_expression const *)expr;
		return (e->get_operand() == get_operand());
	}

	return false;
}
