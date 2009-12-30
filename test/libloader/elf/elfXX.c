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
#include "elfXX.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * ELF bi-endian support
 */

#define ELFXX

__LOADER_FAST_INLINE elfXX_ehdr_t const *
elfXX_image_get_ehdr(loader_image_t const *image)
{ return (elfXX_ehdr_t const *)loader_image_get_base_address(image); }

__LOADER_FAST_INLINE elf_half_t
elfXX_image_swap_half(loader_boolean_t big, elf_half_t value)
{ return big ? loader_swap_big_16(value) : loader_swap_little_16(value); }

__LOADER_FAST_INLINE elf_word_t
elfXX_image_swap_word(loader_boolean_t big, elf_word_t value)
{ return big ? loader_swap_big_32(value) : loader_swap_little_32(value); }

__LOADER_FAST_INLINE elf_xword_t
elfXX_image_swap_xword(loader_boolean_t big, elf_xword_t value)
{ return big ? loader_swap_big_64(value) : loader_swap_little_64(value); }

#define elfXX_image_swap_16 elfXX_image_swap_half
#define elfXX_image_swap_32 elfXX_image_swap_word
#define elfXX_image_swap_64 elfXX_image_swap_xword

#define __elfXX_ZZZ_get_half(eh, s, field)    (elfXX_image_swap_half(elfXX_is_big(eh), (elf_half_t)((s)->field)))
#define __elfXX_ZZZ_get_shalf(eh, s, field)   ((elf_shalf_t)__elfXX_ZZZ_get_half(eh, s, field))
#define __elfXX_ZZZ_get_word(eh, s, field)    (elfXX_image_swap_word(elfXX_is_big(eh), (elf_word_t)((s)->field)))
#define __elfXX_ZZZ_get_sword(eh, s, field)   ((elf_sword_t)__elfXX_ZZZ_get_word(eh, s, field))
#define __elfXX_ZZZ_get_xword(eh, s, field)   (elfXX_image_swap_xword(elfXX_is_big(eh), (elf_xword_t)((s)->field)))
#define __elfXX_ZZZ_get_sxword(eh, s, field)  ((elf_sxword_t)__elfXX_ZZZ_get_xword(eh, s, field))
#define __elfXX_ZZZ_get_offset(eh, s, field)  (elfXX_image_swap_XX(elfXX_is_big(eh), (elfXX_offset_t)((s)->field)))
#define __elfXX_ZZZ_get_address(eh, s, field) (elfXX_image_swap_XX(elfXX_is_big(eh), (elfXX_address_t)((s)->field)))

/*
 * ELF header
 */
#define elfXX_ehdr_get_type(eh)            __elfXX_ZZZ_get_half(eh, eh, type)
#define elfXX_ehdr_get_machine(eh)         __elfXX_ZZZ_get_half(eh, eh, machine)
#define elfXX_ehdr_get_version(eh)         __elfXX_ZZZ_get_word(eh, eh, version)
#define elfXX_ehdr_get_phdr_offset(eh)     __elfXX_ZZZ_get_offset(eh, eh, phdr_offset)
#define elfXX_ehdr_get_shdr_offset(eh)     __elfXX_ZZZ_get_offset(eh, eh, shdr_offset)
#define elfXX_ehdr_get_flags(eh)           __elfXX_ZZZ_get_word(eh, eh, flags)
#define elfXX_ehdr_get_phdr_count(eh)      __elfXX_ZZZ_get_half(eh, eh, ph_count)
#define elfXX_ehdr_get_phdr_size(eh)       __elfXX_ZZZ_get_half(eh, eh, ph_entry_size)
#define elfXX_ehdr_get_shdr_count(eh)      __elfXX_ZZZ_get_half(eh, eh, sh_count)
#define elfXX_ehdr_get_shdr_size(eh)       __elfXX_ZZZ_get_half(eh, eh, sh_entry_size)
#define elfXX_ehdr_get_sh_string_index(eh) __elfXX_ZZZ_get_half(eh, eh, sh_string_index)

/*
 * ELF program header
 */
#define elfXX_phdr_get_type(eh, ph)             __elfXX_ZZZ_get_word(eh, ph, type)
#define elfXX_phdr_get_flags(eh, ph)            __elfXX_ZZZ_get_word(eh, ph, flags)
#define elfXX_phdr_get_file_offset(eh, ph)      __elfXX_ZZZ_get_offset(eh, ph, file_offset)
#define elfXX_phdr_get_virtual_address(eh, ph)  __elfXX_ZZZ_get_address(eh, ph, virtual_address)
#define elfXX_phdr_get_physical_address(eh, ph) __elfXX_ZZZ_get_address(eh, ph, physical_address)
#ifdef ELF64
#define elfXX_phdr_get_file_size(eh, ph)        __elfXX_ZZZ_get_xword(eh, ph, file_size)
#define elfXX_phdr_get_memory_size(eh, ph)      __elfXX_ZZZ_get_xword(eh, ph, memory_size)
#define elfXX_phdr_get_alignment(eh, ph)        __elfXX_ZZZ_get_xword(eh, ph, alignment)
#else
#define elfXX_phdr_get_file_size(eh, ph)        __elfXX_ZZZ_get_word(eh, ph, file_size)
#define elfXX_phdr_get_memory_size(eh, ph)      __elfXX_ZZZ_get_word(eh, ph, memory_size)
#define elfXX_phdr_get_alignment(eh, ph)        __elfXX_ZZZ_get_word(eh, ph, alignment)
#endif

/*
 * ELF section header
 */
