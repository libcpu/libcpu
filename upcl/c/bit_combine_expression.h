#ifndef __upcl_c_bit_combine_expression_h
#define __upcl_c_bit_combine_expression_h

#include <cstdarg>

#include "c/expression.h"
#include "c/unary_expression.h"
#include "c/binary_expression.h"

namespace upcl { namespace c {

class bit_combine_expression : public expression {
	expression_vector m_exprs;
	type *m_type;

protected:
	friend class expression;

	bit_combine_expression(expression_vector const &exprs =
			expression_vector());
	void init(expression *expr1, ...);
	void init(expression *expr1, va_list ap);
	void init(expression_vector const &exprs);

public:
	virtual bool is_constant() const;
	virtual expression *sub_expr(size_t index) const;

	virtual type *get_type() const;

	virtual bool evaluate_as_integer(uint64_t &, bool = false) const;
	virtual bool evaluate_as_float(double &) const;

	virtual expression *simplify(bool = false) const;
	void replace_sub_expr(size_t index, expression *expr);

	virtual expression *expand() const;

public:
	expression *simplify_unary(unary_expression::operation
			const &operation) const;

	expression *simplify_binary(binary_expression::operation
			const &operation, expression *other) const;

private:
	size_t add_exprs(expression_vector const &exprs);

protected:
	friend class bit_slice_expression;

	expression *truncate(size_t nbits) const;

private: // binary operations
	// shift left/right
	expression *shift_by(bool left, expression *value) const;

	void shift_left(expression *other, size_t nbits,
		expression_vector &result) const;
	void shift_right(expression *other, size_t nbits,
		expression_vector &result) const;

	// and/or/xor
	expression *and_with(expression *other) const;
	expression *and_expression(expression *other) const;
	expression *and_bit_combine(expression *other) const;

	expression *or_with(expression *other) const;
	expression *or_expression(expression *other) const;
	expression *or_bit_combine(expression *other) const;

	expression *xor_with(expression *other) const;
	expression *xor_expression(expression *other) const;
	expression *xor_bit_combine(expression *other) const;

private:
	static void make_same_layout(bit_combine_expression *&expr1,
		bit_combine_expression *&expr2, size_t result_size);
};

} }

#endif  // !__upcl_c_bit_combine_expression_h
