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
#include "elf_common.h"

loader_status_t
LOADERCALL
elf_get_loader_arch(elf_half_t        machine,
                    loader_boolean_t  big_endian,
                    loader_uint32_t  *format)
{
	switch (machine) {
		case ELF_EM_NONE:
			return LOADER_ERROR_INVALID_ARCHITECTURE;

			/*
			 * X86
			 */
		case ELF_EM_386:
			*format |= LOADER_IMAGE_MAKE_ARCH_ANY(LOADER_ARCH_X86);
			break;

		case ELF_EM_486:
			*format |= LOADER_IMAGE_MAKE_ARCH_SUB (LOADER_ARCH_X86, LOADER_SUBARCH_X86_80486);
			break;

		case ELF_EM_X86_64:
			*format |= LOADER_IMAGE_MAKE_ARCH_ANY(LOADER_ARCH_X86);
			*format &= ~LOADER_IMAGE_CLASS_MASK;
			*format |= LOADER_IMAGE_CLASS_64BIT;
			break;

			/*
			 * PowerPC
			 */
		case ELF_EM_PPC:
		case ELF_EM_CYGNUS_POWERPC:
			*format |= LOADER_IMAGE_MAKE_ARCH_ANY(LOADER_ARCH_POWERPC);
			*format &= ~LOADER_IMAGE_CLASS_MASK;
			*format |= LOADER_IMAGE_CLASS_32BIT;
			break;

		case ELF_EM_PPC64:
			*format |= LOADER_IMAGE_MAKE_ARCH_ANY(LOADER_ARCH_POWERPC);
			*format &= ~LOADER_IMAGE_CLASS_MASK;
			*format |= LOADER_IMAGE_CLASS_64BIT;
			break;

			/*
			 * ARM
			 */
		case ELF_EM_ARM:
			*format |= LOADER_IMAGE_MAKE_ARCH_ANY(LOADER_ARCH_ARM);
			break;

			/*
			 * MIPS
			 */
		case ELF_EM_MIPS:
			*format |= LOADER_IMAGE_MAKE_ARCH_ANY(LOADER_ARCH_MIPS);
			break;

		case ELF_EM_MIPS_RS3_LE:
			/* case ELF_EM_MIPS_RS4_BE: */
			if (big_endian)
				*format |= LOADER_IMAGE_MAKE_ARCH_SUB (LOADER_ARCH_MIPS, LOADER_SUBARCH_MIPS_III);
			else
				*format |= LOADER_IMAGE_MAKE_ARCH_ANY(LOADER_ARCH_MIPS);
			break;

			/*
			 * SPARC
			 */
		case ELF_EM_SPARC:
			*format |= LOADER_IMAGE_MAKE_ARCH_ANY(LOADER_ARCH_SPARC);
			*format &= ~LOADER_IMAGE_CLASS_MASK;
			*format |= LOADER_IMAGE_CLASS_32BIT;
			break;

		case ELF_EM_SPARC32PLUS:
			*format |= LOADER_IMAGE_MAKE_ARCH_SUB (LOADER_ARCH_SPARC, LOADER_SUBARCH_SPARC_V8PLUS);
			break;

		case ELF_EM_SPARCV9:
		case ELF_EM_OLD_SPARCV9:
			*format |= LOADER_IMAGE_MAKE_ARCH_ANY(LOADER_ARCH_SPARC);
			*format &= ~LOADER_IMAGE_CLASS_MASK;
			*format |= LOADER_IMAGE_CLASS_64BIT;
			break;

			/*
			 * Intel IA64
			 */
		case ELF_EM_IA_64:
			*format |= LOADER_IMAGE_MAKE_ARCH_ANY(LOADER_ARCH_IA64);
			break;

			/*
			 * Reneseas SuperH
			 */
		case ELF_EM_SH:
			*format |= LOADER_IMAGE_MAKE_ARCH_ANY(LOADER_ARCH_SUPERH);
			break;

			/*
			 * Motorola 68000
			 */
		case ELF_EM_68K:
			*format |= LOADER_IMAGE_MAKE_ARCH_ANY(LOADER_ARCH_68000);
			break;

			/*
			 * Freescale ColdFire
			 */
		case ELF_EM_COLDFIRE:
			*format |= LOADER_IMAGE_MAKE_ARCH_ANY(LOADER_ARCH_COLDFIRE);
			break;

			/*
			 * OpenCores OpenRISC
			 */
		case ELF_EM_OPENRISC:
		case ELF_EM_OLD_OPENRISC:
		case ELF_EM_OLD_OR32:
			*format |= LOADER_IMAGE_MAKE_ARCH_ANY(LOADER_ARCH_OPENRISC);
			break;

			/*
			 * IBM ESA
			 */
		case ELF_EM_S370:
			*format |= LOADER_IMAGE_MAKE_ARCH_SUB (LOADER_ARCH_ESA, LOADER_SUBARCH_ESA_370);
			break;

		case ELF_EM_S390:
		case ELF_EM_OLD_S390:
			*format |= LOADER_IMAGE_MAKE_ARCH_SUB (LOADER_ARCH_ESA, LOADER_SUBARCH_ESA_390);
			break;

			/*
			 * Axis CRIS
			 */
		case ELF_EM_CRIS:
			*format |= LOADER_IMAGE_MAKE_ARCH_ANY(LOADER_ARCH_CRIS);
			break;

			/* Educational Architectures */

			/*
			 * Patterson's DLX
			 */
		case ELF_EM_DLX:
			*format |= LOADER_IMAGE_MAKE_ARCH_ANY(LOADER_ARCH_DLX);
			break;

			/*
			 * Knuth's MMIX
			 */
		case ELF_EM_MMIX:
			*format |= LOADER_IMAGE_MAKE_ARCH_ANY(LOADER_ARCH_MMIX);
			break;

			/* Obsoleted Architectures */

			/*
			 * Digital Alpha
			 */
		case ELF_EM_ALPHA:
		case ELF_EM_OLD_ALPHA:
			*format |= LOADER_IMAGE_MAKE_ARCH_ANY(LOADER_ARCH_ALPHA);
			break;

			/*
			 * Digital VAX
			 */
		case ELF_EM_VAX:
			*format |= LOADER_IMAGE_MAKE_ARCH_ANY(LOADER_ARCH_VAX);
			break;

			/*
			 * Digital PDP-11
			 */
		case ELF_EM_PDP11:
			*format |= LOADER_IMAGE_MAKE_ARCH_ANY(LOADER_ARCH_PDP11);
			break;

			/*
			 * Digital PDP-10
			 */
		case ELF_EM_PDP10:
			*format |= LOADER_IMAGE_MAKE_ARCH_ANY(LOADER_ARCH_PDP10);
			break;

			/*
			 * Motorola 88000
			 */
		case ELF_EM_88K:
			*format |= LOADER_IMAGE_MAKE_ARCH_ANY(LOADER_ARCH_88000);
			break;

			/*
			 * HP PA-RISC
			 */
		case ELF_EM_PARISC:
			*format |= LOADER_IMAGE_MAKE_ARCH_ANY(LOADER_ARCH_PARISC);
			break;

			/*
			 * Intel 80860
			 */
		case ELF_EM_860:
			*format |= LOADER_IMAGE_MAKE_ARCH_ANY(LOADER_ARCH_80860);
			break;

			/*
			 * Intel 80960
			 */
		case ELF_EM_960:
			*format |= LOADER_IMAGE_MAKE_ARCH_ANY(LOADER_ARCH_80960);
			break;

			/*
			 * AT&T WE 32000
			 */
		case ELF_EM_M32:
			*format |= LOADER_IMAGE_MAKE_ARCH_ANY(LOADER_ARCH_WE32000);
			break;

			/*
			 * National Semiconductor NS32000
			 */
		case ELF_EM_NS32K:
			*format |= LOADER_IMAGE_MAKE_ARCH_ANY(LOADER_ARCH_NS32000);
			break;

		default:
			return LOADER_ERROR_UNSUPPORTED_ARCHITECTURE;
	}

	return LOADER_SUCCESS;
}

loader_status_t
LOADERCALL
elf_get_loader_type(elf_half_t       type,
                    loader_uint32_t *format)
{
	switch (type) {
		case ELF_ET_NONE:
			*format |= LOADER_IMAGE_TYPE_NONE;
			break;
		case ELF_ET_REL:
			*format |= LOADER_IMAGE_TYPE_RELOCATABLE_OBJECT;
			break;
		case ELF_ET_EXEC:
			*format |= LOADER_IMAGE_TYPE_EXECUTABLE;
			break;
		case ELF_ET_DYN:
			*format |= LOADER_IMAGE_TYPE_SHARED_OBJECT;
			break;
		case ELF_ET_CORE:
			*format |= LOADER_IMAGE_TYPE_CORE;
			break;
		default:
			return LOADER_ERROR_UNSUPPORTED_IMAGE_TYPE;
	}

	return LOADER_SUCCESS;
}

elf_word_t
LOADERCALL
elf_hash(char const *name)
{
	elf_word_t h = 0, g;
	while (*name != '\0') {
		h = (h << 4) + ((elf_small_t)*name++);
		g = h & 0xf0000000;
		if (g != 0)
			h ^= g >> 24;
		h &= 0x0fffffff;
	}
	return h;
}
