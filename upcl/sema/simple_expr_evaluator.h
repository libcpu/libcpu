#ifndef __upcl_sema_simple_expr_evaluator_h
#define __upcl_sema_simple_expr_evaluator_h

#include "ast/ast.h"

#include <map>

namespace upcl { namespace sema {

//
// this class performs simple expression evaluation,
// it does not supports bit-combine and bit-slice operators
// nor floating points.
// it is used while analyzing the register file in order
// to compute register dependencies in complex expressions.
// 
class simple_expr_evaluator { 
private:
	string_uint_map  m_vars;
	string_count_map m_usage;
	bool             m_collect;
	bool             m_failed;

public:
	simple_expr_evaluator();

	// reset the internal state of the evaluator. 
	// collection of literals is disabled and collection
	// information is cleared.
	void reset();

	// like reset(), but also remove all registered variables.
	void clear();

	// calling this method allows parsing an expression
	// and collect literals seen at the same time; the evaluation
	// function interprets the non-registered literals as zero and
	// collect them, and if the expression is well-formed will return
	// true. calling this method means that has_failed() shall be
	// used to know if the expression has been resolved and thus
	// constant.
	void collect_literals();

	// it is possible to assign values to literals in order
	// to evaluate special expressions, an example is the special
	// symbol (_) in repetitive register indexing expression that
	// represents the iteration of the register definition.
	void set_var(std::string const &varname, uint64_t v);

	// if collect_literals() has been called, this method will
	// return true if the varname specified has been seen in the
	// expression, otherwise only registered variables would be
	// found if seen.
	bool is_used(std::string const &varname) const;

	// this method is used is in conjuction with collect_literals()
	// after evaluating an expression to retreive all the non-registered
	// literals seen in the expression.
	void get_used_literals(std::vector<std::string> &literals) const;

public:
	// evaluates an expression directly from the AST.
	// this function is subject to collect_literals(), in that case
	// this method may return true even if the expression didn't
	// evaluate constantly, see collect_literals() and has_failed().
	bool evaluate(ast::expression const *expr, uint64_t &value);

	// if collect_literals() has been invoked, you shall use this
	// method to know if an evaluation has been resolved constantly
	// validating the contents of the output 'value' parameter in the
	// evaluate() method.
	inline bool has_failed() const
	{ return m_failed; }

private:
	bool eval(ast::expression const *expr, uint64_t &value);
	bool eval(ast::expression const *expr, uint64_t &value, bool sign);
	bool eval_literal(ast::literal_expression const *expr, uint64_t &value);
	bool eval_cast(ast::cast_expression const *expr, uint64_t &value,
			bool sign);
	bool eval_unary(ast::unary_expression const *expr, uint64_t &value,
			bool sign);
	bool eval_binary(ast::binary_expression const *expr, uint64_t &value,
			bool sign);

private:
	bool eval_number(ast::number const *number, uint64_t &value);
	bool eval_identifier(ast::identifier const *ident, uint64_t &value);
};

} }

#endif  // !__upcl_sema_simple_expr_evaluator_h
