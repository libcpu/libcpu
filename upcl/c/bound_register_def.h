#ifndef __upcl_c_bound_register_def_h
#define __upcl_c_bound_register_def_h

#include "c/bound_register_def.h"

namespace upcl { namespace c {

class bound_register_def : public register_def {
public:
	bound_register_def(std::string const &name, c::type *ty,
			special_register special)
		: register_def(0, name, ty)
	{
		register_def::set_binding(special);
	}
};

} }

#endif  // !__upcl_c_bound_register_def_h
