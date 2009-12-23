#include <cassert>
#include <cstdio>
#include <cmath>

#include "c/binary_expression.h"
#include "c/integer_expression.h"
#include "c/float_expression.h"
#include "c/bit_combine_expression.h"
#include "c/fast_aliases.h"

using namespace upcl;
using namespace upcl::c;

binary_expression::binary_expression(binary_expression::operation const &op,
		expression *expr1, expression *expr2)
	: expression(BINARY), m_op(op), m_type(0), m_expr1(expr1), m_expr2(expr2)
{
	switch (get_operation()) {
		// relational operator are 1bit
		case EQ: case NE:
		case LE: case LT:
		case GE: case GT:
			m_type = c::type::get_integer_type(1);

		default:
			break;
	}
}

binary_expression::~binary_expression()
{
}

bool
binary_expression::is_constant() const
{
	return (m_expr2->is_constant() && m_expr1->is_constant());
}

type *
binary_expression::get_type() const
{
	if (m_type != 0)
		return m_type;

	if (get_operation() == AND) {
		// AND resolve to least significant.
		if (m_expr1->get_type()->get_bits() < m_expr2->get_type()->get_bits())
			return m_expr1->get_type();
		else
			return m_expr2->get_type();
	} else {
		// otherwise assume RHS is the most significant.
		return m_expr1->get_type();
	}
}

expression *
binary_expression::sub_expr(size_t index) const
{
	if (index == 0)
		return m_expr1;
	else if (index == 1)
		return m_expr2;
	else
		return 0;
}

bool
binary_expression::evaluate_as_integer(uint64_t &result, bool sign) const
{
	uint64_t value1, value2;

	if (!m_expr1->evaluate_as_integer(value1, sign))
		return false;

	if (!m_expr2->evaluate_as_integer(value2, sign))
		return false;

	expression *rv = 0;
	switch (m_op) {
		case ADD:
			rv = new integer_expression(value1 + value2, get_type());
			break;

		case SUB:
			rv = new integer_expression(value1 - value2, get_type());
			break;

		case MUL:
			rv = new integer_expression(value1 * value2, get_type());
			break;

		case DIV:
			if (value2 == 0)
				value1 = 0;
			else if (sign)
				rv = new integer_expression((int64_t)value1 / (int64_t)value2,
					get_type());
			else
				rv = new integer_expression(value1 / value2, get_type());
			break;

		case REM:
			if (value2 == 0)
				value1 = 0;
			else if (sign)
				rv = new integer_expression((int64_t)value1 % (int64_t)value2,
					get_type());
			else
				rv = new integer_expression(value1 % value2, get_type());
			break;

		case SHL:
			rv = new integer_expression(value1 << value2, get_type());
			break;

		case SHR:
			if (sign)
				rv = new integer_expression((int64_t)value1 >> value2,
						get_type());
			else
				rv = new integer_expression(value1 >> value2, get_type());
			break;

		case ROL:
			rv = new integer_expression((value1 << value2) |
					(value1 >> (get_type()->get_bits() - value2)), get_type());
			break;

		case ROR:
			rv = new integer_expression((value1 >> value2) |
					(value1 << (get_type()->get_bits() - value2)), get_type());
			break;

		case AND:
			rv = new integer_expression(value1 & value2, get_type());
			break;

		case OR:
			rv = new integer_expression(value1 | value2, get_type());
			break;

		case XOR:
			rv = new integer_expression(value1 ^ value2, get_type());
			break;

		case EQ:
			rv = new integer_expression(value1 == value2,
					type::get_integer_type(1));
			break;

		case NE:
			rv = new integer_expression(value1 != value2,
					type::get_integer_type(1));
			break;

		case LT:
			if (sign)
				rv = new integer_expression((int64_t)value1 < (int64_t)value2,
						type::get_integer_type(1));
			else
				rv = new integer_expression(value1 < value2,
						type::get_integer_type(1));
			break;

		case LE:
			if (sign)
				rv = new integer_expression((int64_t)value1 <= (int64_t)value2,
						type::get_integer_type(1));
			else
				rv = new integer_expression(value1 <= value2,
						type::get_integer_type(1));
			break;

		case GT:
			if (sign)
				rv = new integer_expression((int64_t)value1 > (int64_t)value2,
						type::get_integer_type(1));
			else
				rv = new integer_expression(value1 > value2,
						type::get_integer_type(1));
			break;

		case GE:
			if (sign)
				rv = new integer_expression((int64_t)value1 >= (int64_t)value2,
						type::get_integer_type(1));
			else
				rv = new integer_expression(value1 >= value2,
						type::get_integer_type(1));
			break;

		default:
			assert(0 && "Binary operation on integer type not implemented.");
			return false;
	}

	bool success = rv->evaluate_as_integer(result);

	delete rv;

	return success;
}

