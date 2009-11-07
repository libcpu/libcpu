/*
 * XEC - Optimizing Dynarec Engine
 *
 * Userspace System Call
 * Copyright (C) 2007 Orlando Bassotto. All rights reserved.
 * 
 * $Id: xec-us-syscall.c 311 2007-06-30 23:21:47Z orlando $
 */
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "xec-us-syscall.h"
#include "xec-mem.h"
#include "xec-debug.h"

void *g_xus_log = NULL;

void
xec_us_syscall_dispatch (xec_us_syscall_if_t *xus,
                         xec_monitor_t       *xmon)
{
  xec_param_t                  result;
  int                          err     = 0;
  size_t                       n       = 0;
  size_t                       nparams = 0;
  char const                  *fmt     = NULL;
  xec_us_syscall_desc_t const *desc    = NULL;
  xec_param_t                 *params  = NULL;

  /* Extract the syscall */
  if (!xec_us_syscall_extract (xus, xmon, &desc))
    {
      XEC_LOG (g_xus_log, XEC_LOG_FATAL, 0, "cannot extract system call number from guest context.", 0);
      
      err = ENOSYS;
      goto error;
    }

  if (desc->callback == NULL)
    {
      XEC_LOG (g_xus_log, XEC_LOG_FATAL, 0, "system call descriptor callback is NULL.", 0);
      
      err = ENOSYS;
      goto error;
    }

  XEC_LOG (g_xus_log, XEC_LOG_DEBUG, 0, "system call \"%s\" (%d) is being executed.", desc->name, desc->number);
  result.type              = XEC_PARAM_INVALID;
  result.value.tnosign.u64 = 0;

  nparams = desc->nparams;
  if (desc->flags & XEC_US_SYSCALL_VARIADIC)
    nparams = xec_us_syscall_get_max_params (xus);

  if (nparams == 0)
    params = NULL;
  else
    {
      params = xec_mem_alloc_ntype (xec_param_t, nparams, 0);
      if (params == NULL)
        {
          err = ENOMEM;
          goto error;
        }
    }

  for (fmt = desc->format, n = 0; fmt != NULL && *fmt != 0; fmt++)
    {
      bool ellipsis         = false;
      xec_param_type_t type = XEC_PARAM_INVALID;

      switch (*fmt)
        {
        case '*': ellipsis = true;           break;
        case 'b': type = XEC_PARAM_BYTE;     break;
        case 'h': type = XEC_PARAM_HALF;     break;
        case 'w': type = XEC_PARAM_WORD;     break;
        case 'd': type = XEC_PARAM_DWORD;    break;
        case 'S': type = XEC_PARAM_SINGLE;   break;
        case 'D': type = XEC_PARAM_DOUBLE;   break;
        case 'X': type = XEC_PARAM_EXTENDED; break;
        case 'p': type = XEC_PARAM_POINTER;  break;
        case 'v': type = XEC_PARAM_VECTOR;   break;
        case 'l': type = XEC_PARAM_INTPTR;   break;
        default:  XEC_ASSERT (g_xus_log, 0); break;
        }

      if (ellipsis)
        break;

      err = xec_us_syscall_get_next_param (xus, xmon, desc->flags, type, params + n);
      if (err != 0)
        goto error;

      n++;
    }

  if (desc->flags & XEC_US_SYSCALL_VARIADIC)
    {
      for (; n < nparams; n++)
        {
          err = xec_us_syscall_get_next_param (xus, xmon, desc->flags, XEC_PARAM_WORD, params + n);
          if (err != 0)
            goto error;
        }
    }
  
  if (desc->rettype != NULL)
    {
      switch (*desc->rettype)
        {
        case 'b': result.type = XEC_PARAM_BYTE;     break;
        case 'h': result.type = XEC_PARAM_HALF;     break;
        case 'w': result.type = XEC_PARAM_WORD;     break;
        case 'd': result.type = XEC_PARAM_DWORD;    break;
        case 'S': result.type = XEC_PARAM_SINGLE;   break;
        case 'D': result.type = XEC_PARAM_DOUBLE;   break;
        case 'X': result.type = XEC_PARAM_EXTENDED; break;
        case 'p': result.type = XEC_PARAM_POINTER;  break;
        case 'v': result.type = XEC_PARAM_VECTOR;   break;
        case 'l': result.type = XEC_PARAM_INTPTR;   break;
        default:  XEC_BUGCHECK (g_xus_log, 9991);   break;
        }
    }

  xec_us_syscall_set_retype (xus, XEC_PARAM_INVALID);

  err = (*(desc->callback))(xus, xmon, params, &result);

  if (params != NULL)
    xec_mem_free (params);

  if (err == ENOSYS)
    {
      XEC_LOG (g_xus_log, XEC_LOG_FATAL, 0, "system call \"%s\" (%d) is not implemented.", desc->name, desc->number);
    }

error:
  if (xec_us_syscall_get_retype (xus) != XEC_PARAM_INVALID)
    result.type = xec_us_syscall_get_retype (xus);

  xec_us_syscall_set_result (xus, xmon, err, &result);
}

