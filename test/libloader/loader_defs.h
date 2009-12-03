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
#ifndef __loader_defs_h
#define __loader_defs_h

/*
 * Image Classes
 */
#define LOADER_CLASS_16BIT 0
#define LOADER_CLASS_32BIT 1
#define LOADER_CLASS_64BIT 2

/*
 * Image Endian
 */
#define LOADER_ENDIAN_BIG 0
#define LOADER_ENDIAN_LITTLE 1

/*
 * Image Types
 */
#define LOADER_TYPE_INVALID            15
#define LOADER_TYPE_NONE               0
#define LOADER_TYPE_RELOCATABLE_OBJECT 1
#define LOADER_TYPE_EXECUTABLE         2
#define LOADER_TYPE_SHARED_OBJECT      3
#define LOADER_TYPE_DYNAMIC_LIBRARY    LOADER_TYPE_SHARED_OBJECT
#define LOADER_TYPE_SHARED_LIBRARY     LOADER_TYPE_SHARED_OBJECT
#define LOADER_TYPE_CORE               4
#define LOADER_TYPE_DYNAMIC_LINKER     5
#define LOADER_TYPE_BUNDLE             6
#define LOADER_TYPE_PRELOAD_EXECUTABLE 7
#define LOADER_TYPE_FIXED_VM_LIBRARY   8
#define LOADER_TYPE_ARCHIVE_LIBRARY    9
#define LOADER_TYPE_CONTAINER          10

/*
 * Architectures
 */
#define LOADER_ARCH_INVALID  0xfff
#define LOADER_ARCH_MULTI    0xffe
#define LOADER_ARCH_UNKNOWN  0
#define LOADER_ARCH_X86      1
#define LOADER_ARCH_POWERPC  2
#define LOADER_ARCH_ARM      3
#define LOADER_ARCH_MIPS     4
#define LOADER_ARCH_SPARC    5
#define LOADER_ARCH_IA64     6
#define LOADER_ARCH_SUPERH   7
#define LOADER_ARCH_68000    8
#define LOADER_ARCH_COLDFIRE 9
#define LOADER_ARCH_OPENRISC 10
#define LOADER_ARCH_ESA      11
#define LOADER_ARCH_CRIS     12

/*
 * Educational archs
 */
#define LOADER_ARCH_DLX      500
#define LOADER_ARCH_MMIX     501

/*
 * Obsolete archs
 */
#define LOADER_ARCH_ALPHA    1000
#define LOADER_ARCH_VAX      1001
#define LOADER_ARCH_PDP11    1002
#define LOADER_ARCH_PDP10    1003
#define LOADER_ARCH_88000    1004
#define LOADER_ARCH_PARISC   1005
#define LOADER_ARCH_80860    1006
#define LOADER_ARCH_80960    1007
#define LOADER_ARCH_WE32000  1008
#define LOADER_ARCH_NS32000  1009
#define LOADER_ARCH_CRAY     1010

#define LOADER_SUBARCH_ANY 0

/*
 * X86 subarch.
 */
#define LOADER_SUBARCH_X86_80186   1
#define LOADER_SUBARCH_X86_80286   2
#define LOADER_SUBARCH_X86_80386   3
#define LOADER_SUBARCH_X86_80486   4
#define LOADER_SUBARCH_X86_PENTIUM 5

/*
 * MIPS subarch.
 */
#define LOADER_SUBARCH_MIPS_I       1
#define LOADER_SUBARCH_MIPS_II      2
#define LOADER_SUBARCH_MIPS_III     3
#define LOADER_SUBARCH_MIPS_IV      4
#define LOADER_SUBARCH_MIPS_V       5
#define LOADER_SUBARCH_MIPS_ISA32   6
#define LOADER_SUBARCH_MIPS_ISA32R2 7
#define LOADER_SUBARCH_MIPS_ISA64   8
#define LOADER_SUBARCH_MIPS_ISA64R2 9

/*
 * SPARC subarch.
 */
#define LOADER_SUBARCH_SPARC_V7     1
#define LOADER_SUBARCH_SPARC_V8     2
#define LOADER_SUBARCH_SPARC_V8PLUS 3
#define LOADER_SUBARCH_SPARC_V9     4

/*
 * ESA subarch.
 */
#define LOADER_SUBARCH_ESA_370      1
#define LOADER_SUBARCH_ESA_390      2

#endif  /* !__loader_defs_h */
