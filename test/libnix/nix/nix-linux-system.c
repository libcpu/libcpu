#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "nix.h"

int
nix_linux_create_module(char const *name, int flags, nix_env_t *env)
{
  return (nix_nosys(env));
}

int
nix_linux_init_module(char const *name, struct nix_linux_module *module,
	nix_env_t *env)
{
  return (nix_nosys(env));
}

int
nix_linux_delete_module(char const *name, nix_env_t *env)
{
  return (nix_nosys(env));
}

int
nix_linux_get_kernel_sym(struct nix_linux_kernel_sym *ksym, nix_env_t *env)
{
  return (nix_nosys(env));
}

int
nix_linux_bdflush(int func, uintmax_t value, nix_env_t *env)
{
  return (nix_nosys(env));
}

int
nix_linux_sysfs(int a, uint32_t b, uint32_t c, nix_env_t *env)
{
  return (nix_nosys(env));
}

int
nix_linux_syslog(int type, char *bufp, size_t len, nix_env_t *env)
{
  return (nix_nosys(env));
}

int
nix_linux_sysinfo(struct nix_linux_sysinfo *si, nix_env_t *env)
{
  return (nix_nosys(env));
}

int
nix_linux_swapoff(char const *path, nix_env_t *env)
{
  return (nix_nosys(env));
}
