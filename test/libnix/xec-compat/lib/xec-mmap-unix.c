/*
 * XEC - Optimizing Dynarec Engine
 *
 * Unix Memory Mapping
 * Copyright (C) 2007 Orlando Bassotto. All rights reserved.
 * 
 * $Id: xec-mmap-unix.c 310 2007-06-30 23:21:01Z orlando $
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "xec-mmap.h"
#include "xec-mem.h"
#include "xec-debug.h"

#ifndef MAP_ANON
# ifdef MAP_ANONYMOUS
#  define MAP_ANON MAP_ANONYMOUS
# else
#  define MAP_ANON 0
# endif
#endif

#ifndef MAP_PRIVATE
# define MAP_PRIVATE 0
#endif

#ifndef MAP_FAILED
# define MAP_FAILED ( (void *)-1)
#endif

#ifndef SIZE_T_MAX
# define SIZE_T_MAX ( (size_t)(-1U) )
#endif

#ifndef NDEBUG
static void *log_cookie = NULL;
#endif

struct _xec_mmap
  {
    void   *ptr;
    size_t  size;
  };

void
__xec_mmap_init (void)
{
#ifndef NDEBUG
  if (log_cookie == NULL)
    log_cookie = xec_log_register ("mmap");
#endif
}

static __inline size_t
_xec_mmap_get_pagesize (void)
{
  static int g_pagesize = -1;
  if (g_pagesize < 0)
    {
      g_pagesize = getpagesize ();
      XEC_ASSERT (log_cookie, g_pagesize > 0);
    }
  return g_pagesize;
}

static __inline bool
_xec_mmap_get_flags (size_t   *size,
                     unsigned  flags,
                     bool      isfile,
                     int      *mmap_flags,
                     int      *mmap_prot)
{
  size_t pagesize = _xec_mmap_get_pagesize ();

  if (*size == 0)
    return false;

  *size = (*size + pagesize - 1) & -pagesize; /* Round to floor */
  if (*size == 0)
    *size = pagesize;

  if (!isfile)
    *mmap_flags = MAP_ANON;
  else
    *mmap_flags = 0;

  if (flags & XEC_MMAP_SHARED)
#ifdef MAP_SHARED
    *mmap_flags |= MAP_SHARED;
#else
    return false;
#endif
  else
    *mmap_flags |= MAP_PRIVATE;

  *mmap_prot = 0;
  if (flags & XEC_MMAP_READ)
    *mmap_prot |= PROT_READ;
  if (flags & XEC_MMAP_WRITE)
    *mmap_prot |= PROT_WRITE;
  if (flags & XEC_MMAP_EXEC)
    *mmap_prot |= PROT_EXEC;

  return true;
}

xec_mmap_t *
xec_mmap_create (size_t   size,
                 unsigned flags)
{
  xec_mmap_t *mm;
  int         mmap_flags;
  int         mmap_prot;

  if (!_xec_mmap_get_flags (&size, flags, false, &mmap_flags, &mmap_prot))
    return NULL;

  mm = xec_mem_alloc_type (xec_mmap_t, 0);
  if (mm != NULL)
    {
      mm->ptr = mmap (NULL, size, mmap_prot, mmap_flags, -1, 0);
      if (mm->ptr == MAP_FAILED)
        {
          xec_mem_free (mm);
          mm = NULL;
        }
      else
        {
          mm->size = size;
        }
    }

  return mm;
}

xec_mmap_t *
xec_mmap_create_with_file (char const *path,
                           off_t       offset,
                           size_t      size,
                           unsigned    flags)
{
  struct stat  st;
  int          mmap_flags;
  int          mmap_prot;
  int          fd         = -1;
  int          open_flags = 0;
  xec_mmap_t  *mm         = NULL;

  switch (flags & (XEC_MMAP_READ | XEC_MMAP_WRITE))
    {
    case XEC_MMAP_READ:
      open_flags  = O_RDONLY;
      break;
    case XEC_MMAP_WRITE:
      open_flags  = O_WRONLY; 
      flags      |= XEC_MMAP_SHARED;
      break;
    case XEC_MMAP_READ | XEC_MMAP_WRITE:
      open_flags  = O_RDWR;
      flags      |= XEC_MMAP_SHARED;
      break;
    default:
      return NULL;
    }

  mm = xec_mem_alloc_type (xec_mmap_t, 0);
  if (mm != NULL)
    {
      size_t pagesize = _xec_mmap_get_pagesize ();

      fd = open (path, open_flags);
      if (fd < 0)
        goto fail;

      if (fstat (fd, &st) < 0)
        goto fail;

      offset = (offset + pagesize - 1) & -pagesize; /* Round to floor */
      
      if (offset >= st.st_size)
        goto fail;

      if (size == XEC_MMAP_WHOLE)
        {
          /* Mapping whole the address space!? */
          if (sizeof (off_t) >= sizeof (size_t)
              && st.st_size >= (off_t)(SIZE_T_MAX >> 1))
            goto fail;

          size = st.st_size;
        }

      size = (size + pagesize) & -pagesize; /* Round to ceil */

      if (offset + size >= st.st_size)
        size = st.st_size - offset;

      size = (size + pagesize) & -pagesize; /* Round to ceil */

      if (size >= (SIZE_T_MAX >> 1)) /* Map whole the address space!? */
        goto fail;

      if (!_xec_mmap_get_flags (&size, flags, true, &mmap_flags, &mmap_prot))
        goto fail;

      mm->ptr = mmap (NULL, size, mmap_prot, mmap_flags, fd, offset);
      if (mm->ptr == MAP_FAILED)
        goto fail;
      
      mm->size = size;

      close (fd); /* Close file */
    }

  return mm;

fail:
  if (!(fd < 0))
    close (fd);
  if (mm != NULL)
    xec_mem_free (mm);
  return NULL;
}

void
xec_mmap_free (xec_mmap_t *mm)
{
  XEC_ASSERT (log_cookie, mm != NULL);
  if (mm != NULL)
    {
      munmap (mm->ptr, mm->size);
      xec_mem_free (mm);
    }
}

void *
xec_mmap_get_bytes (xec_mmap_t const *mm)
{
  return mm->ptr;
}

size_t
xec_mmap_get_size (xec_mmap_t const *mm)
{
  return mm->size;
}
