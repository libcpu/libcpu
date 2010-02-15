#include "c/store_statement.h"

using upcl::c::store_statement;
using upcl::c::expression;

store_statement::store_statement(expression *target, expression *value)
	: statement(STORE), m_target(target), m_value(value)
{
}

expression *
store_statement::get_target() const
{
	return m_target;
}

expression *
store_statement::get_value() const
{
	return m_value;
}
