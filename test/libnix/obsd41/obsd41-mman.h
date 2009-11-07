#ifndef __obsd41_mman_h
#define __obsd41_mman_h

/*	$OpenBSD: mman.h,v 1.18 2003/07/21 22:52:19 tedu Exp $	*/
/*	$NetBSD: mman.h,v 1.11 1995/03/26 20:24:23 jtc Exp $	*/

/*-
 * Copyright (c) 1982, 1986, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)mman.h	8.1 (Berkeley) 6/2/93
 */

/*
 * Protections are chosen from these bits, or-ed together
 */
#define	OBSD41_PROT_NONE	0x00	/* no permissions */
#define	OBSD41_PROT_READ	0x01	/* pages can be read */
#define	OBSD41_PROT_WRITE	0x02	/* pages can be written */
#define	OBSD41_PROT_EXEC	0x04	/* pages can be executed */

/*
 * Flags contain sharing type and options.
 * Sharing types; choose one.
 */
#define	OBSD41_MAP_SHARED	0x0001	/* share changes */
#define	OBSD41_MAP_PRIVATE	0x0002	/* changes are private */
#define	OBSD41_MAP_COPY		0x0004	/* "copy" region at mmap time */

/*
 * Other flags
 */
#define	OBSD41_MAP_FIXED	 0x0010	/* map addr must be exactly as requested */
#define	OBSD41_MAP_RENAME	 0x0020	/* Sun: rename private pages to file */
#define	OBSD41_MAP_NORESERVE	 0x0040	/* Sun: don't reserve needed swap area */
#define	OBSD41_MAP_INHERIT	 0x0080	/* region is retained after exec */
#define	OBSD41_MAP_NOEXTEND	 0x0100	/* for MAP_FILE, don't change file size */
#define	OBSD41_MAP_HASSEMAPHORE	 0x0200	/* region may contain semaphores */
#define	OBSD41_MAP_TRYFIXED	 0x0400 /* attempt hint address, even within heap */

/*
 * Error return from mmap()
 */
#define OBSD41_MAP_FAILED	((void *)-1)

/*
 * Mapping type
 */
#define	OBSD41_MAP_FILE		0x0000	/* map from file (default) */
#define	OBSD41_MAP_ANON		0x1000	/* allocated from memory, swap space */
#define	OBSD41_MAP_FLAGMASK	0x17f7

/*
 * Advice to madvise
 */
#define	OBSD41_MADV_NORMAL	0	/* no further special treatment */
#define	OBSD41_MADV_RANDOM	1	/* expect random page references */
#define	OBSD41_MADV_SEQUENTIAL	2	/* expect sequential page references */
#define	OBSD41_MADV_WILLNEED	3	/* will need these pages */
#define	OBSD41_MADV_DONTNEED	4	/* dont need these pages */
#define	OBSD41_MADV_SPACEAVAIL	5	/* insure that resources are reserved */
#define	OBSD41_MADV_FREE	6	/* pages are empty, free them */

/*
 * Flags to minherit
 */
#define OBSD41_MAP_INHERIT_SHARE	0	/* share with child */
#define OBSD41_MAP_INHERIT_COPY		1	/* copy into child */
#define OBSD41_MAP_INHERIT_NONE		2	/* absent from child */
#define OBSD41_MAP_INHERIT_DONATE_COPY	3	/* copy and delete -- not
					   implemented in UVM */

/*
 * Flags to msync
 */
#define	OBSD41_MS_ASYNC		0x01	/* perform asynchronous writes */
#define	OBSD41_MS_SYNC		0x02	/* perform synchronous writes */
#define	OBSD41_MS_INVALIDATE	0x04	/* invalidate cached data */

/*
 * Flags to mlockall
 */
#define	OBSD41_MCL_CURRENT	0x01	/* lock all pages currently mapped */
#define	OBSD41_MCL_FUTURE	0x02	/* lock all pages mapped in the future */


#endif /* !__obsd41-mman_h */
