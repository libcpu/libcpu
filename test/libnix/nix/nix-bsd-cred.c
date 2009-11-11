#include "nix-config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "nix.h"

int
nix_bsd_issetugid(nix_env_t *env)
{
#ifdef HAVE_ISSETUGID
	return (issetugid());
#else
	return (0);
#endif
}

int
nix_bsd_getlogin(char *buf, size_t bufsiz, nix_env_t *env)
{
	char *login;
  
	if (buf == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	if (bufsiz == 0)
		return (0);

	login = getlogin();

	__nix_try
	{
		strncpy(buf, login, bufsiz);
	}
	__nix_catch_any
	{
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}
	__nix_end_try

	return (0);
}

int
nix_bsd_setlogin(char const *name, nix_env_t *env)
{
	if (name == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

#if defined(HAVE_SETLOGIN)
	if (setlogin(name) != 0) {
		nix_env_set_errno (env, errno);
		return (-1);
	}

	return (0);
#else
	return (nix_nosys(env));
#endif
}

int
nix_bsd_acct(char const *file, nix_env_t *env)
{
#ifdef HAVE_ACCT
	if (file == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	if (acct(file) != 0) {
		nix_env_set_errno (env, errno);
		return (-1);
	}

	return (0);
#else
	return (nix_nosys(env));
#endif
}

int
nix_bsd_getgroups(int gidsetlen, nix_gid_t *gids, nix_env_t *env)
{
	return (nix_nosys(env));
}

int
nix_bsd_setgroups(int gidsetlen, nix_gid_t const *gids, nix_env_t *env)
{
	return (nix_nosys(env));
}
