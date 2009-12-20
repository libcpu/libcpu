#ifndef __upcl_c_integer_expression_h
#define __upcl_c_integer_expression_h

#include "c/expression.h"
#include "c/type.h"

namespace upcl { namespace c {

class integer_expression : public expression {
	uint64_t m_value;
	type *m_type;

public:
	integer_expression(uint64_t value, type *ty)
		: expression(INTEGER), m_value(value), m_type(ty)
	{ }

public:
	virtual ~integer_expression()
	{ }

	virtual expression *sub_expr(size_t) const
	{ return 0; }

	virtual bool is_constant() const
	{ return true; }

	inline uint64_t get_value(bool sign) const
	{ 
		if (m_type->get_bits() >= 64)
			return m_value;
		
		unsigned bits = m_type->get_bits();
		uint64_t value = m_value;

		value &= (1ULL << bits) - 1;

		// if signed, sign extend.
		if (sign && ((value >> bits) & 1) != 0)
			value |= -1ULL << bits;

		return value;
	}

	virtual type *get_type() const
	{ return m_type; }

	virtual bool evaluate_as_integer(uint64_t &result, bool sign = false) const
	{ result = get_value(sign); return true; }
	virtual bool evaluate_as_float(double &result) const
	{ result = get_value(false); return true; }

	virtual bool is_zero() const
	{ return (get_value(false) == 0); }

protected:
	virtual void replace_type(type *ty)
	{ m_type = ty; }
};

} }

#endif  // !__upcl_c_integer_expression_h
