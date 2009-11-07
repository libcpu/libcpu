#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include "nix.h"

int
nix_linux_ustat(nix_dev_t dev, struct nix_linux_ustat *buf, nix_env_t *env)
{
	if (buf == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	return (nix_nosys(env));
}

int
nix_linux_mount(char const *source,
				char const *target,
				char const *filesystemtype,
				uint32_t mountflags,
				void const *data,
				nix_env_t *env)
{
	return (nix_nosys(env));
}
