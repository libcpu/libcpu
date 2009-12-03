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
#ifndef __loader_image_h
#define __loader_image_h

#include "loader_iterator.h"

#define LOADER_IMAGE_CLASS_SHIFT      0
#define LOADER_IMAGE_CLASS_MASK       (3 << LOADER_IMAGE_CLASS_SHIFT)
#define LOADER_IMAGE_CLASS_16BIT      (LOADER_CLASS_16BIT << LOADER_IMAGE_CLASS_SHIFT)
#define LOADER_IMAGE_CLASS_32BIT      (LOADER_CLASS_32BIT << LOADER_IMAGE_CLASS_SHIFT)
#define LOADER_IMAGE_CLASS_64BIT      (LOADER_CLASS_64BIT << LOADER_IMAGE_CLASS_SHIFT)
#define LOADER_IMAGE_CLASS_INVALID    (LOADER_CLASS_INVALID << LOADER_IMAGE_CLASS_SHIFT)
#define LOADER_IMAGE_GET_CLASS(x)     (((x) & LOADER_IMAGE_CLASS_MASK) >> LOADER_IMAGE_CLASS_SHIFT)

#define LOADER_IMAGE_ENDIAN_SHIFT     2
#define LOADER_IMAGE_ENDIAN_MASK      (1 << LOADER_IMAGE_ENDIAN_SHIFT)
#define LOADER_IMAGE_ENDIAN_BIG       (LOADER_ENDIAN_BIG << LOADER_IMAGE_ENDIAN_SHIFT)
#define LOADER_IMAGE_ENDIAN_LITTLE    (LOADER_ENDIAN_LITTLE << LOADER_IMAGE_ENDIAN_SHIFT)
#define LOADER_IMAGE_GET_ENDIAN(x)    (((x) & LOADER_IMAGE_ENDIAN_MASK) >> LOADER_IMAGE_ENDIAN_SHIFT)

#define LOADER_IMAGE_ARCH_SHIFT       3
#define LOADER_IMAGE_ARCH_MASK        (0xfff << LOADER_IMAGE_ARCH_SHIFT)
#define LOADER_IMAGE_MAKE_ARCH(x)     (((x) & 0xfff) << LOADER_IMAGE_ARCH_SHIFT)
#define LOADER_IMAGE_GET_ARCH(x)      (((x) & LOADER_IMAGE_ARCH_MASK) >> LOADER_IMAGE_ARCH_SHIFT)

#define LOADER_IMAGE_SUBARCH_SHIFT    15
#define LOADER_IMAGE_SUBARCH_MASK     (0x1ff << LOADER_IMAGE_SUBARCH_SHIFT)
#define LOADER_IMAGE_MAKE_SUBARCH(x)  (((x) & 0x1ff) << LOADER_IMAGE_SUBARCH_SHIFT)
#define LOADER_IMAGE_GET_SUBARCH(x)   (((x) & LOADER_IMAGE_SUBARCH_MASK) >> LOADER_IMAGE_SUBARCH_SHIFT)

#define LOADER_IMAGE_MAKE_ARCH_SUB(x,y) (LOADER_IMAGE_MAKE_ARCH(x) | LOADER_IMAGE_MAKE_SUBARCH(y))
#define LOADER_IMAGE_MAKE_ARCH_ANY(x)   (LOADER_IMAGE_MAKE_ARCH_SUB(x, LOADER_SUBARCH_ANY))

