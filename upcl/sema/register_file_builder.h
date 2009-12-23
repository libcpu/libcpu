#ifndef __upcl_sema_register_file_builder_h
#define __upcl_sema_register_file_builder_h

#include "sema/register_dep_tracker.h"
#include "c/register_def.h"

namespace upcl { namespace sema {

class register_file_builder {
	register_dep_tracker *m_rdt;

public:
	register_file_builder();

	bool analyze(register_dep_tracker *rdt);

private:
	bool analyze_top(register_info const *);
	//	void analyze_many(std::string const &, register_info_vector const &, size_t);

private:
	c::register_def *create_sub(register_info const *, c::register_def *,
			register_info const *);
	
	c::register_def *create_aliased_sub(register_info const *,
			c::register_def *, c::type const *, register_info const *,
			register_info const *);
	c::register_def *create_pseudo_aliased_sub(register_info const *,
			c::register_def *, c::type *, register_info const *);
};

} }

#endif  // !__upcl_sema_register_file_builder_h
