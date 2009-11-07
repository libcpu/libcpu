/*      $OpenBSD: errno.h,v 1.19 2007/05/21 17:01:49 jasper Exp $       */
/*      $NetBSD: errno.h,v 1.10 1996/01/20 01:33:53 jtc Exp $   */

/*
 * Copyright (c) 1982, 1986, 1989, 1993
 *      The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
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
 *      @(#)errno.h     8.5 (Berkeley) 1/21/94
 */

#ifndef __obsd41_errno_h
#define __obsd41_errno_h

#define OBSD41_PERM             1               /* Operation not permitted */
#define OBSD41_NOENT            2               /* No such file or directory */
#define OBSD41_SRCH             3               /* No such process */
#define OBSD41_INTR             4               /* Interrupted system call */
#define OBSD41_IO               5               /* Input/output error */
#define OBSD41_NXIO             6               /* Device not configured */
#define OBSD41_2BIG             7               /* Argument list too long */
#define OBSD41_NOEXEC           8               /* Exec format error */
#define OBSD41_BADF             9               /* Bad file descriptor */
#define OBSD41_CHILD            10              /* No child processes */
#define OBSD41_DEADLK           11              /* Resource deadlock avoided */
                                        /* 11 was EAGAIN */
#define OBSD41_NOMEM            12              /* Cannot allocate memory */
#define OBSD41_ACCES            13              /* Permission denied */
#define OBSD41_FAULT            14              /* Bad address */
#define OBSD41_NOTBLK           15              /* Block device required */
#define OBSD41_BUSY             16              /* Device busy */
#define OBSD41_EXIST            17              /* File exists */
#define OBSD41_XDEV             18              /* Cross-device link */
#define OBSD41_NODEV            19              /* Operation not supported by device */
#define OBSD41_NOTDIR           20              /* Not a directory */
#define OBSD41_ISDIR            21              /* Is a directory */
#define OBSD41_INVAL            22              /* Invalid argument */
#define OBSD41_NFILE            23              /* Too many open files in system */
#define OBSD41_MFILE            24              /* Too many open files */
#define OBSD41_NOTTY            25              /* Inappropriate ioctl for device */
#define OBSD41_TXTBSY           26              /* Text file busy */
#define OBSD41_FBIG             27              /* File too large */
#define OBSD41_NOSPC            28              /* No space left on device */
#define OBSD41_SPIPE            29              /* Illegal seek */
#define OBSD41_ROFS             30              /* Read-only file system */
#define OBSD41_MLINK            31              /* Too many links */
#define OBSD41_PIPE             32              /* Broken pipe */

/* math software */
#define OBSD41_DOM              33              /* Numerical argument out of domain */
#define OBSD41_RANGE            34              /* Result too large */

/* non-blocking and interrupt i/o */
#define OBSD41_AGAIN            35              /* Resource temporarily unavailable */
#define OBSD41_WOULDBLOCK       EAGAIN          /* Operation would block */
#define OBSD41_INPROGRESS       36              /* Operation now in progress */
#define OBSD41_ALREADY          37              /* Operation already in progress */

/* ipc/network software -- argument errors */
#define OBSD41_NOTSOCK          38              /* Socket operation on non-socket */
#define OBSD41_DESTADDRREQ      39              /* Destination address required */
#define OBSD41_MSGSIZE          40              /* Message too long */
#define OBSD41_PROTOTYPE        41              /* Protocol wrong type for socket */
#define OBSD41_NOPROTOOPT       42              /* Protocol not available */
#define OBSD41_PROTONOSUPPORT   43              /* Protocol not supported */
#define OBSD41_SOCKTNOSUPPORT   44              /* Socket type not supported */
#define OBSD41_OPNOTSUPP        45              /* Operation not supported */
#define OBSD41_PFNOSUPPORT      46              /* Protocol family not supported */
#define OBSD41_AFNOSUPPORT      47              /* Address family not supported by protocol family */
#define OBSD41_ADDRINUSE        48              /* Address already in use */
#define OBSD41_ADDRNOTAVAIL     49              /* Can't assign requested address */

/* ipc/network software -- operational errors */
#define OBSD41_NETDOWN          50              /* Network is down */
#define OBSD41_NETUNREACH       51              /* Network is unreachable */
#define OBSD41_NETRESET         52              /* Network dropped connection on reset */
#define OBSD41_CONNABORTED      53              /* Software caused connection abort */
#define OBSD41_CONNRESET        54              /* Connection reset by peer */
#define OBSD41_NOBUFS           55              /* No buffer space available */
#define OBSD41_ISCONN           56              /* Socket is already connected */
#define OBSD41_NOTCONN          57              /* Socket is not connected */
#define OBSD41_SHUTDOWN         58              /* Can't send after socket shutdown */
#define OBSD41_TOOMANYREFS      59              /* Too many references: can't splice */
#define OBSD41_TIMEDOUT         60              /* Operation timed out */
#define OBSD41_CONNREFUSED      61              /* Connection refused */

#define OBSD41_LOOP             62              /* Too many levels of symbolic links */
#define OBSD41_NAMETOOLONG      63              /* File name too long */

/* should be rearranged */
#define OBSD41_HOSTDOWN         64              /* Host is down */
#define OBSD41_HOSTUNREACH      65              /* No route to host */
#define OBSD41_NOTEMPTY         66              /* Directory not empty */

/* quotas & mush */
#define OBSD41_PROCLIM          67              /* Too many processes */
#define OBSD41_USERS            68              /* Too many users */
#define OBSD41_DQUOT            69              /* Disk quota exceeded */

/* Network File System */
#define OBSD41_STALE            70              /* Stale NFS file handle */
#define OBSD41_REMOTE           71              /* Too many levels of remote in path */
#define OBSD41_BADRPC           72              /* RPC struct is bad */
#define OBSD41_RPCMISMATCH      73              /* RPC version wrong */
#define OBSD41_PROGUNAVAIL      74              /* RPC prog. not avail */
#define OBSD41_PROGMISMATCH     75              /* Program version wrong */
#define OBSD41_PROCUNAVAIL      76              /* Bad procedure for program */

#define OBSD41_NOLCK            77              /* No locks available */
#define OBSD41_NOSYS            78              /* Function not implemented */

#define OBSD41_FTYPE            79              /* Inappropriate file type or format */
#define OBSD41_AUTH             80              /* Authentication error */
#define OBSD41_NEEDAUTH         81              /* Need authenticator */
#define OBSD41_IPSEC            82              /* IPsec processing failure */
#define OBSD41_NOATTR           83              /* Attribute not found */
#define OBSD41_ILSEQ            84              /* Illegal byte sequence */
#define OBSD41_NOMEDIUM         85              /* No medium found */
#define OBSD41_MEDIUMTYPE       86              /* Wrong Medium Type */
#define OBSD41_OVERFLOW         87              /* Conversion overflow */
#define OBSD41_CANCELED         88              /* Operation canceled */
#define OBSD41_LAST             88              /* Must be equal largest errno */

#endif  /* !__obsd41_errno_h */