int
xec_us_syscall_redispatch (xec_us_syscall_if_t *xus,
                           xec_monitor_t       *xmon,
                           int                  scno,
                           xec_param_t const   *params,
                           void                *result)
{
  xec_us_syscall_desc_t const *desc;
  xec_param_t                 *newparams;
  char const                  *fmt;
  size_t                       n, m, nnewparams;
  xec_param_t                  retval;
  int                          err;

  /* Find the syscall */
  if (!xec_us_syscall_find (xus, scno, &desc)
      || desc->callback == NULL)
    {
      return ENOSYS;
    }

  nnewparams = desc->nparams;
  if (desc->flags & XEC_US_SYSCALL_VARIADIC)
    nnewparams = xec_us_syscall_get_max_params (xus);

  if (nnewparams == 0)
    newparams = NULL;
  else
    {
      newparams = xec_mem_alloc_ntype (xec_param_t, nnewparams, 0);
      if (newparams == NULL)
        {
          return ENOMEM;
        }
    }

  for (fmt = desc->format, m = n = 0; fmt != NULL && *fmt != 0; fmt++)
    {
      bool ellipsis         = false;
      xec_param_type_t type = XEC_PARAM_INVALID;

      switch (*fmt)
        {
        case '*': ellipsis = true;           break;
        case 'b': type = XEC_PARAM_BYTE;     break;
        case 'h': type = XEC_PARAM_HALF;     break;
        case 'w': type = XEC_PARAM_WORD;     break;
        case 'd': type = XEC_PARAM_DWORD;    break;
        case 'S': type = XEC_PARAM_SINGLE;   break;
        case 'D': type = XEC_PARAM_DOUBLE;   break;
        case 'X': type = XEC_PARAM_EXTENDED; break;
        case 'p': type = XEC_PARAM_POINTER;  break;
        case 'v': type = XEC_PARAM_VECTOR;   break;
        case 'l': type = XEC_PARAM_INTPTR;   break;
        default:  XEC_ASSERT (g_xus_log, 0); break;
        }

      if (ellipsis)
        break;

      XEC_ASSERT (g_xus_log, params[n].type == XEC_PARAM_WORD);
      
      newparams[m].type = type;
      if (type == XEC_PARAM_DWORD)
        {
          /* XXX THIS IS GUEST DEPENDANT! */
          newparams[m].value.tnosign.u64 = ( (uint64_t)params[n].value.tnosign.u32 << 32)
                                         | (uint64_t)params[n+1].value.tnosign.u32;
          m++, n += 2;
        }
      else
        {
          newparams[m++] = params[n++];
        }
    }

  err = (*(desc->callback))(xus, xmon, newparams, &retval);

  xec_us_syscall_set_retype (xus, retval.type);

  if (err == 0)
    *(uint64_t *)result = retval.value.tnosign.u64;

  if (newparams != NULL)
    xec_mem_free (newparams);

  return err;
}
