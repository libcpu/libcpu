#include <cassert>

#include "c/sema_analyzer.h"
#include "c/simple_const_expr.h"

using namespace upcl;
using upcl::c::sema_analyzer;

sema_analyzer::sema_analyzer()
	: m_arch_tags()
{
}

void
sema_analyzer::parse(ast::token_list const *root)
{
	// root shall be a token list
	assert(root->get_token_type() == ast::token::LIST && "root shall be a list.");

	size_t count = root->size();
	for (size_t n = 0; n < count; n++) {
		ast::token const *t = (*root)[n];

		switch (t->get_token_type()) {
			case ast::token::ARCHITECTURE:
				if (!process_architecture((ast::architecture const *)t))
					return;

				break;

			default:
				break;
		}
	}
}

bool
sema_analyzer::process_architecture(ast::architecture const *arch)
{
	ast::register_file const *reg_file = 0;
	ast::token_list const *def = arch->get_def();

	m_arch_name = destringify(arch->get_name()->get_value());

	size_t count = def->size();
	for (size_t n = 0; n < count; n++) {
		ast::token const *t = (*def)[n];

		switch (t->get_token_type()) {
			case ast::token::TAGGED_VALUE:
				{
					ast::tagged_value const *tv = (ast::tagged_value const *)t;

					if (tv->get_tag() == ast::architecture::NAME) {
						if (!m_arch_full_name.empty()) {
							fprintf(stderr, "warning: redefining architecture full name.\n");
						}

						break;
					}

					uint64_t v = tv->get_value();
					if (tv->is_token()) {
						if (!c::simple_const_expr().eval(
									(ast::expression const *)tv->get_token(), v)) {
							fprintf(stderr, "error: expression is too complex.\n");
							return false;
						}
					}

					if (m_arch_tags[tv->get_tag()] != 0) {
						fprintf(stderr, "error: tag %u has already been specified.\n",
								tv->get_tag());
						return false;
					}

					m_arch_tags[tv->get_tag()] = v;
					break;
				}

			case ast::token::REGISTER_FILE:
				if (reg_file != 0) {
					fprintf(stderr, "error: register file has already been defined.\n");
					return false;
				}

				if (!process_register_file((ast::register_file const *)t))
					return false;

				reg_file = (ast::register_file const *)t;
				break;

			default:
				fprintf(stderr, "fatal error: this case should never happen.\n");
				return false;
		}
	}

	return true;
}

#include <map>

struct register_info;

typedef std::vector <register_info *> register_info_vector;

struct register_info {
	enum {
		UNRESOLVED_FLAG = 1,
		HARDWIRED_FLAG = 2,
		EXPLICIT_FLAG = 4,
		RESERVED_FLAG = 8
	};

	uint32_t               flags;
	std::string            name;
	ast::type const       *type;
	register_info         *super;
	ast::register_splitter const *splitter; // bitfields
	ast::expression const *hwexpr;          // hardwired expression
	ast::expression const *special_eval;    // special evaluation function on assignment.
	uint64_t               repeat_index;    // index to be used to compute aliasing
	                                        // register index value.
	ast::expression const *complex_index;   // index to be used to compute aliasing,
	                                        // but it's too complex to be evaluated
										    // during the pass, it can still evaluate
										    // to a constant value if the only variable
										    // referenced is the special variable '_'
	register_info         *binding;
	register_info_vector   subs;
	register_info_vector   deps;
};

class register_dep_tracker {
	typedef std::map <std::string, register_info *> name_info_map;

	name_info_map m_regs;

public:
	register_info *ref(std::string const &name);
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

	register_info *get(std::string const &name) const;

	void dump()
	{
		for(name_info_map::const_iterator i = m_regs.begin();
				i != m_regs.end(); i++) {
			printf("reg '%s'\n", i->first.c_str());
			for(register_info_vector::const_iterator j = i->second->deps.begin();
					j != i->second->deps.end(); j++) {
				printf("\tdep '%s'\n", (*j)->name.c_str());
			}
		}
	}
};

