#ifndef __upcl_sema_expr_convert_h
#define __upcl_sema_expr_convert_h

#include "ast/ast.h"
#include "c/expression.h"

#include <map>

namespace upcl { namespace sema {

struct expr_convert_lookup {
	virtual c::expression *expr_convert_lookup_identifier(std::string const &name) const = 0;
	virtual c::expression *expr_convert_lookup_identifier(std::string const &base,
			std::string const &name) const = 0;
	virtual c::type *expr_convert_get_default_word_type() const = 0;
};

class expr_convert { 
private:
	expr_convert_lookup *m_lookup;

public:
	expr_convert(expr_convert_lookup *lookup = 0);

public:
	c::expression *convert(ast::expression const *expr);

private:
	c::expression *convert_literal(ast::literal_expression const *expr);
	c::expression *convert_cast(ast::cast_expression const *expr);
	c::expression *convert_unary(ast::unary_expression const *expr);
	c::expression *convert_binary(ast::binary_expression const *expr);

private:
	c::expression *convert_number(ast::number const *number);
	c::expression *convert_identifier(ast::identifier const *ident);
	c::expression *convert_qualified_identifier(ast::qualified_identifier const *ident);
	c::expression *convert_call(ast::call_expression const *expr);
	c::expression *convert_bit_combine(ast::bit_combine_expression const *expr);
public:
	c::expression *convert_memory(ast::memory_expression const *expr,
			c::type *hint_type = 0);
private:
	c::expression *convert_of_trap(ast::OFTRAP_expression const *expr);
	c::expression *convert_cc(ast::CC_expression const *expr);
};

} }

#endif  // !__upcl_sema_expr_convert_h
