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
#ifndef __loader_iterator_h
#define __loader_iterator_h

#include "loader_base.h"

typedef struct _loader_iterator loader_iterator_t;

typedef void (LOADERCALL *loader_iterator_free_callback_t)(void *opaque); 
typedef loader_boolean_t (LOADERCALL *loader_iterator_reset_callback_t)(void *opaque); 
typedef loader_boolean_t (LOADERCALL *loader_iterator_next_callback_t)(void *value, void *opaque);

typedef struct _loader_iterator_callbacks {
	loader_iterator_free_callback_t  free;
	loader_iterator_reset_callback_t reset;
	loader_iterator_next_callback_t  next;
} loader_iterator_callbacks_t;


#ifdef __cplusplus
extern "C" {
#endif

loader_iterator_t *
LOADERCALL
loader_iterator_new(loader_iterator_callbacks_t const *callbacks,
                    void                              *opaque);

void
LOADERCALL
loader_iterator_dispose(loader_iterator_t *iterator);

loader_boolean_t
LOADERCALL
loader_iterator_reset(loader_iterator_t *iterator);

loader_boolean_t
LOADERCALL
loader_iterator_next(loader_iterator_t *iterator,
                     void              *value);

#ifdef __cplusplus
}
#endif

#endif  /* !__loader_iterator_h */
