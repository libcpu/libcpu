#include <cassert>

#include "c/cast_expression.h"
#include "c/fast_aliases.h"
#include "c/type.h"

using namespace upcl;
using namespace upcl::c;

cast_expression::cast_expression(type *ty, expression *expr)
	: expression(CAST), m_type(ty), m_expr(expr)
{
	assert(ty->get_type_id() == type::INTEGER &&
			"Only integer supported now.");
	assert(ty->get_bits() <= 64 &&
			"Only integers up to 64bits are supported now.");

}

expression *
cast_expression::simplify(bool sign) const
{
	expression *expr = m_expr->simplify();

	if (!expr->is_equal(m_expr))
		expr = (new cast_expression(m_type, expr))->simplify(sign);

#if 0
	// If non-signed, and casting a signed expression, make it unsigned!
	if (!sign && expr->get_expression_operation() == expression::SIGNED)
		expr = (new cast_expression(m_type, expr->sub_expr(0)))->simplify();
#endif

	if (expr->get_type()->get_bits() == m_type->get_bits())
		return expr;

	uint64_t value;
	if (evaluate_as_integer(value, sign))
		return expression::fromInteger(value, m_type->get_bits());

	// resolve to a bitslice if smaller.
	if (m_type->get_bits() < expr->get_type()->get_bits())
		return CBITSLICE(expr, CCONST(0), CCONST(m_type->get_bits()))->simplify();
	
	return const_cast<cast_expression*>(this);
}

type *
cast_expression::get_type() const
{
	return m_type;
}

bool
cast_expression::is_constant() const
{
	return m_expr->is_constant();
}

expression *
cast_expression::sub_expr(size_t index) const
{
	return (index == 0 ? m_expr : 0);
}

bool
cast_expression::evaluate_as_integer(uint64_t &value, bool sign) const
{
	if (!m_expr->evaluate_as_integer(value))
		return false;

	value &= (1ULL << get_type()->get_bits()) - 1;

	// if cast is signed, sign extend.
	unsigned bit = m_expr->get_type()->get_bits() - 1;
	if (sign && ((value >> bit) & 1) != 0)
		value |= -1ULL << (bit + 1);

	return true;
}

bool
cast_expression::evaluate_as_float(double &) const
{
	return false;
}

void
cast_expression::replace_sub_expr(size_t index, expression *expr)
{
	if (index == 0)
		m_expr = expr;
	else
		assert(0 && "Invalid expression index.");
}
