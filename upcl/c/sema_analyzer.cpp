#include "c/sema_analyzer.h"
#include "sema/simple_expr_evaluator.h"
#include "sema/register_dep_tracker.h"
#include "sema/register_file_builder.h"
#include "ast/dumper.h"

#include <cassert>
#include <sstream>

using namespace upcl;
using upcl::c::sema_analyzer;
using upcl::sema::register_dep_tracker;
using upcl::sema::register_file_builder;
using upcl::sema::register_info;
using upcl::sema::register_info_vector;

namespace {

	static inline bool is_repeat_identifier(std::string const &s)
	{ return (!s.empty() && s[s.length()-1] == '?'); }

}

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
						if (!sema::simple_expr_evaluator().evaluate(
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

	printf("%s: analyzing register dependencies...\n", __func__);

	size_t count = groups->size();
	for (size_t n = 0; n < count; n++) {
		ast::register_group const *group = (ast::register_group const *)(*groups)[n];
		if (!process_register_group_dep(group))
			return false;
	}

	printf("%s: register dependencies analysis: %zu total registers, "
			"%zu top registers, %zu independent registers, "
			"%zu pseudo registers, %zu stub registers, %zu dependencies.\n",
			__func__, g_dpt.get_all_regs_count(), g_dpt.get_top_regs_count(),
			g_dpt.get_indep_regs_count(), g_dpt.get_pseudo_regs_count(),
			g_dpt.get_stub_count(), g_dpt.get_deps_count());

	//g_dpt.dump_top();

	register_file_builder builder;
	builder.analyze(&g_dpt);

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

static std::string
make_repeat_register_name(ast::identifier const *identifier,
		uint64_t index)
{
	std::string name(identifier->get_value());

	if (identifier->is_repeat()) {
		std::stringstream ss;
		ss << name.substr(0, name.length() - 1) << index;
		return ss.str();
	}

	return name;
}

static upcl::ast::expression *
make_index_expr_relative(upcl::ast::expression *index_expr, uint64_t index)
{
	std::stringstream ss;
	ss << index;
	return new ast::binary_expression(ast::binary_expression::ADD,
			new ast::literal_expression(new ast::number(ss.str())),
			(upcl::ast::expression *)index_expr);
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
// - Aliasing registers (left binding operator <-)
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
	if (repeat_expr != 0 &&
			!sema::simple_expr_evaluator().evaluate(repeat_expr, repeat)) {
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
	bool relative_aliasing_index = false;
	string_vector alias_deps;
	ast::expression const *complex_alias_index_expr = 0;

	//
	// analyze repeatable identifiers.
	//
	if (repeat != 1) {
		ast::repeat_identifier const *repname = (ast::repeat_identifier const *)name;
		ast::expression const *startexpr = repname->get_expression();

		if (startexpr != 0 &&
				!sema::simple_expr_evaluator().evaluate(startexpr, start_index)) {
			fprintf(stderr, "error: register start index must evaluate to a constant value.\n");
			return false;
		}

		// handle the special case where we are aliasing another 
		// repeatable register set.
		if (alias_reg != 0 && alias_reg->is_repeat()) {
			ast::repeat_identifier const *alias_repname = 
				(ast::repeat_identifier const *)alias_reg;
			ast::expression const *alias_expr =
			 	alias_repname->get_expression();

			// if the aliasing index is a complex expression, collect
			// the literals in order to add them as dependencies of
			// the register.
			if (alias_expr != 0) {
				sema::simple_expr_evaluator x;

				// evaluator can fail and has to collect literals.
				x.collect_literals();

				if (!x.evaluate(alias_expr, alias_start_index)) {
					fprintf(stderr, "error: register alias indexing expression is too complex.\n");
					return false;
				} else if (x.has_failed()) {
					// if the expression contains the special symbol _
					// it is assumed to be absolute.
					relative_aliasing_index = !x.is_used("_");

					fprintf(stderr, "info: complex expression to compute aliased register index.\n");
					complex_alias_index_expr = alias_expr;

					// get all seen literals, they are dependencies.
					x.get_used_literals(alias_deps);
				}

				alias_reg = 0;
			}
		}
	}

	// create the register information
	for (uint64_t n = 0; n < repeat; n++) {
		std::string rname = make_repeat_register_name(name, n + start_index);

		// create dependencies.
		register_info *ri;

		if (hw_value != 0) {

			// read-only hardwiring

			// evaluate any register used.
			sema::simple_expr_evaluator e;
			uint64_t v;

			// evaluator can fail and has to collect literals.
			e.collect_literals();

			// evalute, ignore if failed.
			e.evaluate(hw_value, v);
			
			ri = g_dpt.add(rname, type, hw_value);

			string_vector expr_deps;
			e.get_used_literals(expr_deps);

			// cross deps
			g_dpt.make_deps_by(expr_deps, ri);
		} else if (alias_reg != 0) {

			// aliasing definition
			std::string aname;
			// if the indexing expression is complex, it is possible that
			// this expression must be evaluated at translation time, so
			// we make a reference to the repeatable identifier instead
			// of the target register; this is also true when a register
			// references a repeatable register like the case of the
			// stack pointer in m68k.
			if (complex_alias_index_expr != 0 ||
					(is_repeat_identifier(alias_reg->get_value()) &&
					 !is_repeat_identifier(rname))) {
				aname = alias_reg->get_value();
			} else {
				aname = make_repeat_register_name(alias_reg,
						n + alias_start_index);
			}

			ri = g_dpt.add(g_dpt.ref(aname), rname, type, reg_def, 
					register_dep_tracker::NOT_CHILD_FLAG);
			ri->binding = g_dpt.ref(aname);

			if (aname[0] != '%')
				ri->flags |= register_info::REGALIAS_FLAG;

			// bind complex indexing expression
			if (ri != 0 && complex_alias_index_expr != 0) {

				ast::expression *index_expr =
					(ast::expression *)complex_alias_index_expr;

				if (relative_aliasing_index) {
					// make the expression relative
					index_expr = make_index_expr_relative(index_expr,
							n + start_index);
				}

				ri->complex_index = index_expr;
			}

		} else {

			// simple definition
			ri = g_dpt.add(rname, type, reg_def);

		}

		if (ri == 0) {
			fprintf(stderr, "error: register '%s' is already defined.\n",
					name->get_value().c_str());
			return false;
		}

		ri->repeat_index = n + start_index;

		// cross deps
		g_dpt.make_deps_by(alias_deps, ri);

		// At this point, evaluate the bitfields if this register
		// has a definition attached.
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

		// process bitfields in reverse order, since they
		// are written "big endian".
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

// extract the base identifier name from an expression.
// the expression shall be a literal_expression and
// shall be bound to an identifier or a qualified identifier
// token. in the latter case, the qualified identifier
// shall be non-qualified (only the base identifier is
// present).
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

// returns the register name.
// if name is the special identifier (_), this field
// is unnamed.
static std::string
make_register_name(std::string const &base_name,
		std::string const &name, ssize_t offset)
{
	if (name == "_") {
		std::stringstream ss;
		ss << '$' << base_name << '$' << offset;
		return ss.str();
	} else {
		return name;
	}
}

bool
sema_analyzer::process_bound_value_dep(register_info *bri, size_t &offset,
		ast::type const *type, ast::bound_value const *bv, bool expl)
{
	register_info *ri;
	ast::identifier const *name = bv->get_name();
	ast::token const *binding = bv->get_binding();
	size_t max_size = atoi(type->get_value().c_str() + 2);

	// if there's no name check if this expression refers to 
	// an identifier and if so use it.
	if (name == 0) {
		if (!bound_value_name_from_expression(binding, name)) {
			fprintf(stderr, "error: bitfield cannot be qualified in register '%s'.\n",
					bri->name.c_str());
			return false;
		} else if (name != 0) {
			// no binding if name was extracted.
			binding = 0;
		}

		// if name is still null, it's a complex expression or a constant
		// expression.
	}

	std::string bfname;

	if (name != 0) {
		bfname = make_register_name(bri->name, name->get_value(), offset);

		ri = g_dpt.add(bri, bfname, type,
				register_info::SUBREGISTER_FLAG |
				(expl ? register_info::EXPLICIT_FLAG : 0));
	} else {
		bfname = make_register_name(bri->name, "_", offset);

		ri = g_dpt.add(bri, bfname, type, register_info::SUBREGISTER_FLAG |
				register_info::RESERVED_FLAG);
	}

	printf("\tBitfield '%s.%s' [%zu:%zu]\n", bri->name.c_str(), bfname.c_str(),
			offset, max_size);

	if (ri == 0) {
		fprintf(stderr, "error: register '%s' is already defined.\n",
				bfname.c_str());
		return false;
	}

	if (binding != 0) {
		if (binding->get_token_type() == ast::token::QUALIFIED_IDENTIFIER) {
			// if a qualified identifier, check it's only base identifier.
			ast::qualified_identifier const *qi =
			 (ast::qualified_identifier const *)binding;
			if (qi->get_identifier_list() == 0 ||
					qi->get_identifier_list()->size() == 0)
				binding = qi->get_base_identifier();
		}

		if (binding->get_token_type() == ast::token::IDENTIFIER) {
			//
			// The subfield is bound to a register, if it's repeatable,
			// evaluate the expression if possible: if "_" is present
			// in the expression, the expression is assumed to be "absolute",
			// otherwise the result from the expression is added to the
			// repeat index, hence "relative" index.
			//
			// If the expression cannot be resolved, it will be analyzed
			// on the second pass.
			//
			ast::identifier const *bid = (ast::identifier const *)binding;

			sema::simple_expr_evaluator e;

			// evaluator can fail and has to collect literals.
			e.collect_literals();

			// _ is the base register repeat index.
			e.set_var("_", bri->repeat_index);

#if 0
			fprintf(stderr, "INFO: %s has bound an identifier '%s'.\n",
					ri->name.c_str(),
					bid->get_value().c_str());
#endif

			std::string rname(bid->get_value());

			if (bid->is_repeat()) {
				ast::expression const *index_expr =
					((ast::repeat_identifier const *)bid)->get_expression();

				char buf[64];
				// 
				// No index expression, use referenced register.
				//
				if (index_expr == 0) {
					snprintf(buf, sizeof(buf), "%llu", bri->repeat_index);
					rname.replace(rname.find('?'), 1, buf);
				} else {
					uint64_t index;
					if (!e.evaluate(index_expr, index)) {
						fprintf(stderr, "error: bitfield aliasing indexing expression is too complex.\n");
						return false;
					}
					if (e.has_failed()) {
						fprintf(stderr, "info: bitfield aliasing indexing expression depends on other registers.\n");
						if (!e.is_used("_")) {
							// make the expression relative
							index_expr = make_index_expr_relative((upcl::ast::expression*)index_expr,
									bri->repeat_index);
						}
						ri->complex_index = index_expr;
						// add all variables found in the expression as deps.
						string_vector alias_deps;
						e.get_used_literals(alias_deps);
						
						// cross deps
						g_dpt.make_deps_by(alias_deps, ri);
					} else {
						// if the indexing identifier (_) hasn't been used,
						// then it's relative indexing.
						if (!e.is_used("_"))
							index += bri->repeat_index;

						snprintf(buf, sizeof(buf), "%llu", index);
						rname.replace(rname.find('?'), 1, buf);
					}
				}
			}

			if (bv->is_bidi())
				ri->flags |= register_info::BIDIBIND_FLAG;

			ri->binding = g_dpt.ref(rname);
			assert(ri->binding != ri);

			if (bv->is_bidi())
				g_dpt.make_deps_by(ri, ri->binding);
			else
				g_dpt.make_deps_by(ri->binding, ri);
			g_dpt.make_deps_by(ri->binding, bri);
		} else if (binding->get_token_type() == ast::token::EXPRESSION) {
			ri->hwexpr = (ast::expression const *)binding;
		} else if (binding->get_token_type() == ast::token::QUALIFIED_IDENTIFIER) {
			if (bv->is_bidi())
				ri->flags |= register_info::BIDIBIND_FLAG;

			ri->bind_copy = (ast::qualified_identifier const *)binding;

			// add as a dependency.
			register_info *ori = g_dpt.ref(ri->bind_copy->get_base_identifier()->get_value());

			// make ori deps by ri
			g_dpt.make_deps_by(ori, ri);

			//
			// if the binding is bidirectional, if the target register
			// is the same type/size of this bitfield, then make the super
			// of this register, also the super of the register; if it
			// has already a super, then probably that register is shared
			// amongst different registers.
			//
			if (bv->is_bidi() && ori->super == 0 &&
					(ori->type == 0 || ori->type->get_value() == 
					 	ri->type->get_value())) {
				ori->super = ri->super;
				g_dpt.make_deps_by(ori->super, ori);
			}
		} else {
			fprintf(stderr, "WARNING: binding token %u is not handled!\n", 
					binding->get_token_type());
		}
	}

	ri->bit_start = offset;

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
