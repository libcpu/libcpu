#include "c/register_expression.h"

using namespace upcl;
using namespace upcl::c;

register_expression::register_expression(register_def *reg)
	: expression(REGISTER), m_reg(reg)
{
}

bool
register_expression::is_constant() const
{
	return false;
}

type *
register_expression::get_type() const
{
	return m_reg->get_type();
}

expression *
register_expression::sub_expr(size_t) const
{
	return 0;
}

bool
register_expression::evaluate_as_integer(uint64_t &, bool) const
{
	return false;
}

bool
register_expression::evaluate_as_float(double &) const
{
	return false;
}

bool
register_expression::is_equal(expression const *expr) const
{
	if (expression::is_equal(expr)) {
		register_expression const *e = (register_expression const *)expr;
		return (e->get_register() == get_register());
	}

	return false;
}
