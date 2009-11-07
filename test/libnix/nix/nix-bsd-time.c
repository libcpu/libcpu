#include "nix-config.h"

#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "nix.h"

int
nix_bsd_adjtime(struct nix_timeval const *delta, struct nix_timeval *odelta,
	nix_env_t *env)
{
#ifdef HAVE_ADJTIME
	struct timeval ndelta;
	struct timeval nodelta;
	struct timeval *npodelta = odelta != NULL ? &nodelta : NULL;
#endif

	if (delta == NULL) {
		nix_env_set_errno (env, EFAULT);
		return (-1);
	}

#ifdef HAVE_ADJTIME
	__nix_try
    {
		ndelta.tv_sec  = delta->tv_sec;
		ndelta.tv_usec = delta->tv_usec;
		if (adjtime(&ndelta, npodelta) != 0) {
			nix_env_set_errno (env, errno);
        } else if (npodelta != NULL) {
			odelta->tv_sec  = npodelta->tv_sec;
			odelta->tv_usec = npodelta->tv_usec;
		}
	}
	__nix_catch_any
	{
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}
	__nix_end_try
#else
	nix_env_set_errno(env, ENOSYS);
#endif

	return (nix_env_get_errno(env) != 0 ? -1 : 0);
}

int
nix_bsd_adjfreq(struct nix_timeval const *delta, struct nix_timeval *odelta,
	nix_env_t *env)
{
#ifdef HAVE_ADJFREQ
	struct timeval ndelta;
	struct timeval nodelta;
	struct timeval *npodelta = odelta != NULL ? &nodelta : NULL;
#endif

	if (delta == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

#ifdef HAVE_ADJFREQ
	__nix_try
    {
		ndelta.tv_sec  = delta->tv_sec;
		ndelta.tv_usec = delta->tv_usec;
		if (adjfreq(&ndelta, npodelta) != 0) {
			nix_env_set_errno(env, errno);
        } else if (npodelta != NULL) {
			odelta->tv_sec  = npodelta->tv_sec;
			odelta->tv_usec = npodelta->tv_usec;
        }
    }
	__nix_catch_any
	{
		nix_env_set_errno(env, EFAULT);
		return (-1);
    }
	__nix_end_try
#else
	nix_env_set_errno(env, ENOSYS);
#endif

	return (nix_env_get_errno(env) != 0 ? -1 : 0);
}
