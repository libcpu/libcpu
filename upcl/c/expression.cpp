#include <cassert>
#include <cmath>

#include "c/expression.h"
#include "c/binary_expression.h"
#include "c/unary_expression.h"
#include "c/integer_expression.h"
#include "c/float_expression.h"
#include "c/decoder_operand_expression.h"
#include "c/temp_value_expression.h"
#include "c/register_expression.h"
#include "c/bit_slice_expression.h"
#include "c/bit_combine_expression.h"
#include "c/cast_expression.h"
#include "c/memory_ref_expression.h"
#include "c/type.h"

using namespace upcl;
using namespace upcl::c;

expression::expression(operation const &op)
	: m_expr_op(op), m_flags(0), m_trap(0), m_ccf(0)
{
}

expression::~expression()
{
}

void
expression::update_cc(unsigned ccflags)
{
	m_ccf = ccflags;
}

unsigned
expression::get_update_cc() const
{
	return m_ccf;
}

void
expression::overflow_trap(expression *trap_code)
{
	m_trap = trap_code;
}

expression *
expression::get_overflow_trap_code() const
{
	return m_trap;
}

bool
expression::make_signed()
{
	m_flags |= SIGNED;
	return true;
}

bool
expression::is_signed() const
{
	return (m_flags & SIGNED) != 0;
}

bool
expression::make_float_ordered()
{
	m_flags |= FLOAT_ORDERED;
	return true;
}

bool
expression::is_float_ordered() const
{
	return (m_flags & FLOAT_ORDERED) != 0;
}

expression *
expression::fromRegister(register_def *reg)
{
	return new register_expression(reg);
}

expression *
expression::fromDecoderOperand(decoder_operand_def *decopr)
{
	return new decoder_operand_expression(decopr);
}

expression *
expression::fromTempValue(temp_value_def *decopr)
{
	return new temp_value_expression(decopr);
}

expression *
expression::fromInteger(uint64_t x, unsigned bits)
{
	assert(bits > 0);
	if (bits <= 64)
		return new integer_expression(x, 
			c::type::get_integer_type(bits));

	bits -= 64;

	expression_vector exprs;
	while (bits != 0) {
		size_t nbits = bits >= 64 ? 64 : bits;
		exprs.push_back(fromInteger(0U, nbits));
		bits -= nbits;
	}
	exprs.push_back(fromInteger(x, 64));

	return new bit_combine_expression(exprs);
}

expression *
expression::fromInteger(int64_t x, unsigned bits)
{
	return Signed(fromInteger(static_cast<uint64_t>(x), bits));
}

expression *
expression::fromFloat(double x, unsigned bits)
{
	assert(bits == 32 || bits == 64 || bits == 80 || bits == 128);
	return new float_expression(x, c::type::get_float_type(bits));
}

expression *
expression::Neg(expression *expr)
{
	return new unary_expression(unary_expression::NEG, expr);
}

expression *
expression::Com(expression *expr)
{
	return new unary_expression(unary_expression::COM, expr);
}

expression *
expression::Not(expression *expr)
{
	return new unary_expression(unary_expression::NOT, expr);
}

expression *
expression::Add(expression *a, expression *b)
{
	return new binary_expression(binary_expression::ADD, a, b);
}

expression *
expression::Sub(expression *a, expression *b)
{
	return new binary_expression(binary_expression::SUB, a, b);
}

expression *
expression::Mul(expression *a, expression *b)
{
	return new binary_expression(binary_expression::MUL, a, b);
}

expression *
expression::Div(expression *a, expression *b)
{
	return new binary_expression(binary_expression::DIV, a, b);
}

expression *
expression::Rem(expression *a, expression *b)
{
	return new binary_expression(binary_expression::REM, a, b);
}

expression *
expression::Shl(expression *a, expression *b)
{
	return new binary_expression(binary_expression::SHL, a, b);
}

expression *
expression::ShlC(expression *a, expression *b)
{
	return new binary_expression(binary_expression::SHLC, a, b);
}

expression *
expression::Shr(expression *a, expression *b)
{
	return new binary_expression(binary_expression::SHR, a, b);
}

