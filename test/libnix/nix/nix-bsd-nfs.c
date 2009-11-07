#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "nix.h"

int
nix_bsd_nfssvc(int flags, void *arg, nix_env_t *env)
{
	if (arg == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}
	return (nix_nosys(env));
}

int
nix_bsd_getfh(char const *path, nix_fhandle_t *fh, nix_env_t *env)
{
	if (path == NULL || fh == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}
	return (nix_nosys(env));
}

int
nix_bsd_fhstatfs(nix_fhandle_t const *fhp, struct nix_statfs *buf,
	nix_env_t *env)
{
	if (fhp == NULL || buf == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
    }
	return (nix_nosys(env));
}

int
nix_bsd_fhstat(nix_fhandle_t const *fhp, struct nix_stat *buf, nix_env_t *env)
{
	if (fhp == NULL || buf == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}
	return (nix_nosys(env));
}

int
nix_bsd_fhopen(nix_fhandle_t const *fhp, int flags, nix_env_t *env)
{
	if (fhp == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}
	return (nix_nosys(env));
}
