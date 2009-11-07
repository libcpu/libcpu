/*-
 * Copyright 2006 Orlando Bassotto.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	$Id: xec-mem.c 224 2007-06-19 15:41:42Z orlando $
 *
 */

#include <stdlib.h>
#include <string.h>

#include "xec-debug.h"
#include "xec-mem.h"

static void *g_mem_log = NULL;

#define __MAKE_MAGIC(a,b,c,d) ( (a) << 24 | (b) << 16 | (c) << 8 | (d) )

#define _MEMORY_MAGIC1 __MAKE_MAGIC ('m','g','c','1')
#define _MEMORY_MAGIC2 __MAKE_MAGIC ('2','c','g','m')
#define _MEMORY_MAGIC3 __MAKE_MAGIC (  0,'m','g','c')

typedef struct _xec_mem_dbg_header
  {
    uint32_t    magic;
    size_t      size;
    uint32_t   *begin;
    uint32_t   *end;
    char const *filename;
    char const *funcname;
    int         lineno;
    uint32_t    magic2;
  } xec_mem_dbg_header_t;

typedef struct _xec_mem_std_header
  {
    uint32_t    magic;
    size_t      size;
    uint32_t   *begin;
    uint32_t   *end;
    uint32_t    magic2;
  } xec_mem_std_header_t;

void
__xec_mem_init (void)
{
  if (g_mem_log == NULL)
    g_mem_log = xec_log_register ("malloc");
}

void *
_xec_mem_dbg_alloc (char const *funcname,
                    char const *filename,
                    int         lineno,
                    size_t      size,
                    uint32_t    flags)
{
  xec_mem_dbg_header_t *object;
  void                 *data;

  object = (xec_mem_dbg_header_t *)malloc (sizeof (*object) + size + sizeof (uint32_t));
  if (object == NULL)
    return NULL;

  object->magic    = _MEMORY_MAGIC1;
  object->size     = size;
  object->begin    = &object->magic2;
  object->end      = (uint32_t *)( (uintptr_t)object + sizeof (*object) + size);
  object->filename = filename;
  object->lineno   = lineno;
  object->funcname = funcname;
  object->magic2   = _MEMORY_MAGIC2;
  *(object->end)   = _MEMORY_MAGIC3;

  data = (void *)(object->begin + 1);
  if (flags & XEC_MEM_ZERO)
    memset (data, 0, size);

  return data;
}

void *
_xec_mem_std_alloc (size_t   size,
                    uint32_t flags)
{
  xec_mem_std_header_t *object;
  void                 *data;

  object = (xec_mem_std_header_t *)malloc (sizeof (*object) + size + sizeof (uint32_t));
  if (object == NULL)
    return NULL;

  object->magic    = _MEMORY_MAGIC1;
  object->size     = size;
  object->begin    = &object->magic2;
  object->end      = (uint32_t *)( (uintptr_t)object + sizeof (*object) + size);
  object->magic2   = _MEMORY_MAGIC2;
  *(object->end)   = _MEMORY_MAGIC3;

  data = (void *)(object->begin + 1);
  if (flags & XEC_MEM_ZERO)
    memset (data, 0, size);

  return data;
}

