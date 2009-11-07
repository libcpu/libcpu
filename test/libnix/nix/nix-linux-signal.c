#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "nix.h"

int
nix_linux_sgetmask(nix_env_t *env)
{
	nix_sigset_t mask;

	if (nix_sigprocmask(0, NULL, &mask, env) != 0)
		return (-1);
	else
		return (mask);
}

int
nix_linux_ssetmask(int sigmask, nix_env_t *env)
{
	nix_sigset_t mask = sigmask;
  
	return nix_sigprocmask(NIX_SIG_SETMASK, &mask, NULL, env);
}

int
nix_linux_sigqueueinfo(nix_pid_t pid, int signo, nix_siginfo_t *info,
	nix_env_t *env)
{
	return (nix_nosys(env));
}
