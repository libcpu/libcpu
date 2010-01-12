#ifndef __upcl_cg_generate_h
#define __upcl_cg_generate_h

#include <ostream>
#include <string>

#include "c/register_def.h"
#include "c/sub_register_def.h"
#include "c/instruction.h"

namespace upcl { namespace cg {

	void generate_arch_h(std::ostream &o, std::string const &fname,
		std::string const &arch_name);

	void generate_types_h(std::ostream &o, std::string const &fname,
		std::string const &arch_name, c::register_def_vector const &regs);

	void generate_arch_cpp(std::ostream &o, std::string const &fname,
		std::string const &arch_name, std::string const &arch_full_name,
		uint64_t const *arch_tags, c::register_def const *pcr,
		c::register_def const *psr, c::register_def_vector const &regs);

	void generate_opc_h(std::ostream &o, std::string const &fname,
		std::string const &arch_name, c::instruction_vector const &insns);

	void generate_tag_cpp(std::ostream &o, std::string const &fname,
		std::string const &arch_name, c::jump_instruction_vector const &jumps);

} }

#endif  // !__upcl_cg_generate_h