#define elfXX_shdr_get_name(eh, sh)             __elfXX_ZZZ_get_word(eh, sh, name)
#define elfXX_shdr_get_type(eh, sh)             __elfXX_ZZZ_get_word(eh, sh, type)
#define elfXX_shdr_get_virtual_address(eh, sh)  __elfXX_ZZZ_get_address(eh, sh, virtual_address)
#define elfXX_shdr_get_file_offset(eh, sh)      __elfXX_ZZZ_get_offset(eh, sh, file_offset)
#define elfXX_shdr_get_link(eh, sh)             __elfXX_ZZZ_get_word(eh, sh, link)
#define elfXX_shdr_get_info(eh, sh)             __elfXX_ZZZ_get_word(eh, sh, info)
#ifdef ELF64
#define elfXX_shdr_get_flags(eh, sh)            __elfXX_ZZZ_get_xword(eh, sh, flags)
#define elfXX_shdr_get_size(eh, sh)             __elfXX_ZZZ_get_xword(eh, sh, size)
#define elfXX_shdr_get_alignment(eh, sh)        __elfXX_ZZZ_get_xword(eh, sh, alignment)
#define elfXX_shdr_get_entry_size(eh, sh)       __elfXX_ZZZ_get_xword(eh, sh, entry_size)
#else
#define elfXX_shdr_get_flags(eh, sh)            __elfXX_ZZZ_get_word(eh, sh, flags)
#define elfXX_shdr_get_size(eh, sh)             __elfXX_ZZZ_get_word(eh, sh, size)
#define elfXX_shdr_get_alignment(eh, sh)        __elfXX_ZZZ_get_word(eh, sh, alignment)
#define elfXX_shdr_get_entry_size(eh, sh)       __elfXX_ZZZ_get_word(eh, sh, entry_size)
#endif

/*
 * ELF symbol
 */
#define elfXX_sym_get_name(eh, sym)             __elfXX_ZZZ_get_word(eh, sym, name)
#define elfXX_sym_get_info(eh, sym)             ((sym)->info)
#define elfXX_sym_get_other(eh, sym)            ((sym)->other)
#define elfXX_sym_get_section_index(eh, sym)    __elfXX_ZZZ_get_half(eh, sym, section_index)
#define elfXX_sym_get_value(eh, sym)            __elfXX_ZZZ_get_address(eh, sym, value)
#ifdef ELF64
#define elfXX_sym_get_size(eh, sym)             __elfXX_ZZZ_get_xword(eh, sym, size)
#else
#define elfXX_sym_get_size(eh, sym)             __elfXX_ZZZ_get_word(eh, sym, size)
#endif
#define elfXX_sym_get_type(eh, sym)             ELF_ST_TYPE(elfXX_sym_get_info (eh, sym))
#define elfXX_sym_get_bind(eh, sym)             ELF_ST_BIND(elfXX_sym_get_info (eh, sym))

/*
 * ELF relocation.
 */
#define elfXX_rel_get_offset(eh, rel)           __elfXX_ZZZ_get_address(eh, rel, offset)
#ifdef ELF64
#define elfXX_rel_get_info(eh, rel)             __elfXX_ZZZ_get_xword(eh, rel, info)
#else
#define elfXX_rel_get_info(eh, rel)             __elfXX_ZZZ_get_word(eh, rel, info)
#endif

#define elfXX_rela_get_offset(eh, rel)          __elfXX_ZZZ_get_address(eh, rel, offset)
#ifdef ELF64
#define elfXX_rela_get_info(eh, rel)            __elfXX_ZZZ_get_xword(eh, rel, info)
#define elfXX_rela_get_addend(eh, rel)          ((elf_sxword_t)__elfXX_ZZZ_get_xword(eh, rel, addend))
#else
#define elfXX_rela_get_info(eh, rel)            __elfXX_ZZZ_get_word(eh, rel, info)
#define elfXX_rela_get_addend(eh, rel)          ((elf_sword_t)__elfXX_ZZZ_get_word(eh, rel, addend))
#endif

/*
 * ELF header accessors.
 */
__LOADER_FAST_INLINE loader_boolean_t
elfXX_is_big(elfXX_ehdr_t const *ehdr)
{ return (ehdr->ident[ELF_EI_DATA] == ELF_ELFDATA2MSB); }

__LOADER_FAST_INLINE loader_boolean_t
elfXX_is_little(elfXX_ehdr_t const *ehdr)
{ return (ehdr->ident[ELF_EI_DATA] == ELF_ELFDATA2LSB); }

__LOADER_FAST_INLINE loader_boolean_t
elfXX_is_32(elfXX_ehdr_t const *ehdr)
{ return (ehdr->ident[ELF_EI_CLASS] == ELF_ELFCLASS32); }

__LOADER_FAST_INLINE loader_boolean_t
elfXX_is_64(elfXX_ehdr_t const *ehdr)
{ return (ehdr->ident[ELF_EI_CLASS] == ELF_ELFCLASS64); }

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* Internals                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define ELFXX_SECTION_SHSTRTAB 0
#define ELFXX_SECTION_HASH     1
#define ELFXX_SECTION_DYNAMIC  2
#define ELFXX_SECTION_SYMTAB   3
#define ELFXX_SECTION_STRTAB   4
#define ELFXX_SECTION_DYNSYM   5
#define ELFXX_SECTION_DYNSTR   6

#define ELFXX_SECTION_COUNT    7

typedef struct _elfXX_section {
	elfXX_shdr_t const *shdr;
	void const         *base;
	elfXX_offset_t      size;
	elfXX_offset_t      offset;
} elfXX_section_t;

typedef struct _elfXX_image {
	void               *base;
	elfXX_offset_t      size;

	elfXX_ehdr_t const *ehdr;
	elfXX_section_t     sections[ELFXX_SECTION_COUNT];
} elfXX_image_t;

typedef struct _elfXX_sectname {
	elf_half_t  type;
	char const *name;
} elfXX_sectname_t;

static elfXX_sectname_t const elfXX_section_names[] = {
	{ ELF_SHT_HASH,    ".hash"    },
	{ ELF_SHT_DYNAMIC, ".dynamic" },
	{ ELF_SHT_SYMTAB,  ".symtab"  },
	{ ELF_SHT_STRTAB,  ".strtab"  },
	{ ELF_SHT_DYNSYM,  ".dynsym"  },
	{ ELF_SHT_STRTAB,  ".dynstr"  },
	{ ELF_SHT_NULL,    NULL       },
};