expression *
expression::ShrC(expression *a, expression *b)
{
	return new binary_expression(binary_expression::SHLC, a, b);
}

expression *
expression::Rol(expression *a, expression *b)
{
	return new binary_expression(binary_expression::ROL, a, b);
}

expression *
expression::RolC(expression *a, expression *b)
{
	return new binary_expression(binary_expression::ROLC, a, b);
}

expression *
expression::Ror(expression *a, expression *b)
{
	return new binary_expression(binary_expression::ROR, a, b);
}

expression *
expression::RorC(expression *a, expression *b)
{
	return new binary_expression(binary_expression::ROLC, a, b);
}

expression *
expression::And(expression *a, expression *b)
{
	return new binary_expression(binary_expression::AND, a, b);
}

expression *
expression::Or(expression *a, expression *b)
{
	return new binary_expression(binary_expression::OR, a, b);
}

expression *
expression::Xor(expression *a, expression *b)
{
	return new binary_expression(binary_expression::XOR, a, b);
}

expression *
expression::Eq(expression *a, expression *b)
{
	return new binary_expression(binary_expression::EQ, a, b);
}

expression *
expression::Ne(expression *a, expression *b)
{
	return new binary_expression(binary_expression::NE, a, b);
}

expression *
expression::Le(expression *a, expression *b)
{
	return new binary_expression(binary_expression::LE, a, b);
}

expression *
expression::Lt(expression *a, expression *b)
{
	return new binary_expression(binary_expression::LT, a, b);
}

expression *
expression::Ge(expression *a, expression *b)
{
	return new binary_expression(binary_expression::GE, a, b);
}

expression *
expression::Gt(expression *a, expression *b)
{
	return new binary_expression(binary_expression::GT, a, b);
}

expression *
expression::BitSlice(expression *expr, expression *first_bit,
		expression *bit_count)
{
	return new bit_slice_expression(expr, first_bit, bit_count);
}

expression *
expression::BitCombine(expression *expr, ...)
{
	va_list ap;
	bit_combine_expression *x = new bit_combine_expression;

	va_start(ap, expr);
	x->init(expr, ap);
	va_end(ap);

	return x;
}

expression *
expression::BitCombine(expression_vector const &exprs)
{
	return new bit_combine_expression(exprs);
}

expression *
expression::Cast(type *ty, expression *expr)
{
	return new cast_expression(ty, expr);
}

expression *
expression::Signed(expression *expr)
{
	expr->make_signed();
	return expr;
}

expression *
expression::UpdateCC(expression *expr, unsigned ccflags)
{
	expr->update_cc(ccflags);
	return expr;
}

expression *
expression::OverflowTrap(expression *expr, expression *trap_code)
{
	expr->overflow_trap(trap_code);
	return expr;
}

expression *
expression::MemoryReference(type *type, expression *location)
{
	return new memory_ref_expression(type, location);
}

expression *
expression::simplify(bool) const
{
	return const_cast <expression *> (this);
}

void
expression::replace_sub_expr(size_t, expression *)
{
	assert(0 && "This method has not been overridden.");
}

bool
expression::is_compatible(expression const *expr) const
{
	if (expr->get_expression_operation() != get_expression_operation())
		return false;

	// loosy type compatibility, some bits may be lost in conversion.
	if (!expr->get_type()->is_equal(get_type())) {
		if (expr->get_type()->get_type_id() != get_type()->get_type_id())
			return false;
	}

	return true;
}

bool
expression::is_equal(expression const *expr) const
{
	if (expr == this)
		return true;

	if (!is_compatible(expr))
		return false;

	for (size_t n = 0; ; n++) {
		expression *sub1 = expr->sub_expr(n);
		expression *sub2 = sub_expr(n);
		
		if (sub1 == 0 || sub2 == 0)
			return (sub1 == sub2);
		else if (!sub1->is_equal(sub2))
			return false;
	}

	return true;
}

bool
expression::is_zero() const
{
	return false;
}

void
expression::replace_type(type *)
{
	assert(0 && "This method has not been overridden.");
}