void
_xec_mem_dbg_free (char const *funcname,
                   char const *filename,
                   int         lineno,
                   void       *ptr,
                   uint32_t    flags)
{
  xec_mem_dbg_header_t *object;
  
  if (ptr == NULL)
    {
      XEC_LOG (g_mem_log,
               XEC_LOG_FATAL,
               0,
               "freeing NULL pointer in function %s file %s:%u.",
               funcname,
               filename,
               lineno);
    }

  if ( (uintptr_t)ptr & 1) /* Unaligned? */
    {
      XEC_LOG (g_mem_log,
               XEC_LOG_FATAL,
               0,
               "freeing unaligned pointer %p in function %s file %s:%u.",
               ptr,
               funcname,
               filename,
               lineno);
    }

  if (*( (uint32_t *)ptr - 1) != _MEMORY_MAGIC2)
    {
      XEC_LOG (g_mem_log,
               XEC_LOG_ERROR,
               0,
               "freeing pointer %p which is not allocated with "
               "the allocator, in function %s file %s:%u.",
               ptr,
               funcname,
               filename,
               lineno);
      return;
    }

  object = (xec_mem_dbg_header_t *)( (uintptr_t)ptr - sizeof (*object));
  if (object->magic != _MEMORY_MAGIC1)
    {
      XEC_LOG (g_mem_log,
               XEC_LOG_FATAL,
               0,
               "pointer %p magic #1 is invalid, this may have been "
               "caused by a buffer underflow or overflow, from function %s "
               "file %s:%u.",
               ptr,
               funcname,
               filename,
               lineno);
    }

  if (*(object->end) != _MEMORY_MAGIC3)
    {
      XEC_LOG (g_mem_log,
               XEC_LOG_FATAL,
               0,
               "pointer %p, allocated in function %s "
               "in file %s:%u, magic #3 is invalid, this may have been "
               "caused by a buffer overflow.",
               ptr,
               object->funcname,
               object->filename,
               object->lineno,
               funcname,
               filename,
               lineno);
    }

  if (flags & XEC_MEM_ZERO)
    memset (ptr, 0, object->size);

  free (object);
}

void
_xec_mem_std_free (void     *ptr,
                   uint32_t  flags)
{
  xec_mem_std_header_t *object;

  if (ptr == NULL)
    return;

  if ( (uintptr_t)ptr & 1) /* Unaligned? */
    return;

  if (*( (uint32_t *)ptr - 1) != _MEMORY_MAGIC2)
    return;

  object = (xec_mem_std_header_t *)( (uintptr_t)ptr - sizeof (*object));
  if (object->magic != _MEMORY_MAGIC1)
    {
      /* Warn! */
    }

  if (*(object->end) != _MEMORY_MAGIC3)
    {
      /* Warn data overwrite! */
    }

  if (flags & XEC_MEM_ZERO)
    memset (ptr, 0, object->size);

  free (object);
}

void *
_xec_mem_dbg_realloc (char const *funcname,
                       char const *filename,
                       int lineno,
                       void *ptr,
                       size_t newsize,
                       uint32_t flags)
{
  xec_mem_dbg_header_t *object;
  void                 *nptr;
  size_t                copylen;

  if (ptr == NULL)
    return _xec_mem_dbg_alloc (funcname, filename, lineno, newsize, flags);

  if ( (uintptr_t)ptr & 1) /* Unaligned? */
    {
      XEC_LOG (g_mem_log,
               XEC_LOG_FATAL,
               0,
               "passing unaligned pointer %p in function %s file %s:%u.\n",
               ptr,
               funcname,
               filename,
               lineno);
    }

  if (*( (uint32_t *)ptr - 1) != _MEMORY_MAGIC2)
    {
      XEC_LOG (g_mem_log,
               XEC_LOG_FATAL,
               0,
               "reallocating pointer %p which is not allocated with "
               "the allocator, in function %s file %s:%u.",
               ptr,
               funcname,
               filename,
               lineno);
    }

  object = (xec_mem_dbg_header_t *)( (uintptr_t)ptr - sizeof (*object));
  if (object->magic != _MEMORY_MAGIC1)
    {
      XEC_LOG (g_mem_log,
               XEC_LOG_FATAL,
               0,
               "pointer %p magic #1 is invalid, this may have been "
               "caused by a buffer underflow or overflow, from function %s "
               "file %s:%u.",
               ptr,
               funcname,
               filename,
               lineno);
    }

  if (*(object->end) != _MEMORY_MAGIC3)
    {
      XEC_LOG (g_mem_log,
               XEC_LOG_FATAL,
               0,
               "pointer %p, allocated in function %s "
               "in file %s:%u, magic #3 is invalid, this may have been "
               "caused by a buffer overflow.",
               ptr,
               object->funcname,
               object->filename,
               object->lineno,
               funcname,
               filename,
               lineno);
    }

  if (object->size == newsize)
    return ptr;

  nptr = _xec_mem_dbg_alloc (funcname, filename, lineno, newsize, flags);
  if (nptr == NULL)
    return NULL;

  copylen = XEC_MIN (newsize, object->size);
  memcpy (nptr, ptr, copylen);

  if ( (flags & XEC_MEM_ZERO) != 0 && copylen < newsize)
    {
      memset ( (uint8_t *)nptr + copylen, 0, newsize - copylen);
    }

  free (object);
  return nptr;
}

