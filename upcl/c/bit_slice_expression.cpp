#include <cassert>
#include <cstdio>

#include "c/bit_slice_expression.h"
#include "c/bit_combine_expression.h"
#include "c/fast_aliases.h"
#include "c/type.h"

using namespace upcl;
using namespace upcl::c;

bit_slice_expression::bit_slice_expression(expression *expr,
		expression *first_bit, expression *bit_count)
	: expression(BIT_SLICE), m_expr(expr), m_first_bit(first_bit),
	m_bit_count(bit_count), m_type(0)
{
	// warn if bitslice > expr type
	assert(expr->get_type()->get_type_id() != type::FLOAT &&
		"Cannot bit slice floating point data.");
	assert(first_bit->get_type()->get_type_id() == type::INTEGER &&
		"Can index only with integer types.");
	assert(bit_count->get_type()->get_type_id() == type::INTEGER &&
		"Can index only with integer types.");

	uint64_t bit;
	if (m_bit_count->evaluate_as_integer(bit)) {
		size_t nbits = expr->get_type()->get_bits();
		if (bit > nbits) {
			fprintf(stderr, "warning: bit slice operation is bigger than the "
					"expression size, truncating from %llu to %zu bits.\n",
					bit, expr->get_type()->get_bits());

			m_type = expr->get_type();
		} else {
			m_type = type::get_integer_type(bit);
		}
	}
}

expression *
bit_slice_expression::simplify(bool) const
{
	uint64_t bit;

	if (m_first_bit->is_zero() && m_bit_count->is_zero()) {
		assert(0 && "Bit count cannot be zero.");
		return 0;
	}

	if (evaluate(bit))
		return expression::fromInteger(bit, m_type->get_bits());

	if (m_first_bit->evaluate_as_integer(bit)) {
		if (bit >= m_expr->get_type()->get_bits()) {
			fprintf(stderr, "warning: first bit is past the expression "
					"size.\n");

			return expression::fromInteger(0ULL, get_type()->get_bits());
		}
	}

	expression *expr      = m_expr->simplify();
	expression *first_bit = m_first_bit->simplify();
	expression *bit_count = m_bit_count->simplify();

	// is this bit slice useless?
	if (first_bit->is_zero() && bit_count->evaluate_as_integer(bit) &&
			bit == expr->get_type()->get_bits())
		return expr; // yes

	// bitslicing a bitcombine?
	if (expr->get_expression_operation() == expression::BIT_COMBINE) {
		// tricky, shift first, shift are resolved internally to bitcombine.
		expr = CSHR(expr, first_bit)->simplify();
		if (expr->get_expression_operation() == expression::BIT_COMBINE) {
			uint64_t nbits;
			if (bit_count->evaluate(nbits))
				return ((bit_combine_expression *)expr)->truncate(nbits);

			assert(0 && "Bit-slicing bit-combined values must evaluate always constantly.");
		}
	}

	if (!expr->is_equal(m_expr) || !first_bit->is_equal(m_first_bit) ||
			!bit_count->is_equal(m_bit_count))
		return (new bit_slice_expression(expr, first_bit, bit_count))->simplify();
	else
		return const_cast<bit_slice_expression*>(this);
}

type *
bit_slice_expression::get_type() const
{
	if (m_type != 0)
		return m_type;
	else
		// if type is unknown, assume always expression type.
		return m_expr->get_type();
}

bool
bit_slice_expression::is_constant() const
{
	return m_expr->is_constant() && m_first_bit->is_constant()
		&& m_bit_count->is_constant();
}


expression *
bit_slice_expression::sub_expr(size_t index) const
{
	switch (index) {
		case 0: return m_expr;
		case 1: return m_first_bit;
		case 2: return m_bit_count;
		default: return 0;
	}
}

bool
bit_slice_expression::evaluate_as_integer(uint64_t &value, bool sign) const
{
	uint64_t first_bit, bit_count;

	if (m_first_bit->evaluate_as_integer(first_bit) &&
			m_bit_count->evaluate_as_integer(bit_count) &&
			m_expr->evaluate_as_integer(value, sign)) {

		if (bit_count == 0 || first_bit + bit_count > 64) {
			fprintf(stderr, "warning: invalid bit slice, "
					"first_bit = %llu bit_count = %llu while limit = %u, "
					"result will be zero.\n",
					first_bit, bit_count, 64);
			value = 0;
		} else {
			value = (value >> first_bit) & ((1ULL << bit_count) - 1);
		}

		return true;
	}

	return false;
}

bool
bit_slice_expression::evaluate_as_float(double &) const
{
	assert(0 && "Cannot bit slice floating point values");
	return false;
}

expression *
bit_slice_expression::expand() const
{
	// XXX TO BE REMOVED!
	return CCAST(m_type, CAND(CSHR(m_expr, m_first_bit), CMASKBIT(m_bit_count)));//->simplify();
}
