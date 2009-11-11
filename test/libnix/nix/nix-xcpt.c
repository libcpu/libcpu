#include <string.h>
#include <signal.h>

#include "nix-xcpt.h"
#include "xec-xcpt.h"
#include "xec-debug.h"

extern void *g_nix_log;

// XXX MOVE TO ENV!
static jmp_buf            *g_xcpt_jb = NULL;
static int                 g_xcpt_installed = 0;
static xec_xcpt_handler_t  g_xcpt_old = NULL;

static int
_nix_xcpt_jmp_back(int   xcptno  /* read: signal no */,
				   void *context /* host cpu context */)
{
	XEC_ASSERT(g_nix_log, g_xcpt_installed);
	longjmp(*g_xcpt_jb, xcptno);
	return 0;
}

void
nix_xcpt_record(jmp_buf *jb)
{
	XEC_ASSERT(g_nix_log, !g_xcpt_installed);
	g_xcpt_jb = jb;
	g_xcpt_installed = 1;
	g_xcpt_old = xec_xcpt_set_handler(_nix_xcpt_jmp_back);
}

void
nix_xcpt_forget(void)
{
	xec_xcpt_set_handler(g_xcpt_old);
    g_xcpt_jb = NULL;
	g_xcpt_installed = 0;
}
