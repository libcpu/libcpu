#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include "nix.h"

extern void *g_nix_log;

int
nix_bsd_sigblock(int mask, nix_env_t *env)
{
	nix_sigset_t oset;
	nix_sigset_t set = mask;

	if (nix_sigprocmask(NIX_SIG_BLOCK, &set, &oset, env) != 0)
		return (-1);

	return (oset);
}

int
nix_bsd_sigsetmask(int mask, nix_env_t *env)
{
	nix_sigset_t oset;
	nix_sigset_t set = mask;

	if (nix_sigprocmask(NIX_SIG_SETMASK, &set, &oset, env) < 0)
		return (-1);

	return (oset);
}

int
nix_bsd_sigvec (int signo, struct nix_bsd_sigvec const *sv,
	struct nix_bsd_sigvec *osv, nix_env_t *env)
{
	return (nix_sigaction(signo, (struct nix_sigaction const *)sv,
		(struct nix_sigaction *)osv, env));
}

int
nix_bsd_sigstack (int signo, struct nix_bsd_sigstack const *ss,
	struct nix_bsd_sigstack *oss, nix_env_t *env)
{
	return (nix_sigaltstack(signo, (struct nix_sigaltstack const *)ss,
		(struct nix_sigaltstack *)oss, env));
}

int
nix_bsd_sigreturn(struct nix_bsd_sigcontext *ctx, nix_env_t *env)
{
	return (nix_nosys(env));
}
