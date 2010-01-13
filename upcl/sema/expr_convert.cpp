#include "sema/expr_convert.h"
#include "sema/convert.h"

#include "c/fast_aliases.h"

#include <cassert>

using namespace upcl;
using namespace upcl::sema;
using namespace upcl::c;

expr_convert::expr_convert(expr_convert_lookup *lookup)
	: m_lookup(lookup)
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
		case ast::expression::CALL: // XXX
			return CCONST(0);//convert_call((ast::call_expression const *)expr);
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

		case ast::token::QUALIFIED_IDENTIFIER:
			return convert_qualified_identifier(
					(ast::qualified_identifier const *)literal);

		case ast::token::NUMBER:
			return convert_number((ast::number const *)literal);

		default:
			assert(0 && "IMPLEMENT ME!");
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

		case ast::binary_expression::ROL:
			return CROL(a, b);

		case ast::binary_expression::ROR:
			return CROR(a, b);

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

		case ast::binary_expression::EQ:
			return CEQ(a, b);

		case ast::binary_expression::NE:
			return CNE(a, b);

		case ast::binary_expression::LT:
			return CLT(a, b);

		case ast::binary_expression::LE:
			return CLE(a, b);

		case ast::binary_expression::GT:
			return CGT(a, b);

		case ast::binary_expression::GE:
			return CGE(a, b);

		default:
			assert(0 && "IMPLEMENT ME! (binary)");
			break;
	}

	return 0;
}

c::expression *
expr_convert::convert_identifier(ast::identifier const *ident)
{
	if (m_lookup == 0)
		return 0;

	return m_lookup->expr_convert_lookup_identifier(ident->get_value());
}

c::expression *
expr_convert::convert_qualified_identifier(ast::qualified_identifier const *ident)
{
	if (m_lookup == 0)
		return 0;

	ast::identifier const *base = ident->get_base_identifier();
	ast::token_list const *sub_ids = ident->get_identifier_list();

	if (sub_ids == 0)
		return m_lookup->expr_convert_lookup_identifier(base->get_value());

	size_t count = sub_ids->size();
	if (count == 1)
		return m_lookup->expr_convert_lookup_identifier(base->get_value(),
				((ast::identifier const *)(*sub_ids)[0])->get_value());

	// build a bit combine expression
	c::expression_vector exprs;
	for (size_t n = 0; n < count; n++) {
		c::expression *expr;

		expr = m_lookup->expr_convert_lookup_identifier(base->get_value(),
				((ast::identifier const *)(*sub_ids)[n])->get_value());
		if (expr == 0)
			return 0;
		
		exprs.push_back(expr);
	}

	return c::expression::BitCombine(exprs);
}


c::expression *
expr_convert::convert_number(ast::number const *number)
{
	uint64_t value;
	
	if (!convert_integer(number, value))
		return 0;

	return CCONST(value);
}

c::expression *
expr_convert::convert_call(ast::call_expression const *expr)
{
	ast::identifier const *name = expr->get_name();

	// special builtin instrinsics:
	//   @call( identifier, [ expression [ , ... ] ] )
	//   @trap( expression, [ expression [ , ... ] ] )
	//   @eval_cc( decoder operand 'ccflags' or constant expression )
	//   @debug() - enter debugger
	
	if (name->get_value() == "eval_cc")
	{}

	return 0;
}
