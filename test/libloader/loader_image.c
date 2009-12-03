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
#include <string.h>

/* XXX Private! */
loader_status_t
LOADERCALL
loader_registry_identify_handler (loader_image_t                  *image,
                                  loader_image_ident_t            *ident,
                                  loader_image_callbacks_t const **callbacks);

struct _loader_image {
	loader_flags_t                  flags;
	loader_uint32_t                 format;

	loader_uintptr_t                real_base_address;
	loader_size_t                   real_image_size;

	loader_uintptr_t                base_address;
	loader_size_t                   image_size;

	void                           *cookie;
	loader_image_callbacks_t const *callbacks;
};

__LOADER_FAST_INLINE loader_status_t
__loader_image_open(loader_image_t *image)
{
	if (image->callbacks->open != NULL)
		return (*(image->callbacks->open))(image, &image->cookie);
	else
		return LOADER_ERROR_UNSUPPORTED_OPERATION;
}

__LOADER_FAST_INLINE void
__loader_image_close(loader_image_t *image)
{
	if (image->callbacks->close != NULL)
		(*(image->callbacks->close))(image, image->cookie);
}

__LOADER_FAST_INLINE char const *
__loader_image_get_name(loader_image_t const *image)
{
	char const *name = NULL;

	if (image->callbacks->get_name != NULL)
		name = (*(image->callbacks->get_name))(image, image->cookie);
	if (name == NULL)
		name = "<unknown>";

	return name;
}

__LOADER_FAST_INLINE loader_status_t
__loader_image_get_symbol_iterator(loader_image_t     *image,
                                   loader_flags_t      flags,
                                   loader_iterator_t **iterator)
{
	if (image->callbacks->get_symbol_iterator != NULL)
		return (*(image->callbacks->get_symbol_iterator))(image, flags, iterator, image->cookie);
	else
		return LOADER_ERROR_UNSUPPORTED_OPERATION;
}

__LOADER_FAST_INLINE loader_status_t
__loader_image_get_section_iterator(loader_image_t     *image,
                                    loader_flags_t      flags,
                                    loader_iterator_t **iterator)
{
	if (image->callbacks->get_section_iterator != NULL)
		return (*(image->callbacks->get_section_iterator))(image, flags, iterator, image->cookie);
	else
		return LOADER_ERROR_UNSUPPORTED_OPERATION;
}

loader_status_t
LOADERCALL
loader_image_open (loader_uintptr_t   base_address,
                   loader_size_t      image_size,
                   loader_image_t   **result)
{
	loader_status_t       status;
	loader_image_ident_t  ident;
	loader_image_t       *image;

	if (result == NULL)
		return LOADER_ERROR_NULL_POINTER;
	if (base_address == 0 || image_size == 0)
		return LOADER_ERROR_INVALID_ARGUMENT;

	image = (loader_image_t *)loader_allocate(sizeof(*image));
	if (image == NULL)
		return LOADER_ERROR_NO_MEMORY;

	image->real_base_address = base_address;
	image->base_address      = base_address;
	image->real_image_size   = image_size;
	image->image_size        = image_size;

  for (;;) {
	  status = loader_registry_identify_handler(image, &ident, &image->callbacks);
	  if (LOADER_FAILED(status)) {
		  loader_free(image);
		  return status;
	  }

	  /*
	   * Containers return an offset where to read.
	   */
	  if (LOADER_IMAGE_GET_TYPE(ident.format) == LOADER_TYPE_CONTAINER) {
		  if (ident.offset == 0) {
			  loader_free(image);
			  return LOADER_ERROR_UNSUPPORTED_IMAGE;
		  }
		  else if (ident.offset >= image->image_size) {
			  loader_free(image);
			  return LOADER_ERROR_IMAGE_CORRUPTED;
		  } else {
			  image->base_address += ident.offset;
			  image->image_size   -= ident.offset;
			  continue;
		  }
	  }

	  image->format = ident.format;

	  status = __loader_image_open (image);
	  if (LOADER_FAILED(status)) {
		  loader_free(image);
		  return status;
	  }

	  break;
  }

  *result = image;
  return LOADER_SUCCESS;
}

loader_status_t
LOADERCALL
loader_image_close(loader_image_t *image)
{
	if (image == NULL)
		return LOADER_ERROR_NULL_POINTER;

	__loader_image_close(image);
	loader_free(image);

	return LOADER_SUCCESS;
}

