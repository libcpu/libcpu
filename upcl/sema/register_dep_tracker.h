#ifndef __upcl_sema_register_dep_tracker_h
#define __upcl_sema_register_dep_tracker_h

#include <map>
#include <string>

#include "sema/register_info.h"

namespace upcl { namespace sema {

// 
// this class creates the dependency tree of registers
// as parsed from the AST, the register_file_builder uses
// this class to create the final definitions.
//
class register_dep_tracker {
private:
	typedef std::map <std::string, register_info *> name_info_map;

private:
	name_info_map        m_regs;
	register_info_vector m_vregs;
	size_t               m_stubs;
	size_t               m_deps;
	size_t               m_deps_on;
	size_t               m_deps_by;

public:
	register_dep_tracker();

public:
	register_info *get(std::string const &name) const;

public:
	register_info *ref(std::string const &name);

public:
	register_info *add(std::string const &name, ast::type const *type,
			unsigned flags = 0);
	register_info *add(std::string const &name, ast::type const *type,
			ast::expression const *expr, unsigned flags = 0);

	register_info *add(std::string const &name, ast::type const *type,
			ast::register_splitter const *splitter, unsigned flags = 0);

	register_info *add(register_info *super, std::string const &name,
			ast::type const *type, unsigned flags = 0);

	register_info *add(register_info *super, std::string const &name,
			ast::type const *type, ast::register_splitter const *splitter,
			unsigned flags = 0);

public:
	// get regs with no deps.
	void get_indep_regs(register_info_vector &regs) const;

public:
	void make_deps_by(register_info *master, register_info *dep);
	void make_deps_by(string_vector const &masters,
		register_info *dep);

	void remove_deps_on(register_info *master, register_info *dep);

	// resolve dependencies between parent and subs, other deps
	// are kept intact.
	void resolve_subs();

public:
	inline size_t get_stub_count() const
	{ return m_stubs; }
	size_t get_top_regs_count() const;
	size_t get_pseudo_regs_count() const;
	size_t get_indep_regs_count() const;
	inline size_t get_all_regs_count() const
	{ return m_vregs.size(); }
	inline size_t get_deps_count() const
	{ return m_deps; }

public:
	void dump();
	void dump_top();

private:
	void dump_reg(register_info const *ri, register_info_set &seen,
			size_t indent);

	template<typename T>
	void dump_reg_ctr(register_info const *ri, std::string const &title,
			register_info_set &seen, T const &set, size_t indent);
};

} }

#endif  // !__upcl_sema_register_dep_tracker_h
