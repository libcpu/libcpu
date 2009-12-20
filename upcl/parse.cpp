#include <cstdlib>

#include "ast.h"

using namespace upcl;

#include "parse.h"

#include "parser.hpp"

extern int yylex();
extern int yyparse();

extern "C" int yywrap() { return 1; }

#ifdef YYDEBUG
extern int yydebug;
#endif

void yyerror(char const *s)
{
	fprintf(stderr, "error: %s\n", s);
	exit(-1);
}


ast::token_list *g_root = 0;
extern FILE *yyin;

ast::token_list *
ast::parse(FILE *fp)
{
	if (fp == 0)
		return 0;

#ifdef YYDEBUG
	yydebug = 1;
#endif

	yyin = fp;

	int rc = yyparse();

	yyin = 0;
	
	ast::token_list *root = rc ? 0 : g_root;

	g_root = 0;

	return root;
}
