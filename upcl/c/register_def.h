#ifndef __upcl_c_register_def_h
#define __upcl_c_register_def_h

#include "types.h"

#include "c/type.h"
#include "c/expression.h"

namespace upcl { namespace c {

class register_def;
class sub_register_def;
typedef std::vector <sub_register_def *> sub_register_vector;
typedef std::set <register_def *> register_def_set;
typedef std::vector <register_def *> register_def_vector;
typedef std::map <std::string, sub_register_def *> named_sub_register_map;

enum special_register {
  NO_SPECIAL_REGISTER = 0,

  SPECIAL_REGISTER_PC,
  SPECIAL_REGISTER_NPC,
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

class register_set;

class register_def {

	unsigned m_flags;
	std::string m_name;
	type *m_type;
	special_register m_special_bind;
	register_def *m_bind;
	expression *m_expr;
	sub_register_vector m_subs;
	named_sub_register_map m_named_subs;
	register_def_set m_uow;
	register_set *m_set;

protected:
	register_def(unsigned flags, std::string const &name, c::type *type);

public:
	register_def(std::string const &name, c::type *type);
	virtual ~register_def();

protected:
	inline void change_flags(unsigned set, unsigned clr = 0)
	{ m_flags = (m_flags & ~clr) | set; }

protected:
	friend class register_set;
	virtual void set_register_set(register_set *set);

public:
	virtual register_set *get_register_set() const;

protected:
	friend class sub_register_def;
	virtual bool add_sub(sub_register_def *alias);

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

	virtual sub_register_def *get_sub_register(std::string const &name);
	virtual expression *get_expression() const;

public:
	virtual bool add_uow(register_def *rdef);
	virtual bool is_uow() const;
};

} }

#endif  // !__upcl_c_register_def_h
