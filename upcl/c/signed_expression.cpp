#include <cassert>

#include "c/signed_expression.h"
#include "c/type.h"

using namespace upcl;
using namespace upcl::c;

signed_expression::signed_expression(expression *expr)
	: expression(SIGNED), m_expr(expr)
{
	assert(expr->get_type()->get_type_id() != type::VECTOR &&
			"Vectors not supported.");
}

//
// SIGN EXPRESSION are *NEVER* simplified because
// they are used to evaluate signed values.
//
#include <cstdio>

expression *
signed_expression::simplify(bool) const
{
	expression *expr = m_expr->simplify(true);

	// bit combine operations can't be signed.
	if (expr->get_expression_operation() == expression::BIT_COMBINE)
		return expr;

	if (!expr->is_equal(m_expr))
		return new signed_expression(expr);

	return const_cast<signed_expression*>(this);
}

type *
signed_expression::get_type() const
{
	if (m_expr->get_expression_operation() == expression::CAST)
		return m_expr->sub_expr(0)->get_type();
	else
		return m_expr->get_type();
}

bool
signed_expression::is_constant() const
{
	return m_expr->is_constant();
}

expression *
signed_expression::sub_expr(size_t index) const
{
	return (index == 0 ? m_expr : 0);
}

bool
signed_expression::evaluate_as_integer(uint64_t &value, bool) const
{
	return m_expr->evaluate_as_integer(value, true);
}

bool
signed_expression::evaluate_as_float(double &value) const
{
	return m_expr->evaluate_as_float(value);
}

void
signed_expression::replace_sub_expr(size_t index, expression *expr)
{
	if (index == 0)
		m_expr = expr;
	else
		assert(0 && "Invalid expression index.");
}
