#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "nix.h"

int
nix_linux_i386_ioperm(uint32_t from, uint32_t num, int on, nix_env_t *env)
{
	return (nix_nosys(env));
}

int
nix_linux_i386_iopl(uint32_t perm, nix_env_t *env)
{
	return (nix_nosys(env));
}

int
nix_linux_i386_vm86(uint32_t cmd, void *arg, nix_env_t *env)
{
	return (nix_nosys(env));
}

int
nix_linux_i386_modify_ldt(int ldtno, void *arg, uint32_t val, nix_env_t *env)
{
	return (nix_nosys(env));
}