bool
binary_expression::evaluate_as_float(double &result) const
{
	double value1, value2;

	if (!m_expr1->evaluate_as_float(value1))
		return false;
	if (!m_expr2->evaluate_as_float(value2))
		return false;

	expression *rv = 0;
	switch (m_op) {
		case ADD:
			rv = new float_expression(value1 + value2, get_type());
			break;

		case SUB:
			rv = new float_expression(value1 - value2, get_type());
			break;

		case MUL:
			rv = new float_expression(value1 * value2, get_type());
			break;

		case DIV:
			rv = new float_expression(std::fabs(value2) ? value1 / value2 : 0,
					get_type());
			break;

		case REM:
			rv = new float_expression(std::fabs(value2) ?
					fmod(value1, value2) : 0, get_type());
			break;

		case EQ:
			rv = new integer_expression(value1 == value2, get_type());
			break;

		case NE:
			rv = new integer_expression(value1 != value2, get_type());
			break;

		case LT:
			rv = new integer_expression(value1 < value2, get_type());
			break;

		case LE:
			rv = new integer_expression(value1 <= value2, get_type());
			break;

		case GT:
			rv = new integer_expression(value1 > value2, get_type());
			break;

		case GE:
			rv = new integer_expression(value1 >= value2, get_type());
			break;

		default:
			assert(0 && "Binary operation on float type not implemented.");
			return false;
	}

	bool success = rv->evaluate_as_float(result);

	delete rv;

	return success;
}

bool
binary_expression::is_equal(expression const *expr) const
{
	if (expression::is_equal(expr)) {
		return (((binary_expression const*)expr)->get_operation() ==
		 	get_operation());
	} else if (is_compatible(expr)) {
		//
		// This case handles binary expressions where the operator
		// is transitive, like ADD, AND, OR, XOR, etc.
		//
		if (((binary_expression const*)expr)->get_operation() !=
		 	get_operation())
			return false;

		switch (get_operation()) {
			case binary_expression::ADD:
			case binary_expression::MUL:
			case binary_expression::AND:
			case binary_expression::OR:
			case binary_expression::XOR:
			case binary_expression::EQ:
			case binary_expression::NE:
				return sub_expr(0)->is_equal(expr->sub_expr(1)) &&
					sub_expr(1)->is_equal(expr->sub_expr(0));

			default:
				break;
		}
	}

	return false;
}

#include "c/unary_expression.h"

namespace {

#define UNEX(x) static_cast<unary_expression*>(x)
#define IS_UNEX(x) ((x)->get_expression_operation() == expression::UNARY)

	template <typename T>
	static expression *make_constant_expression(T const &value, type *ty)
	{
		switch (ty->get_type_id()) {
			case type::INTEGER:
				return new integer_expression(value, ty);
			case type::FLOAT:
				return new float_expression(value, ty);
			default:
				assert(0 && "This type is not supported yet.");
				abort();
				return 0;
		}
	}

	static bool is_negative(expression *expr, double &tmp)
	{
		if (expr->evaluate(tmp))
			return (std::fabs(tmp) != 0.0 && tmp < 0.0);

		return false;
	}

	static bool is_negative(expression *expr, uint64_t &tmp)
	{
		if (expr->evaluate(tmp, true) &&
				(tmp & (1ULL << ((expr->get_type()->get_bits() - 1)))) != 0)
			return true;

		return false;
	}

	static bool is_negative(expression *expr, bool eval_integer = false)
	{
		switch (expr->get_expression_operation()) {
			case expression::FLOAT:
				{
					double tmp;
					return is_negative(expr, tmp);
				}

			case expression::INTEGER:
				if (eval_integer) {
					uint64_t tmp;
					return is_negative(expr, tmp);
				}
				break;

			case expression::SIGNED:
				return is_negative(UNEX(expr)->sub_expr(0), true);

			case expression::UNARY:
				return (UNEX(expr)->get_operation() == unary_expression::NEG);

			default:
				break;
		}

		return false;
	}

