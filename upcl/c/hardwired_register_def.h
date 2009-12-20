#ifndef __upcl_c_hardwired_register_def_h
#define __upcl_c_hardwired_register_def_h

#include "c/register_def.h"

namespace upcl { namespace c {

class hardwired_register_def : public register_def {

public:
	hardwired_register_def(std::string const &name, c::expression *expression,
			c::type *type)
		: register_def(REGISTER_FLAG_HARDWIRED, name, type)
	{ register_def::set_expression(expression); }
};

} }

#endif  // !__upcl_c_hardwired_register_def_h
