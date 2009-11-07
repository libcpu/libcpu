#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "nix-types.h"
#include "nix-fd.h"

int
nix_bsd_getdirentries(int fd, char *buf, size_t bufsiz, long *lbasep, nix_env_t *env)
{
	return (nix_nosys(env));
}
