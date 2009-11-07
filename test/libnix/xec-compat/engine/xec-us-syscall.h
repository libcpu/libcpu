/*
 * XEC - Optimizing Dynarec Engine
 *
 * Userspace System Call
 * Copyright (C) 2007 Orlando Bassotto. All rights reserved.
 * 
 * $Id: xec-us-syscall.h 125 2007-06-14 00:38:57Z orlando $
 */

#ifndef __xec_us_syscall_h
#define __xec_us_syscall_h

#include "xec-us-syscall-if.h"

#ifdef __cplusplus
extern "C" {
#endif

void
xec_us_syscall_dispatch
  (
    xec_us_syscall_if_t *xus,
    xec_monitor_t       *xmon
  );

int
xec_us_syscall_redispatch
  (
    xec_us_syscall_if_t *xus,
    xec_monitor_t       *xmon,
    int                  scno,
    xec_param_t const   *params,
    void                *result
  );

#ifdef __cplusplus
}
#endif

#endif  /* !__xec_us_syscall_h */
