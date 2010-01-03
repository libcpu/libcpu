#include <cassert>
#include <cstdio>
#include <set>

#include "c/bit_combine_expression.h"
#include "c/fast_aliases.h"
#include "c/type.h"

using namespace upcl;
using namespace upcl::c;

void
print_expr(expression const *expr);

bit_combine_expression::bit_combine_expression(expression_vector const &exprs)
	: expression(BIT_COMBINE), m_type(0)
{
	init(exprs);
}

namespace {

	static bool
	compact_zero_fields(expression_vector const &in, expression_vector &out)
	{
		bool changed = false;
		std::set <expression const *> skip;

		out.clear();

		for (expression_vector::const_reverse_iterator i = in.rbegin();
				i != in.rend(); i++) {

			if (skip.find(*i) != skip.end())
				continue;

			expression *expr1 = *i;
			expression *expr2 = 0;

			if (!expr1->is_constant())
				continue;

			if ((i + 1) != in.rend() && skip.find(*(i + 1)) == skip.end())
				expr2 = *(i + 1);

			// expr2 is zero, expr1->type + expr2->type is still less than or
			// equal to 64.
			if (expr2 != 0 && expr2->is_zero() &&
					(expr1->get_type()->get_bits() +
					 expr2->get_type()->get_bits()) < 64) {
				// expand expr1 data type.
				expr1->get_type()->set_bits_size(
						expr1->get_type()->get_bits() +
						expr2->get_type()->get_bits());
				// reprocess this expression again, and ignore expr2.
				i--; skip.insert(expr2);
				changed = true;
			}

		}

		if (changed) {
			for (expression_vector::const_iterator i = in.begin();
					i != in.end(); i++) {
				if (skip.find(*i) != skip.end())
					continue;

				out.push_back((expression *)*i);
			}
		}

		return changed;
	}

	static bool
	compact_constant_fields(expression_vector const &in, expression_vector &out)
	{
		bool changed = false;
		std::set <expression const *> skip;
		expression_vector temp;

		for (expression_vector::const_reverse_iterator i = in.rbegin();
				i != in.rend(); i++) {

			if (skip.find(*i) != skip.end())
				continue;

			expression *expr1 = *i;
			expression *expr2 = 0;

			if ((i + 1) != in.rend() && skip.find(*(i + 1)) == skip.end())
				expr2 = *(i + 1);

			// we need two consecutive constant expressions.
			if (expr2 == 0 || !expr1->is_constant() || !expr2->is_constant()) {
				temp.insert(temp.begin(), expr1);
				continue;
			}

			// both are constants, if expr1 is < 64bits,
			// then combine the value.
			size_t bits1 = expr1->get_type()->get_bits();
			size_t bits2 = expr2->get_type()->get_bits();
			assert(bits1 <= 64 && bits2 <= 64);

			if (bits1 < 64) {
				uint64_t value1, value2;

				if (!expr1->evaluate_as_integer(value1)) {
					temp.insert(temp.begin(), expr1);
					continue;
				}
				if (!expr2->evaluate_as_integer(value2)) {
					temp.insert(temp.begin(), expr1);
					continue;
				}

				size_t avail = 64 - bits1;
				if (avail > bits2)
					avail = bits2;

				value1 |= (value2 & ((1 << avail) - 1)) << bits1;
				// push expr1
				temp.insert(temp.begin(), expression::fromInteger(value1,
							bits1 + avail));

				// push expr2
				if (avail < bits2) {
					temp.insert(temp.begin(), expression::fromInteger(
								value2 >> avail, bits2 - avail));
				}

				// ignore expr2
				skip.insert(expr2);
				changed = true;
			} else {
				temp.insert(temp.begin(), expr1);
			}
		}

		if (changed)
			out = temp;

		return changed;
	}

}

expression *
bit_combine_expression::simplify(bool) const
{
	expression_vector simply, result;

	if (m_type == 0) {
		assert(0 && "Type is null!");
		return 0;
	}

	// if just one expression, return a cast to it.
	if (m_exprs.size() == 1)
		return (expression::Cast(m_type, m_exprs[0]))->simplify();

	bool changed = false;
	for (expression_vector::const_iterator i = m_exprs.begin();
			i != m_exprs.end(); i++) {
		if ((*i)->get_type()->get_bits() == 0) {
			changed = true;
			continue;
		}
		simply.push_back((*i)->simplify());
		if (!simply.back()->is_equal(*i))
			changed = true;
	}

	if (changed)
		return (new bit_combine_expression(simply))->simplify();

	// compact zeroed fields.
	if (compact_zero_fields(simply, result))
		return (new bit_combine_expression(result))->simplify();
	if (compact_constant_fields(simply, result))
		return (new bit_combine_expression(result))->simplify();

	return const_cast<bit_combine_expression*>(this);
}