register_info *
register_dep_tracker::ref(std::string const &name)
{
	register_info *info;

	info = get(name);
	if (info == 0) {
		info = new register_info;
		info->name = name;
		info->type = 0;
		info->flags = register_info::UNRESOLVED_FLAG;
		info->super = 0;
		info->hwexpr = 0;
		info->splitter = 0;
		info->repeat_index = 0;
		m_regs[name] = info;
	}
	return info;
}

register_info *
register_dep_tracker::add(std::string const &name, ast::type const *type,
		unsigned flags)
{
	register_info *info;

	info = get(name);
	if (info != 0)
		return 0;

	info = new register_info;
	info->name = name;
	info->type = type;
	info->flags = flags & ~register_info::UNRESOLVED_FLAG;
	info->super = 0;
	info->hwexpr = 0;
	info->splitter = 0;
	info->repeat_index = 0;
	m_regs[name] = info;

	return info;
}

register_info *
register_dep_tracker::add(std::string const &name, ast::type const *type,
		ast::expression const *expr, unsigned flags)
{
	register_info *info;

	info = get(name);
	if (info != 0)
		return 0;

	info = new register_info;
	info->name = name;
	info->type = type;
	info->flags = flags & ~register_info::UNRESOLVED_FLAG;
	info->super = 0;
	info->hwexpr = expr;
	info->splitter = 0;
	info->repeat_index = 0;
	m_regs[name] = info;

	return info;
}

register_info *
register_dep_tracker::add(std::string const &name, ast::type const *type,
		ast::register_splitter const *splitter, unsigned flags)
{
	register_info *info;

	info = get(name);
	if (info != 0)
		return 0;

	info = new register_info;
	info->name = name;
	info->type = type;
	info->flags = flags & ~register_info::UNRESOLVED_FLAG;
	info->super = 0;
	info->hwexpr = 0;
	info->splitter = splitter;
	info->repeat_index = 0;
	m_regs[name] = info;

	return info;
}

register_info *
register_dep_tracker::add(register_info *super, std::string const &name,
		ast::type const *type, unsigned flags)
{
	register_info *info;

	info = get(name);
	if (info != 0)
		return 0;

	info = new register_info;
	info->name = name;
	info->type = type;
	info->flags = flags & ~register_info::UNRESOLVED_FLAG;
	info->super = super;
	info->hwexpr = 0;
	info->splitter = 0;
	info->repeat_index = 0;

	if ((info->flags & (register_info::EXPLICIT_FLAG | register_info::RESERVED_FLAG)) == 0)
		m_regs[name] = info;

	if (super != 0) {
		super->deps.push_back(info);
		super->subs.push_back(info);
	}
	info->deps.push_back(super);

	return info;
}

