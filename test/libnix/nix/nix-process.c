#include "nix-config.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/resource.h>

#include "nix.h"
#include "nix-structs.h"

void
nix_exit(int exitcode)
{
	exit(exitcode);
}

nix_pid_t
nix_getpid(nix_env_t *env)
{
	return (getpid());
}

nix_pid_t
nix_getppid(nix_env_t *env)
{
	return (getppid());
}

nix_pid_t
nix_fork(nix_env_t *env)
{
	return (nix_nosys(env));
}

nix_pid_t
nix_vfork(nix_env_t *env)
{
	return (nix_nosys(env));
}

int
nix_execve(char const *path, char const **argv, char const **envp, nix_env_t *env)
{
	/* Here we should do two things:
	 * - if loader is a courtesy of the kernel (like for binfmt_misc), then
	 *   go straight executing the binary, else we fall back to identify
	 *   if the binary that we'll load is for this guest, this requires
	 *   interaction with the loader.
	 */
	return (nix_nosys(env));
}

int
nix_ptrace(int request, nix_pid_t pid, void *address, int data, nix_env_t *env)
{
	/* We'll do you too, don't worry! */
	return (nix_nosys(env));
}

int
nix_getpriority(int which, int who, nix_env_t *env)
{
	/* XXX We need to convert priorities! */
	return (nix_nosys(env));
}

int
nix_setpriority(int which, int who, int prio, nix_env_t *env)
{
	/* XXX We need to convert priorities! */
	return (nix_nosys(env));
}

int
nix_getrusage(int who, struct nix_rusage *rusage, nix_env_t *env)
{
	struct rusage ru;

	if (rusage == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	if (getrusage(who, &ru) != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	rusage_to_nix_rusage(&ru, rusage);

	return (0);
}

int
nix_getrlimit(int resource, struct nix_rlimit *rlim, nix_env_t *env)
{
	struct rlimit rl;
	int           rrsrc;

	if (rlim == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	switch (resource)
	  {
	  case NIX_RLIMIT_CORE:    rrsrc = RLIMIT_CORE;    break;
	  case NIX_RLIMIT_CPU:     rrsrc = RLIMIT_CPU;     break;
	  case NIX_RLIMIT_DATA:    rrsrc = RLIMIT_DATA;    break;
	  case NIX_RLIMIT_FSIZE:   rrsrc = RLIMIT_FSIZE;   break;
#ifdef RLIMIT_MEMLOCK
	  case NIX_RLIMIT_MEMLOCK: rrsrc = RLIMIT_MEMLOCK; break;
#endif
	  case NIX_RLIMIT_NOFILE:  rrsrc = RLIMIT_NOFILE;  break;
#ifdef RLIMIT_NPROC
	  case NIX_RLIMIT_NPROC:   rrsrc = RLIMIT_NPROC;   break;
#endif
#if defined(RLIMIT_RSS)
	  case NIX_RLIMIT_RSS:     rrsrc = RLIMIT_RSS;     break;
#elif defined(RLIMIT_VMEM)
	  case NIX_RLIMIT_RSS:     rrsrc = RLIMIT_VMEM;    break;
#endif
	  case NIX_RLIMIT_STACK:   rrsrc = RLIMIT_STACK;   break;
	  default:				   nix_env_set_errno(env, EINVAL);
							   return (-1);
	  }

	if (getrlimit(rrsrc, &rl) < 0) {
		nix_env_set_errno (env, errno);
		return (-1);
	}

	rlimit_to_nix_rlimit(&rl, rlim);

	return (0);
}

int
nix_setrlimit(int resource, struct nix_rlimit *rlim, nix_env_t *env)
{
  return (nix_nosys(env));
}

nix_pid_t
nix_wait(nix_pid_t wpid, nix_env_t *env)
{
  return (nix_bsd_wait4(wpid, NULL, 0, NULL, env));
}

nix_pid_t
nix_waitpid(nix_pid_t wpid, int options, nix_env_t *env)
{
  return (nix_bsd_wait4(wpid, NULL, options, NULL, env));
}

nix_pid_t
nix_wait3(nix_pid_t wpid, int *status, int options, nix_env_t *env)
{
  return (nix_bsd_wait4(wpid, status, options, NULL, env));
}

int
nix_nice(int incr, nix_env_t *env)
{
#ifdef HAVE_NICE
	if (nice(incr) != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	return (0);
#else
	int prio = nix_getpriority(PRIO_PROCESS, 0, env);
	if (nix_env_get_errno(env) != 0)
		return (-1);

	if (incr == 0)
		return (prio);
  
	return (nix_setpriority(PRIO_PROCESS, 0, prio + incr, env));
#endif
}
