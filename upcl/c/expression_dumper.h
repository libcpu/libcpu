#ifndef __upcl_c_expression_dumper_h
#define __upcl_c_expression_dumper_h

#include "c/expression.h"

namespace upcl { namespace c {

void dump_expression(expression const *expr);
void print_expression(expression const *expr);

} }

#endif  // !__upcl_c_expression_dumper_h
