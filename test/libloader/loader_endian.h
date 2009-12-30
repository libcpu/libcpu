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
#ifndef __loader_endian_h
#define __loader_endian_h

#include "loader_config.h"

#ifdef __APPLE__
#include <libkern/OSByteOrder.h>

#define loader_swap_little_16(x) OSSwapLittleToHostInt16 (x)
#define loader_swap_big_16(x) OSSwapBigToHostInt16 (x)
#define loader_swap_little_32(x) OSSwapLittleToHostInt32 (x)
#define loader_swap_big_32(x) OSSwapBigToHostInt32 (x)
#define loader_swap_little_64(x) OSSwapLittleToHostInt64 (x)
#define loader_swap_big_64(x) OSSwapBigToHostInt64 (x)

#elif defined(HAVE_ENDIAN_H)
#include <endian.h>

#define loader_swap_little_16(x) le16toh(x)
#define loader_swap_big_16(x) be16toh(x)
#define loader_swap_little_32(x) le32toh(x)
#define loader_swap_big_32(x) be32toh(x)
#define loader_swap_little_64(x) le64toh(x)
#define loader_swap_big_64(x) be64toh(x)

#else
#error "Define endian swap functions."
#endif

#endif  /* !__loader_endian_h */
