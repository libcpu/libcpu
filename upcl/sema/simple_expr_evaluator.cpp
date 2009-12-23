#include "sema/simple_expr_evaluator.h"

using namespace upcl;
using namespace upcl::sema;

simple_expr_evaluator::simple_expr_evaluator()
{
	reset();
}

void
simple_expr_evaluator::reset()
{
	m_usage.clear();
	m_collect = false;
	m_failed = false;
}

void
simple_expr_evaluator::clear()
{
	reset();
	m_vars.clear();
}

void
simple_expr_evaluator::collect_literals()
{
	m_collect = true;
}

void
simple_expr_evaluator::set_var(std::string const &varname, uint64_t v)
{
	m_vars[varname] = v;
}

bool
simple_expr_evaluator::is_used(std::string const &varname) const
{
	string_count_map::const_iterator i = m_usage.find(varname);

	if (i != m_usage.end())
		return (i->second != 0);
	else
		return false;
}

void
simple_expr_evaluator::get_used_literals(string_vector &literals) const
{
	literals.clear();

	for (string_count_map::const_iterator i = m_usage.begin();
			i != m_usage.end(); i++) {
		if (m_vars.find(i->first) == m_vars.end())
			literals.push_back(i->first);
	}
}

bool
simple_expr_evaluator::evaluate(ast::expression const *expr, uint64_t &value)
{
	m_failed = false;
	return eval(expr, value);
}

bool
simple_expr_evaluator::eval(ast::expression const *expr, uint64_t &value)
{
	return eval(expr, value, false);
}

bool
simple_expr_evaluator::eval(ast::expression const *expr, uint64_t &value,
		bool sign)
{
	switch (expr->get_expression_type()) {
		case ast::expression::LITERAL:
			return eval_literal((ast::literal_expression const *)expr, value);
		case ast::expression::CAST:
			return eval_cast((ast::cast_expression const *)expr, value, sign);
		case ast::expression::UNARY:
			return eval_unary((ast::unary_expression const *)expr, value, sign);
		case ast::expression::BINARY:
			return eval_binary((ast::binary_expression const *)expr, value,
					sign);
		default:
			return false;
	}
}

bool
simple_expr_evaluator::eval_literal(ast::literal_expression const *expr,
		uint64_t &value)
{
	ast::token const *literal = expr->get_literal();

	switch (literal->get_token_type()) {
		case ast::token::IDENTIFIER:
			return eval_identifier((ast::identifier const *)literal, value);


		case ast::token::QUALIFIED_IDENTIFIER:
		{
			ast::qualified_identifier const *qi =
			 	(ast::qualified_identifier const *)literal;
			if (qi->get_identifier_list() != 0 &&
					qi->get_identifier_list()->size() != 0)
				return false;
			else
				literal = qi->get_base_identifier();

			return eval_identifier((ast::identifier const *)literal, value);
		}

		case ast::token::NUMBER:
			return eval_number((ast::number const *)literal, value);

		default:
			//fprintf(stderr, "error: only numbers are valid in constant expression.\n");
			return false;
	}
}

bool
simple_expr_evaluator::eval_cast(ast::cast_expression const *expr,
		uint64_t &value, bool sign)
{
	ast::type const *t = expr->get_type();

	// only casts to integer are valid.
	if (t->get_value()[1] != 'i')
		return false;

	size_t bits = atoi(t->get_value().c_str()+2);

	if (!eval(expr->get_expression(), value, false))
		return false;

	value &= (1 << bits) - 1;
	if (sign && (1 << (bits - 1)))
		value |= -1ULL << bits;

	return true;
}

bool
simple_expr_evaluator::eval_unary(ast::unary_expression const *expr,
		uint64_t &value, bool)
{
	switch (expr->get_expression_operator()) {
		case ast::unary_expression::SUB:
			if (!eval(expr->get_expression(), value))
				return false;
			break;

		case ast::unary_expression::NEG:
			if (!eval(expr->get_expression(), value))
				return false;
			value = -value;
			break;

		case ast::unary_expression::COM:
			if (!eval(expr->get_expression(), value))
				return false;
			value = ~value;
			break;

		case ast::unary_expression::NOT:
			if (!eval(expr->get_expression(), value))
				return false;
			value = !value;
			break;

		case ast::unary_expression::SIGNED:
			if (!eval(expr->get_expression(), value, true))
				return false;
			break;

		case ast::unary_expression::UNSIGNED:
			if (!eval(expr->get_expression(), value, false))
				return false;
			break;

		default:
			fprintf(stderr, "error: operation is not supported in constant expression.\n");
			return false;
	}

	return true;
}