type *
bit_combine_expression::get_type() const
{
	return m_type;
}

bool
bit_combine_expression::is_constant() const
{
	for (expression_vector::const_iterator i = m_exprs.begin();
			i != m_exprs.end(); i++) {
		if (!(*i)->is_constant())
			return false;
	}
	return true;
}


expression *
bit_combine_expression::sub_expr(size_t index) const
{
	if (index >= m_exprs.size())
		return 0;

	return m_exprs[index];
}

bool
bit_combine_expression::evaluate_as_integer(uint64_t &value, bool sign) const
{
	if (!is_constant())
		return false;

	return false;
}

bool
bit_combine_expression::evaluate_as_float(double &) const
{
	assert(0 && "Cannot bit combine floating point values");
	return false;
}

expression *
bit_combine_expression::expand() const
{
	return const_cast<bit_combine_expression*>(this);
}

void
bit_combine_expression::init(expression *expr, ...)
{
	va_list ap;

	va_start(ap, expr);
	init(expr, ap);
	va_end(ap);
}

void
bit_combine_expression::init(expression *expr, va_list ap)
{
	expression_vector exprs;

	assert(expr->get_type()->get_type_id() == type::INTEGER &&
			"Only integer type can be bit combined.");

	exprs.push_back(expr);

	while ((expr = va_arg(ap, expression *)) != 0) {
		assert(expr->get_type()->get_type_id() == type::INTEGER &&
				"Only integer type can be bit combined.");

		exprs.push_back(expr);
	}

	init(exprs);
}

size_t
bit_combine_expression::add_exprs(expression_vector const &exprs)
{
	size_t bits = 0;
	for (expression_vector::const_iterator i = exprs.begin();
			i != exprs.end(); i++) {
		
		// if inserting a bit combine operation, merge.
		if ((*i)->get_expression_operation() == BIT_COMBINE) {
			bit_combine_expression *bc = (bit_combine_expression *)(*i);
			bits += add_exprs(bc->m_exprs);
		} else {
			m_exprs.push_back(*i);
			bits += (*i)->get_type()->get_bits();
		}
	}
	return bits;
}

void
bit_combine_expression::init(expression_vector const &exprs)
{
	m_exprs.clear();
	size_t bits = add_exprs(exprs);

	if (bits == 0)
		m_type = 0;
	else
		m_type = type::get_integer_type(bits);
}

void
bit_combine_expression::replace_sub_expr(size_t index, expression *expr)
{
	assert(index < m_exprs.size());
	m_exprs[index] = expr;
}

//
// simplify unary operations COM and NEG.
//
expression *
bit_combine_expression::simplify_unary(
		unary_expression::operation const &operation) const
{
	if (operation == unary_expression::COM ||
			operation == unary_expression::NEG) {
		expression_vector result;
		for (expression_vector::const_reverse_iterator i = m_exprs.rbegin();
				i != m_exprs.rend(); i++) {
			if (operation == unary_expression::NEG &&
					i == m_exprs.rbegin())
				result.insert(result.begin(), CNEG(*i));
			else
				result.insert(result.begin(), CCOM(*i));
		}

		return (new bit_combine_expression(result))->simplify();
	}
	return const_cast<bit_combine_expression*>(this);
}

//
// simplify binary operations, most importantly:
// AND, OR, XOR, SHL, SHR, ROL, ROR.
//
// to be done:
// ADD, SUB, MUL, DIV, REM.
//

static inline size_t
ilog2(uint64_t n)
{ 
  size_t i = 0;
  if (n & 0xffffffff00000000ULL) i  = 32, n >>= 32;
  if (n & 0xffff0000)            i |= 16, n >>= 16; 
  if (n & 0xff00)                i |= 8,  n >>= 8; 
  if (n & 0xf0)                  i |= 4,  n >>= 4; 
  if (n & 0xc)                   i |= 2,  n >>= 2; 
  return (i | (n >> 1)); 
} 

