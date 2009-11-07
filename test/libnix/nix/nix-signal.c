#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include "nix.h"
#include "xec-mem.h"
#include "xec-debug.h"

extern void *g_nix_log;

// XXX MOVE TO ENV
static size_t                g_sigcount    = 0;
static struct nix_sigaction *g_sigactions  = NULL;
static nix_sigset_t          g_sigprocmask = 0;
static nix_sigset_t          g_sigpending  = 0;
static struct nix_itimerval  g_timers[3];

int
nix_signal_init(size_t count)
{
	XEC_ASSERT0(count != 0);
	XEC_ASSERT0(count <= (sizeof (nix_sigset_t) << 3));

	g_sigcount = count;

	g_sigactions = xec_mem_alloc_ntype(struct nix_sigaction, g_sigcount, 0);
	XEC_ASSERT0(g_sigactions != NULL);
	return (g_sigactions != NULL);
}

int
nix_kill(nix_pid_t pid, int signo, nix_env_t *env)
{
	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "pid=%d, signo=%d", pid, signo);

	if (signo == 21) // XXX?
		return (0);

	if (signo < 0 || signo >= (int)g_sigcount) {
    	nix_env_set_errno(env, EINVAL);
		return (-1);
	}

	/* XXX We need to convert signals! */
	if (kill((pid_t)pid, signo) != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	return (0);
}

int
nix_killpg(nix_pid_t pgrp, int signo, nix_env_t *env)
{
	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "pgrp=%d, signo=%d", pgrp, signo);

	return (nix_kill(-pgrp, signo, env));
}

int
nix_sigaction(int signo, struct nix_sigaction const *sa,
	struct nix_sigaction *osa, nix_env_t *env)
{
	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "signo=%d, sa=%p, osa=%p", signo, sa, osa);

	if (signo <= 0 || signo >= (int)g_sigcount) {
		nix_env_set_errno(env, EINVAL);
		return (-1);
	}

	if (sa == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	if (osa != NULL) {
		__nix_try
		{
			memcpy(osa, &g_sigactions[signo], sizeof(struct nix_sigaction));
		}
		__nix_catch_any
		{
			nix_env_set_errno(env, EFAULT);
			return (-1);
		}
		__nix_end_try
	  }

	__nix_try
	{
		memcpy(&g_sigactions[signo], sa, sizeof(struct nix_sigaction));
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
nix_sigaltstack(int signo, struct nix_sigaltstack const *ss,
	struct nix_sigaltstack *oss, nix_env_t *env)
{
	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "signo=%d, ss=%p, oss=%p", signo, ss, oss);

	if (signo <= 0 || signo >= (int)g_sigcount) {
		nix_env_set_errno(env, EINVAL);
		return (-1);
	}

	if (ss == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

#if 0
	if (osa != NULL) {
		__nix_try
		{
			memcpy(osa, &g_sigactions[signo], sizeof(struct nix_sigaction));
		}
		__nix_catch_any
		{
			nix_env_set_errno(env, EFAULT);
			return (-1);
		}
		__nix_end_try
	}

	__nix_try
	{
		memcpy(&g_sigactions[signo], sa, sizeof(struct nix_sigaction));
	}
	__nix_catch_any
	{
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}
	__nix_end_try
#endif

	return (nix_nosys(env));
}

int
nix_sigprocmask(int how, nix_sigset_t const *set, nix_sigset_t *oset,
	nix_env_t *env)
{
	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "how=%d, set=%p, oset=%p", how, set, oset);

	__nix_try
	{
		if (oset != NULL)
			*oset = g_sigprocmask;

		if (set != NULL) {
			switch (how) {
				case NIX_SIG_BLOCK:   g_sigprocmask |= *set; break; 
				case NIX_SIG_UNBLOCK: g_sigprocmask &= ~(*set); break; 
				case NIX_SIG_SETMASK: g_sigprocmask = *set; break; 
				default:			  nix_env_set_errno(env, EINVAL);
									  break;
			}
		}
	}
	__nix_catch_any
	{
		nix_env_set_errno (env, EFAULT);
	}
	__nix_end_try

	return (nix_env_get_errno(env) != 0 ? -1 : 0);
}

int
nix_sigsuspend(nix_sigset_t const *set, nix_env_t *env)
{
	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "set=%p", set);

	if (set == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	if (nix_sigprocmask(NIX_SIG_UNBLOCK, set, NULL, env) != 0)
		return (-1);

	/* We should wait for an interrupt. */

	return (nix_nosys(env));
}

int
nix_sigpending(nix_sigset_t *set, nix_env_t *env)
{
	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "set=%p", set);

	if (set == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	__nix_try
	{
		*set = g_sigpending;
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
nix_sigtimedwait(nix_sigset_t const        *set,
				 nix_siginfo_t             *info,
				 struct nix_timespec const *timeout,
				 nix_env_t                 *env)
{
	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "set=%p, info=%p, timeout=%p", set, info, timeout);

	return (nix_nosys(env));
}

int
nix_setitimer(int                         timer,
			  struct nix_itimerval const *value, 
			  struct nix_itimerval       *ovalue,
			  nix_env_t                  *env)
{
	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "timer=%d, value=%p, ovalue=%p", timer, value, ovalue);

	if (value == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	if (timer < 0 || timer > NIX_ITIMER_PROF) {
		nix_env_set_errno(env, EINVAL);
		return (-1);
	}

	__nix_try
	{
		if (ovalue != NULL)
			memcpy(ovalue, &g_timers[timer], sizeof(*ovalue));

		memcpy(&g_timers[timer], value, sizeof(*value));
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
nix_getitimer(int timer, struct nix_itimerval *value, nix_env_t *env)
{
	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "timer=%d, value=%p", timer, value);

	if (value == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	if (timer < 0 || timer > NIX_ITIMER_PROF) {
		nix_env_set_errno(env, EINVAL);
		return (-1);
	}

	__nix_try
	{
		memcpy(value, &g_timers[timer], sizeof(*value));
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
nix_alarm(nix_time_t secs, nix_env_t *env)
{
	struct nix_itimerval it;

	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "secs=%lld", secs);

	if ((int64_t)secs < 0) {
		nix_env_set_errno(env, EINVAL);
		return (-1);
	}

	it.it_interval.tv_sec = 0;
	it.it_interval.tv_usec = 0;
	it.it_value.tv_sec = secs;
	it.it_value.tv_usec = 0;

	return (nix_setitimer(NIX_ITIMER_REAL, &it, NULL, env));
}

int
nix_pause(nix_env_t *env)
{
	nix_sigset_t mask = 0;

	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	return (nix_sigsuspend(&mask, env));
}

uintmax_t
nix_signal(int signo, uintmax_t handler, nix_env_t *env)
{
	struct nix_sigaction sa;
	struct nix_sigaction osa;

	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "signo=%u, handler=%llx", signo, handler);

	sa.__sa_handler = handler;
	sa.sa_flags = 0;
	sa.sa_mask = 0;
	if (nix_sigaction (signo, &sa, &osa, env) != 0)
		return (-1);
	else
		return (osa.__sa_handler);
}
