#ifndef __xec_monitor_priv_h
#define __xec_monitor_priv_h

#include "xec-mmap.h"
#include "xec-monitor.h"

#if 0
#define XEC_MONITOR_CACHELINE_COUNT 32

typedef struct _xec_monitor_cachelines
  {
    union {
      uintmax_t addr64;
      struct {
        uint32_t addr32_h;
        uint32_t addr32_l;
      };
    } addr[XEC_MONITOR_CACHELINE_COUNT];
    union {
      uintmax_t value64;
      struct {
        uint32_t value32_h;
        uint32_t value32_l;
      };
    } value[XEC_MONITOR_CACHELINE_COUNT];
    size_t    size[XEC_MONITOR_CACHELINE_COUNT];
  } xec_monitor_cachelines_t;

static __inline int __xec_monitor_cacheline_hash (uintmax_t p)
{ return ( (p >> 3) ^ p) & (XEC_MONITOR_CACHELINE_COUNT - 1); }
#endif
        
typedef struct _xec_cback_env
  {
    xec_cback_reason_t reason;
    xec_param_t        param1;
    xec_param_t        param2;
  } xec_cback_env_t;

struct _xec_monitor
  {
    void             *opqctx;
    xec_mem_if_t     *mem;

    xec_monitor_callback_t callback;
    xec_cback_env_t        cbe;
    xec_guest_info_t       guest_info;
#if 0
    xec_monitor_cachelines_t cache;
#endif
  };

#endif  /* !__xec_monitor_priv_h */