__LOADER_FAST_INLINE loader_status_t
elfXX_image_get_phdr(elfXX_image_t const  *image,
                     loader_size_t         index,
                     elfXX_phdr_t const  **phdr)
{
	loader_size_t       offset;
	elfXX_ehdr_t const *ehdr  = image->ehdr;
	loader_size_t       count = elfXX_ehdr_get_phdr_count(ehdr);

	if (index >= count)
		return LOADER_ERROR_INVALID_ARGUMENT;

	offset = elfXX_ehdr_get_phdr_offset(ehdr) + (index * elfXX_ehdr_get_phdr_size(ehdr));

	if (offset >= image->size)
		return LOADER_ERROR_IMAGE_CORRUPTED;

	*phdr = (elfXX_phdr_t const *)((loader_uintptr_t)image->base + offset);

	return LOADER_SUCCESS;
}

__LOADER_FAST_INLINE loader_status_t
elfXX_image_get_shdr(elfXX_image_t const  *image,
                     loader_size_t         index,
                     elfXX_shdr_t const  **shdr)
{
	loader_size_t       offset;
	elfXX_ehdr_t const *ehdr  = image->ehdr;
	loader_size_t       count = elfXX_ehdr_get_shdr_count(ehdr);

	if (index >= count)
		return LOADER_ERROR_INVALID_ARGUMENT;

	offset = elfXX_ehdr_get_shdr_offset(ehdr)
	 + (index * elfXX_ehdr_get_shdr_size(ehdr));

	if (offset >= image->size)
		return LOADER_ERROR_IMAGE_CORRUPTED;

	*shdr = (elfXX_shdr_t const *)((loader_uintptr_t)image->base + offset);

	return LOADER_SUCCESS;
}

/*
 * Creates an internal elfXX_section_t from a given
 * elfXX_shdr_t.
 */
__LOADER_FAST_INLINE loader_status_t
elfXX_image_make_section(elfXX_image_t      *image,
                         elfXX_shdr_t const *shdr,
                         elf_half_t          index)
{
	loader_uintptr_t offset;
	elfXX_offset_t   size;
	elfXX_section_t *section = &image->sections[index];

	offset = elfXX_shdr_get_file_offset(image->ehdr, shdr);
	size = elfXX_shdr_get_size(image->ehdr, shdr);
	if (offset >= image->size || (offset + size) > image->size)
		return LOADER_ERROR_IMAGE_CORRUPTED;

	section->shdr   = shdr;
	section->base   = (void *)((loader_uintptr_t)image->base + offset);
	section->offset = offset;
	section->size   = size;

	return LOADER_SUCCESS;
}

/*
 * Get the strings section pointer and length of the
 * section header strings table.
 */
__LOADER_FAST_INLINE loader_status_t
elfXX_image_get_section_names(elfXX_image_t const  *image,
                              char const          **strings,
                              loader_size_t        *strings_size)
{
	if (image->sections[ELFXX_SECTION_SHSTRTAB].base == NULL)
		return LOADER_ERROR_NOT_FOUND;

	*strings = (char const *)image->sections[ELFXX_SECTION_SHSTRTAB].base;
	if (strings_size != NULL)
		*strings_size = image->sections[ELFXX_SECTION_SHSTRTAB].size;

	return LOADER_SUCCESS;
}

/*
 * Get the symbol or dynamic symbol table.
 */
__LOADER_FAST_INLINE loader_status_t
elfXX_image_get_symbol_table(elfXX_image_t const    *image,
                             loader_boolean_t        dynamic,
                             elfXX_section_t const **section)
{
	if (dynamic)
		*section = &image->sections[ELFXX_SECTION_DYNSYM];
	else
		*section = &image->sections[ELFXX_SECTION_SYMTAB];

	if ((*section)->base == NULL)
		return LOADER_ERROR_NOT_FOUND;
	else
		return LOADER_SUCCESS;
}

/*
 * Find an elfXX_shdr_t by name and type.
 */
static loader_status_t
elfXX_image_find_shdr(elfXX_image_t const  *image,
                      char const           *name,
                      elf_half_t            type,
                      elfXX_shdr_t const  **shdr)
{
	loader_size_t       n;
	loader_status_t     status;
	char const         *strings;
	loader_size_t       strings_size;
	elfXX_ehdr_t const *ehdr = image->ehdr;
	loader_size_t       shdr_count = elfXX_ehdr_get_shdr_count(ehdr);

	status = elfXX_image_get_section_names(image, &strings, &strings_size);
	if (LOADER_FAILED(status))
		return status;

	for (n = 0; n < shdr_count; n++) {
		loader_status_t     status;
		elf_half_t          sect_type;
		elf_word_t          name_offset;
		elfXX_shdr_t const *sect;

		status = elfXX_image_get_shdr(image, n, &sect);
		if (LOADER_FAILED(status))
			return status;

		sect_type = elfXX_shdr_get_type(ehdr, sect);
		name_offset = elfXX_shdr_get_name(ehdr, sect);
		if (name_offset >= strings_size)
			return LOADER_ERROR_IMAGE_CORRUPTED;

		if (type == sect_type &&
			loader_strcmp(strings + name_offset, name) == 0) {
			*shdr = sect;
			return LOADER_SUCCESS;
		}
	}

	return LOADER_ERROR_NOT_FOUND;
}

/*
 * Get the symbol string table.
 */
__LOADER_FAST_INLINE loader_status_t
elfXX_image_get_string_table(elfXX_image_t const    *image,
                             loader_boolean_t        dynamic,
                             char const            **strings,
                             loader_size_t          *strings_size)
{
	elfXX_section_t const *section;

	if (dynamic)
		section = &image->sections[ELFXX_SECTION_DYNSTR];
	else
		section = &image->sections[ELFXX_SECTION_STRTAB];

	if (strings == NULL || strings_size == 0)
		return LOADER_ERROR_NOT_FOUND;

	*strings = (char const *)section->base;
	if (strings_size != NULL)
		*strings_size = section->size;

	return LOADER_SUCCESS;
}

