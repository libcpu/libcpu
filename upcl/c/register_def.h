#ifndef __upcl_c_register_def_h
#define __upcl_c_register_def_h

#include <string>
#include <vector>

#include "c/type.h"
#include "c/expression.h"

namespace upcl { namespace c {

class sub_register_def;
typedef std::vector <sub_register_def *> sub_register_vector;

enum special_register {
  NO_SPECIAL_REGISTER = 0,

  SPECIAL_REGISTER_PC,
  SPECIAL_REGISTER_PSR,

  SPECIAL_REGISTER_C,
  SPECIAL_REGISTER_N,
  SPECIAL_REGISTER_P,
  SPECIAL_REGISTER_V,
  SPECIAL_REGISTER_Z
};

enum {
	REGISTER_FLAG_HARDWIRED = 1
};

class register_def {

	unsigned m_flags;
	std::string m_name;
	type *m_type;
	special_register m_special_bind;
	register_def *m_bind;
	expression *m_expr;
	sub_register_vector m_subs;

protected:
	register_def(unsigned flags, std::string const &name, c::type *type)
		: m_flags(flags), m_name(name), m_type(type),
		m_special_bind(NO_SPECIAL_REGISTER), m_bind(0), m_expr(0)
	{ }

public:
	register_def(std::string const &name, c::type *type)
		: m_flags(0), m_name(name), m_type(type),
		m_special_bind(NO_SPECIAL_REGISTER), m_bind(0), m_expr(0)
	{ }

protected:
	friend class sub_register_def;
	virtual void add_sub(sub_register_def *alias);

	virtual bool is_bound() const;

protected:
	virtual void set_expression(c::expression *expression);
	virtual void set_binding(register_def *binding);
	virtual void set_binding(special_register const &special);

public:
	virtual bool is_virtual() const;
	virtual bool is_hardwired() const;

	virtual c::type *get_type() const;

	virtual bool is_sub_register() const;

	virtual register_def *get_bound_register() const;
	virtual special_register get_bound_special_register() const;

	virtual std::string get_name() const;

	inline sub_register_vector const &get_sub_register_vector() const
	{ return m_subs; }
};

} }

#endif  // !__upcl_c_register_def_h
