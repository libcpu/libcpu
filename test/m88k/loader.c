#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xec-debug.h"
#include "xec-mmap.h"
#include "xec-byte-order.h"

#include "m88k-context.h"

#include "loader.h"

static void *g_ldr_log = NULL;

struct coff_aouthdr g_ahdr;
aout_header_t       g_exec;

static int
mapin(xec_mem_if_t *mem_if, xec_mmap_t *mm, aout_header_t const *ah)
{
	void const *bytes;
	void *text;
	void *data;
	xec_mem_flg_t mf;

	g_ahdr.tstart = N_TXTADDR(*ah);
	g_ahdr.dstart = N_DATADDR(*ah);

	g_ahdr.tsize = ah->a_text;
	g_ahdr.dsize = ah->a_data;
	g_ahdr.bsize = ah->a_bss;

	bytes = xec_mmap_get_bytes(mm);

	mf = 0;
	text = (void *)xec_mem_gtoh(mem_if, g_ahdr.tstart, &mf);
	if (mf != 0) return LOADER_INVALID_ADDRESS;
	memcpy(text, (void *)(uintptr_t)bytes, ah->a_text);

	data = (void *)xec_mem_gtoh(mem_if, g_ahdr.dstart, &mf);
	if (mf != 0) return LOADER_INVALID_ADDRESS;
	memcpy(data, (void *)((uintptr_t)bytes + ah->a_text), ah->a_data);

	return LOADER_SUCCESS;
}

static void
aout_dump_sections(aout_header_t const *ah)
{
	unsigned long entry;

	XEC_LOG(g_ldr_log,
			XEC_LOG_INFO,
			0,
			"  .text start = %08lx size = %08lx offset = %08lx",
			(unsigned long)N_TXTADDR(*ah),
			(unsigned long)ah->a_text,
			(unsigned long)N_TXTOFF(*ah));
	XEC_LOG(g_ldr_log,
			XEC_LOG_INFO,
			0,
			"  .data start = %08lx size = %08lx offset = %08lx",
			(unsigned long)N_DATADDR(*ah),
			(unsigned long)ah->a_data,
			(unsigned long)N_DATOFF(*ah));
	XEC_LOG(g_ldr_log,
			XEC_LOG_INFO,
			0,
			"  .bss  start = %08lx size = %08lx",
			(unsigned long)N_BSSADDR(*ah),
			(unsigned long)ah->a_bss);

	entry = (unsigned long)ah->a_entry;

	XEC_LOG(g_ldr_log,
			XEC_LOG_INFO,
			0,
			"  entry point = %08lx",
			entry);

	g_ahdr.entry = entry;
}

static void
aout_swap(aout_header_t const *in,
		  aout_header_t       *out)
{
	out->a_midmag = xec_byte_swap_big_to_host32(in->a_midmag);
	out->a_text   = xec_byte_swap_big_to_host32(in->a_text);
	out->a_data   = xec_byte_swap_big_to_host32(in->a_data);
	out->a_bss    = xec_byte_swap_big_to_host32(in->a_bss);
	out->a_syms   = xec_byte_swap_big_to_host32(in->a_syms);
	out->a_entry  = xec_byte_swap_big_to_host32(in->a_entry);
	out->a_trsize = xec_byte_swap_big_to_host32(in->a_trsize);
	out->a_drsize = xec_byte_swap_big_to_host32(in->a_drsize);
}

static int
aout_process(xec_mem_if_t *mem_if, xec_mmap_t *mm, aout_header_t const *ah)
{
	int            rc;
	m88k_uintptr_t pc;
	aout_header_t  sah;

	XEC_LOG(g_ldr_log, XEC_LOG_INFO, 0, "Found 88K a.out executable signature.", 0);

	XEC_LOG(g_ldr_log, XEC_LOG_INFO, 0, "Sections = %u", 3);

	aout_swap(ah, &sah);
	aout_dump_sections(&sah);

	pc = sah.a_entry;

	rc = mapin(mem_if, mm, &sah);
	if (rc == 0)
		memcpy(&g_exec, &sah, sizeof (sah));

	return rc;
}