void *
_xec_mem_std_realloc (void     *ptr,
                      size_t    newsize,
                      uint32_t  flags)
{
  xec_mem_std_header_t *object;
  void                 *nptr;
  size_t                copylen;

  if (ptr == NULL)
    return _xec_mem_std_alloc (newsize, flags);

  if ( (uintptr_t)ptr & 1) /* Unaligned? */
    return NULL;

  if (*( (uint32_t *)ptr - 1) != _MEMORY_MAGIC2)
    return NULL;

  object = (xec_mem_std_header_t *)( (uintptr_t)ptr - sizeof (*object));
  if (object->magic != _MEMORY_MAGIC1)
    {
      return NULL;
    }

  if (*(object->end) != _MEMORY_MAGIC3)
    {
      return NULL;
    }

  if (object->size == newsize)
    return ptr;

  nptr = _xec_mem_std_alloc (newsize, flags);
  if (nptr == NULL)
    return NULL;

  copylen = XEC_MIN (newsize, object->size);
  memcpy (nptr, ptr, copylen);

  if ( (flags & XEC_MEM_ZERO) != 0 && copylen < newsize)
    memset ( (uint8_t *)nptr + copylen, 0, newsize - copylen);

  free (object);

  return nptr;
}

char *
_xec_mem_dbg_strdup (char const *funcname,
                     char const *filename,
                     int         lineno,
                     char const *string)
{
  char   *nstring;
  size_t  length;
  
  if (string == NULL)
    {
      XEC_LOG (g_mem_log,
               XEC_LOG_WARNING,
               0,
               "duplicating NULL string in function %s file %s:%u.",
               string,
               funcname,
               filename,
               lineno);
      return NULL;
    }
  
  length = strlen (string) + 1;
  nstring = _xec_mem_dbg_alloc (funcname, filename, lineno, length + 1, 0);
  if (nstring == NULL)
    return NULL;

  memcpy (nstring, string, length);
  nstring[length] = 0;

  return nstring;
}

char *
_xec_mem_dbg_strndup (char const *funcname,
                      char const *filename,
                      int         lineno,
                      char const *string,
                      size_t      length)
{
  char   *nstring;
  
  if (string == NULL)
    {
      XEC_LOG (g_mem_log,
               XEC_LOG_WARNING,
               0,
               "duplicating NULL string in function %s file %s:%u.",
               string,
               funcname,
               filename,
               lineno);
      return NULL;
    }
  
  nstring = _xec_mem_dbg_alloc (funcname, filename, lineno, length + 1, 0);
  if (nstring == NULL)
    return NULL;

  memcpy (nstring, string, length);
  nstring[length] = 0;

  return nstring;
}

char *
_xec_mem_std_strdup (char const *string)
{
  char   *nstring;
  size_t  length;
  
  if (string == NULL)
    return NULL;
  
  length = strlen (string) + 1;
  nstring = _xec_mem_std_alloc (length + 1, 0);
  if (nstring == NULL)
    return NULL;

  memcpy (nstring, string, length);
  nstring[length] = 0;

  return nstring;
}

char *
_xec_mem_std_strndup (char const *string,
                      size_t      length)
{
  char   *nstring;
  
  if (string == NULL)
    return NULL;
  
  nstring = _xec_mem_std_alloc (length + 1, 0);
  if (nstring == NULL)
    return NULL;

  memcpy (nstring, string, length);
  nstring[length] = 0;

  return nstring;
}
