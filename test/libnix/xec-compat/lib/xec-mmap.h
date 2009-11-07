#ifndef __xec_mmap_h
#define __xec_mmap_h

#include "xec-base.h"

typedef struct _xec_mmap xec_mmap_t;

#define XEC_MMAP_READ   0x1
#define XEC_MMAP_WRITE  0x2
#define XEC_MMAP_EXEC   0x4
#define XEC_MMAP_SHARED 0x8

#define XEC_MMAP_WHOLE  ( (size_t)-1)

#ifdef __cplusplus
extern "C" {
#endif

xec_mmap_t *
xec_mmap_create
  (
    size_t      size,
    unsigned    flags
  );

xec_mmap_t *
xec_mmap_create_with_file
  (
    char const *path,
    off_t       offset,
    size_t      size,
    unsigned    flags
  );

void *
xec_mmap_get_bytes
  (
    xec_mmap_t const  *mm
  );

size_t
xec_mmap_get_size
  (
    xec_mmap_t const *mm
  );

void
xec_mmap_free
  (
    xec_mmap_t *mm
  );

#ifdef __cplusplus
}
#endif

#endif  /* !__mmap_h */