expression *
bit_combine_expression::simplify_binary(
		binary_expression::operation const &operation,
		expression *other) const
{
	assert (this != other);

	// fast cases
	switch (operation) {
		case binary_expression::MUL:
		case binary_expression::DIV:
		case binary_expression::REM:
			if (other->is_zero())
				return expression::fromInteger(0U, get_type()->get_bits());
			break;

		case binary_expression::AND:
			if (is_zero() || other->is_zero())
				return expression::fromInteger(0U,
						get_type()->get_bits());
			else if (other->is_constant_value()) {
				// this is a common masking case for casting or
				// bit slicing.
				uint64_t v;
				if (other->evaluate(v, true)) {
					size_t n = 0;
					for (n = 0; n < 64; n++) {
						if ((-1ULL << n) == v)
							return CBITSLICE(
								const_cast<bit_combine_expression*>(this),
								CCONST(n),
								CCONST(other->get_type()->get_bits()));
					}
				}
			}
			break;

		case binary_expression::SHL:
		case binary_expression::SHR:
		case binary_expression::ROR:
		case binary_expression::ROL:
		case binary_expression::ADD:
		case binary_expression::SUB:
		case binary_expression::OR:
			if (other->is_zero())
				return const_cast<bit_combine_expression*>(this);
			break;

		case binary_expression::XOR:
			if (other->is_zero())
				return const_cast<bit_combine_expression*>(this);
			else if (other->is_equal(this))
				return expression::fromInteger(0U, get_type()->get_bits());
			break;

		case binary_expression::EQ:
		case binary_expression::LE:
		case binary_expression::GE:
			if (is_zero() && other->is_zero())
				return expression::fromInteger(1U, 1);
			break;
		
		case binary_expression::NE:
		case binary_expression::LT:
		case binary_expression::GT:
			if (is_zero() && other->is_zero())
				return expression::fromInteger(0U, 1);
			break;

		default:
			break;
	}

	switch (operation) {
		case binary_expression::SHR:
		case binary_expression::SHL:
			return shift_by (operation == binary_expression::SHL, other);

		case binary_expression::AND:
			return and_with (other);
			break;

		case binary_expression::OR:
			return or_with (other);

		case binary_expression::XOR:
			return xor_with (other);

		case binary_expression::ROR:
			return COR(CSHR(const_cast<bit_combine_expression*>(this), other),
					CSHL(const_cast<bit_combine_expression*>(this),
						CSUB(CCONST(get_type()->get_bits()), other)))->simplify();

		case binary_expression::ROL:
			return COR(CSHL(const_cast<bit_combine_expression*>(this), other),
					CSHR(const_cast<bit_combine_expression*>(this),
						CSUB(CCONST(get_type()->get_bits()), other)))->simplify();

		default:
			assert(0 && "This operation is not supported on bit-combining.");
	}

	return 0;
}

expression *
bit_combine_expression::truncate(size_t nbits) const
{
	expression_vector exprs;

	if (nbits >= get_type()->get_bits())
		return const_cast<bit_combine_expression*>(this);

	for (expression_vector::const_reverse_iterator i = m_exprs.rbegin();
			i != m_exprs.rend(); i++) {
		expression *sub = (*i);

		if (nbits < sub->get_type()->get_bits()) {
			exprs.insert(exprs.begin(),
					CBITSLICE(sub, CCONST(0), CCONST(nbits)));
			break;
		} else {
			exprs.insert(exprs.begin(), sub);
			nbits -= sub->get_type()->get_bits();
		}
	}

	return (new bit_combine_expression(exprs))->simplify();
}

