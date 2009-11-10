#ifndef __loader_h
#define __loader_h

#include "xec-mem-if.h"
#include "coff.h"
#include "aout.h"

#define LOADER_SUCCESS (0)
#define LOADER_INVALID_ADDRESS (-1)
#define LOADER_NO_MEMORY (-2)
#define LOADER_MAP_FAIL (-3)
#define LOADER_UNSUPPORTED (-4)

extern struct coff_aouthdr g_ahdr;
extern aout_header_t       g_exec;

#ifdef __cplusplus
extern "C" {
#endif

void
loader_init(void);

int
loader_load(xec_mem_if_t *mem_if, char const *path);

#ifdef __cplusplus
}
#endif

#endif  /* !__loader_h */
