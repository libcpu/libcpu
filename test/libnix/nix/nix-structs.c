#include "nix.h"
#include "nix-structs.h"

#include <termios.h>
#include <string.h>

void
timeval_to_nix_timeval(struct timeval const *in,
					   struct nix_timeval   *out)
{
	out->tv_sec  = in->tv_sec;
	out->tv_usec = in->tv_usec;
}

void
nix_timeval_to_timeval(struct nix_timeval const *in,
					   struct timeval           *out)
{
	out->tv_sec  = in->tv_sec;
	out->tv_usec = in->tv_usec;
}

void
timespec_to_nix_timespec(struct timespec const *in,
						 struct nix_timespec   *out)
{
	out->tv_sec  = in->tv_sec;
	out->tv_nsec = in->tv_nsec;
}

void
nix_timespec_to_timespec(struct nix_timespec const *in,
						 struct timespec           *out)
{
	out->tv_sec  = in->tv_sec;
	out->tv_nsec = in->tv_nsec;
}

void
time_to_nix_timespec(time_t               secs,
					 uint32_t             nsecs,
					 struct nix_timespec *out)
{
	out->tv_sec  = secs;
	out->tv_nsec = nsecs;
}

void
timezone_to_nix_timezone(struct timezone const *in,
						 struct nix_timezone   *out)
{
	out->tz_minuteswest = in->tz_minuteswest;
	out->tz_dsttime     = in->tz_dsttime;
}

void
nix_timezone_to_timezone(struct nix_timezone const *in,
						 struct timezone           *out)
{
	out->tz_minuteswest = in->tz_minuteswest;
	out->tz_dsttime     = in->tz_dsttime;
}

void
stat_to_nix_stat(struct stat const *in,
				 struct nix_stat   *out)
{
	out->st_dev     = in->st_dev;
	out->st_ino     = in->st_ino;
	out->st_mode    = in->st_mode;
	out->st_nlink   = in->st_nlink;
	out->st_uid     = in->st_uid;
	out->st_gid     = in->st_gid;
	out->st_rdev    = in->st_rdev;

#ifndef __linux__
	timespec_to_nix_timespec(&in->st_atimespec, &out->st_atimespec);
	timespec_to_nix_timespec(&in->st_mtimespec, &out->st_mtimespec);
	timespec_to_nix_timespec(&in->st_ctimespec, &out->st_ctimespec);
#if defined (__FreeBSD__) || defined (__NetBSD__) || defined (__OpenBSD__)
	timespec_to_nix_timespec(&in->__st_birthtimespec, &out->st_btimespec);
#else
	memset(&out->st_btimespec, 0, sizeof (out->st_btimespec));
#endif
#else
	time_to_nix_timespec(in->st_atime, 0, &out->st_atimespec);
	time_to_nix_timespec(in->st_mtime, 0, &out->st_mtimespec);
	time_to_nix_timespec(in->st_ctime, 0, &out->st_btimespec);
	memset(&out->st_btimespec, 0, sizeof (out->st_btimespec));
#endif

	out->st_size    = in->st_size;
	out->st_blocks  = in->st_blocks;
	out->st_blksize = in->st_blksize;
#ifndef __linux__
	out->st_gen     = in->st_gen;
#endif
}

void
rusage_to_nix_rusage(struct rusage const *in,
					 struct nix_rusage   *out)
{
	timeval_to_nix_timeval(&in->ru_utime, &out->ru_utime);
	timeval_to_nix_timeval(&in->ru_stime, &out->ru_stime);
	out->ru_maxrss   = in->ru_maxrss;
	out->ru_ixrss    = in->ru_ixrss;
	out->ru_idrss    = in->ru_idrss;
	out->ru_isrss    = in->ru_isrss;
	out->ru_minflt   = in->ru_minflt;
	out->ru_majflt   = in->ru_majflt;
	out->ru_nswap    = in->ru_nswap;
	out->ru_inblock  = in->ru_inblock;
	out->ru_oublock  = in->ru_oublock;
	out->ru_msgsnd   = in->ru_msgsnd;
	out->ru_msgrcv   = in->ru_msgrcv;
	out->ru_nsignals = in->ru_nsignals;
	out->ru_nvcsw    = in->ru_nvcsw;
	out->ru_nivcsw   = in->ru_nivcsw;
}

