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
#ifndef __elf64_h
#define __elf64_h

#include "elf_common.h"

#pragma pack(1)

typedef struct _elf64_ehdr {
	elf_small_t     ident[ELF_EI_NIDENT];
	elf_half_t      type;
	elf_half_t      machine;
	elf_word_t      version;
	elf64_address_t entry_point;
	elf64_offset_t  phdr_offset;
	elf64_offset_t  shdr_offset;
	elf_word_t      flags;
	elf_half_t      eh_size;
	elf_half_t      ph_entry_size;
	elf_half_t      ph_count;
	elf_half_t      sh_entry_size;
	elf_half_t      sh_count;
	elf_half_t      sh_string_index;
} elf64_ehdr_t;

typedef struct _elf64_phdr {
	elf_word_t      type;
	elf_word_t      flags;
	elf64_offset_t  file_offset;
	elf64_address_t virtual_address;
	elf64_address_t physical_address;
	elf_xword_t     file_size;
	elf_xword_t     memory_size;
	elf_xword_t     alignment;
} elf64_phdr_t;

typedef struct _elf64_shdr {
	elf_word_t      name;
	elf_word_t      type;
	elf_xword_t     flags;
	elf64_address_t virtual_address;
	elf64_offset_t  file_offset;
	elf_xword_t     size;
	elf_word_t      link;
	elf_word_t      info;
	elf_xword_t     alignment;
	elf_xword_t     entry_size;
} elf64_shdr_t;

typedef struct _elf64_sym {
	elf_word_t      name;
	elf_small_t     info;
	elf_small_t     other;
	elf_half_t      section_index;
	elf64_address_t value;
	elf_xword_t     size;
} elf64_sym_t;

typedef struct _elf64_rel {
	elf64_address_t offset;
	elf_xword_t     info;
} elf64_rel_t;

typedef struct _elf64_rela {
	elf64_address_t offset;
	elf_xword_t     info;
	elf_sxword_t    addend;
} elf64_rela_t;

#define ELF64_R_SYM(i)     ((i) >> 32) 
#define ELF64_R_TYPE(i)    ((i) & 0xffffffffL) 
#define ELF64_R_INFO(s, t) (((elf_xword_t)(s) << 32) | ((t) & 0xffffffffL)) 

#pragma pack()

#ifdef __cplusplus
extern "C" {
#endif

loader_status_t
LOADERCALL
elf64_image_register(void);

#ifdef __cplusplus
}
#endif

#endif  /* !__elf64_h */
