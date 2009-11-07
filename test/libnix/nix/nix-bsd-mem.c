#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "nix.h"

uintmax_t
nix_bsd_mquery(uintmax_t addr, size_t len, int prot, int flags, int fd,
	off_t offset, nix_env_t *env)
{
  return (nix_nosys(env));
}

int
nix_bsd_vadvise(int flags, nix_env_t *env)
{
  return (nix_nosys(env));
}
