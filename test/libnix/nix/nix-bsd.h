#ifndef __nix_bsd_h
#define __nix_bsd_h

#include "nix-types.h"

struct nix_bsd_sigvec {
	uintmax_t sv_handler;
	int       sv_mask;
	int       sv_flags;
#define NIX_SV_ONSTACK   NIX_SA_ONSTACK
#define NIX_SV_INTERRUPT NIX_SA_RESTART  /* same bit, opposite sense */
#define NIX_SV_RESETHAND NIX_SA_RESETHAND
#define NIX_SV_NODEFER   NIX_SA_NODEFER
#define NIX_SV_NOCLDSTOP NIX_SA_NOCLDSTOP
#define NIX_SV_SIGINFO   NIX_SA_SIGINFO
};

struct nix_bsd_sigstack {
	uintmax_t ss_sp;      /* signal stack pointer */
	int       ss_onstack; /* current status */
	int       ss_padding;
};

struct nix_bsd_sigcontext;

struct nix_bsd_kevent;

#endif  /* !__nix_bsd_h */
