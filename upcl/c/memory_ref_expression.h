#ifndef __upcl_c_memory_ref_expression_h
#define __upcl_c_memory_ref_expression_h

#include "c/expression.h"

// XXX maybe, it would be better make SIGNED a flag of "expression".
namespace upcl { namespace c {

class memory_ref_expression : public expression {
	type *m_type;
	expression *m_location;

public:
	memory_ref_expression(type *type, expression *location);

	virtual bool is_constant() const;
	virtual expression *sub_expr(size_t index) const;

	virtual type *get_type() const;

	virtual bool evaluate_as_integer(uint64_t &, bool = false) const;
	virtual bool evaluate_as_float(double &) const;

	virtual expression *simplify(bool = false) const;
	virtual void replace_sub_expr(size_t index, expression *expr);
};

} }

#endif  // !__upcl_c_memory_ref_expression_h
