#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/resource.h>

#include "nix.h"

int
nix_bsd_rfork(int flags, nix_env_t *env)
{
	return (nix_nosys(env));
}

int
nix_bsd_ktrace(char const *trfile, int ops, int tracefile, nix_pid_t pid,
	nix_env_t *env)
{
	return (nix_nosys(env));
}

int
nix_bsd_profil(char *samples, size_t size, unsigned long offset,
	unsigned int scale, nix_env_t *env)
{
	return (nix_nosys(env));
}

nix_pid_t
nix_bsd_wait4(nix_pid_t wpid, int *status, int options,
	struct nix_rusage *rusage, nix_env_t *env)
{
	struct rusage  ru;
	struct rusage *pru = rusage != NULL ? &ru : NULL;
  
	if (wait4(wpid, status, options, pru) != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}
	return (0);
}
