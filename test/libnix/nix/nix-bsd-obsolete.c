#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "nix.h"

int
nix_bsd_vread(int fd, struct nix_iovec const *iov, int iovcnt, nix_env_t *env)
{
	return (nix_readv(fd, iov, iovcnt, env));
}

int
nix_bsd_vwrite(int fd, struct nix_iovec const *iov, int iovcnt, nix_env_t *env)
{
	return (nix_writev(fd, iov, iovcnt, env));
}

int
nix_bsd_vhangup(nix_env_t *env)
{
	return (nix_nosys(env));
}

int
nix_bsd_vlimit(nix_env_t *env)
{
	return (nix_nosys(env));
}

int
nix_bsd_vtimes(nix_env_t *env)
{
	return (nix_nosys(env));
}

int
nix_bsd_vtrace(nix_env_t *env)
{
	return (nix_nosys(env));
}

int
nix_bsd_resuba(nix_env_t *env)
{
	return (nix_nosys(env));
}
