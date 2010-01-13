#include "sema/register_file_builder.h"
#include "sema/register_info.h"
#include "sema/convert.h"

#include "c/register_set.h"
#include "c/sub_register_def.h"
#include "c/bound_register_def.h"
#include "c/bound_sub_register_def.h"
#include "c/hardwired_register_def.h"
#include "c/hardwired_sub_register_def.h"

#include <algorithm>
#include <sstream>
extern "C" {
#include "strnatcmp.h"
}

using namespace upcl;
using upcl::sema::register_file_builder;
using upcl::sema::register_dep_tracker;
using upcl::sema::register_info;
using upcl::sema::register_info_vector;

namespace {

// represent a group of registers with same
// name and type.
struct regset {
	std::string          name;
	std::string          type;
	sema::register_info_vector regs;
};

typedef std::vector <regset> regset_vector;

struct register_info_nat_sort {
	inline bool operator()(register_info const *a,
			register_info const *b) const
	{ return (::strnatcmp(a->name.c_str(), b->name.c_str()) < 0); }
};

static std::string
drop_digits(std::string const &x)
{
	size_t n = x.length();

	if (n == 0)
		return std::string();

	while (isdigit(x[--n]))
		;

	return x.substr(0, n + 1);
}

static std::string
inc_name(std::string const &name)
{
	long index = 1;
	std::string base_name(drop_digits(name));
	if (base_name.length() < name.length()) {
		index = atol(name.substr(base_name.length()).c_str());
		index++;
	}
	std::stringstream ss;
	ss << name << index;
	return ss.str();
}

static inline bool is_pseudo_reg(register_info const *reg) {
	return (reg->name[0] == '%' || reg->name[0] == '$' ||
			reg->name[reg->name.length() - 1] == '?');
}

}

register_file_builder::register_file_builder()
{
}

static void
make_regsets(sema::register_info_vector const &regs,
		regset_vector &regsets)
{
	string_set regset_id;

	std::string last;
	std::string last_type;
	sema::register_info_vector regs_in_set;

	for (sema::register_info_vector::const_iterator i = regs.begin();
			i != regs.end(); i++) {

		// ignore pseudo registers
		if (is_pseudo_reg(*i))
			continue;

		std::string x = drop_digits((*i)->name);
		if (last.empty() || x != last ||
				(*i)->type->get_value() != last_type) {

			if (!last.empty()) {
				regset rs;

				while (regset_id.find(last) != regset_id.end())
					last = inc_name(last);

				rs.name = last;
				rs.type = last_type;
				rs.regs = regs_in_set;

				regsets.push_back(rs);
				regset_id.insert(rs.name);
			}

			last = x;
			if ((*i)->type != 0)
				last_type = (*i)->type->get_value();
			else
				last_type.clear();

			regs_in_set.clear();
			regs_in_set.push_back(*i);
		} else {
			regs_in_set.push_back(*i);
		}
	}

	if (!regs_in_set.empty()) {
		regset rs;

		while (regset_id.find(last) != regset_id.end())
			last = inc_name(last);

		rs.name = last;
		rs.type = last_type;
		rs.regs = regs_in_set;

		regsets.push_back(rs);
		regset_id.insert(rs.name);
	}
}

bool
register_file_builder::analyze(register_dep_tracker *rdt)
{
	printf("register_file_builder starts analysis.\n");

	// find all registers with no dependencies.
	register_info_vector regs;

	rdt->get_indep_regs(regs);
	printf("Indeps Registers=%zu\n", regs.size());
	for(register_info_vector::iterator i = regs.begin();
			i != regs.end(); i++) {
		printf("%s  ", (*i)->name.c_str());
	}
	printf("\n");

	// sort registers naturally.
	std::sort(regs.begin(), regs.end(), register_info_nat_sort());

	// group register sets.
	regset_vector regsets;

	make_regsets(regs, regsets);

	// analyze top independent registers
	for(regset_vector::const_iterator i = regsets.begin();
			i != regsets.end(); i++) {

		if (i->regs.size() == 1) {
			if (!analyze_top(i->regs[0])) {
				printf("Failed analyzing: %s\n", i->regs[0]->name.c_str());
				return false;
			}
		} else {
			if (!analyze_top(i->name, i->regs))
				return false;
		}
	}

	// now do the same for dependencies.
	regs.clear();
	rdt->get_dep_regs(regs);
	printf("Deps Registers=%zu\n", regs.size());
	for(register_info_vector::iterator i = regs.begin();
			i != regs.end(); i++) {
		printf("%s  ", (*i)->name.c_str());
	}
	printf("\n");

	return true;
}

