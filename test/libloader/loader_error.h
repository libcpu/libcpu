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
#ifndef __loader_error_h
#define __loader_error_h

#define LOADER_SUCCESS                        0
#define LOADER_ERROR_INVALID_IMAGE            -1
#define LOADER_ERROR_INVALID_CLASS            -2
#define LOADER_ERROR_INVALID_ENDIAN           -3
#define LOADER_ERROR_UNSUPPORTED_IMAGE        -4
#define LOADER_ERROR_INVALID_ARGUMENT         -5
#define LOADER_ERROR_NULL_POINTER             -6
#define LOADER_ERROR_INVALID_ARCHITECTURE     -7
#define LOADER_ERROR_UNSUPPORTED_ARCHITECTURE -8
#define LOADER_ERROR_UNSUPPORTED_VERSION      -9
#define LOADER_ERROR_IMAGE_CORRUPTED          -10
#define LOADER_ERROR_UNSUPPORTED_IMAGE_TYPE   -11
#define LOADER_ERROR_NOT_FOUND                -12
#define LOADER_ERROR_UNSUPPORTED_OPERATION    -13
#define LOADER_ERROR_NO_MEMORY                -14
#define LOADER_ERROR_ALREADY_REGISTERED       -15

#define LOADER_SUCCEEDED(x) ( (x) >= 0 )
#define LOADER_FAILED(x)    ( (x) < 0 )

#endif  /* !__loader_error_h */