	template <typename T>
	static expression *simplify_binary_expression(binary_expression const *expr,
			bool sign)
	{
		T value = 0;

		assert(expr != 0);

		binary_expression::operation op = expr->get_operation();
		expression *expr1 = expr->sub_expr(0)->simplify();
		expression *expr2 = expr->sub_expr(1)->simplify();

		bool expr1_constant = expr1->is_constant();
		bool expr2_constant = expr2->is_constant();

		if (expr1_constant && expr2_constant) {
			// if both are constants, return the constant value.
			binary_expression tmp(op, expr1, expr2);
			if (tmp.evaluate(value, sign))
				return make_constant_expression(value, expr->get_type());
		}

		if (expr1->get_expression_operation() == expression::BIT_COMBINE) {
			expression *expr =
				((bit_combine_expression*)expr1)->simplify_binary(op, expr2);
			if (expr != 0)
				return expr;
		} else if (expr2->get_expression_operation() == expression::BIT_COMBINE) {
			expression *expr =
				((bit_combine_expression*)expr2)->simplify_binary(op, expr1);
			if (expr != 0)
				return expr;
		}

		switch (op) {
			case binary_expression::ADD:
				// If LHS is zero, return RHS.
				if (expr1->evaluate(value, sign) && value == 0)
					return expr2;
				// If RHS is zero, return LHS.
				else if (expr2->evaluate(value, sign) && value == 0)
					return expr1;
				break;
	
			case binary_expression::SHL:
			case binary_expression::SHR:
			case binary_expression::ROL:
			case binary_expression::ROR:
				// If LHS is zero, return zero.
				if (expr1->evaluate(value, sign) && value == 0)
					return expression::fromInteger(0ULL,
							expr->get_type()->get_bits());
				// If shifting by zero, return only the expr.
				else if (expr2->evaluate(value, sign) && value == 0)
					return expr1;
				break;

			case binary_expression::SUB:
				// If subtracting from zero, generate NEG.
				if (expr1->evaluate(value, sign) && value == 0)
					return expression::Neg(expr2);
				// If subtracting zero, return expr1.
				else if (expr2->evaluate(value, sign) && value == 0)
					return expr1;
				// If subtracting the same expression, return zero!
				else if (expr1->is_equal(expr2))
					return expression::fromInteger(0ULL,
							expr->get_type()->get_bits());
				break;

			case binary_expression::AND:
				// If AND zero, return zero.
				if ((expr1->evaluate(value, sign) && value == 0) ||
						(expr2->evaluate(value, sign) && value == 0))
					return expression::fromInteger(0ULL, 
							expr->get_type()->get_bits());
				// If ANDing same expr, return the first.
				else if (expr1->is_equal(expr2))
					return expr1;
				break;

			case binary_expression::OR:
				// If OR zero or LHS == RHS, return LHS.
				if (expr2->evaluate(value, sign) && value == 0 ||
						expr1->is_equal(expr2))
					return expr1;
				// Do the same but with RHS.
				else if (expr1->evaluate(value, sign) && value == 0)
					return expr2;
				break;

			case binary_expression::XOR:
				// If XOR zero return the first expression.
				if (expr2->evaluate(value, sign) && value == 0)
					return expr1;
				else if (expr1->evaluate(value, sign) && value == 0)
					return expr2;
				// If XORing the same expression, return zero.
				else if (expr2->is_equal(expr1))
					return expression::fromInteger(0ULL,
							expr->get_type()->get_bits());
				break;

			case binary_expression::EQ:
			case binary_expression::LE:
			case binary_expression::GE:
				// If EQ/LE/GE the same expression, it's true.
				if (expr2->is_equal(expr1))
					return expression::fromInteger(1ULL, 1);
				break;

			default:
				break;
		}

		//
		// Force evaluation of negative integer constants only if both
		// expressions are of the same type.
		//
		bool same_type = expr1->get_type()->is_equal(expr2->get_type());
		bool expr1_negative = is_negative(expr1, same_type);
		bool expr2_negative = is_negative(expr2, same_type);

		// optimize ADD case.
		if (op == binary_expression::ADD) {
			if (expr1_negative) {
				// PATTERN: -expr1 + expr2 >> expr2 - expr1
				if (expr1->evaluate(value, sign)) {
					return (new binary_expression(binary_expression::SUB, 
								expr2, make_constant_expression(-value, 
										expr1->get_type())))->simplify();
				} else if (IS_UNEX(expr1)) {
					return (new binary_expression(binary_expression::SUB, 
								expr2, CNEG(expr1)))->simplify();
				}
			} else if (expr2_negative) {
				// PATTERN: expr1 + -expr2 >> expr1 - expr2
				if (expr2->evaluate(value, sign)) {
					return (new binary_expression(binary_expression::SUB, 
								expr1, make_constant_expression(-value, 
									expr1->get_type())))->simplify();
				} else if (IS_UNEX(expr2)) {
					return (new binary_expression(binary_expression::SUB, 
								expr1, CNEG(expr2)))->simplify();
				}
			}

			// if RHS is constant, override with the computed value.
			if (!expr1->is_constant_value() && expr1->evaluate(value))
				return (new binary_expression(op,
							make_constant_expression(value, expr1->get_type()),
							expr2))->simplify();
			// if LHS is constant, override with the computed value.
			if (!expr2->is_constant_value() && expr2->evaluate(value))
				return (new binary_expression(op, expr1,
							make_constant_expression(value,
								expr2->get_type())))->simplify();
		}

		if (!expr1->is_equal(expr->sub_expr(0)) ||
			!expr2->is_equal(expr->sub_expr(1)))
			return new binary_expression(op, expr1, expr2);
		else
			return const_cast<binary_expression*>(expr);
	}
}

expression *
binary_expression::simplify(bool sign) const
{
	switch (get_type()->get_type_id()) {
		case type::INTEGER:
			return simplify_binary_expression<uint64_t>(this, sign);
		case type::FLOAT:
			return simplify_binary_expression<double>(this, sign);
		default:
			assert(0 && "This expression type is not supported yet.");
			abort();
			return 0;
			break;
	}
}

void
binary_expression::replace_sub_expr(size_t index, expression *expr)
{
	if (index == 0)
		m_expr1 = expr;
	else if (index == 1)
		m_expr2 = expr;
	else
		assert(0 && "Invalid expression index.");
}