#if 0
/*
 * Find a symbol by name using slow lookup.
 */
static loader_status_t
elfXX_image_find_symbol_slow(elfXX_image_t const  *image,
                             char const           *name,
                             loader_boolean_t      dynamic,
                             elfXX_sym_t const   **sym)
{
	loader_status_t        status;
	loader_size_t          n, sym_count;
	char const            *strings;
	loader_size_t          strings_size;
	elfXX_section_t const *symtab_section;
	elfXX_sym_t const     *symtab;

	/*
	 * Get the symbol table section.
	 */
	if (dynamic)
		symtab_section = &image->sections[ELFXX_SECTION_DYNSYM];
	else
		symtab_section = &image->sections[ELFXX_SECTION_SYMTAB];
	if (symtab_section == NULL)
		return LOADER_ERROR_NOT_FOUND;

	/*
	 * Get the string table.
	 */
	status = elfXX_image_get_string_table(image, dynamic, &strings, &strings_size);
	if (LOADER_FAILED(status))
		return status;

	if (symtab_section->size % sizeof(elfXX_sym_t))
		return LOADER_ERROR_IMAGE_CORRUPTED;

	symtab    = (elfXX_sym_t const *)symtab_section->base;
	sym_count = symtab_section->size / sizeof(elfXX_sym_t);

	for (n = 0; n < sym_count; n++) {
		elf_word_t  name_offset;
		char const *sym_name;

		name_offset = elfXX_sym_get_name(image->ehdr, &symtab[n]);
		if (name_offset >= strings_size)
			return LOADER_ERROR_IMAGE_CORRUPTED;

		sym_name = strings + name_offset;
		if (*sym_name == 0)
			continue;

		if (loader_strcmp(sym_name, name) == 0) {
			*sym = symtab + n;
			return LOADER_SUCCESS;
		}
	}

	return LOADER_ERROR_NOT_FOUND;
}

/*
 * Find a symbol by name using hashed lookup.
 */
static loader_status_t
elfXX_image_find_symbol_fast(elfXX_image_t const  *image,
                             char const           *name,
                             elfXX_sym_t const   **sym)
{
	loader_size_t          n;
	loader_size_t          sym_count;
	loader_status_t        status;
	elf_word_t             hash;
	elf_word_t             nbucket;
	elf_word_t             nchain;
	elf_word_t             index;
	elf_word_t const      *hash_table;
	elf_word_t const      *chain;
	char const            *strings;
	loader_size_t          strings_size;
	elfXX_sym_t const     *symtab;
	elfXX_section_t const *symtab_section;
	elfXX_section_t const *hash_section;
	loader_boolean_t       big_endian;

	/*
	 * Get the hash section.
	 */
	hash_section = &image->sections[ELFXX_SECTION_HASH];
	if (hash_section->base == NULL)
		return LOADER_ERROR_UNSUPPORTED_OPERATION;

	/*
	 * Get the dynamic symbol table section.
	 */
	symtab_section = &image->sections[ELFXX_SECTION_DYNSYM];
	if (symtab_section->base == NULL)
		return LOADER_ERROR_IMAGE_CORRUPTED;

	if (symtab_section->size % sizeof(elfXX_sym_t))
		return LOADER_ERROR_IMAGE_CORRUPTED;

	hash_table = (elf_word_t const *)hash_section->base;
	symtab     = (elfXX_sym_t const *)symtab_section->base;
	sym_count  = symtab_section->size / sizeof(elfXX_sym_t);
	big_endian = elfXX_is_big(image->ehdr);

	/*
	 * Get the string table.
	 */
	status = elfXX_image_get_string_table(image, LOADER_TRUE, &strings, &strings_size);
	if (LOADER_FAILED(status))
		return status;

	/*
	 * Hash the symbol
	 */
	hash = elf_hash(name);

	nbucket = elfXX_image_swap_word(big_endian, hash_table[0]);
	nchain  = elfXX_image_swap_word(big_endian, hash_table[1]);

	if (nbucket == 0 || nchain == 0)
		return LOADER_ERROR_IMAGE_CORRUPTED;

	index = hash % nbucket;
	chain = &hash_table[2 + index];

	for (n = 0; *chain != ELF_STN_UNDEF; n++) {
		elf_word_t  name_offset;
		char const *sym_name;

		index = elfXX_image_swap_word(big_endian, *chain);
		if (*chain >= nchain)
			return LOADER_ERROR_IMAGE_CORRUPTED;

		name_offset = elfXX_sym_get_name(image->ehdr, symtab + index);
		if (name_offset >= strings_size)
			return LOADER_ERROR_IMAGE_CORRUPTED;

		sym_name = strings + name_offset;

		if (loader_strcmp(sym_name, name) == 0) {
			*sym = symtab + n;
			return LOADER_SUCCESS;
		}

		chain = &hash_table[2 + nbucket + index];
	}

	return LOADER_ERROR_NOT_FOUND;
}
#endif

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* Image Loader Interface                                                    */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

typedef struct _elfXX_symbol_iterator {
	elfXX_image_t const *elf;
	loader_size_t        index;
	elfXX_sym_t const   *symbol_table;
	loader_size_t        symbol_count;
	char const          *string_table;
	loader_size_t        string_size;
} elfXX_symbol_iterator_t;

typedef struct _elfXX_section_iterator {
	elfXX_image_t const *elf;
	loader_size_t        index;
	loader_size_t        count;
	char const          *string_table;
	loader_size_t        string_size;
} elfXX_section_iterator_t;

/*
 * Get the image format name.
 */
static char const *
LOADERCALL
elfXX_image_get_name(loader_image_t const *image,
                     void                 *cookie)
{
	(void)image;
	(void)cookie;

	return "ELFXX";
}

/*
 * Identify an ELF image format.
 */
