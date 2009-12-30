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
#include "loader_registry.h"

#include <sys/queue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _loader_image_handler {
	TAILQ_ENTRY(_loader_image_handler)  link;
	loader_image_callbacks_t const     *callbacks;
	char const                         *name;
} loader_image_handler_t;

typedef TAILQ_HEAD(_loader_image_handler_tailq, _loader_image_handler) loader_image_handler_tailq_t;

static loader_image_handler_tailq_t loader_image_handlers;

void
LOADERCALL
loader_registry_init(void)
{
  TAILQ_INIT(&loader_image_handlers);
}

static loader_boolean_t
loader_registry_find_image(char const              *name,
                           loader_image_handler_t **entry)
{
	loader_image_handler_t *handler;

	TAILQ_FOREACH(handler, &loader_image_handlers, link) {
		if (loader_strcmp(name, handler->name) == 0) {
			if (entry != NULL)
				*entry = handler;
			return LOADER_TRUE;
		}
	}

	return LOADER_FALSE;
}

loader_status_t
LOADERCALL
loader_registry_add_handler(loader_image_callbacks_t const  *callbacks,
                            char const                      *name,
                            void                           **cookie)
{
	loader_image_handler_t *entry;

	if (name == NULL || callbacks == NULL || cookie == NULL)
		return LOADER_ERROR_NULL_POINTER;

	if (*name == 0)
		return LOADER_ERROR_INVALID_ARGUMENT;

	if (loader_registry_find_image(name, &entry))
		return LOADER_ERROR_ALREADY_REGISTERED;

	entry = (loader_image_handler_t *)loader_allocate(sizeof(*entry));
	if (entry == NULL)
		return LOADER_ERROR_NO_MEMORY;

	entry->callbacks = callbacks;
	entry->name      = name;
	TAILQ_INSERT_TAIL(&loader_image_handlers, entry, link);

	*cookie = entry;

	return LOADER_SUCCESS;
}

loader_status_t
LOADERCALL
loader_registry_identify_handler(loader_image_t                  *image,
                                 loader_image_ident_t            *ident,
                                 loader_image_callbacks_t const **callbacks)
{
	loader_status_t               status;
	loader_image_ident_t          best_ident;
	loader_image_handler_t const *best_handler = NULL;
	loader_image_handler_t const *handler      = NULL;

	if (image == NULL || ident == NULL || callbacks == NULL)
		return LOADER_ERROR_NULL_POINTER;

	loader_memset(&best_ident, 0, sizeof(best_ident));
	TAILQ_FOREACH(handler, &loader_image_handlers, link) {
		loader_status_t tstatus;
		if (handler->callbacks->identify == NULL)
			continue;

		ident->format = 0;
		ident->score  = 0;
		ident->offset = 0;

		tstatus = (*(handler->callbacks->identify))(image, ident);
		if (tstatus == LOADER_ERROR_INVALID_IMAGE)
			continue;
		else
			status = tstatus;

		if (LOADER_SUCCEEDED (status) &&
			(best_handler == NULL || ident->score > best_ident.score)) {
			best_ident   = *ident;
			best_handler = handler;
		}
	}

	if (best_handler != NULL) {
		*ident     = best_ident;
		*callbacks = best_handler->callbacks;
		return LOADER_SUCCESS;
	}

	return LOADER_ERROR_INVALID_IMAGE;
}
