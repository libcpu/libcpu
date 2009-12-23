#ifndef __parse_h
#define __parse_h

#include <cstdio>

#include "ast.h"

namespace upcl { namespace ast {

token_list *parse(FILE *fp);

} }

#endif  // !__parse_h
