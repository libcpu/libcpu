#include <stdlib.h>

#include "nix.h"
#include "xec-mem.h"

struct _nix_env {
	xec_mem_if_t *memif;
	intmax_t      error;
};

nix_env_t *
nix_env_create(xec_mem_if_t *mem)
{
	nix_env_t *env;

	nix_init(256, 32); /* Init with defaults, if never inited. */

	env = xec_mem_alloc_type(nix_env_t, 0);
	if (env != NULL) {
		env->memif = mem;
		env->error = 0;
	}
	return env;
}

void
nix_env_set_errno(nix_env_t *env, intmax_t error)
{
	env->error = error;
}

intmax_t
nix_env_get_errno(nix_env_t const *env)
{
	return env->error;
}

xec_mem_if_t *
nix_env_get_memory(nix_env_t const *env)
{
	return env->memif;
}

void
nix_env_set_memory(nix_env_t *env, xec_mem_if_t *memif)
{
	env->memif = memif;
}
