#ifndef __upcl_c_instruction_h
#define __upcl_c_instruction_h

#include "c/type.h"

namespace upcl { namespace c {

class instruction;
typedef std::vector <instruction *> instruction_vector;

class instruction {
private:
	std::string m_name;

public:
	instruction(std::string const &name)
		: m_name(name)
	{ }

	inline std::string get_name() const { return m_name; }
	virtual bool is_jump() const { return false; }
};

class jump_instruction;
typedef std::vector <jump_instruction *> jump_instruction_vector;

class jump_instruction : public instruction {
public:
	enum jump_type {
		BRANCH,
		CALL,
		RETURN,
		TRAP
	};

private:
	jump_type m_type;
	expression *m_condition;

public:
	jump_instruction(jump_type const &type, std::string const &name,
			expression *condition = 0)
		: instruction(name), m_type (type), m_condition(condition)
	{ }

	inline jump_type get_type() const { return m_type; }
	inline expression *get_condition () const { return m_condition; }
	virtual bool is_jump() const { return true; }
};

} }

#endif  // !__upcl_c_instruction_h
