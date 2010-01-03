#include <cassert>
#include <cstdlib>
#include <cstring>
#include <cerrno>

#include "ast/parse.h"
#include "ast/dumper.h"
#include "sema/simple_expr_evaluator.h"
#include "c/sema_analyzer.h"

using namespace upcl;

namespace upcl { namespace c {

class identifier {
	identifier *m_base;
	std::string m_name;

public:
	identifier(identifier *base, std::string const &name);
	identifier(std::string const &name);

	virtual bool is_equal(identifier const *identifier) const;

	virtual std::string const &get_name() const;

	virtual identifier *get_base();
	virtual identifier const *get_base() const;
    
protected:
	virtual ~identifier() {}
};

identifier::identifier(identifier *base, std::string const &name)
	: m_base(base), m_name(name)
{
}

identifier::identifier(std::string const &name)
	: m_base(0), m_name(name)
{
}

bool identifier::is_equal(identifier const *identifier) const
{
	if (this == 0 || identifier == 0)
		return (this == identifier);

	if (identifier == this)
		return true;

	if (identifier->get_base()->is_equal(get_base()))
		return (identifier->get_name() == get_name());

	return false;
}

std::string const &identifier::get_name() const
{ return m_name; }

identifier *identifier::get_base()
{ return m_base; }

identifier const *identifier::get_base() const
{ return m_base; }

} }

int main(int ac, char **av)
{
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
		ast::dumper().dump(root);
    c::sema_analyzer().parse(root);

		delete root;
	} else {
		fprintf(stderr, "error parsing.\n");
	}

	return 0;
}
