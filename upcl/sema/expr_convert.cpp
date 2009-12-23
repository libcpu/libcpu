#include "sema/expr_convert.h"
#include "sema/convert.h"

#include "c/fast_aliases.h"

#include <cassert>

using namespace upcl;
using namespace upcl::sema;
using namespace upcl::c;

expr_convert::expr_convert()
{
}

c::expression *
expr_convert::convert(ast::expression const *expr)
{
	switch (expr->get_expression_type()) {
		case ast::expression::LITERAL:
			return convert_literal((ast::literal_expression const *)expr);
		case ast::expression::CAST:
			return convert_cast((ast::cast_expression const *)expr);
		case ast::expression::UNARY:
			return convert_unary((ast::unary_expression const *)expr);
		case ast::expression::BINARY:
			return convert_binary((ast::binary_expression const *)expr);
		default:
			assert(0 && "IMPLEMENT ME!");
			return 0;
	}
}

c::expression *
expr_convert::convert_literal(ast::literal_expression const *expr)
{
	ast::token const *literal = expr->get_literal();

	switch (literal->get_token_type()) {
		case ast::token::IDENTIFIER:
			return convert_identifier((ast::identifier const *)literal);

#if 0
		case ast::token::QUALIFIED_IDENTIFIER:
			return convert_qualified_identifier(
					(ast::qualified_identifier const *)literal);
#endif

		case ast::token::NUMBER:
			return convert_number((ast::number const *)literal);

		default:
			return 0;
	}
}

c::expression *
expr_convert::convert_cast(ast::cast_expression const *expr)
{
	c::type *type = convert_type(expr->get_type());
	if (type == 0)
		return 0;

	c::expression *result = convert(expr->get_expression());
	if (result == 0)
		return 0;

	return CCAST(type, result);
}

c::expression *
expr_convert::convert_unary(ast::unary_expression const *expr)
{
	c::expression *result = 0;

	result = convert(expr->get_expression());
	if (result == 0)
		return 0;

	switch (expr->get_expression_operator()) {
		case ast::unary_expression::SUB:
			break;

		case ast::unary_expression::NEG:
			result = CNEG(result);
			break;

		case ast::unary_expression::COM:
			result = CCOM(result);
			break;

		case ast::unary_expression::NOT:
			result = CNOT(result);
			break;

		case ast::unary_expression::SIGNED:
			result = CSIGN(result);
			break;

		case ast::unary_expression::UNSIGNED:
			break;

		default:
			assert(0 && "IMPLEMENT ME (unary)");
			return 0;
	}

	return result;
}

c::expression *
expr_convert::convert_binary(ast::binary_expression const *expr)
{
	c::expression *a, *b;

	a = convert(expr->get_expression1());
	if (a == 0)
		return 0;
	b = convert(expr->get_expression2());
	if (b == 0)
		return 0;

	switch (expr->get_expression_operator()) {
		case ast::binary_expression::ADD:
			return CADD(a, b);

		case ast::binary_expression::SUB:
			return CSUB(a, b);

		case ast::binary_expression::MUL:
			return CMUL(a, b);

		case ast::binary_expression::DIV:
			return CDIV(a, b);

		case ast::binary_expression::MOD:
			return CREM(a, b);

		case ast::binary_expression::SHL:
			return CSHL(a, b);

		case ast::binary_expression::SHR:
			return CSHR(a, b);

		case ast::binary_expression::OR:
			return COR(a, b);

		case ast::binary_expression::ORCOM:
			return COR(a, CCOM(b));

		case ast::binary_expression::AND:
			return CAND(a, b);

		case ast::binary_expression::ANDCOM:
			return CAND(a, CCOM(b));

		case ast::binary_expression::XOR:
			return CXOR(a, b);

		case ast::binary_expression::XORCOM:
			return CXOR(a, CCOM(b));

		default:
			assert(0 && "IMPLEMENT ME! (binary)");
			break;
	}

	return 0;
}

c::expression *
expr_convert::convert_identifier(ast::identifier const *ident)
{
	assert(0 && "NYI");
	return 0;
}


c::expression *
expr_convert::convert_number(ast::number const *number)
{
	uint64_t value;
	
	if (!convert_integer(number, value))
		return 0;

	return CCONST(value);
}