char const *
LOADERCALL
loader_image_get_name(loader_image_t const *image)
{
	if (image == NULL)
		return NULL;
	else
		return __loader_image_get_name(image);
}

loader_status_t
LOADERCALL
loader_image_get_symbol_iterator(loader_image_t     *image,
                                 loader_flags_t      flags,
                                 loader_iterator_t **iterator)
{
	if (image == NULL || iterator == NULL)
		return LOADER_ERROR_NULL_POINTER;
	else
		return __loader_image_get_symbol_iterator(image, flags, iterator);
}

loader_status_t
LOADERCALL
loader_image_get_section_iterator(loader_image_t     *image,
                                  loader_flags_t      flags,
                                  loader_iterator_t **iterator)
{
	if (image == NULL || iterator == NULL)
		return LOADER_ERROR_NULL_POINTER;
	else
		return __loader_image_get_section_iterator(image, flags, iterator);
}

loader_uintptr_t
LOADERCALL
loader_image_get_real_base_address(loader_image_t const *image)
{
	if (image == NULL)
		return 0;
	else
		return image->real_base_address;
}

loader_uintptr_t
LOADERCALL
loader_image_get_base_address(loader_image_t const *image)
{
	if (image == NULL)
		return 0;
	else
		return image->base_address;
}

loader_size_t
LOADERCALL
loader_image_get_size(loader_image_t const *image)
{
	if (image == NULL)
		return 0;
	else
		return image->image_size;
}

loader_size_t
LOADERCALL
loader_image_get_real_size(loader_image_t const *image)
{
	if (image == NULL)
		return 0;
	else
		return image->real_image_size;
}

