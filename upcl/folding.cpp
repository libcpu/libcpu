#include "c/unary_expression.h"
#include "c/binary_expression.h"

#define UNEX(x) static_cast<unary_expression*>(x)
#define BINEX(x) static_cast<binary_expression*>(x)

//
// evaluates which expr is a binary expression.
//
// if expr1 is a binary expression, expr2 is
// checked to be one of its subexpressions; the
// same happens if expr1 a sub expression of
// expr2.
//

static bool
fold_match_0(expression *expr1, expression *expr2,
		binary_expression::operation const &op,
		size_t &sub_expr_index)
{
	if (expr1->get_expression_operation() == expression::BINARY &&
			BINEX(expr1)->get_operation() == op) {

		for (size_t n = 0; n < 2; n++) {
			if (expr2->is_equal(expr1->sub_expr(n))) {
				sub_expr_index = n;
				return true;
			}
		}

	}

	return false;
}

static bool
fold_match_1(expression *expr1, expression *expr2,
		binary_expression::operation const &op,
		size_t &sub_expr_index, size_t &mode)
{
	mode = 0; // normal
	if (fold_match_0(expr1, expr2, op, sub_expr_index))
		return true;

	// -expr2 is subexpr of expr1
	mode = 1; // negative
	if (fold_match_0(expr1, CNEG(expr2)->simplify(), op, sub_expr_index))
		return true;

	// ~expr2 is subexpr of expr1
	mode = 2; // complement
	if (fold_match_0(expr1, CCOM(expr2)->simplify(), op, sub_expr_index))
		return true;

	return false;
}

static bool
fold_match_2(expression *expr1, expression *expr2,
		binary_expression::operation const &op,
		size_t &expr_index, size_t &sub_expr_index,
		size_t &mode)
{
	// expr2 is subexpr of expr1
	expr_index = 0;
	if (fold_match_1(expr1, expr2, op, sub_expr_index, mode))
		return true;

	// expr1 is subexpr of expr2
	expr_index = 1;
	if (fold_match_1(expr2, expr1, op, sub_expr_index, mode))
		return true;

	return false;
}

static expression *
fold_constants(expression *expr);

static expression *
fold_constants_unary(unary_expression *expr)
{
	expression *sub;

	sub = fold_constants(expr->sub_expr(0));
	if (!sub->is_equal(expr->sub_expr(0)))
		sub = fold_constants(new unary_expression(expr->get_operation(), sub));
	else
		sub = expr;

	return sub;
}

static expression *
fold_constants_binary(binary_expression *expr)
{
	expression *expr1, *expr2;

	expr1 = fold_constants(expr->sub_expr(0));
	expr2 = fold_constants(expr->sub_expr(1));

	binary_expression::operation operation = expr->get_operation();

	size_t i1, i2, i3;
	switch (operation) {
		case binary_expression::ADD:
			// PATTERN:  X + ( X - Y ) || ( X - Y ) +  X => no change.
			// PATTERN: -X + ( X - Y ) || ( X - Y ) + -X => -Y
			// PATTERN: ~X + ( X - Y ) || ( X - Y ) + ~X => ~Y
			// PATTERN:  Y + ( X - Y ) || ( X - Y ) + Y  => X
			// PATTERN: -Y + ( X - Y ) || ( X - Y ) + -Y => -X
			// PATTERN: ~Y + ( X - Y ) || ( X - Y ) + ~Y => ~X
			if (fold_match_2(expr1, expr2, binary_expression::SUB,
						i1, i2, i3)) {
				expression *e = (i1 ? expr2 : expr1)->sub_expr(i2 ^ 1);
				if (i3 == 2) // complement
					return fold_constants(CCOM(e)->simplify());
				else if (i3 == 1) // neative
					return fold_constants(CNEG(e)->simplify());
				else
					return fold_constants(e->simplify());
			}
			break;

		case binary_expression::SUB:
			// PATTERN: X - ( X - Y ) => Y
			if (fold_match_2(expr1, expr2, binary_expression::SUB,
						i1, i2, i3) && i3 == 0 && i1 == 1 && i2 == 0)
				return fold_constants(expr2->sub_expr(1));
			break;

		case binary_expression::AND:
			// PATTERN: (X & Y) &  Y || (X & Y) &  X => (X & Y)
			// PATTERN: (X & Y) & ~Y || (X & Y) & ~X => 0
			if (fold_match_2(expr1, expr2, binary_expression::AND,
						i1, i2, i3)) {
				if (i3 == 2) // if there's a complement, fold always to zero.
					return expression::fromInteger(0ULL,
							expr->get_type()->get_bits());
				else if (i3 != 1) { // if it is non-negative, fold const
					expression *a = i1 ? expr1 : expr2;
					expression *b = (i1 ? expr2 : expr1)->sub_expr(i2 ^ 1);
					return fold_constants(CAND(b, a)->simplify());
				}
			}
			break;

		case binary_expression::OR:
			// PATTERN: (X | Y) |  Y || (X | Y) |  X => (X | Y)
			// PATTERN: (X | Y) | ~Y || (X | Y) | ~X => -1U
			if (fold_match_2(expr1, expr2, binary_expression::OR,
						i1, i2, i3)) {
				if (i3 == 2) // if there's a complement, fold always to -1U.
					return expression::fromInteger(-1ULL,
							expr->get_type()->get_bits());
				else if (i3 != 1) { // if it is non-negative, fold const
					expression *a = i1 ? expr1 : expr2;
					expression *b = (i1 ? expr2 : expr1)->sub_expr(i2 ^ 1);
					return fold_constants(COR(b, a)->simplify());
				}
			}
			break;

		case binary_expression::XOR:
			// PATTERN: (X ^ Y) ^  Y => X
			// PATTERN: (X ^ Y) ^  X => Y
			// PATTERN: (X ^ Y) ^ ~Y => ~X
			// PATTERN: (X ^ Y) ^ ~X => ~Y
			if (fold_match_2(expr1, expr2, binary_expression::XOR,
						i1, i2, i3)) {
				if (i3 != 1) { // if it is non-negative, fold const
					expression *a = i1 ? expr1 : expr2;
					expression *b = (i1 ? expr2 : expr1)->sub_expr(i2 ^ 1);
					if (i3 == 2)
						return a;//CCOM(b)->simplify());
					else
						return fold_constants(b->simplify());
				}
			}
			break;

		default:
			break;
	}

	if (expr1 != expr->sub_expr(0) || expr2 != expr->sub_expr(1))
		return fold_constants(new binary_expression(operation, expr1, expr2));
	else
		return expr;
}

static expression *
fold_constants_bit_combine(bit_combine_expression *expr)
{
	expression_vector exprs;
	expression *sub;
	bool changed = false;

	for (size_t n = 0; (sub = expr->sub_expr(n)) != 0; n++) {
		exprs.push_back(fold_constants(sub));
		if (!exprs.back()->is_equal(sub))
			changed = true;
	}

	if (changed)
		return expression::BitCombine(exprs);
	else
		return expr;
}

expression *
upcl::c::fold_constants_expression(expression *expr)
{
	switch (expr->get_expression_operation()) {
		case expression::UNARY:
			return fold_constants_unary((unary_expression *)expr);
		case expression::BINARY:
			return fold_constants_binary((binary_expression *)expr);
		case expression::BIT_COMBINE:
			return fold_constants_bit_combine((bit_combine_expression *)expr);
		default:
			return expr;
	}
}
