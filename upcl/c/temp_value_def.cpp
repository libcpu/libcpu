#include <cassert>

#include "c/temp_value_def.h"

using namespace upcl;
using namespace upcl::c;

temp_value_def::temp_value_def(std::string const &name, c::type *type)
	: m_name(name), m_type(type)
{
}

temp_value_def::~temp_value_def()
{
}

c::type *
temp_value_def::get_type() const
{
	return m_type;
}

std::string
temp_value_def::get_name() const
{
	return m_name;
}