void
bit_combine_expression::make_same_layout(bit_combine_expression *&expr1,
		bit_combine_expression *&expr2, size_t result_size)
{
	std::vector<size_t> bits1;
	std::vector<size_t> bits2;
	std::vector<size_t> split1;
	std::vector<size_t> split2;
	std::vector<size_t> zero1;
	std::vector<size_t> zero2;
	size_t              size_expr1 = expr1->get_type()->get_bits();
	size_t              size_expr2 = expr2->get_type()->get_bits();

	// see how both expressions are splitted.
	for (expression_vector::reverse_iterator i = expr1->m_exprs.rbegin();
			i != expr1->m_exprs.rend(); i++)
		bits1.push_back((*i)->get_type()->get_bits());

	for (expression_vector::reverse_iterator i = expr2->m_exprs.rbegin();
			i != expr2->m_exprs.rend(); i++)
		bits2.push_back((*i)->get_type()->get_bits());

	// prepare splits
	size_t n, m;
	ssize_t delta_expr1 = 0;
	ssize_t delta_expr2 = 0;

	for (n = m = 0; n < bits1.size() && m < bits2.size();) {
		size_t size1 = bits1[n] + delta_expr1;
		size_t size2 = bits2[m] + delta_expr2;

		delta_expr1 = 0;
		delta_expr2 = 0;

		if (size1 == size2) {
			split1.push_back(size1);
			split2.push_back(size1);
			n++, m++;
		} else if (size1 < size2) {
			// split expr2
			split1.push_back(size1);
			split2.push_back(size1);
			delta_expr2 = -size1;
			n++;
		} else {
			// split expr1
			split1.push_back(size2);
			split2.push_back(size2);
			delta_expr1 = -size2;
			m++;
		}
	}

	// stuff more zeroes if we need to match the 
	// result size.
	if (size_expr1 < result_size)
		zero1.push_back(result_size - size_expr1);
	if (size_expr2 < result_size)
		zero2.push_back(result_size - size_expr2);

	// create the two new bit combines
	expression_vector exprs1;
	expression_vector exprs2;

	for (size_t n = 0; n < zero1.size(); n++)
		exprs1.insert(exprs1.begin(), CCONSTn(0U, zero1[n]));
	for (size_t n = 0, bits = 0; n < split1.size(); bits += split1[n], n++)
		exprs1.insert(exprs1.begin(), CBITSLICE(expr1, CCONST(bits),
					CCONST(split1[n])));

	for (size_t n = 0; n < zero2.size(); n++)
		exprs2.insert(exprs2.begin(), CCONSTn(0U, zero2[n]));
	for (size_t n = 0, bits = 0; n < split2.size(); bits += split2[n], n++)
		exprs2.insert(exprs2.begin(), CBITSLICE(expr2, CCONST(bits),
					CCONST(split2[n])));

	expr1 = new bit_combine_expression(exprs1);
	expr2 = new bit_combine_expression(exprs2);
}

expression *
bit_combine_expression::shift_by(bool left, expression *other) const
{
	size_t p2 = ilog2(get_type()->get_bits());
	expression *shift;

	// if the type size is a power of 2, use AND.
	if ((1ULL << p2) == get_type()->get_bits())
		shift = CAND(CCAST(type::get_integer_type(32), other),
				CCONST(p2 - 1));
	// otherwise use REM.
	else
		shift = CREM(CCAST(type::get_integer_type(32), other),
				CCONST(get_type()->get_bits()));

	uint64_t nbits = 0;
	if (!shift->evaluate(nbits))
		return 0;

	if (nbits == 0)
		return const_cast<bit_combine_expression*>(this);

	if (nbits >= get_type()->get_bits())
		return expression::fromInteger(0U, get_type()->get_bits());

	expression_vector result;
	if (left)
		shift_left(other, nbits, result);
	else
		shift_right(other, nbits, result);

	if (result.empty())
		return expression::fromInteger(0U, get_type()->get_bits());
	else
		return (new bit_combine_expression(result))->simplify();
}

void
bit_combine_expression::shift_right(expression *other, size_t nbits,
		std::vector<expression*> &result) const
{
	// process all values from lowest to highest.
	size_t bits_to_go = nbits;
	size_t bits_done = 0;

	for (expression_vector::const_reverse_iterator i = m_exprs.rbegin();
			i != m_exprs.rend(); i++) {

		expression *sub = *i;
		size_t sub_bits = sub->get_type()->get_bits();

		if (bits_to_go != 0) {
			size_t bits = std::min(bits_to_go, sub_bits);

			if (sub_bits > bits) {
				result.insert(result.begin(), CBITSLICE(sub,
							CCONST(bits), CCONST(sub_bits - bits)));
				bits_done += sub_bits - bits;
			}

			bits_to_go -= bits;
		} else {
			result.insert(result.begin(), sub);
			bits_done += sub_bits;
		}
	}


	// insert stuffing bits to make the value the same
	// size of the original.
	if (!result.empty() && bits_to_go != 0) {
		result.insert(result.begin(),
				expression::fromInteger(0U, bits_to_go));
		bits_done += bits_to_go;
	}

	// if the result is smaller than ourselves, compute and insert
	// more stuffing bits.
	if (bits_done < get_type()->get_bits())
		result.insert(result.begin(), expression::fromInteger(0U,
					get_type()->get_bits() - bits_done));
}

