#include "nix-config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/reboot.h>

#include "nix.h"

nix_pid_t
nix_getpgrp(nix_env_t *env)
{
	return (getpgrp());
}

nix_pid_t
nix_getpgid(nix_pid_t pid, nix_env_t *env)
{
	return (getpgid(pid));
}

int
nix_setpgid(nix_pid_t pid, nix_pid_t pgrp, nix_env_t *env)
{
	if (setpgid((pid_t)pid, (pid_t)pgrp) != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}
	return (0);
}

nix_pid_t
nix_setsid(nix_env_t *env)
{
	pid_t pid;
  
	if ((pid = setsid()) < 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	return (pid);
}

nix_pid_t
nix_getsid(nix_pid_t pid, nix_env_t *env)
{
	pid_t psid;
  
	if ((psid = getsid(pid)) < 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	return (psid);
}

int
nix_getuid(nix_env_t *env)
{
	return (getuid());
}

int
nix_setuid(nix_uid_t uid, nix_env_t *env)
{
	if (setuid((uid_t)uid) != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}
	return (0);
}

int
nix_getgid(nix_env_t *env)
{
	return (getgid());
}

int
nix_setgid(nix_gid_t gid, nix_env_t *env)
{
	if (setgid((gid_t)gid) != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}
	return (0);
}

int
nix_geteuid(nix_env_t *env)
{
	return (geteuid());
}

int
nix_seteuid(nix_uid_t uid, nix_env_t *env)
{
	if (seteuid((uid_t)uid) != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	return (0);
}

int
nix_getegid(nix_env_t *env)
{
	return (getegid());
}

int
nix_setegid(nix_gid_t gid, nix_env_t *env)
{
	if (setegid((gid_t)gid) != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	return (0);
}

int
nix_getreuid(nix_env_t *env)
{
	return (geteuid());
}

int
nix_setreuid(nix_uid_t uid, nix_uid_t euid, nix_env_t *env)
{
#ifdef HAVE_SETREUID
	if (setreuid(uid, euid) != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}
#else
	if (nix_setuid(uid, env) != 0)
		return (-1);
	if (nix_seteuid(euid, env) != 0)
		return (-1);
#endif
	return (0);
}

int
nix_setregid(nix_gid_t gid, nix_gid_t egid, nix_env_t *env)
{
#ifdef HAVE_SETREGID
	if (setregid(gid, egid) != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}
#else
	if (nix_setgid(gid, env) != 0)
		return (-1);
	if (nix_setegid(egid, env) != 0)
		return (-1);
#endif
	return (0);
}

int
nix_reboot(int howto, nix_env_t *env)
{
	if (reboot(howto) != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}
	return (0);
}
