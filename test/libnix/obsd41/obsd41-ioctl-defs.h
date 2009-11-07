/* NIX: $Id: obsd41-ioctl-defs.h 286 2007-06-26 13:26:56Z orlando $ */
/*      $OpenBSD: ioccom.h,v 1.4 2006/05/18 21:27:25 miod Exp $ */
/*      $NetBSD: ioccom.h,v 1.4 1994/10/30 21:49:56 cgd Exp $   */

/*-
 * Copyright (c) 1982, 1986, 1990, 1993, 1994
 *      The Regents of the University of California.  All rights reserved.
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
 *      @(#)ioccom.h    8.2 (Berkeley) 3/28/94
 */

#ifndef __obsd41_ioctl_common_h
#define __obsd41_ioctl_common_h

#include "obsd41-guest-types.h"

/*
 * Ioctl's have the command encoded in the lower word, and the size of
 * any in or out parameters in the upper word.  The high 3 bits of the
 * upper word are used to encode the in/out status of the parameter.
 */
#define OBSD41_IOCPARM_MASK    0x1fff          /* parameter length, at most 13 bits */
#define OBSD41_IOCPARM_LEN(x)  (((x) >> 16) & OBSD41_IOCPARM_MASK)
#define OBSD41_IOCBASECMD(x)   ((x) & ~(OBSD41_IOCPARM_MASK << 16))
#define OBSD41_IOCGROUP(x)     (((x) >> 8) & 0xff)

#define OBSD41_IOC_VOID        (obsd41_ulong_t)0x20000000
#define OBSD41_IOC_OUT         (obsd41_ulong_t)0x40000000     /* copy parameters out */
#define OBSD41_IOC_IN          (obsd41_ulong_t)0x80000000     /* copy parameters in */
#define OBSD41_IOC_INOUT       (OBSD41_IOC_IN|OBSD41_IOC_OUT) /* copy parameters in and out */
#define OBSD41_IOC_DIRMASK     (obsd41_ulong_t)0xe0000000     /* mask for IN/OUT/VOID */

#define _OBSD41_IOC(inout, group, num, len) \
        (inout | ((len & OBSD41_IOCPARM_MASK) << 16) | ((group) << 8) | (num))
#define _OBSD41_IO(g, n)       _OBSD41_IOC (OBSD41_IOC_VOID,  (g), (n), 0)
#define _OBSD41_IOR(g, n, t)   _OBSD41_IOC (OBSD41_IOC_OUT,   (g), (n), sizeof (t))
#define _OBSD41_IOW(g, n, t)   _OBSD41_IOC (OBSD41_IOC_IN,    (g), (n), sizeof (t))
/* this should be _IORW, but stdio got there first */
#define _OBSD41_IOWR(g, n, t)  _OBSD41_IOC (OBSD41_IOC_INOUT, (g), (n), sizeof (t))

#endif  /* !__obsd41_ioctl_common_h */
