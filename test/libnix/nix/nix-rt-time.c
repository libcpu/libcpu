#include "nix-config.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "nix.h"

int
nix_rt_clock_gettime(nix_clockid_t clockid, struct nix_timespec *tp,
	nix_env_t *env)
{
#ifdef HAVE_CLOCK_GETTIME
	struct timespec nt;
#else
	struct timeval nt;
#endif

	if (tp == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

#ifdef HAVE_CLOCK_GETTIME
	if (clock_gettime(clockid, &nt) < 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	tp->tv_nsec = nt.tv_nsec;
#else
	if (clockid == NIX_CLOCK_REALTIME) {
		if (gettimeofday (&nt, NULL) != 0) {
			nix_env_set_errno(env, errno);
			return (-1);
		}
	}
	else {
		nix_env_set_errno(env, EINVAL);
		return (-1);
	}

	tp->tv_nsec = nt.tv_usec * 1000;
#endif
	tp->tv_sec  = nt.tv_sec;

	return (0);
}

int
nix_rt_clock_settime(nix_clockid_t clockid, struct nix_timespec const *tp,
	nix_env_t *env)
{
#ifdef HAVE_CLOCK_SETTIME
	struct timespec nt;
#else
	struct timeval nt;
#endif

	if (tp == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	nt.tv_sec = tp->tv_sec;
#ifdef HAVE_CLOCK_SETTIME
	nt.tv_nsec = tp->tv_nsec;

	if (clock_settime(clockid, &nt) != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

#else
	nt.tv_usec = tp->tv_nsec / 1000;

	if (clockid == NIX_CLOCK_REALTIME) {
		if (settimeofday(&nt, NULL) != 0) {
			nix_env_set_errno(env, errno);
			return (-1);
		}
	} else {
		nix_env_set_errno(env, EINVAL);
		return (-1);
	}
#endif

	return (0);
}

int
nix_rt_clock_getres(nix_clockid_t clockid, struct nix_timespec *tp,
	nix_env_t *env)
{
#ifdef HAVE_CLOCK_GETRES
	struct timespec nt;
#endif

	if (tp == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

#ifdef HAVE_CLOCK_GETRES
	if (clock_getres(clockid, &nt) != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	tp->tv_sec  = nt.tv_sec;
	tp->tv_nsec = nt.tv_nsec;
#else
	if (clockid == NIX_CLOCK_REALTIME) {
		tp->tv_sec  = 0;
		tp->tv_nsec = 1000000;
	} else {
		nix_env_set_errno(env, EINVAL);
		return (-1);
	}
#endif

	return (0);
}
