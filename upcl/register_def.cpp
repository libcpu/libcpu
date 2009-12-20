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

void
register_def::add_sub(sub_register_def *alias)
{
	m_subs.push_back(alias);
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

void
test_reg_def()
{
	// 
	// try simulate m88k, where d0 alias r0:r1
	//
	register_def *r0 = new hardwired_register_def("r0", CCONST(0ULL),
			type::get_integer_type(32));
	register_def *r1 = new register_def("r1", type::get_integer_type(32));
	register_def *r2 = new register_def("r2", type::get_integer_type(32));

	register_def *d0 = new register_def("d0", type::get_integer_type(64));

	printf("PRE-BINDING\n");
	printf("r0 virtual: %u\n", r0->is_virtual());
	printf("r1 virtual: %u\n", r1->is_virtual());
	printf("d0 virtual: %u\n", d0->is_virtual());

	register_def *d0_0 = new bound_sub_register_def(d0, "d0_0", type::get_integer_type(32),
			32, 32, r0, true);
	register_def *d0_1 = new bound_sub_register_def(d0, "d0_1", type::get_integer_type(32),
			0, 32, r1, true);
	
	printf("POST-BINDING\n");
	printf("d0 virtual: %u\n", d0->is_virtual());
	printf("d0_0 virtual: %u\n", d0_0->is_virtual());
	printf("d0_1 virtual: %u\n", d0_1->is_virtual());

	//assign(d0, expression::fromInteger(0x1234567890abcdefull, 64));


	//
	// try simulate x64
	//
	register_def *rax = new register_def("rax", type::get_integer_type(64));

	register_def *rax_0 = new sub_register_def(rax, "rax_0", type::get_integer_type(16),
			32, 32, true);
	register_def *eax = new sub_register_def(rax, "eax", type::get_integer_type(32),
			0, 32, true);

	register_def *eax_0 = new sub_register_def(eax, "eax_0", type::get_integer_type(16),
			16, 16, true);
	register_def *ax = new sub_register_def(eax, "ax", type::get_integer_type(16),
			0, 16, true);

	register_def *ah = new sub_register_def(ax, "ah",
			type::get_integer_type(8), 8, 8);
	register_def *al = new sub_register_def(ax, "al",
			type::get_integer_type(8), 0, 8);

	printf("rax virtual: %u\n", rax->is_virtual());
	printf("eax virtual: %u\n", eax->is_virtual());
	printf("ax virtual: %u\n", ax->is_virtual());
	printf("ah virtual: %u\n", ah->is_virtual());
	printf("al virtual: %u\n", al->is_virtual());
#if 0
	assign(ax, CCONST(1));
	assign(ah, CCONST(0xaa));
	assign(al, CCONST(0x55));
	assign(rax_0, CCONST(0x55));
#endif
	// rflags/eflags/flags

	register_def *rflags = new bound_register_def("rflags", type::get_integer_type(64),
			SPECIAL_REGISTER_PSR);
	register_def *rflags_0 = new sub_register_def(rflags, "rflags_0", type::get_integer_type(32),
			32, 32, true);
	register_def *eflags = new sub_register_def(rflags, "eflags", type::get_integer_type(32),
			0, 32, true);

	register_def *eflags_0 = new sub_register_def(eflags, "eflags_0", type::get_integer_type(16),
			16, 16, true);
	register_def *flags = new sub_register_def(eflags, "flags", type::get_integer_type(16),
			0, 16, true);

	register_def *flags_O = new bound_sub_register_def(flags, "flags_O", type::get_integer_type(1),
			11, 1, SPECIAL_REGISTER_V, true);
	register_def *flags_D = new sub_register_def(flags, "flags_D", type::get_integer_type(1),
			10, 1, true);
	register_def *flags_I = new sub_register_def(flags, "flags_I", type::get_integer_type(1),
			9, 1, true);
	register_def *flags_T = new sub_register_def(flags, "flags_T", type::get_integer_type(1),
			8, 1, true);
	register_def *flags_S = new bound_sub_register_def(flags, "flags_S", type::get_integer_type(1),
			7, 1, SPECIAL_REGISTER_N, true);
	register_def *flags_Z = new bound_sub_register_def(flags, "flags_Z", type::get_integer_type(1),
			6, 1, SPECIAL_REGISTER_Z, true);
	register_def *flags_P = new bound_sub_register_def(flags, "flags_P", type::get_integer_type(1),
			2, 1, SPECIAL_REGISTER_P, true);
	register_def *flags_C = new bound_sub_register_def(flags, "flags_C", type::get_integer_type(1),
			0, 1, SPECIAL_REGISTER_C, true);
	register_def *flags_A = new bound_sub_register_def(flags, "flags_A", type::get_integer_type(1),
			4, 1, flags_C, false);

	register_def *X = new register_def("X", type::get_integer_type(32));
	register_def *Y = new register_def("Y", type::get_integer_type(32));
}
