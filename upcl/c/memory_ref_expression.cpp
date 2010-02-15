#include <cassert>

#include "c/memory_ref_expression.h"
#include "c/type.h"

using namespace upcl;
using namespace upcl::c;

memory_ref_expression::memory_ref_expression(type *type, expression *location)
	: expression(MEMREF), m_type(type), m_location(location)
{
	assert(location->get_type()->get_type_id() == type::INTEGER &&
			"Index expression in a memory reference shall be of INTEGER type.");
}

//
// MEMORY REF EXPRESSION are *NEVER* simplified,
// only location expression is.
//
expression *
memory_ref_expression::simplify(bool) const
{
	expression *location = m_location->simplify(false);

	if (!location->is_equal(m_location))
		return new memory_ref_expression(m_type, m_location);

	return const_cast<memory_ref_expression*>(this);
}

bool
memory_ref_expression::is_constant() const
{
	return false;
}

type *
memory_ref_expression::get_type() const
{
	return m_type;
}

expression *
memory_ref_expression::sub_expr(size_t index) const
{
	return (index == 0 ? m_location : 0);
}

bool
memory_ref_expression::evaluate_as_integer(uint64_t &, bool) const
{
	return false;
}

bool
memory_ref_expression::evaluate_as_float(double &) const
{
	return false;
}

void
memory_ref_expression::replace_sub_expr(size_t index, expression *expr)
{
	if (index == 0)
		m_location = expr;
	else
		assert(0 && "Invalid expression index.");
}

