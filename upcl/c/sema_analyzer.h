#ifndef __upcl_c_sema_analyzer_h
#define __upcl_c_sema_analyzer_h

#include "ast/ast.h"
#include "c/type.h"
#include "c/expression.h"
#include "sema/register_dep_tracker.h"
#include "sema/register_info.h"

namespace upcl { namespace c {

class sema_analyzer {

	// architecture

	std::string m_arch_name;

	std::string m_arch_full_name;
	uint64_t m_arch_tags[10];

	sema::register_dep_tracker m_dpt;

public:
	sema_analyzer();

	bool parse(ast::token_list const *root);

public:

	bool process_architecture(ast::architecture const *arch);

private:

	bool process_register_file(ast::register_file const *reg_file);
	bool process_register_group_dep(ast::register_group const *group);
	bool process_register_group(ast::register_group const *group);
	bool process_register_dep(ast::register_declaration const *rd,
			std::string const &group_name);

private:

	bool process_register_splitter_dep(sema::register_info *ri);
	bool process_bound_value_dep(sema::register_info *ri, size_t &offset,
			ast::type const *type, ast::bound_value const *bv, bool explic);
	bool process_typed_bound_value_dep(sema::register_info *ri, size_t &offset,
			size_t max_size, ast::type const *type,
			ast::typed_bound_value const *bv, bool explic);

private:
	static inline std::string destringify(std::string const &s)
	{ return s.substr(1, s.length()-2); }
};

} }

#endif  // !__upcl_c_sema_analyzer_h