void
rlimit_to_nix_rlimit(struct rlimit const *in,
					 struct nix_rlimit   *out)
{
	out->rlim_cur = in->rlim_cur;
	out->rlim_max = in->rlim_max;
}

static __inline nix_tcflag_t
termios_iflag_to_nix_termios_iflag(tcflag_t flags)
{
	nix_tcflag_t oflags = 0;

	if (flags == 0)
		return (0);

#ifdef IGNBRK
	if (flags & IGNBRK) oflags |= NIX_IGNBRK;
#endif
#ifdef BRKINT
	if (flags & BRKINT) oflags |= NIX_BRKINT;
#endif
#ifdef PARMRK
	if (flags & PARMRK) oflags |= NIX_PARMRK;
#endif
#ifdef INPCK
	if (flags & INPCK) oflags |= NIX_INPCK;
#endif
#ifdef ISTRIP
	if (flags & ISTRIP) oflags |= NIX_ISTRIP;
#endif
#ifdef INLCR
	if (flags & INLCR) oflags |= NIX_INLCR;
#endif
#ifdef IGNCR
	if (flags & IGNCR) oflags |= NIX_IGNCR;
#endif
#ifdef ICRNL
	if (flags & ICRNL) oflags |= NIX_ICRNL;
#endif
#ifdef IXON
	if (flags & IXON) oflags |= NIX_IXON;
#endif
#ifdef IXOFF
	if (flags & IXOFF) oflags |= NIX_IXOFF;
#endif
#ifdef IXANY
	if (flags & IXANY) oflags |= NIX_IXANY;
#endif
#ifdef IUCLC
	if (flags & IUCLC) oflags |= NIX_IUCLC;
#endif
#ifdef IMAXBEL
	if (flags & IMAXBEL) oflags |= NIX_IMAXBEL;
#endif

	return (oflags);
}

static __inline tcflag_t
nix_termios_iflag_to_termios_iflag(nix_tcflag_t flags)
{
	tcflag_t oflags = 0;

	if (flags == 0)
		return (0);

#ifdef IGNBRK
	if (flags & NIX_IGNBRK) oflags |= IGNBRK;
#endif
#ifdef BRKINT
	if (flags & NIX_BRKINT) oflags |= BRKINT;
#endif
#ifdef PARMRK
	if (flags & NIX_PARMRK) oflags |= PARMRK;
#endif
#ifdef INPCK
	if (flags & NIX_INPCK) oflags |= INPCK;
#endif
#ifdef ISTRIP
	if (flags & NIX_ISTRIP) oflags |= ISTRIP;
#endif
#ifdef INLCR
	if (flags & NIX_INLCR) oflags |= INLCR;
#endif
#ifdef IGNCR
	if (flags & NIX_IGNCR) oflags |= IGNCR;
#endif
#ifdef ICRNL
	if (flags & NIX_ICRNL) oflags |= ICRNL;
#endif
#ifdef IXON
	if (flags & NIX_IXON) oflags |= IXON;
#endif
#ifdef IXOFF
	if (flags & NIX_IXOFF) oflags |= IXOFF;
#endif
#ifdef IXANY
	if (flags & NIX_IXANY) oflags |= IXANY;
#endif
#ifdef IUCLC
	if (flags & NIX_IUCLC) oflags |= IUCLC;
#endif
#ifdef IMAXBEL
	if (flags & NIX_IMAXBEL) oflags |= IMAXBEL;
#endif

	return (oflags);
}

static __inline nix_tcflag_t
termios_oflag_to_nix_termios_oflag(tcflag_t flags)
{
	nix_tcflag_t oflags = 0;

	if (flags == 0)
		return (0);

#ifdef OPOST
	if (flags & OPOST) oflags |= NIX_OPOST;
#endif
#ifdef ONLCR
	if (flags & ONLCR) oflags |= NIX_ONLCR;
#endif
#ifdef OXTABS
	if (flags & OXTABS) oflags |= NIX_OXTABS;
#endif
#ifdef ONOEOT
	if (flags & ONOEOT) oflags |= NIX_ONOEOT;
#endif
#ifdef OCRNL
	if (flags & OCRNL) oflags |= NIX_OCRNL;
#endif
#ifdef OLCUC
	if (flags & OLCUC) oflags |= NIX_OLCUC;
#endif
#ifdef ONOCR
	if (flags & ONOCR) oflags |= NIX_ONOCR;
#endif
#ifdef ONLRET
	if (flags & ONLRET) oflags |= NIX_ONLRET;
#endif

	return (oflags);
}

