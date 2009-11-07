#include "nix.h"

int
nix_nosys(nix_env_t *env)
{
	nix_env_set_errno(env, ENOSYS);
	return (-1);
}