static loader_status_t
LOADERCALL
elfXX_image_identify(loader_image_t const *image,
                     loader_image_ident_t *ident)
{
	loader_status_t     status;
	elfXX_ehdr_t const *ehdr;

	ehdr = elfXX_image_get_ehdr(image);
	if (!elf_is_valid(ehdr->ident))
		return LOADER_ERROR_INVALID_IMAGE;

	ident->score  = 0;
	ident->format = 0;
	ident->offset = 0;

#ifdef ELF32
	if (elfXX_is_32(ehdr))
		ident->format |= LOADER_IMAGE_CLASS_32BIT;
#endif
#ifdef ELF64
	if (elfXX_is_64(ehdr))
		ident->format |= LOADER_IMAGE_CLASS_64BIT;
#endif
	else
		return LOADER_ERROR_INVALID_CLASS;

	if (elfXX_is_little(ehdr))
		ident->format |= LOADER_IMAGE_ENDIAN_LITTLE;
	else if (elfXX_is_big(ehdr))
		ident->format |= LOADER_IMAGE_ENDIAN_BIG;
	else
		return LOADER_ERROR_INVALID_ENDIAN;

	if (ehdr->ident[ELF_EI_VERSION] != ELF_EV_CURRENT ||
		elfXX_ehdr_get_version(ehdr) != ELF_EV_CURRENT)
		return LOADER_ERROR_UNSUPPORTED_VERSION;

	status = elf_get_loader_arch(elfXX_ehdr_get_machine(ehdr),
		elfXX_is_big(ehdr), &ident->format);
	if (LOADER_FAILED(status))
		return status;

	status = elf_get_loader_type(elfXX_ehdr_get_type(ehdr), &ident->format);
	if (LOADER_FAILED(status))
		return status;

	return LOADER_SUCCESS;
}

/*
 * Opens a previously identified ELF image, cache the
 * most commonly used sections.
 */
static loader_status_t
LOADERCALL
elfXX_image_open(loader_image_t const   *image,
                 void                  **cookie)
{
	elf_half_t          n;
	elfXX_image_t      *elf;
	elfXX_shdr_t const *shdr;
	loader_status_t     status;

	elf = (elfXX_image_t *)loader_allocate(sizeof(elfXX_image_t));
	if (elf == NULL)
		return LOADER_ERROR_NO_MEMORY;

	elf->base = (void *)loader_image_get_base_address(image);
	elf->size = loader_image_get_size(image);

	elf->ehdr = (elfXX_ehdr_t const *)elf->base;

	/*
	 * Get the commonly used sections.
	 */

	/* The section names string table is in the ELF header. */
	status = elfXX_image_get_shdr(elf, elfXX_ehdr_get_sh_string_index(elf->ehdr),
		&shdr);
	if (LOADER_FAILED(status))
		goto fail;

	if (elfXX_shdr_get_type(elf->ehdr, shdr) != ELF_SHT_STRTAB)
		return LOADER_ERROR_IMAGE_CORRUPTED;

	status = elfXX_image_make_section(elf, shdr, ELFXX_SECTION_SHSTRTAB);
	if (LOADER_FAILED(status))
		goto fail;

	/* --- */
	for (n = 0; elfXX_section_names[n].type != ELF_SHT_NULL; n++) {
		status = elfXX_image_find_shdr(elf,
				elfXX_section_names[n].name,
				elfXX_section_names[n].type,
				&shdr);
		if (LOADER_FAILED(status)) {
			// printf("elfXX_image_find_shdr failed: status=%d\n", status);
			if (status == LOADER_ERROR_NOT_FOUND)
				continue;

			goto fail;
		}

		status = elfXX_image_make_section(elf, shdr, n + 1);
		if (LOADER_FAILED(status)) {
			// printf("elfXX_image_make_section failed: status=%d\n", status);
			goto fail;
		}
	}

	*cookie = elf;

	return LOADER_SUCCESS;

fail:
	loader_free(elf);

	return status;
}

static void
LOADERCALL
elfXX_image_close(loader_image_t const *image,
                  void                 *cookie)
{
	(void)image;

	if (cookie != NULL)
		loader_free(cookie);
}

/* * * * SYMBOL ITERATOR * * * */

static void
elfXX_symbol_iterator_free(void *opaque)
{
	elfXX_symbol_iterator_t *i = (elfXX_symbol_iterator_t *)opaque;

	loader_free(i);
}

static loader_boolean_t
elfXX_symbol_iterator_reset(void *opaque)
{
	elfXX_symbol_iterator_t *i = (elfXX_symbol_iterator_t *)opaque;

	i->index = 0;

	return LOADER_TRUE;
}