static __inline tcflag_t
nix_termios_oflag_to_termios_oflag(nix_tcflag_t flags)
{
	tcflag_t oflags = 0;

	if (flags == 0)
		return 0;

#ifdef OPOST
	if (flags & NIX_OPOST) oflags |= OPOST;
#endif
#ifdef ONLCR
	if (flags & NIX_ONLCR) oflags |= ONLCR;
#endif
#ifdef OXTABS
	if (flags & NIX_OXTABS) oflags |= OXTABS;
#endif
#ifdef ONOEOT
	if (flags & NIX_ONOEOT) oflags |= ONOEOT;
#endif
#ifdef OCRNL
	if (flags & NIX_OCRNL) oflags |= OCRNL;
#endif
#ifdef OLCUC
	if (flags & NIX_OLCUC) oflags |= OLCUC;
#endif
#ifdef ONOCR
	if (flags & NIX_ONOCR) oflags |= ONOCR;
#endif
#ifdef ONLRET
	if (flags & NIX_ONLRET) oflags |= ONLRET;
#endif

	return (oflags);
}

static __inline nix_tcflag_t
termios_cflag_to_nix_termios_cflag(tcflag_t flags)
{
	nix_tcflag_t oflags = 0;

	if (flags == 0)
	  return (0);

#ifdef CIGNORE
	if (flags & CIGNORE) oflags |= NIX_CIGNORE;
#endif
#ifdef CSIZE
	switch (flags & CSIZE) {
#ifdef CS5
		case CS5: oflags |= NIX_CS5; break;
#endif
#ifdef CS6
		case CS6: oflags |= NIX_CS6; break;
#endif
#ifdef CS7
		case CS7: oflags |= NIX_CS7; break;
#endif
#ifdef CS8
		case CS8: oflags |= NIX_CS8; break;
#endif
	}
#endif
#ifdef CSTOPB
	if (flags & CSTOPB) oflags |= NIX_CSTOPB;
#endif
#ifdef CREAD
	if (flags & CREAD) oflags |= NIX_CREAD;
#endif
#ifdef PARENB
	if (flags & PARENB) oflags |= NIX_PARENB;
#endif
#ifdef PARODD
	if (flags & PARODD) oflags |= NIX_PARODD;
#endif
#ifdef HUPCL
	if (flags & HUPCL) oflags |= NIX_HUPCL;
#endif
#ifdef CLOCAL
	if (flags & CLOCAL) oflags |= NIX_CLOCAL;
#endif
#ifdef CRTSCTS
	if (flags & CRTSCTS) oflags |= NIX_CRTSCTS;
#endif
#ifdef MDMBUF
	if (flags & MDMBUF) oflags |= NIX_MDMBUF;
#endif

	return (oflags);
}

static __inline tcflag_t
nix_termios_cflag_to_termios_cflag(nix_tcflag_t flags)
{
	tcflag_t oflags = 0;

	if (flags == 0)
		return (0);

#ifdef CIGNORE
	if (flags & NIX_CIGNORE) oflags |= CIGNORE;
#endif
#ifdef CSIZE
	switch (flags & NIX_CSIZE) {
#ifdef CS5
		case NIX_CS5: oflags |= CS5; break;
#endif
#ifdef CS6
		case NIX_CS6: oflags |= CS6; break;
#endif
#ifdef CS7
		case NIX_CS7: oflags |= CS7; break;
#endif
#ifdef CS8
		case NIX_CS8: oflags |= CS8; break;
#endif
	}
#endif
#ifdef CSTOPB
	if (flags & NIX_CSTOPB) oflags |= CSTOPB;
#endif
#ifdef CREAD
	if (flags & NIX_CREAD) oflags |= CREAD;
#endif
#ifdef PARENB
	if (flags & NIX_PARENB) oflags |= PARENB;
#endif
#ifdef PARODD
	if (flags & NIX_PARODD) oflags |= PARODD;
#endif
#ifdef HUPCL
	if (flags & NIX_HUPCL) oflags |= HUPCL;
#endif
#ifdef CLOCAL
	if (flags & NIX_CLOCAL) oflags |= CLOCAL;
#endif
#ifdef CRTSCTS
	if (flags & NIX_CRTSCTS) oflags |= CRTSCTS;
#endif
#ifdef MDMBUF
	if (flags & NIX_MDMBUF) oflags |= MDMBUF;
#endif

	return (oflags);
}

