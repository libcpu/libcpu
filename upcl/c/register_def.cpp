#include <cassert>

#include "c/register_def.h"
#include "c/sub_register_def.h"
#include "c/hardwired_register_def.h"
#include "c/bound_register_def.h"
#include "c/bound_sub_register_def.h"

#include "c/fast_aliases.h"
#include "c/bit_slice_expression.h"
#include "c/bit_combine_expression.h"

using namespace upcl;
using namespace upcl::c;

register_def::register_def(unsigned flags, std::string const &name,
		c::type *type)
	: m_flags(flags), m_name(name), m_type(type),
	m_special_bind(NO_SPECIAL_REGISTER), m_bind(0), m_expr(0)
{
}

register_def::register_def(std::string const &name, c::type *type)
	: m_flags(0), m_name(name), m_type(type),
	m_special_bind(NO_SPECIAL_REGISTER), m_bind(0), m_expr(0)
{
}

register_def::~register_def()
{
}

bool
register_def::is_hardwired() const
{
	return ((m_flags & REGISTER_FLAG_HARDWIRED) != 0);
}

bool
register_def::is_bound() const
{
	return (m_special_bind != NO_SPECIAL_REGISTER || m_bind != 0);
}

bool
register_def::is_sub_register() const
{
	return false;
}

std::string
register_def::get_name() const
{
	return m_name;
}

register_def *
register_def::get_bound_register() const
{
	return m_bind;
}

special_register
register_def::get_bound_special_register() const
{
	return m_special_bind;
}

void
register_def::set_expression(expression *expression)
{
	m_expr = expression;
}

void
register_def::set_binding(register_def *binding)
{
	m_special_bind = NO_SPECIAL_REGISTER;
	m_bind = binding;
}

void
register_def::set_binding(special_register const &special)
{
	m_special_bind = special;
	m_bind = 0;
}

bool
register_def::add_sub(sub_register_def *alias)
{
	if (alias == this || alias == 0)
		return false;

	if (m_named_subs.find(alias->get_name()) != m_named_subs.end())
		return false;

	m_subs.push_back(alias);
	m_named_subs[alias->get_name()] = alias;
	return true;
}

type *
register_def::get_type() const
{
	return m_type;
}

//
// A register can be considered "virtual" (ie with no
// phyisical storage) because:
//
// - It's aliasing physical registers totally and not partially:
//   
bool
register_def::is_virtual() const
{

	// if there are no subs, it has physical storage.
	if (m_subs.empty())
		return false;

	// - if the sub registers are aliasing other registers,
	//   check how much of this register is aliased, if there
	//   are bits uncovered by the aliasing, then it has
	//   physical storage to cover those bits, unless those bits
	//   are hardwired to a constant value or computable runtime
	//   expression.
	// - if all the sub registers are hardwired the register is
	//   virtual.
	// - if any of the sub registers is evaluted through a runtime
	//   expression then it's virtual and uncovered bits are assumed
	//   to be zero.
	bool probably_virtual = true;
	for (sub_register_vector::const_iterator i = m_subs.begin();
		i != m_subs.end(); i++) {

		sub_register_def const *sub = (*i);

		// is this sub register bound to another register?
		if (sub->is_bound_to_register()) {

			// is this sub-register full-aliasing this register?
			if (sub->is_full_alias()) {
				register_def *bound = sub->get_bound_register();

				// check if the bound register is bigger or smaller.
				int diff = 0;// bound->get_type()->compare(get_type());
				
				// is the bound register bigger or equal ?
				if (diff >= 0) {
					// bigger, this register is virtual.
					return true;
				} else {
					// smaller, assume there are uncovered bits.
					probably_virtual = false;
				}
			} else {
				uint64_t first_bit, bit_count;
				// this expression shall be constant.
				if (!sub->get_first_bit()->evaluate_as_integer(first_bit) ||
					!sub->get_bit_count()->evaluate_as_integer(bit_count)) {
					assert(0 && "Expression in register splitter shall be constant.\n");
					return false;
				}
			}

		} else if (sub->is_bound_to_special()) {
			abort();
		} else if (!sub->is_hardwired()) {
			probably_virtual = false;
		}
	}

	return probably_virtual;
}

void dump_expr(expression const *e);

void
assign(register_def *reg, expression *v)
{
#if 0
	if (reg->is_hardwired()) {
		fprintf(stderr, "warning: trying to assign hardwired register '%s'\n", reg->get_name().c_str());
		return;
	}

	if (reg->is_virtual()) {
		if (reg->is_sub_register()) {
			sub_register_def const *sub = (sub_register_def const *)reg;
			if (sub->is_bound_to_register()) {
				assign(sub->get_bound_register(), v);
				if (!sub->is_bidi())
					goto assign;
			} else {
assign:
				if (sub->is_full_alias())
					assign(sub->get_master_register(), v);
				else {
					// XXX BROKEN!
					assign(sub->get_master_register(), 
							CSHL(CAND(v, CMASKBIT(sub->get_bit_count())),
								sub->get_first_bit()));
				}
			}
		} else {
			// this register has virtual sub registers.
			sub_register_vector const &subs = reg->get_sub_register_vector();
			for (sub_register_vector::const_iterator i = subs.begin();
					i != subs.end(); i++) {

				expression *first_bit = (*i)->get_first_bit();
				expression *bit_count = (*i)->get_bit_count();

				expression *tv = v;
				if (first_bit != 0 && bit_count != 0)
					tv = CAND(CSHR(v, first_bit), CMASK(bit_count));

				assign((*i), tv);
			}
		}
	} else {
		uint64_t value;

		if (v->evaluate_as_integer(value))
			printf("reg %s = %llx\n", reg->get_name().c_str(), value);
	}
#endif
}

sub_register_def *
register_def::get_sub_register(std::string const &name)
{
	named_sub_register_map::iterator i = m_named_subs.find(name);
	if (i != m_named_subs.end())
		return i->second;
	else
		return 0;
}

bool
register_def::add_uow(register_def *rdef)
{
	if (rdef == this || rdef == 0)
		return false;

	m_uow.insert(rdef);
	return true;
}

bool
register_def::is_uow() const
{
	return !m_uow.empty();
}

void
register_def::set_register_set(register_set *set)
{
	m_set = set;
}

register_set *
register_def::get_register_set() const
{
	return m_set;
}
