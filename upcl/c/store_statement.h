#ifndef __upcl_c_store_statement_h
#define __upcl_c_store_statement_h

#include "c/statement.h"

namespace upcl { namespace c {

// represents a register update or memory store.
class store_statement : public statement {
	expression *m_target;
	expression *m_value;

public:
	store_statement(expression *target, expression *value);

	virtual expression *get_target() const;
	virtual expression *get_value() const;
};

} }

#endif  // !__upcl_c_store_statement_h
