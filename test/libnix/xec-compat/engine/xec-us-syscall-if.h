/*
 * XEC - Optimizing Dynarec Engine
 *
 * Userspace System Call Interface
 * Copyright (C) 2007 Orlando Bassotto. All rights reserved.
 * 
 * $Id: xec-us-syscall-if.h 253 2007-06-22 23:07:24Z orlando $
 */

#ifndef __xec_us_syscall_if_h
#define __xec_us_syscall_if_h

#include "xec-monitor.h"

typedef struct _xec_us_syscall_if xec_us_syscall_if_t;

typedef int (*xec_us_syscall_callback_t)(xec_us_syscall_if_t *xus,
                                         xec_monitor_t       *xmon,
                                         xec_param_t const   *args,
                                         xec_param_t         *retval);

typedef struct _xec_us_syscall_desc
  {
    int                        number;
    char const                *name;
    char const                *format;
    char const                *rettype;
    unsigned                   flags;
#define XEC_US_SYSCALL_VARIADIC 1
    size_t                     nparams;
    xec_us_syscall_callback_t  callback;
  } xec_us_syscall_desc_t;

typedef struct _xec_us_syscall_if_vtbl xec_us_syscall_if_vtbl_t;

struct _xec_us_syscall_if_vtbl
  {
    size_t (*get_max_params)(void *self);

    bool (*find)(void                         *self,
                 int                           scno,
                 xec_us_syscall_desc_t const **desc);

    bool (*extract)(void                         *self,
                    xec_monitor_t                *xmon,
                    xec_us_syscall_desc_t const **desc);

    int (*get_next_param)(void                *self,
                          xec_monitor_t       *xmon,
                          unsigned             flags,
                          xec_param_type_t     type,
                          xec_param_t         *param);

    void (*set_result)(void                *self,
                       xec_monitor_t       *xmon,
                       int                  error,
                       xec_param_t const   *result);

    xec_param_type_t (*get_retype)(void *self);
    void (*set_retype)(void *self, xec_param_type_t type);
  };

struct _xec_us_syscall_if
  {
    struct _xec_us_syscall_if_vtbl const *vtbl;
  };

#define xec_us_syscall_get_max_params(self) \
  (self)->vtbl->get_max_params (self)

#define xec_us_syscall_extract(self, xmon, desc) \
  (self)->vtbl->extract (self, xmon, desc)

#define xec_us_syscall_find(self, scno, desc) \
  (self)->vtbl->find (self, scno, desc)

#define xec_us_syscall_get_next_param(self, xmon, flags, type, param) \
  (self)->vtbl->get_next_param (self, xmon, flags, type, param)

#define xec_us_syscall_set_result(self, xmon, error, result) \
  (self)->vtbl->set_result (self, xmon, error, result)

#define xec_us_syscall_get_retype(self) \
  (self)->vtbl->get_retype (self)

#define xec_us_syscall_set_retype(self, type) \
  (self)->vtbl->set_retype (self, type)

#endif  /* !__xec_us_syscall_if_h */
