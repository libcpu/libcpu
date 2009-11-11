#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "nix.h"

int
nix_gethostname(char *buf, size_t bufsiz, nix_env_t *env)
{
	if (buf == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	if (bufsiz == 0)
		return (0);

	if (gethostname(buf, bufsiz) != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	return (0);
}

int
nix_sethostname(char const *hostname, size_t len, nix_env_t *env)
{
	if (hostname == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	if (sethostname((char *)hostname, len) != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	return (0);
}

int
nix_getdomainname(char *buf, size_t bufsiz, nix_env_t *env)
{
	if (buf == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
    }

	if (bufsiz == 0)
		return (0);

	if (getdomainname(buf, bufsiz) != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	return (0);
}

int
nix_setdomainname(char const *domainname, size_t len, nix_env_t *env)
{
	if (domainname == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	if (setdomainname (domainname, len) != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	return (0);
}
