#ifndef __upcl_c_assign_expression_h
#define __upcl_c_assign_expression_h

#include "c/expression.h"

namespace upcl { namespace c {

class assign_expression : public expression {
	expression *m_lhs;
	expression *m_rhs;
	type *m_type;

public:
	assign_expression(expression *lhs, expression *rhs, type *ty = 0);

	virtual bool is_constant() const;
	virtual expression *sub_expr(size_t index) const;

	virtual type *get_type() const;

	virtual bool evaluate_as_integer(uint64_t &, bool = false) const;
	virtual bool evaluate_as_float(double &) const;

	virtual expression *simplify(bool = false) const;
};

} }

#endif  // !__upcl_c_assign_expression_h