bool
register_file_builder::analyze_top(std::string const &rset_name,
		register_info_vector const &riv)
{
	printf("Register set '%s'\n", rset_name.c_str());

	c::register_set *rset = new c::register_set(rset_name, riv.size());
	if (rset == 0)
		return false;

	for (register_info_vector::const_iterator i = riv.begin();
			i != riv.end(); i++) {

		c::register_def *rdef = create_top(*i);
		if (rdef == 0)
			return false;

		rset->set((*i)->repeat_index, rdef);
		m_rdefs.push_back(rdef);
		m_named_rdefs[(*i)->name] = rdef;
	}

	m_rsets.push_back(rset);

	return true;
}

bool
register_file_builder::analyze_top(register_info const *ri)
{
	c::register_def *rdef = create_top(ri);
	if (rdef == 0)
		return false;

	m_rdefs.push_back(rdef);
	m_named_rdefs[ri->name] = rdef;

	return true;
}

c::register_def *
register_file_builder::create_top(register_info const *ri)
{
#if 0
	printf("Register '%s' Type '%s'\n", 
			ri->name.c_str(), ri->type->get_value().c_str());
#endif
	c::type *rtype = convert_type(ri->type);
	if (rtype == 0)
		return 0;

	c::register_def *rdef = 0;

	// hardwired?
	if (ri->hwexpr != 0) {
		c::expression *expr = convert_expression(ri->hwexpr);
		if (expr == 0)
			return 0;

		rdef = new c::hardwired_register_def(ri->name, expr, rtype);
	} else if (ri->binding != 0) {
		c::special_register preg = c::NO_SPECIAL_REGISTER;

		// bound to special?
		if (ri->binding->name[0] == '%') {
			// yep, this can be only: PSR, PC or NPC.
			std::string pseudo_name(ri->binding->name.substr(1));
			if (pseudo_name == "PC")
				preg = c::SPECIAL_REGISTER_PC;
			else if (pseudo_name == "NPC")
				preg = c::SPECIAL_REGISTER_NPC;
			else if (pseudo_name == "PSR")
				preg = c::SPECIAL_REGISTER_PSR;
			else {
				fprintf(stderr, "error: register '%s' binds pseudo register '%s' which is not recognized.\n",
						ri->name.c_str(), pseudo_name.c_str());
				return false;
			}
		} else {
			assert(0 && "Binding to a register is not yet implemented.");
		}

		rdef = new c::bound_register_def(ri->name, rtype, preg);
	} else if (ri->special_eval != 0) {
		// special evaluation?
		fprintf(stderr, "info: special evaluation function in register not yet supported!\n");
	} else {
		// normal register
		rdef = new c::register_def(ri->name, rtype);
	}

	if (rdef == 0) {
		fprintf(stderr, "error: cannot create register '%s'.\n",
				ri->name.c_str());
		return 0;
	}

	// create sub registers.
	for(register_info_vector::const_iterator i = ri->subs.begin();
			i != ri->subs.end(); i++) {

		c::register_def *sub = create_sub(ri, rdef, *i);
		if (sub == 0)
			return 0;
	}

	return rdef;
}

c::register_def *
register_file_builder::create_sub(register_info const *top_ri,
		c::register_def *top_rdef, register_info const *sub_ri)
{
	c::type *type = 0;
	c::register_def *rdef = 0;
	bool is_explicit = (sub_ri->flags & register_info::EXPLICIT_FLAG) != 0;

	//printf("analyze_sub(%s) [expl %u]\n", sub_ri->name.c_str(), is_explicit);

	type = convert_type(sub_ri->type);
	if (type == 0)
		return 0;

	// if this register is an hardwired expression, translate
	// the expression.
	if (sub_ri->hwexpr != 0) {
		c::expression *expr = convert_expression(sub_ri->hwexpr);
		if (expr == 0)
			return 0;

		rdef = new c::hardwired_sub_register_def(top_rdef, sub_ri->name,
				type, sub_ri->bit_start, type->get_bits(),
				expr);
	}
	// if this register is bound bidirectionally to a register,
	// this register is virtual and it's an indirect reference
	// to the bound register.
	else if (sub_ri->flags & register_info::BIDIBIND_FLAG) {
		c::type *bind_type;
		register_info *binding_ri = sub_ri->binding;

		bind_type = convert_type(binding_ri->type);
		if (bind_type == 0)
			return 0;

		if (!bind_type->is_equal(type)) {
			fprintf(stderr, "error: bidirectional binding '%s' requires that "
					"the bitfield '%s' size (%zu) matches the "
					"aliased register (%s) size (%zu) and type.\n",
					top_ri->name.c_str(),
					sub_ri->name.c_str(),
					type->get_bits(),
					binding_ri->name.c_str(),
					bind_type->get_bits());
			return 0;
		}

		rdef = create_aliased_sub(top_ri, top_rdef, type, sub_ri,
				binding_ri);

		// XXX add alternative name if this is a named bitfield
	}
	// if this register is bound to something check it.
	else if (sub_ri->binding != 0) {
		// special handling is required when binding
		// pseudo registers.
		if (sub_ri->binding->name[0] == '%') {
			rdef = create_pseudo_aliased_sub(top_ri, top_rdef, type,
					sub_ri);
		}
		// otherwise it is bound to a register as a update-on-write.
		else {
			rdef = new c::sub_register_def(top_rdef, sub_ri->name,
					type, sub_ri->bit_start, type->get_bits(),
					true);

			if (sub_ri->binding->name != top_ri->name) {
				// if the bound register has been generated, reference it,
				// otherwise record for lazy resolving.

				// find first in the sub registers of our top register.
				c::register_def *bound =
				 top_rdef->get_sub_register(sub_ri->binding->name);

				// if not found, try to lookup it globally.
				if (bound == 0)
					bound = m_named_rdefs[sub_ri->binding->name];

				if (bound != 0) {
					// add this register to the update-on-write vector
					// of the bound register.
					if (!bound->add_uow(rdef)) {
						fprintf(stderr, "fatal error: register '%s' binds to itself.\n",
								bound->get_name().c_str());
						return 0;
					}
				} else {
					// record for lazy resolving.
					//m_lazy_uow[sub_ri->binding->name].push_back(rdef);
					assert(0 && "IMPLEMENT ME ! (record lazy resolving!)");
				}
			}
		}
	}
	// if this register has a special evaluation function,
	// then subregisters total size may be different from
	// the final size, so that's treated specially.
	else if (sub_ri->special_eval != 0) {
		printf("WARNING: special evaluation is not yet implemented.\n");
	} else {
		// a simple sub register.
		rdef = new c::sub_register_def(top_rdef, sub_ri->name,
				type, sub_ri->bit_start, type->get_bits(),
				true);
	}

	if (rdef != 0) {
		for(register_info_vector::const_iterator i = sub_ri->subs.begin();
				i != sub_ri->subs.end(); i++) {

			c::register_def *sub = create_sub(sub_ri, rdef, *i);
			if (sub == 0)
				return 0;
		}

		// if not explicit, register in global defs.
		if (!is_explicit)
			m_named_rdefs[rdef->get_name()] = rdef;
	} else {
		fprintf(stderr, "FAILED CREATING REG %s\n", 
				sub_ri->name.c_str());
	}

	return rdef;
}