__LOADER_FAST_INLINE loader_status_t
elfXX_image_get_symbol(elfXX_image_t const  *image,
                       loader_size_t         index,
                       loader_boolean_t      dynamic,
                       elfXX_sym_t const   **symbol,
                       char const          **name)
{
	elf_word_t             name_offset;
	elf_word_t             symbol_count;
	elfXX_sym_t const     *symbol_table;
	elfXX_sym_t const     *elf_symbol;
	elfXX_section_t const *symtab_section;

	if (dynamic)
		symtab_section = &image->sections[ELFXX_SECTION_DYNSYM];
	else
		symtab_section = &image->sections[ELFXX_SECTION_SYMTAB];

	if (symtab_section->base == NULL)
		return LOADER_ERROR_UNSUPPORTED_OPERATION;

	if (symtab_section->size % sizeof(elfXX_sym_t))
		return LOADER_ERROR_IMAGE_CORRUPTED;

	symbol_count = symtab_section->size / sizeof(elfXX_sym_t);
	if (index >= symbol_count)
		return LOADER_ERROR_INVALID_ARGUMENT;

	if (dynamic)
		symtab_section = &image->sections[ELFXX_SECTION_DYNSYM];
	else
		symtab_section = &image->sections[ELFXX_SECTION_SYMTAB];

	if (symtab_section->base == NULL)
		return LOADER_ERROR_UNSUPPORTED_OPERATION;

	symbol_table = (elfXX_sym_t const *)symtab_section->base;
	elf_symbol = symbol_table + index;

	if (name != NULL) {
		elf_small_t      type;
		loader_status_t  status;
		char const      *strings;
		loader_size_t    strings_size;

		*name = NULL;

		type = elfXX_sym_get_type(image->ehdr, elf_symbol);

		if (type == ELF_STT_SECTION) {
			elfXX_shdr_t const *sect;
			elf_half_t          section_index;

			status = elfXX_image_get_section_names(image, &strings, &strings_size);
			if (LOADER_FAILED(status))
				return status;

			section_index = elfXX_sym_get_section_index(image->ehdr, elf_symbol);

			status = elfXX_image_get_shdr(image, section_index, &sect);
			if (LOADER_FAILED(status))
				return status;

			name_offset = elfXX_shdr_get_name(image->ehdr, sect);
			if (name_offset >= strings_size)
				return LOADER_FALSE;

			*name = strings + name_offset;
		} else {
			status = elfXX_image_get_string_table(image, dynamic, &strings, &strings_size);

			if (LOADER_SUCCEEDED(status)) {
				name_offset = elfXX_sym_get_name(image->ehdr, elf_symbol);
				if (name_offset < strings_size)
					*name = strings + name_offset;
			} else {
				*name = "*ABS*";
			}
		}
	}

	*symbol = elf_symbol;
	return LOADER_SUCCESS;
}

static loader_boolean_t
elfXX_symbol_iterator_next(void *value, void *opaque)
{
	elf_word_t               name_offset;
	elf_half_t               section_index;
	elfXX_offset_t           symbol_offset;
	elfXX_address_t          symbol_virtual_address;
	elfXX_address_t          symbol_physical_address;
	char const              *sym_name;
	elfXX_sym_t const       *elf_symbol;
	elfXX_shdr_t const      *elf_section;
	loader_symbol_t         *symbol = (loader_symbol_t *)value;
	elfXX_symbol_iterator_t *i      = (elfXX_symbol_iterator_t *)opaque;

	for (;;) {
		symbol->type  = LOADER_SYMBOL_TYPE_INVALID;
		symbol->flags = 0;

		if (i->index >= i->symbol_count)
			return LOADER_FALSE;

		elf_symbol = i->symbol_table + i->index;

		name_offset = elfXX_sym_get_name(i->elf->ehdr, elf_symbol);
		if (name_offset >= i->string_size)
			return LOADER_FALSE;

		sym_name = i->string_table + name_offset;
		if (*sym_name == 0) {
			i->index++;
			continue;
		}

		section_index = elfXX_sym_get_section_index(i->elf->ehdr, elf_symbol);
		if (section_index == ELF_SHN_UNDEF)
			break;

		switch (ELF_ST_BIND(elf_symbol->info)) {
			case ELF_STB_WEAK:
				symbol->flags |= LOADER_SYMBOL_FLAG_WEAK;

			case ELF_STB_GLOBAL:
				symbol->flags |= LOADER_SYMBOL_FLAG_GLOBAL;
				switch (ELF_ST_TYPE(elf_symbol->info)) {
					case ELF_STT_NOTYPE:
						symbol->type = LOADER_SYMBOL_TYPE_ANCHOR;
						break;

					case ELF_STT_OBJECT:
						symbol->type = LOADER_SYMBOL_TYPE_OBJECT;
						break;

					case ELF_STT_FUNC:
						symbol->type = LOADER_SYMBOL_TYPE_FUNCTION;
						break;

					case ELF_STT_SECTION:
						symbol->type = LOADER_SYMBOL_TYPE_SECTION;
						break;

					default:
						i->index++;
						continue;
				}
				break;

			default:
				i->index++;
				continue;
		}

		break;
	}

	elf_section = NULL;

	switch (section_index) {
		case ELF_SHN_UNDEF:
			symbol->flags |= LOADER_SYMBOL_FLAG_EXTERNAL;
			break;

		case ELF_SHN_ABS:
			symbol->flags |= LOADER_SYMBOL_FLAG_ABSOLUTE;
			break;

		case ELF_SHN_COMMON:
			symbol->flags |= LOADER_SYMBOL_FLAG_COMMON;
			break;

		default:
			if (LOADER_FAILED(elfXX_image_get_shdr(i->elf, section_index - 1, &elf_section)))
				return LOADER_FALSE;
			break;
	}

	symbol_offset           = 0;
	symbol_virtual_address  = 0;
	symbol_physical_address = 0;
	if (elf_section != NULL) {
		symbol_offset          = elfXX_shdr_get_file_offset(i->elf->ehdr, elf_section);
		symbol_virtual_address = elfXX_shdr_get_virtual_address(i->elf->ehdr, elf_section);
	}

	if (section_index != ELF_SHN_UNDEF) {
		elfXX_address_t value = elfXX_sym_get_value(i->elf->ehdr, elf_symbol);

		symbol_offset += value;
		if (elfXX_ehdr_get_type(i->elf->ehdr) == ELF_ET_REL) {
			symbol_physical_address += value;
			symbol_virtual_address  += value;
		} else {
			symbol_offset -= symbol_virtual_address;
		}
	}

	symbol->module           = NULL;
	symbol->name             = sym_name;
	symbol->file_offset      = symbol_offset;
	symbol->virtual_address  = symbol_virtual_address;
	symbol->physical_address = symbol_physical_address;
	if (section_index != ELF_SHN_UNDEF)
		symbol->size           = elfXX_sym_get_size(i->elf->ehdr, elf_symbol);
	else
		symbol->size           = 0;

	i->index++;

	return LOADER_TRUE;
}

static loader_iterator_callbacks_t const elfXX_symbol_iterator_callbacks = {
	elfXX_symbol_iterator_free,
	elfXX_symbol_iterator_reset,
	elfXX_symbol_iterator_next
};

