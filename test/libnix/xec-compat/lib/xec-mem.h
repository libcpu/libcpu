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
 *	$Id: xec-mem.h 224 2007-06-19 15:41:42Z orlando $
 *
 */

#ifndef __xec_mem_h
#define __xec_mem_h

#include "xec-base.h"

#define XEC_MEM_ZERO   0x0001
#define XEC_MEM_RANDOM 0x0002

#ifndef NDEBUG
#define xec_mem_alloc(size, flags) \
  _xec_mem_dbg_alloc (__func__, __FILE__, __LINE__, size, flags)
#define xec_mem_realloc(ptr, size, flags) \
  _xec_mem_dbg_realloc (__func__, __FILE__, __LINE__, ptr, size, flags)
#define xec_mem_freef(ptr, flags) \
  _xec_mem_dbg_free (__func__, __FILE__, __LINE__, ptr, flags)
#define xec_mem_strdup(string) \
  _xec_mem_dbg_strdup (__func__, __FILE__, __LINE__, string)
#define xec_mem_strndup(string, length) \
  _xec_mem_dbg_strndup (__func__, __FILE__, __LINE__, string, length)
#else
#define xec_mem_alloc(size, flags) \
  _xec_mem_std_alloc (size, flags)
#define xec_mem_realloc(ptr, size, flags) \
  _xec_mem_std_realloc (ptr, size, flags)
#define xec_mem_freef(ptr, flags) \
  _xec_mem_std_free (ptr, flags)
#define xec_mem_strdup(string) \
  _xec_mem_std_strdup (string)
#define xec_mem_strndup(string, length) \
  _xec_mem_std_strndup (string, length)
#endif
#define xec_mem_free(ptr) \
  xec_mem_freef (ptr, 0)
#define xec_mem_alloc_type_and_size(type, size, flags) \
  ( (type *)xec_mem_alloc (sizeof (type) + (size), (flags) | XEC_MEM_ZERO))
#define xec_mem_alloc_type(type, flags) \
  ( (type *)xec_mem_alloc_type_and_size (type, 0, (flags)))
#define xec_mem_alloc_ntype(type, count, flags) \
  ( (type *)xec_mem_alloc (sizeof (type) * (count), (flags) | XEC_MEM_ZERO))

#ifdef __cplusplus
extern "C" {
#endif

void *
_xec_mem_dbg_alloc
  (
    char const *funcname,
    char const *filename,
    int         lineno,
    size_t      size,
    uint32_t    flags
  );

void *
_xec_mem_std_alloc
  (
    size_t   size,
    uint32_t flags
  );

void
_xec_mem_dbg_free
  (
    char const *funcname,
    char const *filename,
    int         lineno,
    void       *ptr,
    uint32_t    flags
  );

void
_xec_mem_std_free
  (
    void     *ptr,
    uint32_t  flags
  );

void *
_xec_mem_dbg_realloc
  (
    char const *funcname,
    char const *filename,
    int         lineno,
    void       *ptr,
    size_t      newsize,
    uint32_t    flags
  );

void *
_xec_mem_std_realloc
  (
    void     *ptr,
    size_t    newsize,
    uint32_t  flags
  );

char *
_xec_mem_dbg_strdup
  (
    char const *funcname,
    char const *filename,
    int         lineno,
    char const *string
  );

char *
_xec_mem_std_strdup
  (
    char const *string
  );

char *
_xec_mem_dbg_strndup
  (
    char const *funcname,
    char const *filename,
    int         lineno,
    char const *string,
    size_t      length
  );

char *
_xec_mem_std_strndup
  (
    char const *string,
    size_t      length
  );

#ifdef __cplusplus
}
#endif

#endif  /* !__xec_mem_h */
