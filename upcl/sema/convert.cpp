#include "sema/convert.h"
#include "sema/expr_convert.h"

namespace upcl { namespace sema {

c::type *
convert_type(ast::type const *ast_type)
{
	std::string name(ast_type->get_value());
	int size = atoi(name.c_str()+2);

	switch (name[1]) {
		case 'i':
			return c::type::get_integer_type(size);
		case 'f':
			return c::type::get_float_type(size);
		case 'v':
			return c::type::get_vector_type(size);
		default:
			fprintf(stderr, "error: character '%c' does not express a valid data type.\n",
					name[1]);
			return 0;
	}
}

c::expression *
convert_expression(ast::expression const *expr)
{
	return expr_convert().convert(expr);
}

static bool
convert_integer(std::string const &v, uint64_t &value)
{
	value = 0;
	if (v[0] == '0') {
		if (v.empty())
			return true;

		char const *cs = v.c_str() + 1;
		size_t len = v.length() - 1;

		if (tolower(*cs) == 'b') { // base 2
			if (*++cs == '\0')
				return false;

			// skip leading zeroes
			while (*cs == '0')
				cs++, len--;

			if (len - 1 > 64)
				fprintf(stderr, "warning: binary constant is too large.\n");

			value = strtoull(cs, NULL, 2);
		} else if (tolower(*cs) == 'x') { // base 16
			if (*++cs == '\0')
				return false;

			// skip leading zeroes
			while (*cs == '0')
				cs++, len--;

			if (len - 1 > 16)
				fprintf(stderr, "warning: hexadecimal constant is too large.\n");

			value = strtoull(cs, NULL, 16);
		} else { // base 8
			// skip leading zeroes
			while (*cs == '0')
				cs++, len--;

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

bool
convert_integer(ast::number const *v, uint64_t &value)
{
	return convert_integer(v->get_value(), value);
}

} }
