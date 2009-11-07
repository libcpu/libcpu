/*
 * XEC - Optimizing Dynarec Engine
 *
 * Win32 Memory Mapping
 * Copyright (C) 2007 Orlando Bassotto. All rights reserved.
 * 
 * $Id: xec-mmap-win32.c 310 2007-06-30 23:21:01Z orlando $
 */
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include <stdlib.h>

#include "xec-mmap.h"
#include "xec-mem.h"

#ifndef SIZE_T_MAX
# define SIZE_T_MAX ( (1 << (sizeof (size_t) << 3)) - 1)
#endif

#ifndef FILE_MAP_EXECUTE
# define FILE_MAP_EXECUTE 0x00000020
#endif

#ifndef NDEBUG
static void *log_cookie = NULL;
#endif

struct _xec_mmap
  {
    void   *ptr;
    size_t  size;
    HANDLE  hmap;
    HANDLE  hfile;
  };

void
__xec_mmap_init (void)
{
#ifndef NDEBUG
  if (log_cookie == NULL)
    log_cookie = xec_log_register ("mmap");
#endif
}

static __inline uint64_t
_xec_mmap_get_filesize (HANDLE hfile)
{
  DWORD hi, lo;

  lo = GetFileSize (hfile, &hi);

  return ( (uint64_t)hi << 32) | (uint64_t)lo;
}

static __inline size_t
_xec_mmap_get_pagesize (void)
{
  static SYSTEM_INFO si;
  static bool        gotsi = false;
  if (!gotsi)
    {
      GetSystemInfo (&si);
      XEC_ASSERT (log_cookie, si.dwPageSize > 0);
      gotsi = true;
    }
  return si.dwPageSize;
}

static __inline size_t
_xec_mmap_get_min_largepage (void)
{
#ifdef MEM_LARGE_PAGES
  static DWORD pagesize = 0;

  if (pagesize == 0)
    {
      pagesize = GetLargePageMinimum ();
      XEC_ASSERT (log_cookie, pagesize > 0);
    }

  return pagesize;
#else
  return _xec_mmap_get_pagesize ();
#endif
}

static __inline bool
_xec_mmap_get_flags (size_t   *size,
                     unsigned  flags,
                     bool      isfile,
                     DWORD    *mem_flags,
                     DWORD    *prot_flags)
{
  size_t pagesize = _xec_mmap_get_pagesize ();

  if (*size == 0)
    return false;

  *size = (*size + pagesize - 1) & -pagesize; /* Round to floor */
  if (*size == 0)
    *size = pagesize;

  if (isfile)
    *mem_flags = SEC_COMMIT | SEC_RESERVE;
  else
    *mem_flags = MEM_COMMIT | MEM_RESERVE;

  *prot_flags = 0;

  if (!isfile && (flags & XEC_MMAP_SHARED) != 0)
    return false;

#ifdef MEM_LARGE_PAGES
  if (!isfile && *size >= _xec_mmap_get_min_largepage ())
    *mem_flags |= MEM_LARGE_PAGES;
#endif

  switch (flags & (XEC_MMAP_READ | XEC_MMAP_WRITE | XEC_MMAP_EXEC))
    {
    case XEC_MMAP_READ:
      *prot_flags = PAGE_READONLY;
      break;
    case XEC_MMAP_WRITE:
    case XEC_MMAP_READ | XEC_MMAP_WRITE:
      if (flags & XEC_MMAP_SHARED)
        *prot_flags = PAGE_READWRITE;
      else
        *prot_flags = PAGE_WRITECOPY;
      break;
    case XEC_MMAP_EXEC:
      *prot_flags = PAGE_EXECUTE;
      break;
    case XEC_MMAP_READ | XEC_MMAP_EXEC:
      *prot_flags = PAGE_EXECUTE_READ;
      break;
    case XEC_MMAP_WRITE | XEC_MMAP_EXEC:
    case XEC_MMAP_READ | XEC_MMAP_WRITE | XEC_MMAP_EXEC:
      *prot_flags = PAGE_EXECUTE_READWRITE;
      break;
    default:
      *prot_flags = PAGE_NOACCESS;
      break;
    }

  return true;
}

