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
 *   * Neither the name of the author nor the names of its contributors
 *     may be used to endorse or promote products derived from this software
 *     without specific prior written permission.
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
#include "loader.h"

#include <stdio.h>
#include <stdlib.h>

loader_status_t
LOADERCALL
loader_init (void);

static loader_uint8_t *
read_file(char const    *path,
          loader_size_t *size)
{
	loader_uint8_t *data;
	FILE           *fp;


	fp = fopen(path, "rb");
	if (fp == NULL) {
		fprintf(stderr, "%s: error: cannot open file '%s'\n", __func__, path);
		return NULL;
	}

	fseek(fp, 0, SEEK_END);

	*size = ftell(fp);

	if (*size == 0) {
		fclose(fp);
		fprintf(stderr, "%s: error: file '%s' is empty\n", __func__, path);
		return NULL;
	}

	data = (loader_uint8_t *)loader_allocate(*size);

	rewind(fp);
	fread(data, 1, *size, fp);
	fclose(fp);

	return data;
}

int
main(int ac, char **av)
{
	loader_status_t    status;
	loader_size_t      image_size;
	loader_uint8_t    *image_data;
	loader_image_t    *image;
	loader_iterator_t *i;

	status = loader_init();
	if (LOADER_FAILED(status)) {
		fprintf(stderr, "%s: error initializing loader\n", __func__);
		return EXIT_FAILURE;
	}

	if (ac < 2) {
		fprintf(stderr, "usage: %s <filename>\n", av[0]);
		return EXIT_FAILURE;
	}

	image_data = read_file(av[1], &image_size);
	if (image_data == NULL)
		return EXIT_FAILURE;

	status = loader_image_open((loader_uintptr_t)image_data, image_size, &image);
	if (LOADER_FAILED(status)) {
		fprintf (stderr, "error: cannot open image '%s', status=%d\n", av[1], status);
		return EXIT_FAILURE;
	}

	loader_image_print(image);

	i = NULL;
	status = loader_image_get_section_iterator(image, 0, &i);
	if (LOADER_SUCCEEDED(i)) {
		loader_section_t section;

		printf ("Got section iterator\n");

		while (loader_iterator_next(i, &section)) {
			printf("Module=%s Name=%s Offset=%#llx Virtual Address=%p Physical Address=%p Size=%#zx Type=%u Flags=%#x\n",
					(section.module == NULL) && (section.flags & LOADER_SYMBOL_FLAG_EXTERNAL) == 0
					? av[1] : section.module,
					section.name,
					(unsigned long long)section.file_offset,
					(void *)section.virtual_address,
					(void *)section.physical_address,
					section.size,
					section.type,
					section.flags);
		}

		loader_iterator_dispose(i);
		i = NULL;
	}


	status = loader_image_get_symbol_iterator(image, 0, &i);
	if (LOADER_SUCCEEDED(i)) {
		loader_symbol_t symbol;

		printf ("Got symbol iterator\n");

		while (loader_iterator_next(i, &symbol)) {
			//if (symbol.flags & LOADER_SYMBOL_FLAG_EXTERNAL)
			//  continue;

			printf("Module=%s Symbol=%s Offset=%#llx Virtual Address=%p Physical Address=%p Size=%#zx Type=%u Flags=%#x\n",
					(symbol.module == NULL) && (symbol.flags & LOADER_SYMBOL_FLAG_EXTERNAL) == 0
					? av[1] : symbol.module,
					symbol.name,
					(unsigned long long)symbol.file_offset,
					(void *)symbol.virtual_address,
					(void *)symbol.physical_address,
					symbol.size,
					symbol.type,
					symbol.flags);
		}

		loader_iterator_dispose(i);
		i = NULL;
	}


	status = loader_image_get_symbol_iterator(image, LOADER_IMAGE_ITERATE_SYMBOL_DYNAMIC, &i);
	if (LOADER_SUCCEEDED(i)) {
		loader_symbol_t symbol;

		printf ("Got dynamic symbol iterator\n");

		while (loader_iterator_next(i, &symbol)) {
			//if (symbol.flags & LOADER_SYMBOL_FLAG_EXTERNAL)
			//  continue;

			printf("Module=%s Symbol=%s Offset=%#llx Virtual Address=%p Physical Address=%p Size=%#zx Type=%u Flags=%#x\n",
					(symbol.module == NULL) && (symbol.flags & LOADER_SYMBOL_FLAG_EXTERNAL) == 0
					? av[1] : symbol.module,
					symbol.name,
					(unsigned long long)symbol.file_offset,
					(void *)symbol.virtual_address,
					(void *)symbol.physical_address,
					symbol.size,
					symbol.type,
					symbol.flags);
		}

		loader_iterator_dispose(i);
	}

	loader_image_close(image);

	free(image_data);

	return EXIT_SUCCESS;
}
