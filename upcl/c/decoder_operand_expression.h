#ifndef __upcl_c_decoder_operand_expression_h
#define __upcl_c_decoder_operand_expression_h

#include "c/expression.h"
#include "c/decoder_operand_def.h"

namespace upcl { namespace c {


class decoder_operand_expression : public expression {
	decoder_operand_def *m_decopr;
public:
	decoder_operand_expression(decoder_operand_def *decopr);

public:
	virtual bool is_constant() const;

	virtual expression *sub_expr(size_t) const;

	virtual type *get_type() const;

	virtual bool evaluate_as_integer(uint64_t &, bool = false) const;
	virtual bool evaluate_as_float(double &) const;

	inline decoder_operand_def *get_operand() const
	{ return m_decopr; }

	virtual bool is_equal(expression const *expr) const;
};

} }


#endif  // !__upcl_c_decoder_operand_expression_h
