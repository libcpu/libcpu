#ifndef __c_simple_const_expr_h
#define __c_simple_const_expr_h

#include "ast.h"
#include <map>

namespace upcl { namespace c {

class simple_const_expr {
private:
	std::map<std::string, uint64_t> m_local_vars;
	std::map<std::string, size_t> m_local_usage;

public:
	inline void set_var(std::string const &varname, uint64_t v)
	{ m_local_vars[varname] = v; }

	inline bool is_used(std::string const &varname) const
	{ 
		std::map<std::string, size_t>::const_iterator i =
			m_local_usage.find(varname);
		if (i != m_local_usage.end())
			return (i->second != 0);
		else
			return false;
	}

public:
	bool eval(ast::expression const *expr, uint64_t &value);

private:
	bool eval(ast::expression const *expr, uint64_t &value, bool sign);
	bool eval_literal(ast::literal_expression const *expr, uint64_t &value);
	bool eval_unary(ast::unary_expression const *expr, uint64_t &value);
	bool eval_binary(ast::binary_expression const *expr, uint64_t &value,
			bool sign);

private:
	bool eval_number(ast::number const *number, uint64_t &value);
	bool eval_identifier(ast::identifier const *ident, uint64_t &value);
};

} }

#endif  // !__c_simple_const_expr_h