c::register_def *
register_file_builder::create_aliased_sub(register_info const *top_ri,
		c::register_def *top_rdef, c::type const *type,
		register_info const *sub, register_info const *alias)
{
	c::register_def *rdef;

	if (alias == sub) {
		fprintf(stderr, "error: register can't alias itself.\n");
		return 0;
	}

	// first create the bound register.
	rdef = create_sub(top_ri, top_rdef, alias);
	if (rdef == 0)
		return 0;

	// then change its subfield range.
	((c::sub_register_def *)rdef)->set_aliasing_range(sub->bit_start,
		type->get_bits());

	return rdef;
}

c::register_def *
register_file_builder::create_pseudo_aliased_sub(register_info const *top_ri,
		c::register_def *top_rdef, c::type *type,
		register_info const *sub_ri)
{
	c::register_def *rdef = 0;

	// pseudo registers bindable at bitfield level are only
	// conditional flags:
	//
	// C, N, P, V, Z (1 bit)
	//
	std::string pseudo_name (sub_ri->binding->name.substr(1));

	if (pseudo_name != "C" && pseudo_name != "N" && pseudo_name != "P" &&
			pseudo_name != "V" && pseudo_name != "Z") {
		fprintf(stderr, "error: only conditional pseudo registers may be "
				"aliased in bitfields.\n");
		return 0;
	}

	// the size of the bitfield shall always be 1.
	if (type->get_bits() != 1) {
		fprintf(stderr, "error: bound conditional bit flag is %zu bits in "
				"size, it shall be one.\n", type->get_bits());
		return 0;
	}

#if 0 // XXX we may not know yet if parent is bound to PSR, so we just assume it is.

	// CC flags shall always part of a register bound to the
	// pseudo register PSR.
	if (top_rdef->get_bound_special_register() != c::SPECIAL_REGISTER_PSR) {
		fprintf(stderr, "error: conditional flags may be aliased only when "
				"parent register is bound to the pseudo register %%PSR.\n");
		return 0;
	}
#endif

	c::special_register preg;

	switch (pseudo_name[0]) {
		case 'C': preg = c::SPECIAL_REGISTER_C; break;
		case 'N': preg = c::SPECIAL_REGISTER_N; break;
		case 'P': preg = c::SPECIAL_REGISTER_P; break;
		case 'V': preg = c::SPECIAL_REGISTER_V; break;
		case 'Z': preg = c::SPECIAL_REGISTER_Z; break;
	}

	rdef = new c::bound_sub_register_def(top_rdef, sub_ri->name,
			type, sub_ri->bit_start, type->get_bits(), preg,
			true);

	return rdef;
}

void
register_file_builder::dump_named() const
{
	for (named_register_def_map::const_iterator i = m_named_rdefs.begin();
			i != m_named_rdefs.end(); i++) {
		printf("NAMED: %s\n", i->first.c_str());
	}
}
