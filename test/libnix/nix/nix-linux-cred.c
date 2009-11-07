#include "nix-config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "nix.h"

#ifndef HAVE_SETFSUID
static nix_uid_t g_fsuid = -1; // XXX move in ENV
static nix_uid_t g_fsgid = -1;
#endif

nix_uid_t
nix_linux_setfsuid(nix_uid_t uid, nix_env_t *env)
{
#ifdef HAVE_SETFSUID
	int rv;

	errno = 0;
	rv = setfsuid((uid_t)uid);
	if (errno != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	return (rv);
#else
	nix_uid_t last = g_fsuid;
	g_fsuid = uid;
	return (last);
#endif
}

nix_uid_t
nix_linux_setfsgid(nix_gid_t gid, nix_env_t *env)
{
#ifdef HAVE_SETFSGID
	int rv;

	errno = 0;
	rv = setfsgid((gid_t)gid);
	if (errno != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	return (rv);
#else
	nix_gid_t last = g_fsgid;
	g_fsgid = gid;
	return (last);
#endif
}
