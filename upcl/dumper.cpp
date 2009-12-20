#include <cstdio>
#include <cassert>

#include "dumper.h"

using namespace upcl::ast;

bool
dumper::dump_range(range const *range)
{
	if (!dump_expression(range->get_a()))
		return false;
	if (range->is_distance())
		printf(" : ");
	else
		printf(" .. ");
	if (!dump_expression(range->get_b()))
		return false;
	return true;
}

bool
dumper::dump_expression(expression const *e)
{
	assert(e->get_token_type() == token::EXPRESSION
			&& "Trying to dump a non-expression");

	switch (e->get_expression_type()) {

		case expression::LITERAL:
			return dump_literal_expression((literal_expression const *)e);

		case expression::UNARY:
			return dump_unary_expression((unary_expression const *)e);

		case expression::BINARY:
			return dump_binary_expression((binary_expression const *)e);

		case expression::CALL:
			return dump_call_expression((call_expression const *)e);

		case expression::CAST:
			return dump_cast_expression((cast_expression const *)e);

		case expression::MEMORY:
			return dump_memory_expression((memory_expression const *)e);

		case expression::BIT_COMBINE:
			return dump_bit_combine_expression((bit_combine_expression const *)e);

		case expression::BIT_SLICE:
			return dump_bit_slice_expression((bit_slice_expression const *)e);

		default:
			assert(0 && "Not yet implemented.");
			break;
	}

	return false;
}

bool
dumper::dump_literal_string(string const *literal)
{
	printf("%s", literal->get_value().c_str());
	return true;
}

bool
dumper::dump_literal_number(number const *literal)
{
	printf("%s", literal->get_value().c_str());
	return true;
}

bool
dumper::dump_literal_type(type const *literal)
{
	printf("%s", literal->get_value().c_str());
	return true;
}

bool
dumper::dump_literal_identifier(identifier const *literal)
{
	printf("%s", literal->get_value().c_str());
	if (literal->is_repeat()) {
		repeat_identifier const *ri = (repeat_identifier const *)literal;
		expression const *expr = ri->get_expression();
		if (expr != 0) {
			printf(":");
			printf("( ");
			if (!dump_expression(expr))
				return false;
			printf(" )");
		}
	}
	return true;
}

bool
dumper::dump_literal_qualified_identifier(qualified_identifier const *literal)
{

	if (!dump_literal_identifier(literal->get_base_identifier()))
		return false;

	token_list const *ids = literal->get_identifier_list();
	size_t count = ids ? ids->size() : 0;

	if (count != 0) {
		printf(".");

		if (count == 1) {
			if(!dump_literal_identifier((identifier const *)(*ids)[0]))
				return false;
		} else {
			printf("[ ");
			for(size_t n = 0; n < count; n++) {
				identifier const *id = (identifier const *)(*ids)[n];

				if (n != 0)
					printf(", ");

				if(!dump_literal_identifier(id))
					return false;

			}
			printf(" ]");
		}
	}

	return true;
}

bool
dumper::dump_literal_expression(literal_expression const *e)
{
	token const *literal = e->get_literal();
	switch (literal->get_token_type()) {
		case token::STRING:
			return dump_literal_string((string const *)literal);

		case token::NUMBER:
			return dump_literal_number((number const *)literal);

		case token::TYPE:
			return dump_literal_type((type const *)literal);

		case token::IDENTIFIER:
			return dump_literal_identifier((identifier const *)literal);

		case token::QUALIFIED_IDENTIFIER:
			return dump_literal_qualified_identifier(
				(qualified_identifier const *)literal);

		default:
			assert(0 && "Not yet implemented.");
			break;
	}
	return false;
}


bool
dumper::dump_unary_expression(unary_expression const *e)
{
	unary_expression::unary_expression_operator oper =
		e->get_expression_operator();

	if (oper == unary_expression::SUB ||
		oper == unary_expression::SIGNED || 
		oper == unary_expression::UNSIGNED) {

		if (oper == unary_expression::SIGNED)
			printf("%%S ");
		else if (oper == unary_expression::UNSIGNED)
			printf("%%U ");

		printf("( ");
		if (!dump_expression(e->get_expression()))
			return false;
		printf(" )");
		return true;
	} else switch (oper) {
		case unary_expression::NEG: printf ("-"); break;
		case unary_expression::NOT: printf ("!"); break;
		case unary_expression::COM: printf ("~"); break;

		default:
			assert(0 && "Not yet implemented.");
			break;
	}

	if (!dump_expression(e->get_expression()))
		return false;

	return true;
}

