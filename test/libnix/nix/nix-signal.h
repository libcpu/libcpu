#ifndef __nix_signal_h
#define __nix_signal_h

#include "nix-types.h"

typedef uintmax_t nix_sigset_t;

struct nix_siginfo;

struct nix_sigaction {
	uintmax_t    __sa_handler;
	int          sa_flags;
#define NIX_SA_ONSTACK   0x0001 /* take signal on signal stack */
#define NIX_SA_RESTART   0x0002 /* restart system on signal return */
#define NIX_SA_DISABLE   0x0004 /* disable taking signals on alternate stack */
#define NIX_SA_RESETHAND 0x0004 /* reset to SIG_DFL when taking signal */
#define NIX_SA_NOCLDSTOP 0x0008 /* do not generate SIGCHLD on child stop */
#define NIX_SA_NODEFER   0x0010 /* don't mask the signal we're delivering */
#define NIX_SA_NOCLDWAIT 0x0020 /* don't keep zombies around */
#define NIX_SA_SIGINFO   0x0040 /* signal handler with SA_SIGINFO args */

	nix_sigset_t sa_mask;
};

struct nix_sigaltstack {
	uintmax_t ss_sp;      /* signal stack pointer */
	int       ss_onstack; /* current status */
	int       ss_flags;
};

struct nix_itimerval {
	struct nix_timeval it_interval;
	struct nix_timeval it_value;
};

#define NIX_ITIMER_REAL    0
#define NIX_ITIMER_VIRTUAL 1
#define NIX_ITIMER_PROF    2

/* BSD Stuff */

#define NIX_SIG_BLOCK   0
#define NIX_SIG_UNBLOCK 1
#define NIX_SIG_SETMASK 2

#endif  /* !__nix_signal_h */
