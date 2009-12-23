#include "sema/register_dep_tracker.h"
#include "ast/dumper.h"

#include <cassert>

using namespace upcl;
using namespace upcl::sema;

register_dep_tracker::register_dep_tracker()
{
	m_stubs = 0;
	m_deps = 0;
}

register_info *
register_dep_tracker::ref(std::string const &name)
{
	register_info *info;

	info = get(name);
	if (info == 0) {
		info = new register_info;
		if (info == 0)
			return 0;

		info->name = name;
		if (name[0] == '$')
			m_stubs++;

		info->type = 0;
		info->flags = register_info::UNRESOLVED_FLAG;
		info->super = 0;
		info->hwexpr = 0;
		info->special_eval = 0;
		info->splitter = 0;
		info->repeat_index = 0;
		info->complex_index = 0;
		info->binding = 0;
		info->bind_copy = 0;
		info->bit_start = 0;
		m_regs[name] = info;
		m_vregs.push_back(info);
	}
	return info;
}

register_info *
register_dep_tracker::add(std::string const &name, ast::type const *type,
		unsigned flags)
{
	register_info *info;

	info = ref(name);
	if (info == 0 || (info->flags & register_info::UNRESOLVED_FLAG) == 0)
		return 0;

	if ((flags & (register_info::EXPLICIT_FLAG |
					register_info::RESERVED_FLAG)) != 0) {
		m_regs.erase(m_regs.find(name));
	}

	info->flags = flags & ~register_info::UNRESOLVED_FLAG;
	info->type = type;

	return info;
}

register_info *
register_dep_tracker::add(std::string const &name, ast::type const *type,
		ast::expression const *expr, unsigned flags)
{
	register_info *info;

	info = ref(name);
	if (info == 0 || (info->flags & register_info::UNRESOLVED_FLAG) == 0)
		return 0;

	if ((flags & (register_info::EXPLICIT_FLAG |
					register_info::RESERVED_FLAG)) != 0) {
		m_regs.erase(m_regs.find(name));
	}

	info->flags = flags & ~register_info::UNRESOLVED_FLAG;
	info->type = type;
	info->hwexpr = expr;

	return info;
}

register_info *
register_dep_tracker::add(std::string const &name, ast::type const *type,
		ast::register_splitter const *splitter, unsigned flags)
{
	register_info *info;

	info = ref(name);
	if (info == 0 || (info->flags & register_info::UNRESOLVED_FLAG) == 0)
		return 0;

	if ((flags & (register_info::EXPLICIT_FLAG |
					register_info::RESERVED_FLAG)) != 0) {
		m_regs.erase(m_regs.find(name));
	}

	info->type = type;
	info->flags = flags & ~register_info::UNRESOLVED_FLAG;
	info->splitter = splitter;

	return info;
}

register_info *
register_dep_tracker::add(register_info *super, std::string const &name,
		ast::type const *type, unsigned flags)
{
	register_info *info;

	info = ref(name);
	if (info == 0 || (info->flags & register_info::UNRESOLVED_FLAG) == 0)
		return 0;

	if ((flags & (register_info::EXPLICIT_FLAG |
					register_info::RESERVED_FLAG)) != 0) {
		m_regs.erase(m_regs.find(name));
	}

	info->type = type;
	info->flags = flags & ~register_info::UNRESOLVED_FLAG;
	info->super = super;

	if (super != 0) {
		make_deps_by(super, info);

		super->subs.push_back(info);
	}

	return info;
}

register_info *
register_dep_tracker::add(register_info *super, std::string const &name,
		ast::type const *type, ast::register_splitter const *splitter,
		unsigned flags)
{
	register_info *info;

	info = ref(name);
	if (info == 0 || (info->flags & register_info::UNRESOLVED_FLAG) == 0)
		return 0;

	if ((flags & (register_info::EXPLICIT_FLAG |
					register_info::RESERVED_FLAG)) != 0) {
		m_regs.erase(m_regs.find(name));
	}

	info->type = type;
	info->flags = flags & ~register_info::UNRESOLVED_FLAG;
	info->super = super;
	info->splitter = splitter;

	if (super != 0) {
		make_deps_by(super, info);

		super->subs.push_back(info);
	}

	return info;
}

