#ifndef __upcl_sema_expr_convert_h
#define __upcl_sema_expr_convert_h

#include "ast/ast.h"
#include "c/expression.h"

#include <map>

namespace upcl { namespace sema {

class expr_convert { 
public:
	expr_convert();

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
};

} }

#endif  // !__upcl_sema_expr_convert_h
