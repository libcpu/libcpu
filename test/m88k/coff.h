#ifndef __m88k_coff_h
#define __m88k_coff_h

#include "xec-base.h"

struct coff_filehdr {
	uint16_t f_magic;
	uint16_t f_nscns;
	int32_t  f_timdat;
	int32_t  f_symptr;
	int32_t  f_nsyms;
	uint16_t f_opthdr;
	uint16_t f_flags;
};

struct coff_aouthdr {
	/* aouthdr */
	int16_t magic;
	int16_t vstamp;
	int32_t tsize;
	int32_t dsize;
	int32_t bsize;
	int32_t entry;
	int32_t tstart;
	int32_t dstart;
};

struct coff_secthdr {
	char     s_name[8];
	int32_t  s_paddr;
	int32_t  s_vaddr;
	int32_t  s_size;
	int32_t  s_scnptr;
	int32_t  s_relptr;
	int32_t  s_lnnoptr;
	uint32_t s_nreloc; /* XXX M88K wants uint32_t, usually uint16_t */
	uint32_t s_nlnno;  /* XXX M88K wants uint32_t, usually uint16_t */
	int32_t  s_flags;
};

/*
 * On M88K BCS COFF there's a padding 32bit word after the
 * section headers.
 */

#define COFF_MAGIC_M88K 0x16d

#endif /* !__m88k_coff_h */
