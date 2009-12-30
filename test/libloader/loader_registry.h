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
#ifndef __loader_registry_h
#define __loader_registry_h

#include "loader_image.h"
#include "loader_iterator.h"

typedef struct _loader_image_ident {
	loader_uint32_t format;
	loader_sint32_t score;
	loader_size_t   offset;
} loader_image_ident_t;

typedef loader_status_t (LOADERCALL *loader_image_identify_callback_t)(loader_image_t const *image,
                                                                       loader_image_ident_t *ident);

typedef loader_status_t (LOADERCALL *loader_image_open_callback_t)(loader_image_t const   *image,
                                                                   void                  **cookie);
typedef char const * (LOADERCALL *loader_image_get_name_callback_t)(loader_image_t const  *image,
                                                                    void                  *cookie);
typedef void (LOADERCALL *loader_image_close_callback_t)(loader_image_t const  *image,
                                                         void                  *cookie);
typedef loader_status_t (LOADERCALL *loader_image_get_symbol_iterator_callback_t)(loader_image_t const  *image,
                                                                                  loader_flags_t         flags,
																				  loader_iterator_t    **iterator,
																				  void                  *cookie);
typedef loader_status_t (LOADERCALL *loader_image_get_section_iterator_callback_t)(loader_image_t const  *image,
                                                                                   loader_flags_t         flags,
  																				   loader_iterator_t    **iterator,
																				   void                  *cookie);

typedef struct _loader_image_callbacks {
	loader_image_identify_callback_t             identify;
	loader_image_open_callback_t                 open;
	loader_image_close_callback_t                close;
	loader_image_get_name_callback_t             get_name;
	loader_image_get_symbol_iterator_callback_t  get_symbol_iterator;
	loader_image_get_section_iterator_callback_t get_section_iterator;
} loader_image_callbacks_t;

#ifdef __cplusplus
extern "C" {
#endif

void
LOADERCALL
loader_registry_init(void);

loader_status_t
LOADERCALL
loader_registry_add_handler(loader_image_callbacks_t const  *callbacks,
                            char const                      *name,
                            void                           **cookie);

#ifdef __cplusplus
}
#endif

#endif  /* !__loader_registry_h */