bool
dumper::dump_binary_expression(binary_expression const *e)
{
	if (!dump_expression(e->get_expression1()))
		return false;

	switch (e->get_expression_operator()) {
		case binary_expression::ADD:    printf(" + ");   break;
		case binary_expression::SUB:    printf(" - ");   break;
		case binary_expression::MUL:    printf(" * ");   break;
		case binary_expression::DIV:    printf(" / ");   break;
		case binary_expression::MOD:    printf(" %% ");  break;

		case binary_expression::SHL:    printf(" << ");  break;
		case binary_expression::SHLC:   printf(" ^<< "); break;

		case binary_expression::SHR:    printf(" >> ");  break;
		case binary_expression::SHRC:   printf(" ^>> "); break;

		case binary_expression::OR:     printf(" | ");   break;
		case binary_expression::ORCOM:  printf(" |~ ");  break;

		case binary_expression::AND:    printf(" & ");   break;
		case binary_expression::ANDCOM: printf(" &~ ");  break;

		case binary_expression::XOR:    printf(" ^ ");   break;
		case binary_expression::XORCOM: printf(" ^~ ");  break;

		case binary_expression::LOR:    printf(" || ");  break;
		case binary_expression::LAND:   printf(" && ");  break;
		
		case binary_expression::EQ:     printf(" == ");  break;
		case binary_expression::NE:     printf(" != ");  break;
		case binary_expression::LT:     printf(" < ");   break;
		case binary_expression::LE:     printf(" <= ");  break;
		case binary_expression::GT:     printf(" > ");   break;
		case binary_expression::GE:     printf(" >= ");  break;

		case binary_expression::IS:     printf(" is ");  break;

		default:
			assert(0 && "Not yet implemented.");
			break;
	}

	if (!dump_expression(e->get_expression2()))
		return false;

	return true;
}

bool
dumper::dump_cast_expression(cast_expression const *e)
{
	printf("[ ");
	if (!dump_literal_type(e->get_type()))
		return false;

	printf(" ");

	if (!dump_expression(e->get_expression()))
		return false;
	printf(" ]");
	return true;
}

bool
dumper::dump_call_expression(call_expression const *e)
{
	if (!dump_literal_identifier(e->get_name()))
		return false;

	printf(" ");
	printf("( ");

	token_list const *exprs = e->get_expression_list();
	size_t count = exprs ? exprs->size() : 0;

	for (size_t n = 0; n < count; n++) {
		expression const *expr = (expression const *)(*exprs)[n];

		if (n != 0)
			printf(", ");

		if (!dump_expression(expr))
			return false;
	}
	printf(" )");
	return true;
}

bool
dumper::dump_memory_expression(memory_expression const *e)
{
	type const *type = e->get_type();
	if (type != NULL) {
		if (!dump_literal_type(type))
			return false;
		printf(" ");
	}

	printf("%%M[ ");
	if (!dump_expression(e->get_expression()))
		return false;
	printf(" ]");
	return true;
}

bool
dumper::dump_bit_combine_expression(bit_combine_expression const *e)
{
	token_list const *exprs = e->get_expressions();

	printf("( ");
	size_t count = exprs->size();
	for (size_t n = 0; n < count; n++) {
		expression const *expr = (expression const *)(*exprs)[n];
		if (n != 0)
			printf(" : ");

		if (!dump_expression(expr))
			return false;
	}
	printf(" )");
	return true;
}

bool
dumper::dump_bit_slice_expression(bit_slice_expression const *e)
{
	if (!dump_expression(e->get_expression()))
		return false;
	printf("[ ");
	if (!dump_range(e->get_range()))
		return false;
	printf(" ]");
	return true;
}

bool
dumper::dump_arch_tagged_value(tagged_value const *tv)
{
	char const *tag_name = 0;

	switch (tv->get_tag()) {
		case architecture::NAME:
			tag_name = "name";
			break;

		case architecture::BYTE_SIZE:
			tag_name = "byte_size";
			break;

		case architecture::WORD_SIZE:
			tag_name = "word_size";
			break;

		case architecture::FLOAT_SIZE:
			tag_name = "flat_size";
			break;

		case architecture::ADDRESS_SIZE:
			tag_name = "address_size";
			break;

		case architecture::PSR_SIZE:
			tag_name = "psr_size";
			break;
			
		case architecture::ENDIAN:
			tag_name = "endian";
			break;

		case architecture::DEFAULT_ENDIAN:
			tag_name = "default_endian";
			break;

		case architecture::MIN_PAGE_SIZE:
			tag_name = "min_page_size";
			break;

		case architecture::MAX_PAGE_SIZE:
			tag_name = "max_page_size";
			break;

		case architecture::DEFAULT_PAGE_SIZE:
			tag_name = "default_page_size";
			break;

		case architecture::PAGE_SIZE:
			tag_name = "page_size";
			break;

		default:
			assert(0 && "Not yet implemented.");
			break;
	}


	printf("\tTag = %s\n", tag_name);
	if (tv->is_token()) {
		expression const *e = (expression const *)tv->get_token();
		printf("\tExpression = ");
		assert(e->get_token_type() == token::EXPRESSION &&
				"Only expression valid!");
		if (!dump_expression(e))
			return false;
		printf("\n");
	} else {
		printf("\tValue = %d\n", tv->get_value());
	}

	return true;
}

