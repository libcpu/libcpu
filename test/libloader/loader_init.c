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

extern loader_status_t LOADERCALL elf32_image_register(void);
extern loader_status_t LOADERCALL elf64_image_register(void);
#if 0
extern loader_status_t LOADERCALL mz_image_register(void);
extern loader_status_t LOADERCALL coff_image_register(void);
extern loader_status_t LOADERCALL pe32_image_register(void);
extern loader_status_t LOADERCALL pe64_image_register(void);
#endif

loader_status_t
LOADERCALL
loader_init(void)
{
	loader_status_t status;

	loader_registry_init();

	status = elf32_image_register();
	if (LOADER_FAILED(status))
		return status;

	status = elf64_image_register();
	if (LOADER_FAILED(status))
		return status;

#if 0
	status = mz_image_register();
	if (LOADER_FAILED(status))
		return status;

	status = coff_image_register();
	if (LOADER_FAILED(status))
		return status;

	status = pe32_image_register();
	if (LOADER_FAILED(status))
		return status;

	status = pe64_image_register();
	if (LOADER_FAILED(status))
		return status;
#endif

	return LOADER_SUCCESS;
}
