#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "nix.h"

int
nix_linux_clone(uintmax_t sp, int flags, nix_env_t *env)
{
	return (nix_nosys(env));
}

int
nix_linux_idle(nix_env_t *env)
{
	return (nix_nosys(env));
}

int
nix_linux_uselib(char const *path, nix_env_t *env)
{
	return (nix_nosys(env));
}

int
nix_linux_capget (nix_linux_cap_user_handler_t handler,
				  nix_linux_cap_user_data_t data,
				  nix_env_t *env)
{
  return (nix_nosys(env));
}

int
nix_linux_capset (nix_linux_cap_user_handler_t handler,
				  nix_linux_cap_user_data_t const data,
				  nix_env_t *env)
{
  return (nix_nosys(env));
}

int
nix_linux_personality(uintmax_t personality, nix_env_t *env)
{
  return (nix_nosys(env));
}

int
nix_linux_prctl(int option, uintmax_t arg2, uintmax_t arg3 , uintmax_t arg4,
	uintmax_t arg5, nix_env_t *env)
{
  return (nix_nosys(env));
}