bool
dumper::dump_register_file(register_file const *rf)
{
	token_list const *groups = rf->get_groups();
	size_t count = (groups ? groups->size() : 0);

	printf("register_file {\n");
	for (size_t n = 0; n < count; n++) {
		register_group const *rg = (register_group const *)(*groups)[n];

		if (!dump_register_group(rg))
			return false;
	}
	printf("}\n");

	return true;
}

bool
dumper::dump_register_group(register_group const *rg)
{
	token_list const *regs = rg->get_registers();

	if (regs == 0 || regs->size() == 0)
		return true;

	printf("\tgroup ");
	if (!dump_literal_identifier(rg->get_name()))
		return false;

	printf(" ");

	size_t count = regs->size();
	if (count == 1) {
		printf (":");
		printf(" ");
	} else
		printf ("{\n\t\t");

	for (size_t n = 0; n < count; n++) {
		register_declaration const *rd = (register_declaration *)(*regs)[n];

		if (n != 0)
			printf(",\n\t\t");

		if (!dump_register_declaration(rd))
			return false;
	}

	if (regs->size() == 1)
		printf(";\n");
	else
		printf ("\n\t}\n");

	return true;
}

bool
dumper::dump_register_declaration(register_declaration const *rd)
{
	expression const *rep = rd->get_repeat();
	token const *binding = rd->get_binding();

	printf("[ ");
	if (rep != 0) {
		if (!dump_expression(rep))
			return false;
		printf (" ** ");
	}
	if (!dump_register_type_name(rd->get_def()))
		return false;

	if (binding != 0) {
		switch (binding->get_token_type()) {
			case token::REGISTER_BINDER:
				printf(" -> ");
				if (!dump_register_binder((register_binder const *)binding))
					return false;
				break;

			case token::REGISTER_SPLITTER1:
			case token::REGISTER_SPLITTER2:
				printf(" -> ");
				if (!dump_register_splitter((register_splitter const *)binding))
					return false;
				break;

			case token::ALIAS_VALUE:
				if (!dump_alias_value((alias_value const *)binding))
					return false;
				break;

			default:
				assert(0 && "Not yet implemented.");
				break;
		}
	}
	printf(" ]");
	return true;
}

bool
dumper::dump_register_type_name(register_type_name const *rtn)
{
	if (!dump_literal_type(rtn->get_type()))
		return false;

	printf(" ");

	if (!dump_literal_identifier(rtn->get_name()))
		return false;

	return true;
}

bool
dumper::dump_register_binder(register_binder const *rb)
{
	token const *splitter = rb->get_splitter();
	if (!dump_literal_identifier(rb->get_register()))
		return false;

	if (splitter != 0) {
		printf(" ");
		switch (splitter->get_token_type()) {
			case token::REGISTER_SPLITTER1:
			case token::REGISTER_SPLITTER2:
				if (!dump_register_splitter((register_splitter const *)splitter))
					return false;
				break;

			default:
				assert(0 && "Not yet implemented.");
				return false;
		}
	}

	return true;
}

bool
dumper::dump_typed_bound_value(typed_bound_value const *tbv)
{
	type const *typ = tbv->get_type();

	if (typ != 0) {
		if (!dump_literal_type(typ))
			return false;

		printf(" ");
	}

	token_list const *sub_values = tbv->get_values();
	size_t sub_count = (sub_values ? sub_values->size() : 0);
	if (sub_count == 0)
		return false;

	if (sub_count != 1) printf("[ ");

	for (size_t m = 0; m < sub_count; m++) {
		token const *sub_value = (*sub_values)[m];

		if (m != 0)
			printf(", ");

		switch (sub_value->get_token_type()) {
			case token::BOUND_VALUE:
				if (!dump_bound_value((bound_value const *)sub_value))
					return false;
				break;

			case token::TYPED_BOUND_VALUE:
				if (!dump_typed_bound_value(
							(typed_bound_value const *)sub_value))
					return false;
				break;

			default:
				assert(0 && "Not yet implemented.");
		}
	}
	
	if (sub_count != 1) printf(" ]");

	return true;
}