__LOADER_FAST_INLINE loader_status_t
elfXX_symbol_iterator_new(elfXX_image_t const   *image,
                          loader_flags_t         flags,
                          loader_iterator_t    **iterator)
{
	loader_status_t          status;
	elfXX_symbol_iterator_t *i;
	elfXX_section_t const   *symtab_section;
	char const              *strings;
	loader_size_t            strings_size;

	if (flags & LOADER_IMAGE_ITERATE_SYMBOL_DYNAMIC) {
		if (elfXX_ehdr_get_type(image->ehdr) == ELF_ET_REL)
			flags &= ~LOADER_IMAGE_ITERATE_SYMBOL_DYNAMIC;
	}

	if (flags & LOADER_IMAGE_ITERATE_SYMBOL_DYNAMIC)
		symtab_section = &image->sections[ELFXX_SECTION_DYNSYM];
	else
		symtab_section = &image->sections[ELFXX_SECTION_SYMTAB];

	if (symtab_section->base == NULL)
		return LOADER_ERROR_UNSUPPORTED_OPERATION;

	if (symtab_section->size % sizeof(elfXX_sym_t))
		return LOADER_ERROR_IMAGE_CORRUPTED;

	status = elfXX_image_get_string_table(image,
			(flags & LOADER_IMAGE_ITERATE_SYMBOL_DYNAMIC) != 0,
			&strings, &strings_size);
	if (LOADER_FAILED(status))
		return status;

	i = (elfXX_symbol_iterator_t *)loader_allocate(sizeof(*i));
	if (i == NULL)
		return LOADER_ERROR_NO_MEMORY;

	i->elf          = image;
	i->index        = 0;
	i->symbol_count = symtab_section->size / sizeof(elfXX_sym_t);
	i->symbol_table = (elfXX_sym_t const *)symtab_section->base;
	i->string_table = strings;
	i->string_size  = strings_size;

	*iterator = loader_iterator_new(&elfXX_symbol_iterator_callbacks, i);
	if (*iterator == NULL) {
		loader_free(i);
		return LOADER_ERROR_NO_MEMORY;
	}

	return LOADER_SUCCESS;
}

/* * * * SECTION ITERATOR * * * */

static void
elfXX_section_iterator_free(void *opaque)
{
	elfXX_section_iterator_t *i = (elfXX_section_iterator_t *)opaque;

	loader_free(i);
}

static loader_boolean_t
elfXX_section_iterator_reset(void *opaque)
{
	elfXX_section_iterator_t *i = (elfXX_section_iterator_t *)opaque;

	i->index = 0;

	return LOADER_TRUE;
}

static loader_boolean_t
elfXX_section_iterator_next(void *value, void *opaque)
{
	loader_status_t           status;
	elf_word_t                name_offset;
	elfXX_shdr_t const       *elf_section;
	loader_section_t         *section = (loader_section_t *)value;
	elfXX_section_iterator_t *i       = (elfXX_section_iterator_t *)opaque;

	section->type  = LOADER_SYMBOL_TYPE_INVALID;
	section->flags = 0;

	if (i->index >= i->count)
		return LOADER_FALSE;

	status = elfXX_image_get_shdr(i->elf, i->index, &elf_section);
	if (LOADER_FAILED(status))
		return LOADER_FALSE;

	name_offset = elfXX_shdr_get_name(i->elf->ehdr, elf_section);
	if (name_offset >= i->string_size)
		return LOADER_FALSE;

	section->module           = NULL;
	section->name             = i->string_table + name_offset;
	section->file_offset      = elfXX_shdr_get_file_offset(i->elf->ehdr, elf_section);
	section->virtual_address  = elfXX_shdr_get_virtual_address(i->elf->ehdr, elf_section);
	section->physical_address = 0;
	section->size             = elfXX_shdr_get_size(i->elf->ehdr, elf_section);
	section->alignment        = elfXX_shdr_get_alignment(i->elf->ehdr, elf_section);

	i->index++;

	return LOADER_TRUE;
}

static loader_iterator_callbacks_t const elfXX_section_iterator_callbacks = {
	elfXX_section_iterator_free,
	elfXX_section_iterator_reset,
	elfXX_section_iterator_next
};

__LOADER_FAST_INLINE loader_status_t
elfXX_section_iterator_new(elfXX_image_t const   *image,
                           loader_flags_t         flags,
                           loader_iterator_t    **iterator)
{
	loader_status_t           status;
	elfXX_section_iterator_t *i;
	char const               *strings;
	loader_size_t             strings_size;

	status = elfXX_image_get_section_names(image,
			&strings, &strings_size);
	if (LOADER_FAILED(status))
		return status;

	i = (elfXX_section_iterator_t *)loader_allocate(sizeof(*i));
	if (i == NULL)
		return LOADER_ERROR_NO_MEMORY;

	i->elf          = image;
	i->index        = 0;
	i->count        = elfXX_ehdr_get_shdr_count(image->ehdr);
	i->string_table = strings;
	i->string_size  = strings_size;

	*iterator = loader_iterator_new(&elfXX_section_iterator_callbacks, i);
	if (*iterator == NULL) {
		loader_free(i);
		return LOADER_ERROR_NO_MEMORY;
	}

	return LOADER_SUCCESS;
}

static loader_status_t
LOADERCALL
elfXX_image_get_section_iterator(loader_image_t const  *image,
                                 loader_flags_t         flags,
                                 loader_iterator_t    **iterator,
                                 void                  *cookie)
{
	elfXX_image_t const *elf = (elfXX_image_t const *)cookie;
	(void)image;
	return elfXX_section_iterator_new(elf, flags, iterator);
}

/* * * * RELOCATION ITERATOR * * * */