static __inline nix_tcflag_t
termios_lflag_to_nix_termios_lflag(tcflag_t flags)
{
	nix_tcflag_t oflags = 0;

	if (flags == 0)
		return (0);

#ifdef ECHOKE
	if (flags & ECHOKE) oflags |= NIX_ECHOKE;
#endif
#ifdef ECHOE
	if (flags & ECHOE) oflags |= NIX_ECHOE;
#endif
#ifdef ECHOK
	if (flags & ECHOK) oflags |= NIX_ECHOK;
#endif
#ifdef ECHO
	if (flags & ECHO) oflags |= NIX_ECHO;
#endif
#ifdef ECHONL
	if (flags & ECHONL) oflags |= NIX_ECHONL;
#endif
#ifdef ECHOPRT
	if (flags & ECHOPRT) oflags |= NIX_ECHOPRT;
#endif
#ifdef ECHOCTL
	if (flags & ECHOCTL) oflags |= NIX_ECHOCTL;
#endif
#ifdef ISIG
	if (flags & ISIG) oflags |= NIX_ISIG;
#endif
#ifdef ICANON
	if (flags & ICANON) oflags |= NIX_ICANON;
#endif
#ifdef ALTWERASE
	if (flags & ALTWERASE) oflags |= NIX_ALTWERASE;
#endif
#ifdef IEXTEN
	if (flags & IEXTEN) oflags |= NIX_IEXTEN;
#endif
#ifdef EXTPROC
	if (flags & EXTPROC) oflags |= NIX_EXTPROC;
#endif
#ifdef TOSTOP
	if (flags & TOSTOP) oflags |= NIX_TOSTOP;
#endif
#ifdef FLUSHO
	if (flags & FLUSHO) oflags |= NIX_FLUSHO;
#endif
#ifdef XCASE
	if (flags & XCASE) oflags |= NIX_XCASE;
#endif
#ifdef NOKERNINFO
	if (flags & NOKERNINFO) oflags |= NIX_NOKERNINFO;
#endif
#ifdef PENDIN
	if (flags & PENDIN) oflags |= NIX_PENDIN;
#endif
#ifdef NOFLSH
	if (flags & NOFLSH) oflags |= NIX_NOFLSH;
#endif

	return (oflags);
}

static __inline tcflag_t
nix_termios_lflag_to_termios_lflag(nix_tcflag_t flags)
{
	tcflag_t oflags = 0;

	if (flags == 0)
		return (0);

#ifdef ECHOKE
	if (flags & NIX_ECHOKE) oflags |= ECHOKE;
#endif
#ifdef ECHOE
	if (flags & NIX_ECHOE) oflags |= ECHOE;
#endif
#ifdef ECHOK
	if (flags & NIX_ECHOK) oflags |= ECHOK;
#endif
#ifdef ECHO
	if (flags & NIX_ECHO) oflags |= ECHO;
#endif
#ifdef ECHONL
	if (flags & NIX_ECHONL) oflags |= ECHONL;
#endif
#ifdef ECHOPRT
	if (flags & NIX_ECHOPRT) oflags |= ECHOPRT;
#endif
#ifdef ECHOCTL
	if (flags & NIX_ECHOCTL) oflags |= ECHOCTL;
#endif
#ifdef ISIG
	if (flags & NIX_ISIG) oflags |= ISIG;
#endif
#ifdef ICANON
	if (flags & NIX_ICANON) oflags |= ICANON;
#endif
#ifdef ALTWERASE
	if (flags & NIX_ALTWERASE) oflags |= ALTWERASE;
#endif
#ifdef IEXTEN
	if (flags & NIX_IEXTEN) oflags |= IEXTEN;
#endif
#ifdef EXTPROC
	if (flags & NIX_EXTPROC) oflags |= EXTPROC;
#endif
#ifdef TOSTOP
	if (flags & NIX_TOSTOP) oflags |= TOSTOP;
#endif
#ifdef FLUSHO
	if (flags & NIX_FLUSHO) oflags |= FLUSHO;
#endif
#ifdef XCASE
	if (flags & NIX_XCASE) oflags |= XCASE;
#endif
#ifdef NOKERNINFO
	if (flags & NIX_NOKERNINFO) oflags |= NOKERNINFO;
#endif
#ifdef PENDIN
	if (flags & NIX_PENDIN) oflags |= PENDIN;
#endif
#ifdef NOFLSH
	if (flags & NIX_NOFLSH) oflags |= NOFLSH;
#endif

	return (oflags);
}

