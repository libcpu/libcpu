#ifndef __xec_base_h
#define __xec_base_h

#include <sys/types.h>
#include <stddef.h>
#include <stdarg.h>
#include <inttypes.h>
#include <stdbool.h>
#include <assert.h>

#cmakedefine __XEC_HAVE_SYS_QUEUE_H 1
#ifdef __XEC_HAVE_SYS_QUEUE_H
#include <sys/queue.h>
#else
#include "bsdqueue.h"
#endif

#define XEC_OFFSET_OF(type, field) ((uintptr_t)(&((type *)0)->field))

typedef uint64_t uint128_t[2];

typedef int xec_reg_t;
typedef int xec_label_t;

#ifdef XEC64
typedef int64_t  xec_int_t;
typedef uint64_t xec_uint_t;
#else
typedef int32_t  xec_int_t;
typedef uint32_t xec_uint_t;
#endif

typedef uintptr_t xec_haddr_t;
typedef uintmax_t xec_gaddr_t;

typedef enum _xec_endian {
    XEC_ENDIAN_LITTLE,
    XEC_ENDIAN_BIG
} xec_endian_t;

/*XXX: Remove these defs */
#ifdef __LITTLE_ENDIAN__
#define XEC_ENDIAN_NATIVE XEC_ENDIAN_LITTLE
#else
#define XEC_ENDIAN_NATIVE XEC_ENDIAN_BIG
#endif

/*XXX Move this structure away */
typedef struct _xec_guest_info
  {
    char const   *name;

    xec_endian_t  endian;

    uint32_t      byte_size;
    uint32_t      word_size;
    uint32_t      page_size;
  } xec_guest_info_t;

#define XEC_MIN(a,b) ( (a) < (b) ? (a) : (b) )
#define XEC_MAX(a,b) ( (a) > (b) ? (a) : (b) )

static __inline uint32_t
xec_ilog2 (uint32_t n)
{ 
  register int i = (n & 0xffff0000) ? 16 : 0; 
  if ((n >>= i) & 0xff00) i |= 8, n >>= 8; 
  if (n & 0xf0)           i |= 4, n >>= 4; 
  if (n & 0xc)            i |= 2, n >>= 2; 
  return (i | (n >> 1)); 
} 

#ifdef __cplusplus
extern "C" {
#endif

void xec_init(void);

#ifdef __cplusplus
}
#endif

#endif  /* !__xec_base_h */
