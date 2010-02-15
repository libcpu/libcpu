#ifndef __upcl_c_temp_value_def_h
#define __upcl_c_temp_value_def_h

#include "types.h"

#include "c/type.h"
#include "c/expression.h"

namespace upcl { namespace c {

class temp_value_def;
typedef std::vector <temp_value_def *> temp_value_vector;
typedef std::map <std::string, temp_value_def *> temp_value_map;

class temp_value_def {

	std::string m_name;
	type *m_type;

public:
	temp_value_def(std::string const &name, c::type *type);
	virtual ~temp_value_def();

public:
	virtual c::type *get_type() const;

	virtual std::string get_name() const;
};

} }

#endif  // !__upcl_c_temp_value_def_h