static __inline void
termios_cc_to_nix_termios_cc(cc_t const *in,
							 nix_cc_t   *out)
{
#ifdef VEOF
	out[NIX_VEOF]     = in[VEOF];
#endif
#ifdef VEOL
	out[NIX_VEOL]     = in[VEOL];
#endif
#ifdef VEOL2
	out[NIX_VEOL2]    = in[VEOL2];
#endif
#ifdef VERASE
	out[NIX_VERASE]   = in[VERASE];
#endif
#ifdef VWERASE
	out[NIX_VWERASE]  = in[VWERASE];
#endif
#ifdef VKILL
	out[NIX_VKILL]    = in[VKILL];
#endif
#ifdef VREPRINT
	out[NIX_VREPRINT] = in[VREPRINT];
#endif
#ifdef VINTR
	out[NIX_VINTR]    = in[VINTR];
#endif
#ifdef VQUIT
	out[NIX_VQUIT]    = in[VQUIT];
#endif
#ifdef VSUSP
	out[NIX_VSUSP]    = in[VSUSP];
#endif
#ifdef VDSUSP
	out[NIX_VDSUSP]   = in[VDSUSP];
#endif
#ifdef VSTART
	out[NIX_VSTART]   = in[VSTART];
#endif
#ifdef VSTOP
	out[NIX_VSTOP]    = in[VSTOP];
#endif
#ifdef VLNEXT
	out[NIX_VLNEXT]   = in[VLNEXT];
#endif
#ifdef VDISCARD
	out[NIX_VDISCARD] = in[VDISCARD];
#endif
#ifdef VMIN
	out[NIX_VMIN]     = in[VMIN];
#endif
#ifdef VTIME
	out[NIX_VTIME]    = in[VTIME];
#endif
#ifdef VSTATUS
	out[NIX_VSTATUS]  = in[VSTATUS];
#endif
}

static __inline void
nix_termios_cc_to_termios_cc(cc_t const *in,
							 nix_cc_t   *out)
{
#ifdef VEOF
	out[VEOF]     = in[NIX_VEOF];
#endif
#ifdef VEOL
	out[VEOL]     = in[NIX_VEOL];
#endif
#ifdef VEOL2
	out[VEOL2]    = in[NIX_VEOL2];
#endif
#ifdef VERASE
	out[VERASE]   = in[NIX_VERASE];
#endif
#ifdef VWERASE
	out[VWERASE]  = in[NIX_VWERASE];
#endif
#ifdef VKILL
	out[VKILL]    = in[NIX_VKILL];
#endif
#ifdef VREPRINT
	out[VREPRINT] = in[NIX_VREPRINT];
#endif
#ifdef VINTR
	out[VINTR]    = in[NIX_VINTR];
#endif
#ifdef VQUIT
	out[VQUIT]    = in[NIX_VQUIT];
#endif
#ifdef VSUSP
	out[VSUSP]    = in[NIX_VSUSP];
#endif
#ifdef VDSUSP
	out[VDSUSP]   = in[NIX_VDSUSP];
#endif
#ifdef VSTART
	out[VSTART]   = in[NIX_VSTART];
#endif
#ifdef VSTOP
	out[VSTOP]    = in[NIX_VSTOP];
#endif
#ifdef VLNEXT
	out[VLNEXT]   = in[NIX_VLNEXT];
#endif
#ifdef VDISCARD
	out[VDISCARD] = in[NIX_VDISCARD];
#endif
#ifdef VMIN
	out[VMIN]     = in[NIX_VMIN];
#endif
#ifdef VTIME
	out[VTIME]    = in[NIX_VTIME];
#endif
#ifdef VSTATUS
	out[VSTATUS]  = in[NIX_VSTATUS];
#endif
}

