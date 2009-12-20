#ifndef __upcl_c_binary_expresson_h
#define __upcl_c_binary_expresson_h

#include "c/expression.h"

namespace upcl { namespace c {

class binary_expression : public expression {
public:
	enum operation {
		ADD, SUB, MUL, DIV, REM,
		SHL, SHLC, SHR, SHRC,
		ROL, ROLC, ROR, RORC,
		AND, OR, XOR,
		EQ, NE, LT, LE, GT, GE
	};

private:
	operation   m_op;
	type       *m_type;
	expression *m_expr1;
	expression *m_expr2;

public:
	binary_expression(operation const &op, expression *expr1,
			expression *expr2);

	virtual ~binary_expression();

public:
	virtual type *get_type() const;

public:
	virtual bool evaluate_as_integer(uint64_t &result, bool sign = false) const;
	virtual bool evaluate_as_float(double &result) const;

	virtual bool is_equal(expression const *expr) const;
	virtual bool is_constant() const;

	virtual expression *sub_expr(size_t index) const;

	inline operation get_operation() const
	{ return m_op; }

	virtual expression *simplify(bool sign = false) const;
	virtual void replace_sub_expr(size_t index, expression *expr);
};

} }

#endif  // !__upcl_c_binary_expresson_h
