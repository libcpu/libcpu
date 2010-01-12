#include "c/sub_register_def.h"

using namespace upcl;
using namespace upcl::c;

sub_register_def::sub_register_def(register_def *master,
		std::string const &name, type *ty, bool bidi)
	: register_def(name, ty), m_master(master), m_bidi(bidi),
	m_first_bit(0), m_bit_count(0)
{
	if (!m_master->add_sub(this))
		abort();
}

sub_register_def::sub_register_def(register_def *master,
		std::string const &name, type *ty, expression *first_bit,
		expression *bit_count, bool bidi)
	: register_def(name, ty), m_master(master), m_bidi(bidi),
	m_first_bit(first_bit), m_bit_count(bit_count)
{
	if (!m_master->add_sub(this))
		abort();
}

sub_register_def::sub_register_def(register_def *master,
		std::string const &name, type *ty, size_t first_bit,
		size_t bit_count, bool bidi)
	: register_def(name, ty), m_master(master), m_bidi(bidi),
	m_first_bit(expression::fromInteger(first_bit, sizeof(first_bit)*8)),
	m_bit_count(expression::fromInteger(bit_count, sizeof(bit_count)*8))
{
	if (!m_master->add_sub(this))
		abort();
}

sub_register_def::~sub_register_def()
{
}

bool
sub_register_def::is_bound() const
{
	return register_def::is_bound();
}

bool
sub_register_def::is_virtual() const
{
	return true;
}

bool
sub_register_def::is_sub_register() const
{
	return true;
}

bool
sub_register_def::is_full_alias() const
{
	return (m_first_bit == 0 && m_bit_count == 0);
}

bool
sub_register_def::is_bidi() const
{
	return (m_bidi);
}

register_def *
sub_register_def::get_master_register() const
{
	return m_master;
}

void
sub_register_def::set_aliasing_range(size_t first_bit, size_t bit_count)
{
	set_aliasing_range(expression::fromInteger(first_bit, sizeof(size_t)*8),
			expression::fromInteger(bit_count, sizeof(size_t)*8));
}

void
sub_register_def::set_aliasing_range(expression *first_bit,
		expression *bit_count)
{
	m_first_bit = first_bit;
	m_bit_count = bit_count;
}

expression *
sub_register_def::get_first_bit() const
{
	return m_first_bit;
}

expression *
sub_register_def::get_bit_count() const
{
	return m_bit_count;
}

bool
sub_register_def::is_bound_to_special() const
{
	return (m_special_bind != NO_SPECIAL_REGISTER);
}

bool
sub_register_def::is_bound_to_register() const
{
	return (m_bind != 0);
}
