#include "c/assign_expression.h"

using namespace upcl;
using namespace upcl::c;

assign_expression::assign_expression(expression *lhs, expression *rhs,
		type *ty)
	: expression(ASSIGN), m_lhs(lhs), m_rhs(rhs), m_type(ty)
{
}

expression *
assign_expression::simplify(bool) const
{
	expression *lhs = m_lhs->simplify();
	expression *rhs = m_rhs->simplify();

	if (lhs->is_constant())
		return 0;

	if (m_lhs != lhs || m_rhs != rhs)
		return new assign_expression(lhs, rhs, get_type());
	else
		return const_cast<assign_expression*>(this);
}

type *
assign_expression::get_type() const
{
	if (m_type != 0)
		return m_type;
	else
		return m_lhs->get_type();
}

bool
assign_expression::is_constant() const
{
	return false;
}


expression *
assign_expression::sub_expr(size_t index) const
{
	if (index == 0)
		return m_lhs;
	else if (index == 1)
		return m_rhs;
	else
		return 0;
}

bool
assign_expression::evaluate_as_integer(uint64_t &, bool) const
{
	return false;
}

bool
assign_expression::evaluate_as_float(double &) const
{
	return false;
}