#if 0
static void
process_rel(elfXX_image_t const *image,
            elfXX_offset_t       file_offset,
            elf_word_t           size)
{
	elfXX_rel_t const *reloc;
	loader_size_t      n, reloc_count;

	reloc_count = size / sizeof(elfXX_rel_t);
	reloc = (elfXX_rel_t const *)(loader_uintptr_t)((loader_uintptr_t)image->base + file_offset);

	for (n = 0; n < reloc_count; n++) {
		loader_status_t    status;
		elfXX_sym_t const *symbol;
		char const        *name;

		status = elfXX_image_get_symbol(image, 
				ELFXX_R_SYM(elfXX_rel_get_info(image->ehdr, reloc + n)),
				(elfXX_ehdr_get_type(image->ehdr) != ELF_ET_REL),
				&symbol,
				&name);

		if (LOADER_FAILED(status))
			continue;

		printf("Offset %llx Info %llx [Symbol %llu Type %llu] Name '%s'\n",
				(unsigned long long)elfXX_rel_get_offset(image->ehdr, reloc + n),
				(unsigned long long)elfXX_rel_get_info(image->ehdr, reloc + n),
        (unsigned long long)ELFXX_R_SYM(elfXX_rel_get_info(image->ehdr, reloc + n)),
        (unsigned long long)ELFXX_R_TYPE(elfXX_rel_get_info(image->ehdr, reloc + n)),
				name);
	}
}

static void
process_rela(elfXX_image_t const *image,
             elfXX_offset_t       file_offset,
             elf_word_t           size)
{
	elfXX_rela_t const *reloc;
	loader_size_t       n, reloc_count;

	reloc_count = size / sizeof(elfXX_rela_t);
	reloc = (elfXX_rela_t const *)(loader_uintptr_t)((loader_uintptr_t)image->base + file_offset);

	for (n = 0; n < reloc_count; n++) {
		loader_status_t    status;
		elfXX_sym_t const *symbol;
		char const        *name;

		status = elfXX_image_get_symbol(image, 
				ELFXX_R_SYM(elfXX_rel_get_info(image->ehdr, reloc + n)),
				LOADER_FALSE,
				&symbol,
				&name);
		if (LOADER_FAILED(status))
			continue;

		printf("Offset %llx Info %llx [Symbol %u Type %u] Addend %llx Name '%s'\n",
				(unsigned long long)elfXX_rela_get_offset(image->ehdr, reloc + n),
				(unsigned long long)elfXX_rela_get_info(image->ehdr, reloc + n),
				(unsigned)ELFXX_R_SYM(elfXX_rel_get_info(image->ehdr, reloc + n)),
				(unsigned)ELFXX_R_TYPE(elfXX_rel_get_info(image->ehdr, reloc + n)),
				(long long)elfXX_rela_get_addend(image->ehdr, reloc + n),
				name
			   );
	}
}

static void
elfXX_iter_reloc(elfXX_image_t const *image)
{
	loader_size_t   n, section_count;
	loader_status_t status;

	section_count = elfXX_ehdr_get_shdr_count(image->ehdr);
	for (n = 0; n < section_count; n++) {
		elfXX_shdr_t const *shdr;
		elfXX_shdr_t const *ref_shdr;
		elf_small_t         type;

		status = elfXX_image_get_shdr(image, n, &shdr);
		if (LOADER_FAILED(status))
			continue;

		/* We always assume .symtab and .dynsym are used appropriaterly. */
		type = elfXX_shdr_get_type(image->ehdr, shdr);

		if (type == ELF_SHT_REL || type == ELF_SHT_RELA) {
			status = elfXX_image_get_shdr(image, elfXX_shdr_get_info(image->ehdr, shdr),
					&ref_shdr);
			if (LOADER_FAILED(status))
				continue;

			/* Skip over non-allocated sections. */
			if ((elfXX_shdr_get_flags(image->ehdr, ref_shdr) & ELF_SHF_ALLOC) == 0)
				continue;

			if (type == ELF_SHT_REL) {
				printf("REL Section %u (%llx) is interesting.\n", (unsigned)n,
			(unsigned long long)elfXX_shdr_get_virtual_address(image->ehdr, ref_shdr));

				process_rel(image,
						elfXX_shdr_get_file_offset(image->ehdr, shdr),
						elfXX_shdr_get_size(image->ehdr, shdr));
			} else {
				printf("RELA Section %u (%llx) is interesting.\n", (unsigned)n,
						(unsigned long long)elfXX_shdr_get_virtual_address(image->ehdr, ref_shdr));

				process_rela(image,
						elfXX_shdr_get_file_offset(image->ehdr, shdr),
						elfXX_shdr_get_size(image->ehdr, shdr));
			}
		}
	}

}
#endif

static loader_status_t
LOADERCALL
elfXX_image_get_symbol_iterator(loader_image_t const  *image,
                                loader_flags_t         flags,
                                loader_iterator_t    **iterator,
                                void                  *cookie)
{
	elfXX_image_t const *elf = (elfXX_image_t const *)cookie;
	(void)image;
//	elfXX_iter_reloc(elf);
	return elfXX_symbol_iterator_new(elf, flags, iterator);
}

#if 0
static loader_status_t
LOADERCALL
elfXX_image_fixup(loader_image_t const *image,
                  loader_env_t const   *env,
                  void                 *cookie)
{
	loader_status_t      status;
	loader_symbol_t      symbol;
	char const          *symbol_name;
	elfXX_image_t const *elf = (elfXX_image_t const *)cookie;

	status = loader_env_lookup(env,
			NULL,
			symbol_name,
			LOADER_ENV_LOOKUP_FLAT,
			&symbol);
	if (LOADER_FAILED(status))
		return status;
}
#endif

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* Registration                                                              */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static void *elfXX_image_handler_cookie = NULL;
static loader_image_callbacks_t const elfXX_image_callbacks = {
	elfXX_image_identify,
	elfXX_image_open,
	elfXX_image_close,
	elfXX_image_get_name,
	elfXX_image_get_symbol_iterator,
	elfXX_image_get_section_iterator
};

loader_status_t
LOADERCALL
elfXX_image_register(void)
{
	return loader_registry_add_handler(&elfXX_image_callbacks, "ELFXX", &elfXX_image_handler_cookie);
}
