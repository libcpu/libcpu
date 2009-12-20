#ifndef __upcl_c_signed_expression_h
#define __upcl_c_signed_expression_h

#include "c/expression.h"

namespace upcl { namespace c {

class signed_expression : public expression {
	expression *m_expr;

public:
	signed_expression(expression *expr);

	virtual bool is_constant() const;
	virtual expression *sub_expr(size_t index) const;

	virtual type *get_type() const;

	virtual bool evaluate_as_integer(uint64_t &, bool = false) const;
	virtual bool evaluate_as_float(double &) const;

	virtual expression *simplify(bool = false) const;
	virtual void replace_sub_expr(size_t index, expression *expr);
};

} }

#endif  // !__upcl_c_signed_expression_h
