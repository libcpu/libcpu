/*
 * XEC - Optimizing Dynarec Engine
 *
 * Translation/Execution Monitor
 * Copyright (C) 2007 Orlando Bassotto. All rights reserved.
 * Copyright (C) 2007 Gianluca Guida. All rights reserved.
 * 
 * $Id: xec-monitor.c 311 2007-06-30 23:21:47Z orlando $
 */

#include <stdio.h>
#include <stdlib.h>

#include "xec-monitor-priv.h"
#include "xec-mem.h"
#include "xec-mmap.h"
#include "xec-debug.h"

void *g_xmon_log = NULL;

xec_monitor_t *
xec_monitor_create (xec_guest_info_t const *guest_info,
                    xec_mem_if_t           *mem,
                    void                   *context,
                    xec_monitor_callback_t  callback)
{
  xec_monitor_t *xmon;

  xmon = xec_mem_alloc_type (xec_monitor_t, 0);
  if (xmon != NULL)
    {
      xmon->mem        = mem;
      xmon->opqctx     = context;
      xmon->callback   = callback;
      xmon->guest_info = *guest_info;
    }

  return xmon;
}

xec_mem_if_t *
xec_monitor_get_memory (xec_monitor_t const *xmon)
{
  return xmon->mem;
}

void *
xec_monitor_get_context (xec_monitor_t const *xmon)
{
  return xmon->opqctx;
}

void
xec_monitor_event (xec_monitor_t      *xmon,
                   xec_cback_reason_t  reason,
                   xec_param_t        *param1,
                   xec_param_t        *param2)
{
  // XMON_LOCK (xmon);

  (*(xmon->callback))(xmon, reason, param1, param2);
  
  // XMON_UNLOCK (xmon);
}

void
xec_monitor_get_guest_info (xec_monitor_t const *xmon,
                            xec_guest_info_t *guest_info)
{
  *guest_info = xmon->guest_info;
}