xec_mmap_t *
xec_mmap_create (size_t   size,
                 unsigned flags)
{
  xec_mmap_t *mm;
  DWORD       mem_flags;
  DWORD       prot_flags;

  if (!_xec_mmap_get_flags (&size, flags, false, &mem_flags, &prot_flags))
    return NULL;

  mm = xec_mem_alloc_type (xec_mmap_t, 0);
  if (mm != NULL)
    {
      mm->hmap  = NULL;
      mm->hfile = NULL;

      mm->ptr = VirtualAlloc (NULL, size, mem_flags, prot_flags);
      if (mm->ptr == NULL)
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
  DWORD       acc_flags   = 0;
  DWORD       share_flags = 0;
  DWORD       disp_flags  = OPEN_EXISTING;
  DWORD       attr_flags  = 0;
  DWORD       mem_flags   = 0;
  DWORD       prot_flags  = 0;
  DWORD       map_flags   = 0;
  uint64_t    filesize    = 0;
  HANDLE      hfile       = INVALID_HANDLE_VALUE;
  HANDLE      hmap        = NULL;
  xec_mmap_t *mm          = NULL;

  if (flags & XEC_MMAP_SHARED)
    {
      if ( (flags & (XEC_MMAP_READ | XEC_MMAP_WRITE | XEC_MMAP_EXEC)) != (XEC_MMAP_READ | XEC_MMAP_WRITE))
        return NULL;
    }

  switch (flags & (XEC_MMAP_READ | XEC_MMAP_WRITE | XEC_MMAP_EXEC))
    {
    case XEC_MMAP_READ:
      acc_flags    = GENERIC_READ;
      map_flags    = FILE_MAP_READ; 
      share_flags |= FILE_SHARE_READ;
      break;

    case XEC_MMAP_EXEC:
    case XEC_MMAP_READ | XEC_MMAP_EXEC:
      acc_flags    = GENERIC_READ | GENERIC_EXECUTE;
      map_flags    = FILE_MAP_READ | FILE_MAP_EXECUTE; 
      share_flags |= FILE_SHARE_READ;
      break;
      
    case XEC_MMAP_WRITE:
      acc_flags    = GENERIC_WRITE;
      map_flags    = FILE_MAP_WRITE;
      share_flags |= FILE_SHARE_READ;
      break;

    case XEC_MMAP_READ | XEC_MMAP_WRITE:
      acc_flags    = GENERIC_READ | GENERIC_WRITE;
      map_flags    = FILE_MAP_READ;
      share_flags |= FILE_SHARE_READ;
      if (flags & XEC_MMAP_SHARED)
        map_flags |= FILE_MAP_WRITE;
      else
        map_flags |= FILE_MAP_COPY;
      break;

    case XEC_MMAP_WRITE | XEC_MMAP_EXEC:
    case XEC_MMAP_READ | XEC_MMAP_WRITE | XEC_MMAP_EXEC:
      acc_flags    = GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE;
      map_flags    = FILE_MAP_READ | FILE_MAP_WRITE | FILE_MAP_EXECUTE;
      share_flags |= FILE_SHARE_READ;
      break;

    default:
      return NULL;
    }

  mm = (xec_mmap_t *)calloc (1, sizeof (*mm));
  if (mm != NULL)
    {
      size_t pagesize = _xec_mmap_get_pagesize ();

      /* Open the file... */
      hfile = CreateFileA (path, acc_flags, share_flags, NULL, disp_flags, attr_flags, NULL); /* Use ASCII version */
      if (hfile == INVALID_HANDLE_VALUE)
        goto fail;

      /* See how much we need... */  
      filesize = _xec_mmap_get_filesize (hfile);

      offset = (offset + pagesize - 1) & -pagesize; /* Round to floor */
      if (offset >= filesize)
        goto fail;

      if (size == XEC_MMAP_WHOLE)
        {
          if (filesize >= (SIZE_T_MAX >> 1)) /* Map whole the address space!? */
            goto fail;

          size = filesize;
        }

      size = (size + pagesize) & -pagesize; /* Round to ceil */

      if (offset + size >= filesize)
        size = filesize - offset;

      size = (size + pagesize) & -pagesize; /* Round to ceil */

      if (size >= (SIZE_T_MAX >> 1)) /* Map whole the address space!? */
        goto fail;

      if (!_xec_mmap_get_flags (&size, flags, true, &mem_flags, &prot_flags))
        goto fail;

      /* Create the file mapping... */
      hmap = CreateFileMappingA (hfile, NULL, prot_flags, 0, 0, NULL);
      if (hmap == NULL)
        goto fail;

      /* Map the file (sheesh, 3 steps to mmap a file!?) */
      mm->ptr = MapViewOfFile (hmap, map_flags, (offset >> 32), offset, size);
      if (mm->ptr == NULL)
        goto fail;
      
      mm->hmap  = hmap;
      mm->hfile = hfile;
      mm->size  = size;
    }

  return mm;

fail:
  if (hmap != NULL)
    CloseHandle (hmap);
  if (hfile != INVALID_HANDLE_VALUE)
    CloseHandle (hfile);
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
      if (mm->hmap != NULL)
        {
          XEC_ASSERT (log_cookie, mm->hfile != NULL);
          UnmapViewOfFile (mm->ptr);
          CloseHandle (mm->hmap);
          CloseHandle (mm->hfile);
        }
      else
        {
          XEC_ASSERT (log_cookie, mm->hfile == NULL);
          VirtualFree (mm->ptr, mm->size, MEM_RELEASE);
        }

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
