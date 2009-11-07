#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "nix.h"

int
nix_linux_adjtimex(struct nix_linux_timex *buf, nix_env_t *env)
{
  return (nix_nosys(env));
}