bool
simple_expr_evaluator::eval_binary(ast::binary_expression const *expr,
		uint64_t &value, bool sign)
{
	uint64_t a, b;

	if (!eval(expr->get_expression1(), a))
		return false;
	if (!eval(expr->get_expression2(), b))
		return false;

	switch (expr->get_expression_operator()) {
		case ast::binary_expression::ADD:
			if (sign) value = (int64_t)a + (int64_t)b;
			else value = a + b;
			return true;
		case ast::binary_expression::SUB:
			if (sign) value = (int64_t)a - (int64_t)b;
			else value = a - b;
			return true;
		case ast::binary_expression::MUL:
			if (sign) value = (int64_t)a * (int64_t)b;
			else value = a * b;
			return true;
		case ast::binary_expression::DIV:
			if (b == 0) {
				fprintf(stderr, "error: division by zero.\n");
				return false;
			}
			if (sign) value = (int64_t)a / (int64_t)b;
			else value = a / b;
			return true;
		case ast::binary_expression::MOD:
			if (b == 0) {
				fprintf(stderr, "error: division by zero.\n");
				return false;
			}
			if (sign) value = (int64_t)a % (int64_t)b;
			else value = a % b;
			return true;
		case ast::binary_expression::SHL:
			if (b > 63) {
				fprintf(stderr, "error: shift is too large.\n");
				return false;
			}
			value = a << b;
			return true;
		case ast::binary_expression::SHR:
			if (b > 63) {
				fprintf(stderr, "error: shift is too large.\n");
				return false;
			}
			if (sign) value = (int64_t)a >> b;
			else value = a >> b;
			return true;
		case ast::binary_expression::OR:
			value = a | b;
			return true;
		case ast::binary_expression::ORCOM:
			value = a | ~b;
			return true;
		case ast::binary_expression::AND:
			value = a & b;
			return true;
		case ast::binary_expression::ANDCOM:
			value = a & ~b;
			return true;
		case ast::binary_expression::XOR:
			value = a ^ b;
			return true;
		case ast::binary_expression::XORCOM:
			value = a ^ ~b;
			return true;
		default:
			fprintf(stderr, "error: operation is not supported in constant expression.\n");
			return false;
	}


	return true;
}

bool
simple_expr_evaluator::eval_identifier(ast::identifier const *ident,
		uint64_t &value)
{
	string_uint_map::const_iterator i = m_vars.find(ident->get_value());

	if (i != m_vars.end()) {
		m_usage[ident->get_value()]++;
		value = i->second;
		return true;
	} else if (m_collect) {
		m_usage[ident->get_value()]++;
		value = 0;
		m_failed = true;
		return true;
	}

	return false;
}

bool
simple_expr_evaluator::eval_number(ast::number const *number, uint64_t &value)
{
	std::string const &v = number->get_value();

	value = 0;
	if (v[0] == '0') {
		if (v.empty())
			return true;

		char const *cs = v.c_str() + 1;
		size_t len = v.length() - 1;

		if (tolower(*cs) == 'b') { // base 2
			if (len - 1 > 64)
				fprintf(stderr, "warning: binary constant is too large.\n");

			value = strtoull(cs + 1, NULL, 2);
		} else if (tolower(*cs) == 'x') { // base 16
			if (len - 1 > 16)
				fprintf(stderr, "warning: hexadecimal constant is too large.\n");

			value = strtoull(cs + 1, NULL, 16);
		} else { // base 8
			if (len > 24)
				fprintf(stderr, "warning: octal constant is too large.\n");

			value = strtoull(cs, NULL, 8);
		}
	} else { // base 10
		if (v.length() > 20)
			fprintf(stderr, "warning: decimal constant is too large.\n");

		value = strtoull(v.c_str(), NULL, 10);
	}

	return true;
}
