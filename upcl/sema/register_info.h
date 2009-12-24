#ifndef __upcl_sema_register_info_h
#define __upcl_sema_register_info_h

#include "ast/ast.h"

namespace upcl { namespace sema {

struct register_info;

typedef std::set <register_info *> register_info_set;
typedef std::vector <register_info *> register_info_vector;

//
// this structure is used to resolve dependencies between
// registers in the register file, it's the temporary structure
// holding information extracted from the AST in a simpler
// form to create the final register definitions.
//
struct register_info {
	enum {
		UNRESOLVED_FLAG = 1,
		HARDWIRED_FLAG = 2,
		EXPLICIT_FLAG = 4,
		RESERVED_FLAG = 8,
		SUBREGISTER_FLAG = 16,
		BIDIBIND_FLAG = 32,
		FULLALIAS_FLAG = 64,
		REGALIAS_FLAG = 128
	};

	uint32_t               flags;
	std::string            name;
	ast::type const       *type;
	register_info         *super;           // parent register.
	ast::register_splitter const *splitter; // splitter alas bitfields.
	ast::expression const *hwexpr;          // hardwired expression.
	ast::expression const *special_eval;    // special evaluation function on assignment.
	uint64_t               repeat_index;    // index to be used to compute aliasing
	                                        // register index value.
	ast::expression const *complex_index;   // index to be used to compute aliasing,
	                                        // but it's too complex to be evaluated
										    // constantly, it will done at translation
											// time.
	register_info         *binding;
	ast::qualified_identifier const *bind_copy; // bind as a copy-on-write from the register,
	                                        // if the bound register is written, then a
											// copy of the value is placed into this one.
	register_info_vector   subs;
	register_info_set      deps_on;         // register that we depends on.
	register_info_set      deps_by;         // registers that depend on us.
	size_t                 bit_start;       // start offset of a larger register.
};

} }

#endif  // !__upcl_sema_register_info_h
