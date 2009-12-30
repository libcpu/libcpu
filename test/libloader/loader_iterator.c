/*
 * Copyright (c) 2007-2008, Orlando Bassotto. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "loader_iterator.h"

#if !defined (LOADER_METRO)
#include <stdlib.h>

struct _loader_iterator
  {
    loader_iterator_callbacks_t const *callbacks;
    void                              *opaque;
  };

__LOADER_FAST_INLINE void
__loader_iterator_free (loader_iterator_t *i)
{
  if (i->callbacks->free != NULL)
    (*(i->callbacks->free))(i->opaque);
}

__LOADER_FAST_INLINE loader_boolean_t
__loader_iterator_reset (loader_iterator_t *i)
{
  if (i->callbacks->free != NULL)
    return (*(i->callbacks->reset))(i->opaque);
  else
    return LOADER_FALSE;
}

__LOADER_FAST_INLINE loader_boolean_t
__loader_iterator_next (loader_iterator_t *i,
                        void              *value)
{
  if (i->callbacks->next != NULL)
    return (*(i->callbacks->next))(value, i->opaque);
  else
    return LOADER_FALSE;
}

loader_iterator_t *
LOADERCALL
loader_iterator_new (loader_iterator_callbacks_t const *callbacks,
                     void                              *opaque)
{
  loader_iterator_t *i;

  if (callbacks == NULL)
    return NULL;

  i = (loader_iterator_t *)malloc (sizeof (*i));
  if (i == NULL)
    return NULL;

  i->callbacks = callbacks;
  i->opaque = opaque;
 
  if (!loader_iterator_reset (i))
    {
      loader_iterator_dispose (i);
      i = NULL;
    }
  return i;
}

void
LOADERCALL
loader_iterator_dispose (loader_iterator_t *iterator)
{
  if (iterator == NULL)
    return;

  __loader_iterator_free (iterator);

  free (iterator);
}

loader_boolean_t
LOADERCALL
loader_iterator_reset (loader_iterator_t *iterator)
{
  if (iterator == NULL)
    return LOADER_FALSE;
  else
    return __loader_iterator_reset (iterator);
}

loader_boolean_t
LOADERCALL
loader_iterator_next (loader_iterator_t *iterator, void *value)
{
  if (iterator == NULL)
    return LOADER_FALSE;
  else
    return __loader_iterator_next (iterator, value);
}
#endif
