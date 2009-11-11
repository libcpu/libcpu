#include "xec-base.h"

extern void __xec_mem_init(void);
extern void __xec_log_init(void);
extern void __xec_mmap_init(void);
extern void __xec_xcpt_init(void);

void
xec_init(void)
{
	__xec_log_init();
	__xec_mem_init();
	__xec_mmap_init();
	__xec_xcpt_init();
}
