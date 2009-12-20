#ifndef __upcl_c_sub_register_def_h
#define __upcl_c_sub_register_def_h

#include "c/register_def.h"

namespace upcl { namespace c {

class sub_register_def : public register_def {

	register_def *m_master;
	bool m_bidi;
	expression *m_first_bit;
	expression *m_bit_count;

public:
 	sub_register_def(register_def *master, std::string const &name,
			type *ty, bool bidi = false)
		: register_def(name, ty), m_master(master), m_bidi(bidi),
		m_first_bit(0), m_bit_count(0)
 	{
 		m_master->add_sub(this);
 	}
 
	sub_register_def(register_def *master, std::string const &name,
			type *ty, expression *first_bit = 0,
			expression *bit_count = 0, bool bidi = false)
		: register_def(name, ty), m_master(master), m_bidi(bidi),
		m_first_bit(first_bit), m_bit_count(bit_count)
	{
		m_master->add_sub(this);
	}

	sub_register_def(register_def *master, std::string const &name,
			type *ty, size_t first_bit, size_t bit_count,
			bool bidi = false)
		: register_def(name, ty), m_master(master), m_bidi(bidi),
		m_first_bit(expression::fromInteger(first_bit, sizeof(first_bit)*8)),
		m_bit_count(expression::fromInteger(bit_count, sizeof(bit_count)*8))
	{
		m_master->add_sub(this);
	}

	virtual bool is_virtual() const;
	virtual bool is_bound() const;
	virtual bool is_bound_to_special() const;
	virtual bool is_bound_to_register() const;
	virtual bool is_full_alias() const;
	virtual bool is_bidi() const;
	virtual bool is_sub_register() const;

protected:
	virtual void set_aliasing_range(size_t first_bit, size_t bit_count);
	virtual void set_aliasing_range(expression *first_bit,
			expression *bit_count);

public:
	virtual expression *get_first_bit() const;
	virtual expression *get_bit_count() const;

	virtual register_def *get_master_register() const;
};

} }

#endif  // !__upcl_c_sub_register_def_h