static int
coff_process(xec_mem_if_t *mem_if, xec_mmap_t *mm, struct coff_filehdr const *fh)
{
	struct coff_aouthdr const *ah;
	struct coff_secthdr const *sh;
	uint8_t const             *data;
	m88k_uintptr_t             pc;
	size_t                     n;
	size_t                     toff = 0;

	ah = (struct coff_aouthdr const *)(fh + 1);
	sh = (struct coff_secthdr const *)(ah + 1);

	XEC_LOG(g_ldr_log, XEC_LOG_INFO, 0, "Found 88open BCS COFF executable signature.", 0);

	XEC_LOG(g_ldr_log, XEC_LOG_INFO, 0, "Sections = %u", xec_byte_swap_big_to_host16(fh->f_nscns));
	aout_dump_sections((aout_header_t const *)ah);

	for (n = 0; n < xec_byte_swap_big_to_host16(fh->f_nscns); n++) {
		XEC_LOG(g_ldr_log, XEC_LOG_INFO, 0, "section #%u: %s", n, sh[n].s_name);
		XEC_LOG(g_ldr_log, XEC_LOG_INFO, 0, "  paddr  = %08lx", xec_byte_swap_big_to_host32(sh[n].s_paddr));
		XEC_LOG(g_ldr_log, XEC_LOG_INFO, 0, "  vaddr  = %08lx", xec_byte_swap_big_to_host32(sh[n].s_vaddr));
		XEC_LOG(g_ldr_log, XEC_LOG_INFO, 0, "  filoff = %08lx", xec_byte_swap_big_to_host32(sh[n].s_scnptr));
		if (strcmp(sh[n].s_name, ".text") == 0)
		  toff = xec_byte_swap_big_to_host32(sh[n].s_scnptr);
	}

	XEC_ASSERT(NULL, toff != 0);

	pc = xec_byte_swap_big_to_host32(ah->tstart);

	data = ((uint8_t const *)fh) + toff;

	return aout_process(mem_if, mm, (aout_header_t const *)ah);
}

void
loader_init(void)
{
	if (g_ldr_log == NULL)
		g_ldr_log = xec_log_register("loader");
}

int
loader_load(xec_mem_if_t *mem_if, char const *path)
{
	int                        rc;
	xec_mmap_t                *mm;
	struct coff_filehdr const *fh;

	mm = xec_mmap_create_with_file(path, 0, XEC_MMAP_WHOLE, XEC_MMAP_READ);
	if (mm == NULL) {
		XEC_LOG (g_ldr_log, XEC_LOG_FATAL, XEC_LOG_DONTEXIT, "Failed opening `%s' or cannot map it.", path);
		return LOADER_MAP_FAIL;
	}

	XEC_LOG(g_ldr_log, XEC_LOG_INFO, 0, "Opened file `%s'", path);
	XEC_LOG(g_ldr_log, XEC_LOG_INFO, 0, "  Base Pointer = %p", xec_mmap_get_bytes(mm));
	XEC_LOG(g_ldr_log, XEC_LOG_INFO, 0, "  Size         = %lu", (unsigned long)xec_mmap_get_size(mm));

	fh = (struct coff_filehdr const *)xec_mmap_get_bytes(mm);
	switch(xec_byte_swap_big_to_host16(fh->f_magic)) {
		case COFF_MAGIC_M88K:
			rc = coff_process(mem_if, mm, fh);
			break;

		case 0231: /* OpenBSD/m88k a.out magic */
			rc = aout_process(mem_if, mm, (aout_header_t const *)fh);
			break;

		default:
			XEC_LOG(g_ldr_log, XEC_LOG_FATAL, XEC_LOG_DONTEXIT, "File `%s' is not an 88open BCS COFF nor 88k a.out executable.", path);
			rc = LOADER_UNSUPPORTED;
	}

	xec_mmap_free(mm);

	return rc;
}
