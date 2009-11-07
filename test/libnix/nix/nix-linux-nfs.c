#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "nix.h"

int
nix_linux_nfsservctl(int cmd, struct nix_linux_nfsctl_arg *arg,
	union nix_linux_nfsctl_res *resp, nix_env_t *env)
{
	return (nix_nosys(env));
}
