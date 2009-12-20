#ifndef __upcl_c_unary_expression_h
#define __upcl_c_unary_expression_h

#include "c/expression.h"

namespace upcl { namespace c {

class unary_expression : public expression {
public:
	enum operation {
		NEG, COM, NOT
	};

private:
	operation   m_op;
	expression *m_expr;
	type       *m_type;

public:
	unary_expression(operation const &op, expression *expr);

public:
	virtual ~unary_expression();

public:
	virtual type *get_type() const;

public:
	virtual bool evaluate_as_integer(uint64_t &result, bool sign = false) const;
	virtual bool evaluate_as_float(double &result) const;

	virtual bool is_equal(expression const *expr) const;
	virtual bool is_constant() const;

	virtual expression *sub_expr(size_t index) const;

	virtual expression *simplify(bool sign = false) const;

	virtual void replace_sub_expr(size_t index, expression *expr);

public:
	inline operation get_operation() const
	{ return m_op; }
};

} }

#endif  // !__upcl_c_unary_expression_h
