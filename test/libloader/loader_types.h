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
#ifndef __loader_types_h
#define __loader_types_h

typedef int loader_status_t;

typedef uint8_t loader_uint8_t;
typedef int8_t loader_sint8_t;
typedef uint16_t loader_uint16_t;
typedef int16_t loader_sint16_t;
typedef uint32_t loader_uint32_t;
typedef int32_t loader_sint32_t;
typedef uint64_t loader_uint64_t;
typedef int64_t loader_sint64_t;
typedef uintptr_t loader_uintptr_t;
typedef intptr_t loader_intptr_t;
typedef size_t loader_size_t;
typedef ssize_t loader_ssize_t;

typedef unsigned char loader_boolean_t;
#define LOADER_FALSE 0
#define LOADER_TRUE 1

typedef loader_uint32_t loader_flags_t;

#endif  /* !__loader_types_h */
