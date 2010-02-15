#ifndef __upcl_c_statement_h
#define __upcl_c_statement_h

#include "c/expression.h"

namespace upcl { namespace c {

class statement;
typedef std::vector <statement *> statement_vector;

class statement {
public:
	enum type_id {
		STORE,

		C_ASSIGN,
	};

	type_id m_type_id;

protected:
	statement(type_id const &type);

public:
	virtual type_id get_type_id() const;
};

} }

#endif  // __upcl_c_statement_h
