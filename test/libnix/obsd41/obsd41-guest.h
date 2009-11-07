#ifndef __obsd41_guest_h
#define __obsd41_guest_h

#include "xec-monitor.h"
#include "obsd41-us-syscall-priv.h"

void
obsd41_guest_get_syscall (obsd41_us_syscall_t *self,
                          xec_monitor_t *xmon,
                          int *scno);

int
obsd41_guest_get_next_param (void                *self,
                             xec_monitor_t       *xmon,
                             unsigned             flags,
                             xec_param_type_t     type,
                             xec_param_t         *param);

void
obsd41_guest_set_result (void                *self,
                         xec_monitor_t       *xmon,
                         int                  error,
                         xec_param_t const   *result);

#endif  /* !__obsd41_guest_h */
