/*
 * XEC - Optimizing Dynarec Engine
 *
 * Exception and Unix Signal Handling
 * Copyright (C) 2007 Orlando Bassotto. All rights reserved.
 * 
 * $Id: xec-xcpt.c 310 2007-06-30 23:21:01Z orlando $
 */
#include <signal.h>

#include "xec-debug.h"
#include "xec-xcpt.h"

#ifdef SIGBUS
static struct sigaction    g_old_sigbus;
#endif
static struct sigaction    g_old_sigsegv;
static xec_xcpt_handler_t  g_xcpt_handler = NULL;
static void               *g_xcpt_log = NULL;

void
_xec_signal_handler (int signo)
{
  XEC_ASSERT (g_xcpt_log, g_xcpt_handler != NULL);
  (*g_xcpt_handler)(signo, NULL);
}

xec_xcpt_handler_t
xec_xcpt_set_handler (xec_xcpt_handler_t handler)
{
  xec_xcpt_handler_t oldhandler = g_xcpt_handler;
  g_xcpt_handler = handler;
  return oldhandler;
}

void
__xec_xcpt_init (void)
{
  struct sigaction sa;

  if (g_xcpt_log != NULL)
    return;

  g_xcpt_log = xec_log_register ("xcpt");
    
  sa.sa_handler = _xec_signal_handler;
  sa.sa_flags = 0;
  sigemptyset (&sa.sa_mask);
#ifdef SIGBUS
  sigaction (SIGBUS, &sa, &g_old_sigbus);
#endif
  sigaction (SIGSEGV, &sa, &g_old_sigsegv);
}