bool
dumper::dump_register_splitter(register_splitter const *rs)
{
	type const *type = rs->get_type();
	if (type != 0) {
		if (!dump_literal_type(type))
			return false;

		printf(" ");
	}

	if (rs->is_explicit()) {
		printf("explicit");
		printf(" ");
	}

	expression const *eval = rs->get_evaluate();
	if (eval != 0) {
		printf("evaluate");
		printf(" ");
		printf("( ");
		if (!dump_expression(eval))
			return false;

		printf(" )");
		printf(" ");
	}

	token_list const *values = rs->get_values();
	size_t count = (values ? values->size() : 0);
	if (count == 0)
		return false;

	if (rs->get_token_type() == token::REGISTER_SPLITTER1) {
		printf("[ ");
		for (size_t n = 0; n < count; n++) {
			typed_bound_value const *tbv = (typed_bound_value const *)(*values)[n];

			if (n != 0)
				printf(", ");

			if (!dump_typed_bound_value(tbv))
				return false;
		}
		printf(" ]");
	} else {
		printf("( ");
		for (size_t n = 0; n < count; n++) {
			bound_value const *bv = (bound_value const *)(*values)[n];

			if (n != 0)
				printf(" : ");

			if (!dump_bound_value(bv))
				return false;
		}

		printf(" )");
	}

	return true;
}

bool
dumper::dump_bound_value(bound_value const *bv)
{
	assert(bv->get_token_type() == token::BOUND_VALUE);

	identifier const *name = bv->get_name();
	token const *value = bv->get_binding();
	if (name != 0) {
		if (!dump_literal_identifier(name))
			return false;
		if (bv->is_bidi())
			printf(" <-> ");
		else if (value->get_token_type() == token::IDENTIFIER)
			printf(" -> ");
		else
			printf(" <- ");

		switch (value->get_token_type()) {
			case token::IDENTIFIER:
				if (!dump_literal_identifier((identifier const *)value))
					return false;
				break;

			case token::QUALIFIED_IDENTIFIER:
				if (!dump_literal_qualified_identifier((qualified_identifier const *)value))
					return false;
				break;

			case token::EXPRESSION:
				printf("( ");
				if (!dump_expression((expression const *)value))
					return false;
				printf(" )");
				break;

			default:
				assert(0 && "Not yet implemented.");
				return false;
		}
	} else if (value != 0 && value->get_token_type() == token::EXPRESSION) {
		if (!dump_expression((expression const *)value))
			return false;
	} else {
		assert(0 && "Not yet implemented.");
		return false;
	}

	return true;
}

bool
dumper::dump_alias_value(alias_value const *av)
{
	if (av->is_bidi())
		printf(" <-> ");
	else
		printf(" <- ");

	switch (av->get_alias()->get_token_type()) {
		case token::EXPRESSION:
			if (!dump_expression((expression const *)av->get_alias()))
				return false;
			break;

		case token::IDENTIFIER:
			if (!dump_literal_identifier((identifier const *)av->get_alias()))
				return false;
			break;

		default:
			assert(0 && "Not yet implemented.");
			return false;
	}

	return true;
}

bool
dumper::dump_architecture(architecture const *arch)
{
	token_list const *def = arch->get_def();
	string const *name = arch->get_name();

	printf("Architecture Name = %s\n", name->get_value().c_str());

	size_t count = def->size();
	for (size_t n = 0; n < count; n++) {
		token const *t = (*def)[n];

		switch (t->get_token_type()) {
			case token::TAGGED_VALUE:
				if (!dump_arch_tagged_value((tagged_value const *)t))
					return false;
				break;

			case token::REGISTER_FILE:
				if (!dump_register_file((register_file const *)t))
					return false;
				break;

			default:
				assert(0 && "Not yet implemented.");
				break;
		}
	}

	return false;
}

void
dumper::dump(token_list const *root)
{
	// root shall be a token list
	assert(root->get_token_type() == token::LIST && "root shall be a list.");

	size_t count = root->size();
	for (size_t n = 0; n < count; n++) {
		token const *t = (*root)[n];

		switch (t->get_token_type()) {
			case token::ARCHITECTURE:
				if (!dump_architecture((architecture const *)t))
					return;

				break;

			default:
				assert(0 && "Not yet implemented.");
				break;
		}
	}
}

