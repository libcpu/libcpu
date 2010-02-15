#ifndef __upcl_c_temp_value_expression_h
#define __upcl_c_temp_value_expression_h

#include "c/expression.h"
#include "c/temp_value_def.h"

namespace upcl { namespace c {


class temp_value_expression : public expression {
	temp_value_def *m_tmpval;
public:
	temp_value_expression(temp_value_def *tmpval);

public:
	virtual bool is_constant() const;

	virtual expression *sub_expr(size_t) const;

	virtual type *get_type() const;

	virtual bool evaluate_as_integer(uint64_t &, bool = false) const;
	virtual bool evaluate_as_float(double &) const;

	inline temp_value_def *get_operand() const
	{ return m_tmpval; }

	virtual bool is_equal(expression const *expr) const;
};

} }


#endif  // !__upcl_c_temp_value_expression_h
