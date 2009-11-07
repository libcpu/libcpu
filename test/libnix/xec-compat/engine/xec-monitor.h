#ifndef __xec_monitor_h
#define __xec_monitor_h

#include "xec-base.h"

#include "xec-param.h"
#include "xec-mem-if.h"

typedef struct _xec_monitor xec_monitor_t;

typedef enum _xec_cback_reason {
	XEC_CBACK_TRAP
} xec_cback_reason_t;

typedef void (*xec_monitor_callback_t)(xec_monitor_t      *xmon,
                                       xec_cback_reason_t  reason,
                                       xec_param_t        *param1,
                                       xec_param_t        *param2);

#ifdef __cplusplus
extern "C" {
#endif

xec_monitor_t *
xec_monitor_create (xec_guest_info_t const *guet_info,
                    xec_mem_if_t           *mem,
                    void                   *context,
                    xec_monitor_callback_t  callback);

xec_mem_if_t *
xec_monitor_get_memory (xec_monitor_t const *xmon);

void *
xec_monitor_get_context (xec_monitor_t const *xmon);

void
xec_monitor_event
  (
    xec_monitor_t      *xmon,
    xec_cback_reason_t  reason,
    xec_param_t        *param1,
    xec_param_t        *param2
  );

#ifdef __cplusplus
}
#endif

#endif /* !__xec_monitor_h */
