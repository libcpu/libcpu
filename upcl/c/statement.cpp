#include "c/statement.h"

using upcl::c::statement;

statement::statement(type_id const &type)
	: m_type_id(type)
{
}

statement::type_id
statement::get_type_id() const
{
	return m_type_id;
}