void
bit_combine_expression::shift_left(expression *other, size_t nbits,
		std::vector<expression*> &result) const
{
	size_t avail = get_type()->get_bits() - nbits;

	// insert stuffing bits.
	result.insert(result.begin(), expression::fromInteger(0U, nbits));

	// insert remaining bits.
	size_t bits_to_go = avail;
	for (expression_vector::const_reverse_iterator i = m_exprs.rbegin();
			i != m_exprs.rend() && bits_to_go != 0; i++) {

		expression *sub = *i;
		size_t sub_bits = sub->get_type()->get_bits();
		size_t bits = std::min(bits_to_go, sub_bits);

		if (sub_bits <= bits)
			result.insert(result.begin(), sub);
		else
			result.insert(result.begin(), CBITSLICE(sub, CCONST(0),
						CCONST(bits)));

		bits_to_go -= bits;
	}
}

expression *
bit_combine_expression::and_with(expression *other) const
{
	if (other->get_expression_operation() != binary_expression::BIT_COMBINE)
		return and_expression(other);
	else
		return and_bit_combine(other);
}

expression *
bit_combine_expression::or_with(expression *other) const
{
	if (other->get_expression_operation() != binary_expression::BIT_COMBINE)
		return or_expression(other);
	else
		return or_bit_combine(other);
}

expression *
bit_combine_expression::xor_with(expression *other) const
{
	if (other->get_expression_operation() != binary_expression::BIT_COMBINE)
		return xor_expression(other);
	else
		return xor_bit_combine(other);
}

expression *
bit_combine_expression::and_expression(expression *other) const
{
	expression_vector exprs;
	exprs.push_back(CCONSTn(0U, get_type()->get_bits() - 
				other->get_type()->get_bits()));
	exprs.push_back(other);

	return and_bit_combine(new bit_combine_expression(exprs));
}

expression *
bit_combine_expression::and_bit_combine(expression *expr) const
{
	bit_combine_expression *other =
		static_cast<bit_combine_expression*>(expr->simplify());

	if (other->get_expression_operation() != binary_expression::BIT_COMBINE)
		return and_expression(expr);

	bit_combine_expression *me = const_cast<bit_combine_expression*>(this);

	// to simplify AND'ing, make both bit_combine have similar layout.
	make_same_layout(me, other, get_type()->get_bits());

	expression_vector exprs;
	size_t nexprs = me->m_exprs.size();
	for (size_t n = 0; n < nexprs; n++)
		exprs.push_back(CAND(me->m_exprs[n], other->m_exprs[n]));

	expression *result = new bit_combine_expression(exprs);

	print_expr(result);

	return result->simplify();
}

expression *
bit_combine_expression::or_expression(expression *other) const
{
	expression_vector exprs;
	exprs.push_back(CCONSTn(0U, get_type()->get_bits() - 
				other->get_type()->get_bits()));
	exprs.push_back(other);

	return or_bit_combine(new bit_combine_expression(exprs));
}

expression *
bit_combine_expression::or_bit_combine(expression *expr) const
{
	bit_combine_expression *other =
		static_cast<bit_combine_expression*>(expr->simplify());

	if (other->get_expression_operation() != binary_expression::BIT_COMBINE)
		return or_expression(expr);

	bit_combine_expression *me = const_cast<bit_combine_expression*>(this);

	// to simplify OR'ing, make both bit_combine have similar layout.
	make_same_layout(me, other, get_type()->get_bits());

	expression_vector exprs;
	size_t nexprs = me->m_exprs.size();
	for (size_t n = 0; n < nexprs; n++)
		exprs.push_back(COR(me->m_exprs[n], other->m_exprs[n]));

	expression *result = new bit_combine_expression(exprs);

	print_expr(result);

	return result->simplify();
}

expression *
bit_combine_expression::xor_expression(expression *other) const
{
	expression_vector exprs;
	exprs.push_back(CCONSTn(0U, get_type()->get_bits() - 
				other->get_type()->get_bits()));
	exprs.push_back(other);

	return xor_bit_combine(new bit_combine_expression(exprs));
}

expression *
bit_combine_expression::xor_bit_combine(expression *expr) const
{
	bit_combine_expression *other =
		static_cast<bit_combine_expression*>(expr->simplify());

	if (other->get_expression_operation() != binary_expression::BIT_COMBINE)
		return xor_expression(expr);

	bit_combine_expression *me = const_cast<bit_combine_expression*>(this);

	// to simplify XOR'ing, make both bit_combine have similar layout.
	make_same_layout(me, other, get_type()->get_bits());

	expression_vector exprs;
	size_t nexprs = me->m_exprs.size();
	for (size_t n = 0; n < nexprs; n++)
		exprs.push_back(CXOR(me->m_exprs[n], other->m_exprs[n]));

	expression *result = new bit_combine_expression(exprs);

	print_expr(result);

	return result->simplify();
}
