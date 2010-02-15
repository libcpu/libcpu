#include "c/temp_value_expression.h"

using namespace upcl;
using namespace upcl::c;

temp_value_expression::temp_value_expression(temp_value_def *tmpval)
	: expression(TMPVAL), m_tmpval(tmpval)
{
}

bool
temp_value_expression::is_constant() const
{
	return false;
}

type *
temp_value_expression::get_type() const
{
	return m_tmpval->get_type();
}

expression *
temp_value_expression::sub_expr(size_t) const
{
	return 0;
}

bool
temp_value_expression::evaluate_as_integer(uint64_t &, bool) const
{
	return false;
}

bool
temp_value_expression::evaluate_as_float(double &) const
{
	return false;
}

bool
temp_value_expression::is_equal(expression const *expr) const
{
	if (expression::is_equal(expr)) {
		temp_value_expression const *e = (temp_value_expression const *)expr;
		return (e->get_operand() == get_operand());
	}

	return false;
}
