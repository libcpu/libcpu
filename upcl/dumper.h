#ifndef __upcl_ast_dumper_h
#define __upcl_ast_dumper_h

#include "ast.h"

namespace upcl { namespace ast {

class dumper {

public:
    void dump(token_list const *root);

private:
    bool dump_architecture(architecture const *arch);
    bool dump_arch_tagged_value(tagged_value const *tv);

private:
	bool dump_expression(expression const *e);
	bool dump_unary_expression(unary_expression const *e);
	bool dump_binary_expression(binary_expression const *e);
	bool dump_literal_expression(literal_expression const *e);
	bool dump_cast_expression(cast_expression const *e);
	bool dump_call_expression(call_expression const *e);
	bool dump_memory_expression(memory_expression const *e);
	bool dump_bit_combine_expression(bit_combine_expression const *e);
	bool dump_bit_slice_expression(bit_slice_expression const *e);

private:
	bool dump_literal_string(string const *literal);
	bool dump_literal_number(number const *literal);
	bool dump_literal_type(type const *literal);
	bool dump_literal_identifier(identifier const *literal);
	bool dump_literal_qualified_identifier(qualified_identifier const *literal);
	bool dump_range(range const *range);

private:
	bool dump_register_file(register_file const *rf);
	bool dump_register_group(register_group const *rg);
	bool dump_register_declaration(register_declaration const *rd);
	bool dump_register_type_name(register_type_name const *rtn);
	bool dump_register_binder(register_binder const *rb);
	bool dump_register_splitter(register_splitter const *rs);
	bool dump_bound_value(bound_value const *bv);
	bool dump_typed_bound_value(typed_bound_value const *tbv);
	bool dump_alias_value(alias_value const *av);
};

} }

#endif  // !__upcl_ast_dumper_h
