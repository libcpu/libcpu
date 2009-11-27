#include "nix-config.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "nix.h"

static long g_hostid = 0xba550770;

int
nix_bsd_sysarch(int number, void *args, nix_env_t *env)
{
	return (nix_nosys(env));
}

int
nix_bsd_quotactl(char const *path, int cmd, nix_uid_t uid, void *arg,
	nix_env_t *env)
{
	return (nix_nosys(env));
}

int
nix_bsd_swapctl(int cmd, void const *arg, int misc, nix_env_t *env)
{
	return (nix_nosys(env));
}

int
nix_bsd_getkerninfo(int op, char *where, size_t *size, int arg, nix_env_t *env)
{
	/* XXX: This should be a direct dependency of sysctl! */
	return (nix_nosys(env));
}

int
nix_bsd_quota(char const *name, unsigned int current, unsigned int max,
	nix_env_t *env)
{
	return (nix_nosys(env));
}

int
nix_bsd_swapon(char const *name, nix_env_t *env)
{
  return nix_nosys (env);
}

int
nix_bsd_gethostid(char *buf, size_t bufsiz, nix_env_t *env)
{
#if defined (HAVE_GETHOSTID) && defined (HAVE_SETHOSTID)
	if (g_hostid == 0xba550770)
		g_hostid = gethostid();
#endif

	snprintf(buf, bufsiz, "%08lx", g_hostid);
	return (0);
}

int
nix_bsd_sethostid(char const *hostid, nix_env_t *env)
{
	long hid = strtoul(hostid, NULL, 16);

#if !defined (HAVE_GETHOSTID) || !defined (HAVE_SETHOSTID)
	if (nix_geteuid(env) != 0) {
		nix_env_set_errno(env, EPERM);
		return (-1);
    }
	g_hostid = hid;
#else
	errno = 0;
	sethostid(hid);
	if (errno != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}
#endif

	return (0);
}
