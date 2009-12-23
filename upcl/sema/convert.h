#ifndef __upcl_sema_convert_h
#define __upcl_sema_convert_h

#include "ast/ast.h"
#include "c/expression.h"
#include "c/type.h"

namespace upcl { namespace sema {

c::type *convert_type(ast::type const *type);
c::expression *convert_expression(ast::expression const *expr);
bool convert_integer(ast::number const *v, uint64_t &value);
bool convert_float(ast::number const *v, double &value);

} }

#endif  // !__upcl_sema_convert_h
