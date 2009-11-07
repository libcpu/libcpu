#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "nix.h"
#include "nix-structs.h"

int
nix_gettimeofday (struct nix_timeval  *tp,
				  struct nix_timezone *tzp,
				  nix_env_t           *env)
{
	struct timeval   nt;
	struct timezone  ntz;
	struct timeval  *npt  = tp != NULL ? &nt : NULL;
	struct timezone *nptz = tzp != NULL ? &ntz : NULL;

	if (npt == NULL && nptz == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	if (gettimeofday(npt, nptz) < 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	if (npt != NULL)
	  timeval_to_nix_timeval(npt, tp);

	if (nptz != NULL)
	  timezone_to_nix_timezone(nptz, tzp);

	return (0);
}

int
nix_settimeofday (struct nix_timeval const  *tp,
				  struct nix_timezone const *tzp,
				  nix_env_t                 *env)
{
	struct timeval   nt;
	struct timezone  ntz;
	struct timeval  *npt  = tp != NULL ? &nt : NULL;
	struct timezone *nptz = tzp != NULL ? &ntz : NULL;

	if (npt == NULL && nptz == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	if (npt != NULL)
	  nix_timeval_to_timeval(tp, npt);

	if (nptz != NULL)
	  nix_timezone_to_timezone(tzp, nptz);

	if (settimeofday(npt, nptz) != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	return (0);
}

int
nix_nanosleep(struct nix_timespec const *rqtp,
			  struct nix_timespec       *rmtp,
			  nix_env_t                 *env)
{
	struct timespec  nrqt;
	struct timespec  nrmt;
	struct timespec *nrmtp = rmtp != NULL ? &nrmt : NULL;
	int              rc    = -1;

	if (rqtp == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	nrqt.tv_sec  = rqtp->tv_sec;
	nrqt.tv_nsec = rqtp->tv_nsec;

	rc = nanosleep(&nrqt, nrmtp);
	if (rc != 0)
	  nix_env_set_errno(env, errno);
	else if (nrmtp != NULL) {
		rmtp->tv_sec = nrmtp->tv_sec;
		rmtp->tv_nsec = nrmtp->tv_nsec;
	}

	return (rc);
}

int
nix_time(nix_time_t *tp, nix_env_t *env)
{
	struct nix_timeval tv;

	if (tp == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	if (nix_gettimeofday(&tv, NULL, env) != 0)
	  return (-1);

	__nix_try
	{
		*tp = tv.tv_sec;
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
nix_stime(nix_time_t t, nix_env_t *env)
{
	struct nix_timeval tv;

	tv.tv_sec  = t;
	tv.tv_usec = 0;

	return (nix_settimeofday(&tv, NULL, env));
}
