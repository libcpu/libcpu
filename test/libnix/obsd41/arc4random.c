/*
 * Arc4 random number generator for OpenBSD.
 * Copyright 1996 David Mazieres <dm@lcs.mit.edu>.
 *
 * Modification and redistribution in source and binary forms is
 * permitted provided that due credit is given to the author and the
 * OpenBSD project by leaving this copyright notice intact.
 */

/*
 * This code is derived from section 17.1 of Applied Cryptography,
 * second edition, which describes a stream cipher allegedly
 * compatible with RSA Labs "RC4" cipher (the actual description of
 * which is a trade secret).  The same algorithm is used as a stream
 * cipher called "arcfour" in Tatu Ylonen's ssh package.
 *
 * Here the stream cipher has been modified always to include the time
 * when initializing the state.  That makes it impossible to
 * regenerate the same random sequence twice, so this can't be used
 * for encryption, but will generate good random numbers.
 *
 * RC4 is a registered trademark of RSA Laboratories.
 */

/*
 * Modified by Robert Connolly from OpenBSD lib/libc/crypt/arc4random.c v1.11.
 * This is arc4random(3) using urandom.
 */

#include "nix-obsd41-config.h"

#include <sys/param.h>
#include <sys/types.h>
#include <sys/time.h>
#if HAVE_SYS_SYSCTL_H
#include <sys/sysctl.h>
#endif
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#include "arc4random.h"
#include "xec-debug.h"

extern void *g_bsd_log;

struct arc4_stream {
	uint8_t i;
	uint8_t j;
	uint8_t s[256];
};

static int rs_initialized;
static struct arc4_stream rs;
static pid_t arc4_stir_pid;

static __inline uint8_t arc4_getbyte(struct arc4_stream *);

static __inline void
arc4_init(struct arc4_stream *as)
{
	int n;

	for (n = 0; n < 256; n++)
	  as->s[n] = n;
	as->i = 0;
	as->j = 0;
}

static void
arc4_addrandom(struct arc4_stream *as, uint8_t const *dat, size_t datlen)
{
	size_t n;
	uint8_t si;

	as->i--;
	for (n = 0; n < 256; n++) {
		as->i = (as->i + 1);
		si = as->s[as->i];
		as->j = (as->j + si + dat[n % datlen]);
		as->s[as->i] = as->s[as->j];
		as->s[as->j] = si;
	}
	as->j = as->i;
}

static void
arc4_stir(struct arc4_stream *as)
{
	int n, fd;
	struct {
		struct timeval tv;
		uint32_t rnd[(128 - sizeof(struct timeval)) / sizeof(uint32_t)];
	} rdat;

	gettimeofday(&rdat.tv, NULL);

	/* /dev/urandom is a multithread interface, sysctl is not. */
	/* Try to use /dev/urandom before sysctl. */
	fd = open("/dev/urandom", O_RDONLY);
	if (fd != -1) {
		read(fd, rdat.rnd, sizeof(rdat.rnd));
		close(fd);
	} else {
		/* /dev/urandom failed? Maybe we're in a chroot. */
#ifdef HAVE_LINUX_SYSCTL_H
		/* XXX this is for Linux, which uses enums */
		int mib[3];
		size_t i, len;

		mib[0] = CTL_KERN;
		mib[1] = KERN_RANDOM;
		mib[2] = RANDOM_UUID;

		for (i = 0; i < sizeof (rdat.rnd) / sizeof (uint32_t); i ++) {
			len = sizeof (uint32_t);
			if (sysctl (mib, 3, &rdat.rnd[i], &len, NULL, 0) < 0) {
				fprintf (stderr, "warning: no entropy source\n");
				break;
			}
		}
#else
		/* */
#endif
	}

	arc4_stir_pid = getpid();
	/*
	 * Time to give up. If no entropy could be found then we will just
	 * use gettimeofday.
	 */
	arc4_addrandom(as, (void *)&rdat, sizeof(rdat));

	/*
	 * Discard early keystream, as per recommendations in:
	 * http://www.wisdom.weizmann.ac.il/~itsik/RC4/Papers/Rc4_ksa.ps
	 * We discard 256 words. A long word is 4 bytes.
	 */
	for (n = 0; n < 256 * 4; n ++)
		arc4_getbyte(as);
}

static __inline uint8_t
arc4_getbyte(struct arc4_stream *as)
{
	uint8_t si, sj;

	as->i = (as->i + 1);
	si = as->s[as->i];
	as->j = (as->j + si);
	sj = as->s[as->j];
	as->s[as->i] = sj;
	as->s[as->j] = si;
	return (as->s[(si + sj) & 0xff]);
}

static __inline uint32_t
arc4_getword(struct arc4_stream *as)
{
	uint32_t val;
	val = arc4_getbyte(as) << 24;
	val |= arc4_getbyte(as) << 16;
	val |= arc4_getbyte(as) << 8;
	val |= arc4_getbyte(as);
	return (val);
}

void bsd_arc4random_stir(void)
{
	if (!rs_initialized) {
		arc4_init(&rs);
		rs_initialized = 1;
	}
	arc4_stir(&rs);
}

void bsd_arc4random_addrandom (uint8_t const *dat, size_t datlen)
{
	if (!rs_initialized)
		bsd_arc4random_stir();

	arc4_addrandom(&rs, dat, datlen);
}

uint32_t bsd_arc4random(void)
{
	if (!rs_initialized || arc4_stir_pid != getpid())
		bsd_arc4random_stir();

	return (arc4_getword(&rs));
}

/* Utility */
void bsd_arc4random_bytes(uint8_t *out, size_t len)
{
	size_t n;

	if (!rs_initialized)
	  bsd_arc4random_stir();

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked: output=%p length=%zu", out, len);

	for (n = 0; n < XEC_MIN(len, 256); n++)
	  out[n] = arc4_getbyte(&rs);
}
