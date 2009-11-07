#ifndef __obsd41_us_syscall_priv_h
#define __obsd41_us_syscall_priv_h

#include "xec-us-syscall-if.h"
#include "nix.h"

typedef struct _obsd41_us_syscall obsd41_us_syscall_t;

struct _obsd41_us_syscall {
	xec_us_syscall_if_vtbl_t const *vtbl;
	nix_env_t					   *env;
	uint32_t						last_param;
	xec_param_type_t				retype;
};

static __inline nix_env_t *
obsd41_us_syscall_get_nix_env(xec_us_syscall_if_t *xus)
{ return ((obsd41_us_syscall_t *)xus)->env; }

#endif  /* !__obsd41_us_syscall_priv_h */