register_info *
register_dep_tracker::add(register_info *super, std::string const &name,
		ast::type const *type, ast::register_splitter const *splitter,
		unsigned flags)
{
	register_info *info;

	info = get(name);
	if (info != 0)
		return 0;

	info = new register_info;
	info->name = name;
	info->type = type;
	info->flags = flags & ~register_info::UNRESOLVED_FLAG;
	info->super = super;
	info->hwexpr = 0;
	info->splitter = splitter;
	info->repeat_index = 0;
	
	if ((info->flags & (register_info::EXPLICIT_FLAG | register_info::RESERVED_FLAG)) == 0)
		m_regs[name] = info;

	if (super != 0) {
		super->deps.push_back(info);
		super->subs.push_back(info);
	}
	info->deps.push_back(super);

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

register_dep_tracker g_dpt;

bool
sema_analyzer::process_register_file(ast::register_file const *reg_file)
{
	ast::token_list const *groups = reg_file->get_groups();

	//
	// In order to simplify the register definitions, we move the complexity
	// in the analysis of the register file.
	//
	// Initially we construct a dependency between registers so to produce
	// registers with correct order. This approach ends up easing the register
	// definition for composite registers. 
	//
	// As an example, the lower part of the x86 "eax" register is aliasing
	// "ax" which, in turn, is itself a composite register too, made by "ah" and
	// "al". To define this kind of hierarchy we can use both "left binding"
	// in the register defintion and "bidirectional" or "right binding"
	// operators in one of the sub-registers in the composite register.
	//
	// Once the dependencies are tracked and met, they are sorted to match
	// the aliasing, and lastly the register definitions are created.
	// 

	size_t count = groups->size();
	for (size_t n = 0; n < count; n++) {
		ast::register_group const *group = (ast::register_group const *)(*groups)[n];
		if (!process_register_group_dep(group))
			return false;
	}

	g_dpt.dump();

	return true;
}

bool
sema_analyzer::process_register_group_dep(ast::register_group const *group)
{
	ast::token_list const *regs = group->get_registers();

	fprintf(stderr, "%s: register group '%s'\n", __func__, group->get_name()->get_value().c_str());

	size_t count = regs->size();
	for (size_t n = 0; n < count; n++) {
		ast::register_declaration const *rd = (ast::register_declaration const *)(*regs)[n];

		if (!process_register_dep(rd))
			return false;
	}

	return true;
}

// 
// A more in-depth about bindings.
//
// First of all, there are two different level of bindings with
// almost identical functionality, and they can be used only with
// integer data types and possibly vector data types, although not
// yet implemented; the bindings are:
//
// - register level binding
// - register bitfield level binding
//
// While register level binding provides a global register binding,
// the more fine grained bitfield level binding can aid in complex
// bitfield constructions; both can be used to perform complex tasks
// as evaluation of expressions at translation time.
//
// For example, suppose you have four flags: C, N, Z and P (Positive,
// to make this example easy to understand) and the PSR is represented
// by an 8-bit value that looks like "1:1:1:1:P:N:C:Z" where the LSB
// bits shall be always 1, you can express the PSR easily like this:
//
// [ #i8 psr -> %PSR explicit [ #i4 0xf, // Fixed LSB
// 								#i1 P <- (!N),
// 							  	#i1 N -> %N,
// 							  	#i1 C -> %C,
// 							  	#i1 Z -> %Z ]
// 
// You can see here both register level binding (flags -> %PSR ...) and
// bitfield level binding (in the variable length variant).
//
// There are three kinds of binding at register level:
//
// - Register bitfield definitions (right binding operator ->).
//
//   It's used to describe bitfields of the register, it can be used
//   to build composite registers, like "ax", "ah" and "al" of the
//   8086. 
//   Most common use is PSR handling.
//
// - Aliasing registers (right binding operator ->)
//
//   
// - Hardwiring values (left binding operator <-)
//
//
// There are four kinds of binding at bitfield level:
//
// - Auto-updating bitfields (left binding operator <-).
//
//   The right operand shall be a register, then updating that
//   register value will also in turn update this one with a copy,
//   not viceversa.
//   This bitfield has physical storage.
//
// - Hardwired bitfields (left binding operator <-).
//
//   The right operand isn't a register and it is evaluated upon reading
//   it at translation time, but an error will be thrown in case of
//   writing.
//   This bitfield has no physical storage.
//
// - Register aliasing (right bind operator ->).
//   
//   The right operand shall evaluate to an existing register.
//   Both registers shall be of the same type, not necessarily the
//   same size.
//
//   * If this bitfield definition match the aliased (->) register,
//     this bitfield is considered virtual and fully aliasing the 
//     register.
//   
//   * In case this bitfield is smaller than the aliased register,
//     writing to this bitfield will modify only the LSB part
//     of the aliased register with the specified value.
//     This case also makes the bitfield virtual.
//
//   * In case the bitfield is bigger than the aliased register,
//     this is quasi-virtual bitfield, that means there's physical 
//     storage allocated for the extra bits missing. Reading from
//     this bitfield will return the composite value, writing to it
//     will automatically split the writes.
//
//   It's used to create aliasing of sub registers like in the case
//   of the X86/X86_64.
//
//   WARNING! WARNING! WARNING! WARNING! WARNING! WARNING! WARNING!
//
//   - Used in conjuction with the PSR special register, you shall
//     use this operator to bind special conditional flags, the 
//     conditional flags are always 1 bit wide of integer type.
//
//   - Used in conjuction with the PC special register, writing to
//     this bitfield or the aliased register doesn't trigger the jump.
//
//     In this case, the follow restriction applies: the width of the
//     field shall match the bound register width.
//
//   WARNING! WARNING! WARNING! WARNING! WARNING! WARNING! WARNING!
//
// - The bidirectional operator (<->) is equivalent of right bind 
//   operator but it has special use with registers binding the special
//   register PC.
//
//   The right operand shall always evaluate to an existing register.
//
//   This operator, when used in conjuction with the PC special
//   register, will trigger a jump upon being written or by modifing
//   the aliased register value.
//
bool
sema_analyzer::process_register_dep(ast::register_declaration const *rd)
{
	ast::identifier const *name = rd->get_def()->get_name();
	ast::type const *type = rd->get_def()->get_type();
	ast::expression const *repeat_expr = rd->get_repeat();
	ast::token const *binding = rd->get_binding();
	uint64_t repeat = 1;

	fprintf(stderr, "%s: register '%s'\n", __func__, name->get_value().c_str());

	// repeat expression shall be constant.
	if (repeat_expr != 0 && !c::simple_const_expr().eval(repeat_expr, repeat)) {
		fprintf(stderr, "error: repeat expression shall be constant.\n");
		return false;
	}

	if (repeat == 0) {
		fprintf(stderr, "error: zero is not a valid repeat count.\n");
		return false;
	}

	ast::register_splitter const *reg_def = 0;
	ast::identifier const *alias_reg = 0;
	ast::expression const *hw_value = 0;

	// if this register has binding track its dependencies.
	if (binding != 0) {
		printf("binding : %u\n", binding->get_token_type());
		switch (binding->get_token_type()) {
			case ast::token::REGISTER_SPLITTER1:
			case ast::token::REGISTER_SPLITTER2:
				reg_def = (ast::register_splitter const *)binding;
				break;

			case ast::token::REGISTER_BINDER:
				{
					// if bound to a special register record it.
					ast::register_binder const *rb = (ast::register_binder const *)binding;
					alias_reg = rb->get_register();
					reg_def = (ast::register_splitter const *)rb->get_splitter();
				}
				break;

			case ast::token::ALIAS_VALUE:
				{
					ast::alias_value const *av = (ast::alias_value const *)binding;
					ast::token const *alias = av->get_alias();

					switch (alias->get_token_type()) {
						case ast::token::EXPRESSION:
							// hardwiring
							hw_value = (ast::expression const *)alias;
							break;

						case ast::token::IDENTIFIER:
							// aliasing a register
							alias_reg = (ast::identifier const *)alias;
							break;

						default:
							fprintf(stderr, "error: invalid token while processing register aliasing.\n");
							return false;
					}
				}
				break;

			default:
				fprintf(stderr, "error: invalid token while processing register binding.\n");
				return false;
		}
	}

	uint64_t start_index = 0;
	uint64_t alias_start_index = 0;
	ast::expression const *complex_alias_index_expr = 0;
	if (repeat != 1) {
		ast::repeat_identifier const *repname = (ast::repeat_identifier const *)name;
		ast::expression const *startexpr = repname->get_expression();

		if (startexpr != 0 &&
				!c::simple_const_expr().eval(startexpr, start_index)) {
			fprintf(stderr, "error: register start index must evaluate to a constant value.\n");
			return false;
		}

		if (alias_reg != 0 && alias_reg->is_repeat()) {
			ast::repeat_identifier const *aliasrepname = (ast::repeat_identifier const *)alias_reg;
			ast::expression const *aliasexpr = aliasrepname->get_expression();

			if (aliasexpr != 0 &&
					!c::simple_const_expr().eval(aliasexpr, alias_start_index)) {
				fprintf(stderr, "info: complex expression to compute aliased register index.\n");
				// if it's complex compute it later.
				complex_alias_index_expr = aliasexpr;
			}
		}
	}

	for (uint64_t n = 0; n < repeat; n++) {
		char buf[64];
		std::string rname(name->get_value());
		if (name->is_repeat()) {
			snprintf(buf, sizeof(buf), "%llu", n + start_index);
			rname.replace(rname.find('?'), 1, buf);
		}

		// create dependency.
		register_info *ri;
		if (hw_value != 0)
			ri = g_dpt.add(rname, type, hw_value);
		else if (alias_reg != 0) {
			std::string aname(alias_reg->get_value());
			if (alias_reg->is_repeat() && complex_alias_index_expr == 0) {
				// if it's not complex, use the alias_start_index.
				snprintf(buf, sizeof(buf), "%llu", n + alias_start_index);
				aname.replace(aname.find('?'), 1, buf);
			}

			ri = g_dpt.add(g_dpt.ref(aname), rname, type, reg_def);

			if (ri != 0)
				ri->complex_index = complex_alias_index_expr;
		}
		else
			ri = g_dpt.add(rname, type, reg_def);

		if (ri == 0) {
			fprintf(stderr, "error: register '%s' is already defined.\n",
					name->get_value().c_str());
			return false;
		}

		ri->repeat_index = n + start_index;

		// At this point, evaluate the bitfields if this register has a 
		// definition attached.
		if (ri->splitter != 0 && !process_register_splitter_dep(ri))
			return false;
	}

	return true;
}

bool
sema_analyzer::process_register_splitter_dep(register_info *ri)
{
	assert(ri->splitter != 0);

	ast::register_splitter const *splitter = ri->splitter;
	ast::expression const *special_evaluation = splitter->get_evaluate();
	ast::token_list const *values = splitter->get_values();
	ast::type const *type = splitter->get_type();
	bool expl = splitter->is_explicit();

	if (special_evaluation != 0)
		ri->special_eval = special_evaluation;

	if (values != 0) {
		size_t offset = 0;
		size_t count = values->size();

		// process in reverse order!
		for (size_t n = 0; n < count; n++) {
			ast::token *t = (*values)[count - n - 1];

			switch (t->get_token_type()) {
				case ast::token::BOUND_VALUE:
					if (!process_bound_value_dep(ri, offset, type,
								(ast::bound_value const *)t, expl))
						return false;
					break;

				case ast::token::TYPED_BOUND_VALUE:
					if (!process_typed_bound_value_dep(ri, offset, 0, 0,
								(ast::typed_bound_value const *)t, expl))
						return false;
					break;

				default:
					fprintf(stderr,
							"error: invalid token processing register '%s' values.\n",
							ri->name.c_str());
					return false;
			}
		}
	}

	return true;
}

namespace {

// fails only if identifier is qualified!
static bool
bound_value_name_from_expression(ast::token const *binding,
		ast::identifier const *&name)
{
	name = 0;

	if (binding->get_token_type() == ast::token::EXPRESSION &&
			((ast::expression const *)binding)->get_expression_type() 
			== ast::expression::LITERAL) {
		ast::token const *t = ((ast::literal_expression *)binding)->get_literal();
		if (t->get_token_type() == ast::token::IDENTIFIER)
			name = (ast::identifier const *)t;
		else if (t->get_token_type() == ast::token::QUALIFIED_IDENTIFIER) {
			// this shall have always the identifiers count to zro.
			ast::qualified_identifier *qi = (ast::qualified_identifier *)t;
			name = qi->get_base_identifier();

			ast::token_list const *idlist = qi->get_identifier_list();
			if (idlist != 0 && idlist->size() != 0) {
				fprintf(stderr, "error: use of qualified identifiers is forbidden in register binding.\n");
				return false;
			}
		}
	}

	return true;
}

}

bool
sema_analyzer::process_bound_value_dep(register_info *ri, size_t &offset,
		ast::type const *type, ast::bound_value const *bv, bool expl)
{
	ast::identifier const *name = bv->get_name();
	ast::token const *binding = bv->get_binding();
	size_t max_size = atoi(type->get_value().c_str() + 2);

	// if there's no name check this expression refers to an identifier,
	// if so use it.
	if (name == 0) {
		if (!bound_value_name_from_expression(binding, name)) {
			fprintf(stderr, "error: bitfield cannot be qualified in register '%s'.\n",
					ri->name.c_str());
			return false;
		}

		// if name is still null, it's a complex expression or a constant
		// expression, in the both cases this expression is computed at
		// the second pass.
	}

	char const *bitfield_name = (name != 0 ? name->get_value().c_str() :
			"<expr>");

	printf("\tBitfield '%s.%s' [%zu:%zu]\n", ri->name.c_str(), bitfield_name,
			offset, max_size);

	if (name != 0) {
		std::string bfname(name->get_value());
		if (bfname == "_") {
			char buf[64];
			snprintf(buf, sizeof(buf), "$%s$%u", ri->name.c_str(), offset);
			bfname = buf;
		}

		ri = g_dpt.add(ri, bfname, type,
				expl ? register_info::EXPLICIT_FLAG : 0);
		
		if (ri == 0) {
			fprintf(stderr, "error: register '%s' is already defined.\n",
					bfname.c_str());
			return false;
		}
	} else {
		char buf[64];
		snprintf(buf, sizeof(buf), "$%s$%u", ri->name.c_str(), offset);

		ri = g_dpt.add(ri, buf, type, register_info::RESERVED_FLAG);
		if (ri == 0) {
			fprintf(stderr, "error: register '%s' is already defined.\n", buf);
			return false;
		}
	}

	if (binding != 0) {
		if (binding->get_token_type() == ast::token::IDENTIFIER) {
			//
			// The subfield is bound to a register, if it's a repeatable,
			// evaluate the expression if possible, if "_" is present
			// in the expression, the expression is assumed to be "absolute",
			// otherwise the result from the expression is added to the
			// repeat index, hence "relative" index.
			//
			ast::identifier const *bid = (ast::identifier const *)binding;

			c::simple_const_expr x;
			x.set_var("_", ri->repeat_index);

			fprintf(stderr, "INFO: %s has bound an identifier '%s'.\n",
					ri->name.c_str(),
					bid->get_value().c_str());

			if (bid->is_repeat()) {
				ast::expression const *index_expr =
					((ast::repeat_identifier const *)bid)->get_expression();

				if (index_expr == 0) {

				}
			}
		}
	}

	offset += max_size;

	return true;
}

bool
sema_analyzer::process_typed_bound_value_dep(register_info *ri, size_t &offset,
		size_t max_size, ast::type const *type,
		ast::typed_bound_value const *bv, bool expl)
{
	ast::token_list const *bindings = bv->get_values();
	size_t count = bindings->size();

	if (type == 0)
		type = bv->get_type();

	if (type == 0) {
		fprintf(stderr, "fatal error: processing a typed bitfield the type is nil.\n");
		return false;
	}
	if (type->get_value()[1] != 'i') {
		fprintf(stderr, "error: bitfield types shall only be of integer data type.\n");
		return false;
	}
		
	// special case for single typed bound values.
	if (count == 1) {
		ast::token const *t = (*bindings)[0];
		if (t->get_token_type() == ast::token::BOUND_VALUE)
			return process_bound_value_dep(ri, offset, type,
					(ast::bound_value const *)t, expl);
	}

	// process in reverse order!
	for (size_t n = 0; n < count; n++) {
		ast::typed_bound_value const *tbv =
			(ast::typed_bound_value const *)(*bindings)[count - n - 1];

		if (!process_typed_bound_value_dep(ri, offset, max_size, type,
					tbv, expl))
			return false;
	}

	return true;
}