#define LOADER_IMAGE_TYPE_SHIFT              24
#define LOADER_IMAGE_TYPE_MASK               (0xf << LOADER_IMAGE_TYPE_SHIFT)
#define LOADER_IMAGE_TYPE_INVALID            (LOADER_TYPE_INVALID << LOADER_IMAGE_TYPE_SHIFT)
#define LOADER_IMAGE_TYPE_NONE               (LOADER_TYPE_NONE << LOADER_IMAGE_TYPE_SHIFT)
#define LOADER_IMAGE_TYPE_RELOCATABLE_OBJECT (LOADER_TYPE_RELOCATABLE_OBJECT << LOADER_IMAGE_TYPE_SHIFT)
#define LOADER_IMAGE_TYPE_EXECUTABLE         (LOADER_TYPE_EXECUTABLE << LOADER_IMAGE_TYPE_SHIFT)
#define LOADER_IMAGE_TYPE_SHARED_OBJECT      (LOADER_TYPE_SHARED_OBJECT << LOADER_IMAGE_TYPE_SHIFT)
#define LOADER_IMAGE_TYPE_DYNAMIC_LIBRARY    LOADER_IMAGE_TYPE_SHARED_OBJECT
#define LOADER_IMAGE_TYPE_SHARED_LIBRARY     LOADER_IMAGE_TYPE_SHARED_OBJECT
#define LOADER_IMAGE_TYPE_CORE               (LOADER_TYPE_CORE << LOADER_IMAGE_TYPE_SHIFT)
#define LOADER_IMAGE_TYPE_DYNAMIC_LINKER     (LOADER_TYPE_DYNAMIC_LINKER << LOADER_IMAGE_TYPE_SHIFT)
#define LOADER_IMAGE_TYPE_BUNDLE             (LOADER_TYPE_BUNDLE << LOADER_IMAGE_TYPE_SHIFT)
#define LOADER_IMAGE_TYPE_PRELOAD_EXECUTABLE (LOADER_TYPE_PRELOAD_EXECUTABLE << LOADER_IMAGE_TYPE_SHIFT)
#define LOADER_IMAGE_TYPE_FIXED_VM_LIBRARY   (LOADER_TYPE_FIXED_VM_LIBRARY << LOADER_IMAGE_TYPE_SHIFT)
#define LOADER_IMAGE_TYPE_ARCHIVE_LIBRARY    (LOADER_TYPE_ARCHIVE_LIBRARY << LOADER_IMAGE_TYPE_SHIFT)
#define LOADER_IMAGE_TYPE_CONTAINER          (LOADER_TYPE_CONTAINER << LOADER_IMAGE_TYPE_SHIFT)
#define LOADER_IMAGE_GET_TYPE(x)             (((x) & LOADER_IMAGE_TYPE_MASK) >> LOADER_IMAGE_TYPE_SHIFT)

#define LOADER_IMAGE_TYPE_STATIC             (1 << 31)
#define LOADER_IMAGE_IS_DYNAMIC(x)           (((x) & LOADER_IMAGE_TYPE_STATIC) == 0 )

typedef struct _loader_image loader_image_t;

#define LOADER_IMAGE_ITERATE_SYMBOL_DYNAMIC 1

#ifdef __cplusplus
extern "C" {
#endif

loader_status_t
LOADERCALL
loader_image_open(loader_uintptr_t   base_address,
                  loader_size_t      image_size,
                  loader_image_t   **result);

loader_status_t
LOADERCALL
loader_image_close(loader_image_t *image);

loader_uintptr_t
LOADERCALL
loader_image_get_base_address(loader_image_t const *image);

loader_uintptr_t
LOADERCALL
loader_image_get_real_base_address(loader_image_t const *image);

char const *
LOADERCALL
loader_image_get_name(loader_image_t const *image);

loader_size_t
LOADERCALL
loader_image_get_size(loader_image_t const *image);

loader_size_t
LOADERCALL
loader_image_get_real_size(loader_image_t const *image);

loader_uint32_t
LOADERCALL
loader_image_get_format(loader_image_t const *image);

loader_status_t
LOADERCALL
loader_image_get_section_iterator(loader_image_t     *image,
                                  loader_flags_t      flags,
                                  loader_iterator_t **iterator);

loader_status_t
LOADERCALL
loader_image_get_symbol_iterator(loader_image_t     *image,
                                 loader_flags_t      flags,
                                 loader_iterator_t **iterator);

loader_status_t
LOADERCALL
loader_image_print (loader_image_t const *image);

#ifdef __cplusplus
}
#endif

#endif  /* !__loader_image_h */
