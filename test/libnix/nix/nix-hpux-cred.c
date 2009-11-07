#include "nix-config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "nix.h"

int
nix_hpux_getresuid(nix_uid_t *ruid, nix_uid_t *euid, nix_uid_t *suid, nix_env_t *env)
{
	uid_t nruid, neuid, nsuid;
  
	if (ruid == NULL || euid == NULL || suid == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

#ifdef HAVE_GETRESUID
	if (getresuid(&nruid, &neuid, &nsuid) != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}
#else
	errno = 0;
	nruid = getuid();
	nsuid = neuid = geteuid();
	if (errno != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}
#endif

	__nix_try
	{
		*ruid = nruid;
		*euid = neuid;
		*suid = nsuid;
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
nix_hpux_setresuid(nix_uid_t ruid, nix_uid_t euid, nix_uid_t suid, nix_env_t *env)
{
#ifdef HAVE_GETRESGID
	if (setresuid(ruid, euid, suid) != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	return (0);
#else
	if (euid != suid && suid != (nix_uid_t)-1) {
		nix_env_set_errno(env, EINVAL);
		return (-1);
	}

	return (nix_setreuid(ruid, euid, env));
#endif
}

int
nix_hpux_getresgid(nix_gid_t *rgid, nix_gid_t *egid, nix_gid_t *sgid, nix_env_t *env)
{
	gid_t nrgid, negid, nsgid;
  
	if (rgid == NULL || egid == NULL || sgid == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

#ifdef HAVE_GETRESGID
	if (getresgid(&nrgid, &negid, &nsgid) != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}
#else
	errno = 0;
	nrgid = getgid();
	nsgid = negid = getegid();
	if (errno != 0) {
		nix_env_set_errno(env, errno);
		return -1;
	}
#endif

	__nix_try
    {
		*rgid = nrgid;
		*egid = negid;
		*sgid = nsgid;
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
nix_hpux_setresgid (nix_gid_t rgid, nix_gid_t egid, nix_gid_t sgid, nix_env_t *env)
{
#ifdef HAVE_GETRESGID
	if (setresgid(rgid, egid, sgid) < 0) {
		nix_env_set_errno(env, errno);
		return (-1);
    }

	return (0);
#else
	if (egid != sgid && sgid != (nix_gid_t)-1) {
		nix_env_set_errno(env, EINVAL);
		return (-1);
	}

	return (nix_setregid(rgid, egid, env));
#endif
}
