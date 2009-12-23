#include <cassert>
#include <cmath>

#include "c/unary_expression.h"
#include "c/integer_expression.h"
#include "c/float_expression.h"
#include "c/bit_combine_expression.h"
#include "c/fast_aliases.h"

using namespace upcl;
using namespace upcl::c;

unary_expression::unary_expression(unary_expression::operation const &op,
		expression *expr)
	: expression(UNARY), m_op(op), m_expr(expr), m_type(0)
{
	// NOT is always 1-bit
	if (op == NOT)
		m_type = type::get_integer_type(1);
}

unary_expression::~unary_expression()
{
}

bool
unary_expression::is_constant() const
{
	return (m_expr->is_constant());
}

type *
unary_expression::get_type() const
{
	return (m_type ? m_type : m_expr->get_type());
}

expression *
unary_expression::sub_expr(size_t index) const
{
	return (index == 0 ? m_expr : 0);
}

bool
unary_expression::evaluate_as_integer(uint64_t &result, bool sign) const
{
	uint64_t value;

	if (!m_expr->evaluate_as_integer(value, sign))
		return false;

	expression *rv = 0;
	switch (m_op) {
		case NOT:
			rv = new integer_expression(!value, get_type());
			break;

		case COM:
			rv = new integer_expression(~value, get_type());
			break;

		case NEG:
			rv = new integer_expression(-value, get_type());
			break;

		default:
			assert(0 && "Unary operation on integer type not implemented.");
			return false;
	}

	bool success = rv->evaluate_as_integer(result);

	delete rv;

	return success;
}

bool
unary_expression::evaluate_as_float(double &result) const
{
	double value;
	if (!m_expr->evaluate_as_float(value))
		return false;

	expression *rv = 0;
	switch (m_op) {
		case NOT:
			rv = new integer_expression(std::fabs(value) != 0.0, get_type());
			break;

		case COM:
			rv = new integer_expression(~llrint(value), get_type());
			break;

		case NEG:
			rv = new float_expression(-value, get_type());
			break;

		default:
			assert(0 && "Unary operation on float type not implemented.");
			return false;
	}

	bool success = rv->evaluate_as_float(result);

	delete rv;

	return success;
}

bool
unary_expression::is_equal(expression const *expr) const
{
	if (expression::is_equal(expr))
		return (((unary_expression const*)expr)->get_operation() ==
		 	get_operation());

	return false;
}

namespace {

	template <typename T>
	expression *make_constant_expression(T const &value, type *ty)
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

#define UNEX(x) static_cast<unary_expression*>(x)
#define IS_UNEX(x) ((x)->get_expression_operation() == expression::UNARY)

	template <typename T>
	static expression *
	simplify_unary_expression(unary_expression const *expr, bool sign)
	{
		T value = 0;

		if (expr->evaluate(value, sign)) {
			expression *r = make_constant_expression(value, expr->get_type());
			if (expr->get_operation() == unary_expression::NEG &&
					r->get_type()->get_type_id() == type::INTEGER)
				r = CSIGN(r);
			return r;
		} else {
			expression *sub_expr = expr->sub_expr(0)->simplify();
			bool do_again = true;

			if (sub_expr->get_expression_operation() ==
					expression::BIT_COMBINE) {
				sub_expr = ((bit_combine_expression *)sub_expr)->
					simplify_unary(expr->get_operation());

				return sub_expr->simplify();
			}

			if (!sub_expr->is_equal(expr->sub_expr(0))) {
				if (sub_expr->is_constant())
					return sub_expr;
				else if (do_again)
					return sub_expr->simplify();
			}

			switch (expr->get_operation()) {
				case unary_expression::COM:
				case unary_expression::NEG:
					if (IS_UNEX(sub_expr)) {

						unary_expression::operation op = 
						 	UNEX(sub_expr)->get_operation();

						if (op == expr->get_operation())
							// COM(COM(x)) => x
							// NEG(NEG(x)) => x
							return sub_expr->sub_expr(0)->simplify();
						else if (op == unary_expression::NEG ||
								op == unary_expression::COM)
							// COM(NEG(x)) => x-1
							// NEG(COM(x)) => x-1
							return CSUB(sub_expr->sub_expr(0),
									CCONST(1))->simplify();
					}
					break;

				default:
					break;
			}


			return const_cast<unary_expression*>(expr);
		}
	}

}

expression *
unary_expression::simplify(bool sign) const
{
	switch (get_type()->get_type_id()) {
		case type::INTEGER:
			return simplify_unary_expression<uint64_t>(this, sign);
		case type::FLOAT:
			return simplify_unary_expression<double>(this, sign);
		default:
			assert(0 && "This expression type is not supported yet.");
			abort();
			return 0;
			break;
	}
}

void
unary_expression::replace_sub_expr(size_t index, expression *expr)
{
	if (index == 0)
		m_expr = expr;
	else
		assert(0 && "Invalid expression index.");
}
