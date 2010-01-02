#ifndef __upcl_types_h
#define __upcl_types_h

#include <string>
#include <vector>
#include <map>
#include <set>

#include "libcpu.h"

namespace upcl {

typedef std::vector<std::string> string_vector;
typedef std::set<std::string> string_set;
typedef std::map<std::string, size_t> string_count_map;
typedef std::map<std::string, uint64_t> string_uint_map;
typedef std::map<std::string, double> string_float_map;

}

#endif  // !__upcl_types_h
