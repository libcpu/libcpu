#ifndef __upcl_c_assign_statement_h
#define __upcl_c_assign_statement_h

#include "c/statement.h"

namespace upcl { namespace c {

// represents a C assignment
class assign_statement : public statement {
public:
	assign_statement();
};

} }

#endif  // !__upcl_c_assign_statement_h
