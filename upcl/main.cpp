#include <cassert>
#include <cstdlib>
#include <cstring>
#include <cerrno>

#include "ast/parse.h"
#include "ast/dumper.h"
#include "sema/simple_expr_evaluator.h"
#include "c/sema_analyzer.h"

using namespace upcl;

#ifdef YYDEBUG
extern int yydebug;
#endif

int main(int ac, char **av)
{
#ifdef YYDEBUG
	yydebug = 1;
#endif

	if (ac < 2) {
		fprintf(stderr, "usage: %s filename\n", *av);
		return -1;
	}

	FILE *fp = fopen(av[1], "rt");
	if (fp == 0) {
		fprintf(stderr, "error opening '%s' for reading: %s\n",
				av[1], strerror(errno));
		return -1;
	}

	ast::token_list const *root = ast::parse(fp);
	
	fclose(fp);

	if (root != 0) {
		//ast::dumper().dump(root);

		if (c::sema_analyzer().parse(root)) {
			printf("Semantic analysis succeded.\n");
		} else {
			printf("Error performing semantic analysis.\n");
		}

		delete root;
	} else {
		fprintf(stderr, "Error parsing.\n");
	}

	return 0;
}
