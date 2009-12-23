#ifndef __upcl_c_hardwired_sub_register_def_h
#define __upcl_c_hardwired_sub_register_def_h

#include "c/sub_register_def.h"

namespace upcl { namespace c {

class hardwired_sub_register_def : public sub_register_def {
public:
	hardwired_sub_register_def(register_def *master, std::string const &name, c::type *ty,
			size_t first_bit, size_t bit_count, c::expression *expression)
		: sub_register_def(master, name, ty, first_bit, bit_count, false)
	{
		register_def::set_expression(expression);
	}
};

} }

#endif  // !__upcl_c_hardwired_sub_register_def_h
