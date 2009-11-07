#include "nix-config.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif
#ifdef HAVE_SCHED_H
#include <sched.h>
#endif

#include "nix.h"

int
nix_rt_sched_yield(nix_env_t *env)
{
#if defined(HAVE_SCHED_YIELD)
	sched_yield();
#elif defined(HAVE_PTHREAD_YIELD)
	pthread_yield();
#endif

	return (0);
}

int
nix_rt_sched_setparam(nix_pid_t pid, struct nix_rt_sched_param const *param,
	nix_env_t *env)
{
	return (nix_nosys(env));
}

int
nix_rt_sched_getparam(nix_pid_t pid, struct nix_rt_sched_param *param,
	nix_env_t *env)
{
	return (nix_nosys(env));
}

int
nix_rt_sched_setscheduler(nix_pid_t pid, int policy,
	struct nix_rt_sched_param const *param, nix_env_t *env)
{
	return (nix_nosys(env));
}

int
nix_rt_sched_getscheduler(nix_pid_t pid, int policy,
	struct nix_rt_sched_param *param, nix_env_t *env)
{
	return (nix_nosys(env));
}

int
nix_rt_sched_get_priority_min(int policy, nix_env_t *env)
{
	return (nix_nosys(env));
}

int
nix_rt_sched_get_priority_max(int policy, nix_env_t *env)
{
	return (nix_nosys(env));
}

int
nix_rt_sched_rr_get_interval(nix_pid_t pid, struct nix_timespec *interval,
	nix_env_t *env)
{
	return (nix_nosys(env));
}
