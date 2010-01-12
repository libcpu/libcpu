#include <cassert>

#include "c/decoder_operand_def.h"

using namespace upcl;
using namespace upcl::c;

decoder_operand_def::decoder_operand_def(unsigned flags,
		std::string const &name, c::type *type)
	: m_flags(flags), m_name(name), m_type(type)
{
}

decoder_operand_def::decoder_operand_def(std::string const &name,
		c::type *type)
	: m_flags(0), m_name(name), m_type(type)
{
}

decoder_operand_def::~decoder_operand_def()
{
}

c::type *
decoder_operand_def::get_type() const
{
	return m_type;
}

std::string
decoder_operand_def::get_name() const
{
	return m_name;
}