static __inline nix_speed_t
termios_speed_to_nix_termios_speed (speed_t speed)
{
	if (speed == 0)
		return (0);

	switch (speed) {
#ifdef B0
		case B0: return (0);
#endif
#ifdef B50
		case B50: return (50);
#endif
#ifdef B75
		case B75: return (75);
#endif
#ifdef B110
		case B110: return (110);
#endif
#ifdef B134
		case B134: return (134);
#endif
#ifdef B150
		case B150: return (150);
#endif
#ifdef B200
		case B200: return (200);
#endif
#ifdef B300
		case B300: return (300);
#endif
#ifdef B600
		case B600: return (600);
#endif
#ifdef B1200
		case B1200: return (1200);
#endif
#ifdef B1800
		case B1800: return (1800);
#endif
#ifdef B2400
		case B2400: return (2400);
#endif
#ifdef B4800
		case B4800: return (4800);
#endif
#ifdef B7200
		case B7200: return (7200);
#endif
#ifdef B9600
		case B9600: return (9600);
#endif
#ifdef B14400
		case B14400: return (14400);
#endif
#ifdef B19200
		case B19200: return (19200);
#endif
#ifdef B28800
		case B28800: return (28800);
#endif
#ifdef B38400
		case B38400: return (38400);
#endif
#ifdef B57600
		case B57600: return (57600);
#endif
#ifdef B76800
		case B76800: return (76800);
#endif
#ifdef B115200
		case B115200: return (115200);
#endif
#ifdef B230400
		case B230400: return (230400);
#endif
	}

	return (0);
}

static __inline nix_speed_t
nix_termios_speed_to_termios_speed(speed_t speed)
{
	if (speed == 0)
		return ((0));

	switch (speed) {
#ifdef B0
		case 0: return (B0);
#endif
#ifdef B50
		case 50: return (B50);
#endif
#ifdef B75
		case 75: return (B75);
#endif
#ifdef B110
		case 110: return (B110);
#endif
#ifdef B134
		case 134: return (B134);
#endif
#ifdef B150
		case 150: return (B150);
#endif
#ifdef B200
		case 200: return (B200);
#endif
#ifdef B300
		case 300: return (B300);
#endif
#ifdef B600
		case 600: return (B600);
#endif
#ifdef B1200
		case 1200: return (B1200);
#endif
#ifdef B1800
		case 1800: return (B1800);
#endif
#ifdef B2400
		case 2400: return (B2400);
#endif
#ifdef B4800
		case 4800: return (B4800);
#endif
#ifdef B7200
		case 7200: return (B7200);
#endif
#ifdef B9600
		case 9600: return (B9600);
#endif
#ifdef B14400
		case 14400: return (B14400);
#endif
#ifdef B19200
		case 19200: return (B19200);
#endif
#ifdef B28800
		case 28800: return (B28800);
#endif
#ifdef B38400
		case 38400: return (B38400);
#endif
#ifdef B57600
		case 57600: return (B57600);
#endif
#ifdef B76800
		case 76800: return (B76800);
#endif
#ifdef B115200
		case 115200: return (B115200);
#endif
#ifdef B230400
		case 230400: return (B230400);
#endif
	}

	return (0);
}

void
termios_to_nix_termios(struct termios const *in,
					   struct nix_termios   *out)
{
	out->c_iflag  = termios_iflag_to_nix_termios_iflag(in->c_iflag);
	out->c_oflag  = termios_oflag_to_nix_termios_oflag(in->c_oflag);
	out->c_cflag  = termios_cflag_to_nix_termios_cflag(in->c_cflag);
	out->c_lflag  = termios_lflag_to_nix_termios_lflag(in->c_lflag);
	out->c_ispeed = termios_speed_to_nix_termios_speed(in->c_ispeed);
	out->c_ospeed = termios_speed_to_nix_termios_speed(in->c_ospeed);
	termios_cc_to_nix_termios_cc(in->c_cc, out->c_cc);
}

void
nix_termios_to_termios(struct nix_termios const *in,
					   struct termios           *out)
{
	out->c_iflag  = nix_termios_iflag_to_termios_iflag(in->c_iflag);
	out->c_oflag  = nix_termios_oflag_to_termios_oflag(in->c_oflag);
	out->c_cflag  = nix_termios_cflag_to_termios_cflag(in->c_cflag);
	out->c_lflag  = nix_termios_lflag_to_termios_lflag(in->c_lflag);
	out->c_ispeed = nix_termios_speed_to_termios_speed(in->c_ispeed);
	out->c_ospeed = nix_termios_speed_to_termios_speed(in->c_ospeed);
	nix_termios_cc_to_termios_cc(in->c_cc, out->c_cc);
}
