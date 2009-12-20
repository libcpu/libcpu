#ifndef __upcl_c_float_expression_h
#define __upcl_c_float_expression_h

#include <cmath>
#include "c/expression.h"

namespace upcl { namespace c {

class float_expression : public expression {
	double m_value;
	type *m_type;

public:
	float_expression(double value, type *ty)
		: expression(FLOAT), m_value(value), m_type(ty)
	{ }

public:
	virtual ~float_expression()
	{ }

	virtual expression *sub_expr(size_t) const
	{ return 0; }

	virtual bool is_constant() const
	{ return true; }

	virtual double get_value() const
	{
		if (m_type->get_bits() < 64)
			return static_cast<float>(m_value);
		else
			return m_value;
	}

	virtual type *get_type() const
	{ return m_type; }

	virtual bool evaluate_as_integer(uint64_t &result, bool = false) const
	{ result = get_value(); return true; }
	virtual bool evaluate_as_float(double &result) const
	{ result = get_value(); return true; }

	virtual bool is_zero() const
	{ return (std::fabs(get_value()) == 0.0); }
};

} }

#endif  // !__upcl_c_float_expression_h
