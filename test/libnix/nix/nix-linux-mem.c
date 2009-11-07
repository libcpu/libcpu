#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "nix.h"

uintmax_t
nix_linux_mremap(uintmax_t oldaddr, size_t oldsize, size_t newsize,
	int flags, nix_env_t *env)
{
	return (nix_nosys(env));
}
