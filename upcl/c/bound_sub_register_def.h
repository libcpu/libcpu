#ifndef __upcl_c_bound_sub_register_def_h
#define __upcl_c_bound_sub_register_def_h

#include "c/sub_register_def.h"

namespace upcl { namespace c {

class bound_sub_register_def : public sub_register_def {
public:
	bound_sub_register_def(register_def *master, std::string const &name, c::type *ty,
			size_t first_bit, size_t bit_count, register_def *binding,
			bool bidi)
		: sub_register_def(master, name, ty, first_bit, bit_count, bidi)
	{
		assert(binding != master && "You cannot bind and alias the same register!");
		register_def::set_binding(binding);
	}
	bound_sub_register_def(register_def *master, std::string const &name, c::type *ty,
			size_t first_bit, size_t bit_count, special_register const &binding,
			bool bidi)
		: sub_register_def(master, name, ty, first_bit, bit_count, bidi)
	{
		register_def::set_binding(binding);
	}
};

} }

#endif  // !__upcl_c_bound_sub_register_def_h
