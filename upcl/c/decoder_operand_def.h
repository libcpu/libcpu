#ifndef __upcl_c_decoder_operand_def_h
#define __upcl_c_decoder_operand_def_h

#include "types.h"

#include "c/type.h"
#include "c/expression.h"

namespace upcl { namespace c {

class decoder_operand_def;
typedef std::vector <decoder_operand_def *> decoder_operand_vector;

class decoder_operand_def {

	unsigned m_flags;
	std::string m_name;
	type *m_type;

protected:
	decoder_operand_def(unsigned flags, std::string const &name, c::type *type);

public:
	decoder_operand_def(std::string const &name, c::type *type);
	virtual ~decoder_operand_def();

protected:
	inline void change_flags(unsigned set, unsigned clr = 0)
	{ m_flags = (m_flags & ~clr) | set; }

public:
	virtual c::type *get_type() const;

	virtual std::string get_name() const;
};

} }

#endif  // !__upcl_c_decoder_operand_def_h
