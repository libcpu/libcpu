#ifndef __upcl_c_cast_expression_h
#define __upcl_c_cast_expression_h

#include "c/expression.h"

namespace upcl { namespace c {

class cast_expression : public expression {
	type *m_type;
	expression *m_expr;

public:
	cast_expression(type *ty, expression *expr);

	virtual bool is_constant() const;
	virtual expression *sub_expr(size_t index) const;

	virtual type *get_type() const;

	virtual bool evaluate_as_integer(uint64_t &, bool = false) const;
	virtual bool evaluate_as_float(double &) const;

	virtual expression *simplify(bool = false) const;
	void replace_sub_expr(size_t index, expression *expr);
};

} }

#endif  // !__upcl_c_cast_expression_h
