#ifndef __upcl_c_bit_slice_expression_h
#define __upcl_c_bit_slice_expression_h

#include "c/expression.h"

namespace upcl { namespace c {

class bit_slice_expression : public expression {
	expression *m_expr;
	expression *m_first_bit;
	expression *m_bit_count;
	type *m_type;

public:
	bit_slice_expression(expression *expr, expression *first_bit,
			expression *bit_count);

	virtual bool is_constant() const;
	virtual expression *sub_expr(size_t index) const;

	virtual type *get_type() const;

	virtual bool evaluate_as_integer(uint64_t &, bool = false) const;
	virtual bool evaluate_as_float(double &) const;

	virtual expression *simplify(bool = false) const;

	virtual expression *expand() const;
};

} }

#endif  // !__upcl_c_bit_slice_expression_h
