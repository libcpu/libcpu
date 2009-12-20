#ifndef __upcl_c_register_expression_h
#define __upcl_c_register_expression_h

#include "c/expression.h"
#include "c/register_def.h"

namespace upcl { namespace c {


class register_expression : public expression {
	register_def *m_reg;
public:
	register_expression(register_def *reg);

public:
	virtual bool is_constant() const;

	virtual expression *sub_expr(size_t) const;

	virtual type *get_type() const;

	virtual bool evaluate_as_integer(uint64_t &, bool = false) const;
	virtual bool evaluate_as_float(double &) const;

	inline register_def *get_register() const
	{ return m_reg; }

	virtual bool is_equal(expression const *expr) const;
};

} }


#endif  // !__upcl_c_register_expression_h