loader_uint32_t
LOADERCALL
loader_image_get_format(loader_image_t const *image)
{
	if (image == NULL)
		return 0;
	else
		return image->format;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* Dump                                                                      */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static char const *
loader_arch_get_name(loader_uint32_t arch)
{
	switch (arch & 0xfff) {
		case LOADER_ARCH_INVALID:  return "<invalid>";
		case LOADER_ARCH_MULTI:    return "<multiple>";
		case LOADER_ARCH_UNKNOWN:  return "<unknown>";
		case LOADER_ARCH_X86:      return "X86";
		case LOADER_ARCH_POWERPC:  return "POWERPC";
		case LOADER_ARCH_ARM:      return "ARM";
		case LOADER_ARCH_MIPS:     return "MIPS";
		case LOADER_ARCH_SPARC:    return "SPARC";
		case LOADER_ARCH_IA64:     return "IA64";
		case LOADER_ARCH_SUPERH:   return "SUPERH";
		case LOADER_ARCH_68000:    return "68000";
		case LOADER_ARCH_COLDFIRE: return "SUPERH";
		case LOADER_ARCH_ESA:      return "ESA";
		case LOADER_ARCH_CRIS:     return "CRIS";

								   /* Educational */
		case LOADER_ARCH_DLX:      return "DLX";
		case LOADER_ARCH_MMIX:     return "MMIX";

								   /* Obsoleted */
		case LOADER_ARCH_ALPHA:    return "ALPHA";
		case LOADER_ARCH_VAX:      return "VAX";
		case LOADER_ARCH_PDP11:    return "PDP11";
		case LOADER_ARCH_PDP10:    return "PDP10";
		case LOADER_ARCH_88000:    return "88000";
		case LOADER_ARCH_PARISC:   return "PARISC";
		case LOADER_ARCH_80860:    return "80860";
		case LOADER_ARCH_80960:    return "80960";
		case LOADER_ARCH_WE32000:  return "WE32000";
		case LOADER_ARCH_NS32000:  return "NS32000";
		case LOADER_ARCH_CRAY:     return "CRAY";
		default:                   return "<unhandled>";
	}
}

static char const *
loader_arch_get_sub_name(loader_uint32_t arch,
                         loader_uint32_t subarch)
{
	if (subarch == LOADER_SUBARCH_ANY)
		return "ANY";

	switch (arch) {
		case LOADER_ARCH_X86:
			switch (subarch) {
				case LOADER_SUBARCH_X86_80186:   return "80186";
				case LOADER_SUBARCH_X86_80286:   return "80286";
				case LOADER_SUBARCH_X86_80386:   return "80386";
				case LOADER_SUBARCH_X86_80486:   return "80486";
				case LOADER_SUBARCH_X86_PENTIUM: return "PENTIUM";
				default:                         return "<X86:unhandled>";
			}
			break;

		case LOADER_ARCH_MIPS:
			switch (subarch) {
				case LOADER_SUBARCH_MIPS_I:       return "ISA I";
				case LOADER_SUBARCH_MIPS_II:      return "ISA II";
				case LOADER_SUBARCH_MIPS_III:     return "ISA III";
				case LOADER_SUBARCH_MIPS_IV:      return "ISA IV";
				case LOADER_SUBARCH_MIPS_V:       return "ISA V";
				case LOADER_SUBARCH_MIPS_ISA32:   return "ISA32";
				case LOADER_SUBARCH_MIPS_ISA32R2: return "ISA32R2";
				case LOADER_SUBARCH_MIPS_ISA64:   return "ISA64";
				case LOADER_SUBARCH_MIPS_ISA64R2: return "ISA64R2";
				default:                          return "<MIPS:unhandled>";
			}
			break;

		case LOADER_ARCH_SPARC:
			switch (subarch) {
				case LOADER_SUBARCH_SPARC_V7:     return "V7";
				case LOADER_SUBARCH_SPARC_V8:     return "V8";
				case LOADER_SUBARCH_SPARC_V8PLUS: return "V8+";
				case LOADER_SUBARCH_SPARC_V9:     return "V9";
				default:                          return "<SPARC:unhandled>";
			}
			break;

		case LOADER_ARCH_ESA:
			switch (subarch) {
				case LOADER_SUBARCH_ESA_370:      return "370";
				case LOADER_SUBARCH_ESA_390:      return "390";
				default:                          return "<ESA:unhandled>";
			}

		default:
			return "<unhandled>";
	}
}

static char const *
loader_type_get_name(loader_uint32_t type)
{
	switch (type) {
		case LOADER_TYPE_INVALID:            return "<invalid>";
		case LOADER_TYPE_NONE:               return "<none>";
		case LOADER_TYPE_RELOCATABLE_OBJECT: return "RELOCATABLE OBJECT";
		case LOADER_TYPE_EXECUTABLE:         return "EXECUTABLE";
		case LOADER_TYPE_SHARED_LIBRARY:     return "SHARED LIBRARY";
		case LOADER_TYPE_CORE:               return "CORE";
		case LOADER_TYPE_DYNAMIC_LINKER:     return "DYNAMIC LINKER";
		case LOADER_TYPE_BUNDLE:             return "BUNDLE";
		case LOADER_TYPE_PRELOAD_EXECUTABLE: return "PRELOAD EXECUTABLE";
		case LOADER_TYPE_FIXED_VM_LIBRARY:   return "FIXED VM LIBRARY";
		case LOADER_TYPE_ARCHIVE_LIBRARY:    return "ARCHIVE LIBRARY";
		default:                             return "<unhandled>";
	}
}

loader_status_t
LOADERCALL
loader_image_print(loader_image_t const *image)
{
	char const       *arch_name;
	char const       *subarch_name;
	char const       *endian;
	unsigned          class;
	loader_boolean_t  dynamic;

	if (image == NULL)
		return LOADER_ERROR_NULL_POINTER;

	arch_name    = loader_arch_get_name(LOADER_IMAGE_GET_ARCH (image->format));
	subarch_name = loader_arch_get_sub_name(LOADER_IMAGE_GET_ARCH (image->format),
			 LOADER_IMAGE_GET_SUBARCH (image->format));
	dynamic      = LOADER_IMAGE_IS_DYNAMIC(image->format);

	switch (LOADER_IMAGE_GET_CLASS(image->format)) {
		case LOADER_CLASS_16BIT: class = 16; break;
		case LOADER_CLASS_32BIT: class = 32; break;
		case LOADER_CLASS_64BIT: class = 64; break;
		default:                 class = 0; break;
	}

	switch (LOADER_IMAGE_GET_ENDIAN(image->format)) {
		case LOADER_ENDIAN_LITTLE: endian = "LITTLE"; break;
		case LOADER_ENDIAN_BIG:    endian = "BIG"; break;
		default:                   endian = "<unknown>"; break;
	}

	printf ("Format: %s\n", loader_image_get_name (image));
	printf ("[Architecture %s:%s] [Class %uBITS] [Endian %s] [Type %s] [%s]\n",
			arch_name, subarch_name, class, endian,
			loader_type_get_name (LOADER_IMAGE_GET_TYPE (image->format)),
			dynamic ? "DYNAMIC" : "STATIC");

	return LOADER_SUCCESS;
}
