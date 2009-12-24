#ifndef __upcl_c_register_set_h
#define __upcl_c_register_set_h

#include "c/register_def.h"

namespace upcl { namespace c {

class register_set;
typedef std::vector<register_set *> register_set_vector;

class register_set {
	register_def_vector m_regs;
	std::string         m_name;

public:
	register_set(std::string const &name, size_t count)
		: m_name(name)
	{ m_regs.resize(count); }

	inline bool set(size_t index, register_def *rdef)
	{
		if (index >= m_regs.size())
			return false;

		if (rdef->get_register_set() != 0 &&
				rdef->get_register_set() != this)
			return false;

		register_def *old = m_regs[index];
		if (old == rdef)
			return true;

		if (old != 0)
			old->set_register_set(0);

		rdef->set_register_set(this);
		m_regs[index] = rdef;

		return true;
	}
	
	inline register_def *get(size_t index) const
	{ return (index < m_regs.size() ? m_regs[index] : 0); }

	inline size_t count() const
	{ return m_regs.size(); }
};

} }

#endif  // !__upcl_c_register_set_h