register_info *
register_dep_tracker::get(std::string const &name) const
{
	name_info_map::const_iterator i = m_regs.find(name);
	if (i != m_regs.end())
		return i->second;
	else
		return 0;
}

void
register_dep_tracker::get_indep_regs(register_info_vector &regs) const
{
	regs.clear();
	for (register_info_vector::const_iterator i = m_vregs.begin();
			i != m_vregs.end(); i++) {

		if (((*i)->flags & register_info::UNRESOLVED_FLAG) != 0 ||
				(*i)->name[0] == '$' || (*i)->name[0] == '%' ||
				(*i)->name[(*i)->name.length()-1] == '?')
			continue;

		// registers that depends only on pseudo
		// registers are considered independant.
		
		size_t ndeps = (*i)->deps_on.size();
		if (ndeps != 0) {
			for (register_info_set::const_iterator j = (*i)->deps_on.begin();
					j != (*i)->deps_on.end(); j++) {
				if ((*j)->name[0] != '%')
					goto next_reg;
			}
		}

		regs.push_back(*i);

next_reg:
		;
	}
}

void
register_dep_tracker::make_deps_by(register_info *master, register_info *dep)
{
	assert(dep != master);

	dep->deps_on.insert(master);
	master->deps_by.insert(dep);

	m_deps += 2;

	// if the master is part of a bigger register, then this
	// register is also part of it.
	if (master->super != 0)
		make_deps_by(master->super, dep);
}

void
register_dep_tracker::make_deps_by(string_vector const &masters,
		register_info *dep)
{
	for(string_vector::const_iterator i = masters.begin();
			i != masters.end(); i++)
		make_deps_by(ref(*i), dep);
}

void
register_dep_tracker::remove_deps_on(register_info *master, register_info *dep)
{
	register_info_set::iterator i;
	
	i = dep->deps_by.find(master);
	if (i != dep->deps_by.end()) {
		dep->deps_by.erase(i);
		m_deps--;
	}

	i = master->deps_on.begin();
	if (i != master->deps_on.end()) {
		master->deps_on.erase(i);
		m_deps--;
	}

	// if the master is part of a bigger register, then this
	// register may depend also on it, so recurse.
	if (master->super != 0)
		remove_deps_on(master->super, dep);
}

size_t
register_dep_tracker::get_top_regs_count() const
{
	size_t count = 0;
	for(name_info_map::const_iterator i = m_regs.begin();
			i != m_regs.end(); i++) {
		if (i->first[0] == '%' || i->first[0] == '$')
			continue;
		count++;
	}
	return count;
}

size_t
register_dep_tracker::get_pseudo_regs_count() const
{
	size_t count = 0;
	for(name_info_map::const_iterator i = m_regs.begin();
			i != m_regs.end(); i++) {
		if (i->first[0] != '%')
			continue;
		count++;
	}
	return count;
}

size_t
register_dep_tracker::get_indep_regs_count() const
{
	size_t count = 0;
	for (register_info_vector::const_iterator i = m_vregs.begin();
			i != m_vregs.end(); i++) {

		if (((*i)->flags & register_info::UNRESOLVED_FLAG) != 0 ||
				(*i)->name[0] == '$' || (*i)->name[0] == '%' ||
				(*i)->name[(*i)->name.length()-1] == '?')
			continue;

		// registers that depends only on pseudo
		// registers are considered independant.
		
		size_t ndeps = (*i)->deps_on.size();
		if (ndeps != 0) {
			for (register_info_set::const_iterator j = (*i)->deps_on.begin();
					j != (*i)->deps_on.end(); j++) {
				if ((*j)->name[0] != '%')
					goto next_reg;
			}
		}

		count++;

next_reg:
		;
	}
	return count;
}

void
register_dep_tracker::dump_reg(register_info const *ri,
		register_info_set &seen, size_t indent)
{
	printf("%*sRegister '%s' ", (int)indent, "", ri->name.c_str());
	if (ri->type != 0)
		printf("[%s] ", ri->type->get_value().c_str());
	indent++;

	if (ri->flags & register_info::SUBREGISTER_FLAG) {
		printf("(Bitfield %zu:%u of register %s)\n", 
				ri->bit_start, 
				atoi(ri->type->get_value().c_str()+2),
				ri->super->name.c_str());
	} else {
		printf("\n");
	}

	size_t v_size = 0;
	for(register_info_vector::const_iterator j = ri->subs.begin();
			j != ri->subs.end(); j++) {
		if ((*j)->type != 0)
			v_size += atoi((*j)->type->get_value().c_str()+2);
	}

	if (!ri->subs.empty() && ri->type != 0 && 
			v_size != (size_t)atoi(ri->type->get_value().c_str()+2))
		printf("%*sVirtual Size: %zu\n", (int)indent, "", v_size);

	if (ri->hwexpr != 0) {
		printf("%*sHardwired Expression: ", (int)indent, "");
		ast::dumper().dump_expression(ri->hwexpr);
		printf("\n");
	} else if (ri->binding != 0) {
		printf("%*sAliasing: %s\n", (int)indent, "",
				ri->binding->name.c_str());
	}

	if (ri->special_eval != 0) {
		printf("%*sSpecial Evaluation: ", (int)indent, "");
		ast::dumper().dump_expression(ri->special_eval);
		printf("\n");
	}

	if (ri->complex_index != 0) {
		printf("%*sComplex Indexing: ", (int)indent, "");
		ast::dumper().dump_expression(ri->complex_index);
		printf(" of register %s\n", ri->super->name.c_str());
	}

	if (ri->bind_copy != 0) {
		printf("%*sBind Copy-on-Write: ", (int)indent, "");
		ast::dumper().dump_literal_qualified_identifier(ri->bind_copy);
		printf("\n");
	}

	dump_reg_ctr(ri, "Subregisters", seen, ri->subs, indent);
	dump_reg_ctr(ri, "Depends On", seen, ri->deps_on, indent);
	dump_reg_ctr(ri, "Dependency Of", seen, ri->deps_by, indent);
}

template<typename T>
void
register_dep_tracker::dump_reg_ctr(register_info const *ri,
	std::string const &title, register_info_set &seen, T const &ctr,
	size_t indent)
{
	bool dep_banner = false;
	for (typename T::const_iterator i = ctr.begin(); i != ctr.end(); i++) {
		if (*i == ri->super)
			continue;
		else {
			if (!dep_banner) {
				printf("%*s%s:\n", (int)indent, "",
						title.c_str());
				indent++, dep_banner = true;
			}

			if (seen.find(*i) != seen.end())
				printf("%*sRegister '%s' (...loop recursion...)\n",
						(int)indent, "", (*i)->name.c_str());
			else {
				seen.insert(*i);
				dump_reg(*i, seen, indent);
			}
		}
	}
}

void
register_dep_tracker::dump()
{
	register_info_set seen;

	for(register_info_vector::const_iterator i = m_vregs.begin();
			i != m_vregs.end(); i++) {
		dump_reg(*i, seen, 0);
	}
}

void
register_dep_tracker::dump_top()
{
	for(name_info_map::const_iterator i = m_regs.begin();
			i != m_regs.end(); i++) {
		if (i->first[0] == '%' || i->first[0] == '$')
			continue;

		printf("Top Register: %s\n", i->first.c_str());
	}
}

void
register_dep_tracker::resolve_subs()
{
	for (register_info_vector::iterator i = m_vregs.begin();
			i != m_vregs.end(); i++) {

		if ((*i)->name[0] == '%')
			continue;

		for (register_info_set::iterator j = (*i)->deps_on.begin();
				j != (*i)->deps_on.end();) {

			if ((*j)->name[0] != '%') {
				for (register_info_vector::iterator k = (*i)->subs.begin();
						k != (*i)->subs.end(); k++) {

					if ((*j)->name == (*k)->name) {
						remove_deps_on(*i, *j);
						j = (*i)->deps_on.begin();
						goto again;
					}

				}
			}

			j++;

again:
			;
		}
	}
}
