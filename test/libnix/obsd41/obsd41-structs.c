#include "openbsd41.h"
#include "xec-debug.h"
#include "xec-byte-order.h"

#include <string.h>

extern void *g_bsd_log;

#define GE16(x) (((endian) != XEC_ENDIAN_NATIVE) ? xec_byte_swap_int16(x) : (uint16_t)(x))
#define GE32(x) (((endian) != XEC_ENDIAN_NATIVE) ? xec_byte_swap_int32(x) : (uint32_t)(x))
#define GE64(x) (((endian) != XEC_ENDIAN_NATIVE) ? xec_byte_swap_int64(x) : (uint64_t)(x))

#define GELONG(x) ((sizeof(obsd41_long_t) == sizeof(uint64_t)) ?	\
				   (obsd41_long_t)GE64(x) : (obsd41_long_t)GE32(x))

void
nix_timezone_to_obsd41_timezone(xec_endian_t               endian,
								struct nix_timezone const *in,
								struct obsd41_timezone    *out)
{
	out->tz_minuteswest = GE32(in->tz_minuteswest);
	out->tz_dsttime     = GE32(in->tz_dsttime);
}

void
obsd41_timezone_to_nix_timezone(xec_endian_t                  endian,
								struct obsd41_timezone const *in,
								struct nix_timezone          *out)
{
	out->tz_minuteswest = GE32(in->tz_minuteswest);
	out->tz_dsttime     = GE32(in->tz_dsttime);
}

void
nix_timespec_to_obsd41_timespec(xec_endian_t               endian,
								struct nix_timespec const *in,
								struct obsd41_timespec    *out)
{
	if (sizeof(out->tv_sec) == sizeof(uint32_t))
		out->tv_sec = GE32(in->tv_sec & 0xffffffff);
	else if (sizeof(out->tv_sec) == sizeof(uint64_t))
		out->tv_sec = GE64(in->tv_sec);
	else
		XEC_ASSERT(g_bsd_log, 0);

	if (sizeof(out->tv_nsec) == sizeof(uint32_t))
		out->tv_nsec = GE32(in->tv_nsec & 0xffffffff);
	else if (sizeof(out->tv_nsec) == sizeof(uint64_t))
		out->tv_nsec = GE64(in->tv_nsec);
	else
		XEC_ASSERT(g_bsd_log, 0);
}

void
obsd41_timespec_to_nix_timespec(xec_endian_t                  endian,
								struct obsd41_timespec const *in,
								struct nix_timespec          *out)
{
	if (sizeof(in->tv_sec) == sizeof(uint32_t))
		out->tv_sec = GE32(in->tv_sec);
	else if (sizeof(in->tv_sec) == sizeof(uint64_t))
		out->tv_sec = GE64(in->tv_sec);
	else
		XEC_ASSERT(g_bsd_log, 0);

	if (sizeof(in->tv_nsec) == sizeof(uint32_t))
		out->tv_nsec = GE32(in->tv_nsec);
	else if (sizeof(in->tv_nsec) == sizeof(uint64_t))
		out->tv_nsec = GE64(in->tv_nsec);
	else
		XEC_ASSERT(g_bsd_log, 0);
}

void
nix_timeval_to_obsd41_timeval(xec_endian_t              endian,
                              struct nix_timeval const *in,
                              struct obsd41_timeval    *out)
{
	if (sizeof(out->tv_sec) == sizeof(uint32_t))
		out->tv_sec = GE32(in->tv_sec & 0xffffffff);
	else if (sizeof(out->tv_sec) == sizeof(uint64_t))
		out->tv_sec = GE64(in->tv_sec);
	else
		XEC_ASSERT(g_bsd_log, 0);

	if (sizeof(in->tv_usec) == sizeof(uint32_t))
		out->tv_usec = GE32(in->tv_usec & 0xffffffff);
	else if (sizeof(in->tv_usec) == sizeof(uint64_t))
		out->tv_usec = GE64(in->tv_usec);
	else
		XEC_ASSERT(g_bsd_log, 0);
}

void
obsd41_timeval_to_nix_timeval(xec_endian_t                 endian,
							  struct obsd41_timeval const *in,
							  struct nix_timeval          *out)
{
	if (sizeof(in->tv_sec) == sizeof(uint32_t))
		out->tv_sec = GE32(in->tv_sec & 0xffffffff);
	else if (sizeof(in->tv_sec) == sizeof(uint64_t))
		out->tv_sec = GE64(in->tv_sec);
	else
		XEC_ASSERT(g_bsd_log, 0);

	if (sizeof(in->tv_usec) == sizeof(uint32_t))
		out->tv_usec = GE32(in->tv_usec & 0xffffffff);
	else if (sizeof(in->tv_usec) == sizeof(uint64_t))
		out->tv_usec = GE64(in->tv_usec);
	else
		XEC_ASSERT(g_bsd_log, 0);
}

void
nix_stat_to_obsd41_stat(xec_endian_t           endian,
						struct nix_stat const *in,
						struct obsd41_stat    *out)
{
	out->st_dev     = GE32(in->st_dev);
	out->st_ino     = GE32(in->st_ino);
	out->st_mode    = GE32(in->st_mode);
	out->st_nlink   = GE32(in->st_nlink);
	out->st_uid     = GE32(in->st_uid);
	out->st_gid     = GE32(in->st_gid);
	out->st_rdev    = GE32(in->st_rdev);
	nix_timespec_to_obsd41_timespec(endian, &in->st_atimespec, &out->st_atimespec);
	nix_timespec_to_obsd41_timespec(endian, &in->st_mtimespec, &out->st_mtimespec);
	nix_timespec_to_obsd41_timespec(endian, &in->st_ctimespec, &out->st_ctimespec);
	nix_timespec_to_obsd41_timespec(endian, &in->st_btimespec, &out->__st_birthtimespec);
	out->st_size    = GE64(in->st_size);
	out->st_blocks  = GE32(in->st_blocks);
	out->st_blksize = GE32(in->st_blksize);
	/* XXX FLAGS SHOULD BE CONVERTED! */
	out->st_flags   = GE32(in->st_flags);
	out->st_gen     = GE32(in->st_gen & 0xffffffff);
}

void
obsd41_sigaction32_to_nix_sigaction(xec_endian_t                     endian,
                                    struct obsd41_sigaction32 const *in,
                                    struct nix_sigaction            *out)
{
	out->__sa_handler = GE32(in->__sa_handler);
	out->sa_flags     = GE32(in->sa_flags);
	out->sa_mask      = GE32(in->sa_mask);
}

void
nix_sigaction_to_obsd41_sigaction32(xec_endian_t                endian,
									struct nix_sigaction const *in,
									struct obsd41_sigaction32  *out)
{
	out->__sa_handler = GE32(in->__sa_handler);
	out->sa_flags     = GE32(in->sa_flags);
	out->sa_mask      = GE32(in->sa_mask);
}

void
nix_statfs_to_obsd41_statfs (xec_endian_t             endian,
                             struct nix_statfs const *in,
                             struct obsd41_statfs    *out)
{
	out->f_flags       = GE32(in->f_flags);
	out->f_bsize       = GE32(in->f_bsize);
	out->f_iosize      = GE32(in->f_iosize);
	out->f_blocks      = GE32(in->f_blocks);
	out->f_bfree       = GE32(in->f_bfree);
	out->f_bavail      = GE32(in->f_bavail);
	out->f_files       = GE32(in->f_files);
	out->f_ffree       = GE32(in->f_ffree);
	out->f_fsid.val[0] = GE32(in->f_fsid >> 32);
	out->f_fsid.val[1] = GE32(in->f_fsid & 0xffffffff);
	out->f_owner       = GE32(in->f_owner);
	out->f_syncwrites  = GE32(in->f_syncwrites);
	out->f_asyncwrites = GE32(in->f_asyncwrites);
	out->f_ctime       = GE32(in->f_ctime.tv_sec & 0xffffffff);
	out->f_spare[0]    = 0;
	out->f_spare[1]    = 0;
	out->f_spare[2]    = 0;
	strncpy(out->f_fstypename, in->f_fstypename, XEC_MIN(sizeof(out->f_fstypename), sizeof(in->f_fstypename)));
	strncpy(out->f_mntonname, in->f_mntonname, XEC_MIN(sizeof(out->f_mntonname), sizeof(in->f_mntonname)));
	strncpy(out->f_mntfromname, in->f_mntfromname, XEC_MIN(sizeof(out->f_mntfromname), sizeof(in->f_mntfromname)));

	memset(&out->mount_info, 0, sizeof(out->mount_info));
}

static void
nix_sockaddr_in_to_obsd41_sockaddr_in(xec_endian_t                  endian,
									  struct nix_sockaddr_in const *in,
									  struct obsd41_sockaddr_in    *out)
{
	out->sin_len    = sizeof(*out);
	out->sin_family = in->sin_family;
	out->sin_port   = in->sin_port;
	out->sin_addr   = in->sin_addr;
}

static void
obsd41_sockaddr_in_to_nix_sockaddr_in(xec_endian_t                     endian,
                                      struct obsd41_sockaddr_in const *in,
                                      struct nix_sockaddr_in          *out)
{
	XEC_ASSERT(g_bsd_log, in->sin_len == 0 || in->sin_len == sizeof(*in));
	out->sin_family = in->sin_family;
	out->sin_port   = in->sin_port;
	out->sin_addr   = in->sin_addr;
}

static void
nix_sockaddr_un_to_obsd41_sockaddr_un(xec_endian_t                  endian,
                                      struct nix_sockaddr_un const *in,
                                      struct obsd41_sockaddr_un    *out)
{
	out->sun_len    = sizeof(*out);
	out->sun_family = in->sun_family;
	strncpy(out->sun_path, in->sun_path, XEC_MIN(sizeof(in->sun_path), sizeof(out->sun_path)));
}

static void
obsd41_sockaddr_un_to_nix_sockaddr_un(xec_endian_t                     endian,
									  struct obsd41_sockaddr_un const *in,
									  struct nix_sockaddr_un          *out)
{
	XEC_ASSERT(g_bsd_log, in->sun_len == 0 || in->sun_len == sizeof(*in));
	out->sun_family = in->sun_family;
	strncpy(out->sun_path, in->sun_path, XEC_MIN(sizeof(in->sun_path), sizeof(out->sun_path)));
}

int
nix_sockaddr_to_obsd41_sockaddr(xec_endian_t               endian,
								struct nix_sockaddr const *in,
								nix_socklen_t              inlen,
								struct obsd41_sockaddr    *out,
								obsd41_socklen_t          *outlen)
{
	switch (in->sa_family) {
		case NIX_AF_UNIX:
			XEC_ASSERT(g_bsd_log, inlen >= sizeof(struct nix_sockaddr_un));
			nix_sockaddr_un_to_obsd41_sockaddr_un(endian,
			   (struct nix_sockaddr_un const *)in, (struct obsd41_sockaddr_un *)out);
			*outlen = GE32(sizeof(struct obsd41_sockaddr_un));
			return (1);

		case NIX_AF_INET:
			XEC_ASSERT(g_bsd_log, inlen >= sizeof(struct nix_sockaddr_in));
			nix_sockaddr_in_to_obsd41_sockaddr_in(endian,
				(struct nix_sockaddr_in const *)in, (struct obsd41_sockaddr_in *)out);
			*outlen = GE32(sizeof(struct obsd41_sockaddr_in));
			return (1);
	}

	return (0);
}

int
obsd41_sockaddr_to_nix_sockaddr(xec_endian_t                  endian,
								struct obsd41_sockaddr const *in,
								obsd41_socklen_t              inlen,
								struct nix_sockaddr          *out,
								nix_socklen_t                *outlen)
{
	switch (in->sa_family) {
		case NIX_AF_UNIX:
		  XEC_ASSERT(g_bsd_log, inlen >= (obsd41_socklen_t)sizeof(struct obsd41_sockaddr_un));
		  XEC_ASSERT(g_bsd_log, *outlen >= (nix_socklen_t)sizeof(struct nix_sockaddr_un));
		  obsd41_sockaddr_un_to_nix_sockaddr_un(endian,
			 (struct obsd41_sockaddr_un const *)in, (struct nix_sockaddr_un *)out);
		  *outlen = sizeof(struct nix_sockaddr_un);
		  return (1);

		case NIX_AF_INET:
		  XEC_ASSERT(g_bsd_log, inlen >= (obsd41_socklen_t)sizeof(struct obsd41_sockaddr_in));
		  XEC_ASSERT(g_bsd_log, *outlen >= (nix_socklen_t)sizeof(struct nix_sockaddr_in));
		  obsd41_sockaddr_in_to_nix_sockaddr_in(endian,
			 (struct obsd41_sockaddr_in const *)in, (struct nix_sockaddr_in *)out);
		  *outlen = sizeof(struct nix_sockaddr_in);
		  return (1);
	}

	return (0);
}

void
nix_rusage_to_obsd41_rusage(xec_endian_t             endian,
							struct nix_rusage const *in,
							struct obsd41_rusage    *out)
{
	nix_timeval_to_obsd41_timeval(endian, &in->ru_utime, &out->ru_utime);
	nix_timeval_to_obsd41_timeval(endian, &in->ru_stime, &out->ru_stime);
	out->ru_maxrss   = GELONG(in->ru_maxrss);
	out->ru_ixrss    = GELONG(in->ru_ixrss);
	out->ru_idrss    = GELONG(in->ru_idrss);
	out->ru_isrss    = GELONG(in->ru_isrss);
	out->ru_minflt   = GELONG(in->ru_minflt);
	out->ru_majflt   = GELONG(in->ru_majflt);
	out->ru_nswap    = GELONG(in->ru_nswap);
	out->ru_inblock  = GELONG(in->ru_inblock);
	out->ru_oublock  = GELONG(in->ru_oublock);
	out->ru_msgsnd   = GELONG(in->ru_msgsnd);
	out->ru_msgrcv   = GELONG(in->ru_msgrcv);
	out->ru_nsignals = GELONG(in->ru_nsignals);
	out->ru_nvcsw    = GELONG(in->ru_nvcsw);
	out->ru_nivcsw   = GELONG(in->ru_nivcsw);
}

void
nix_rlimit_to_obsd41_rlimit(xec_endian_t             endian,
							struct nix_rlimit const *in,
							struct obsd41_rlimit    *out)
{
	out->rlim_cur = GE64(in->rlim_cur);
	out->rlim_max = GE64(in->rlim_max);
}

static __inline int16_t
nix_poll_event_to_obsd41_event(int events)
{
	int16_t oevents = 0;

	if (events & NIX_POLLIN) oevents |= OBSD41_POLLIN;
	if (events & NIX_POLLPRI) oevents |= OBSD41_POLLPRI;
	if (events & NIX_POLLOUT) oevents |= OBSD41_POLLOUT;
	if (events & NIX_POLLRDNORM) oevents |= OBSD41_POLLRDNORM;
	if (events & NIX_POLLRDBAND) oevents |= OBSD41_POLLRDBAND;
	if (events & NIX_POLLWRBAND) oevents |= OBSD41_POLLWRBAND;
	if (events & NIX_POLLERR) oevents |= OBSD41_POLLERR;
	if (events & NIX_POLLHUP) oevents |= OBSD41_POLLHUP;
	if (events & NIX_POLLNVAL) oevents |= OBSD41_POLLNVAL;

	return (oevents);
}

static __inline int
obsd41_poll_event_to_nix_event (int16_t events)
{
	int xevents = 0;

	if (events & OBSD41_POLLIN) xevents |= NIX_POLLIN;
	if (events & OBSD41_POLLPRI) xevents |= NIX_POLLPRI;
	if (events & OBSD41_POLLOUT) xevents |= NIX_POLLOUT;
	if (events & OBSD41_POLLRDNORM) xevents |= NIX_POLLRDNORM;
	if (events & OBSD41_POLLRDBAND) xevents |= NIX_POLLRDBAND;
	if (events & OBSD41_POLLWRBAND) xevents |= NIX_POLLWRBAND;
	if (events & OBSD41_POLLERR) xevents |= NIX_POLLERR;
	if (events & OBSD41_POLLHUP) xevents |= NIX_POLLHUP;
	if (events & OBSD41_POLLNVAL) xevents |= NIX_POLLNVAL;

	return (xevents);
}

void
obsd41_pollfd_to_nix_pollfd(xec_endian_t                endian,
							struct obsd41_pollfd const *in,
							struct nix_pollfd          *out)
{
	out->fd      = GE32(in->fd);
	out->events  = obsd41_poll_event_to_nix_event(GE16(in->events));
	out->revents = 0;
}

void
nix_pollfd_to_obsd41_pollfd(xec_endian_t             endian,
							struct nix_pollfd const *in,
							struct obsd41_pollfd    *out)
{
	out->revents = GE16(nix_poll_event_to_obsd41_event(in->revents));
}

static __inline nix_tcflag_t
obsd41_termios_iflag_to_nix_termios_iflag(obsd41_tcflag_t flags)
{
	nix_tcflag_t oflags = 0;

	if (flags == 0)
	  return (0);

	if (flags & OBSD41_IGNBRK) oflags |= NIX_IGNBRK;
	if (flags & OBSD41_BRKINT) oflags |= NIX_BRKINT;
	if (flags & OBSD41_PARMRK) oflags |= NIX_PARMRK;
	if (flags & OBSD41_INPCK) oflags |= NIX_INPCK;
	if (flags & OBSD41_ISTRIP) oflags |= NIX_ISTRIP;
	if (flags & OBSD41_INLCR) oflags |= NIX_INLCR;
	if (flags & OBSD41_IGNCR) oflags |= NIX_IGNCR;
	if (flags & OBSD41_ICRNL) oflags |= NIX_ICRNL;
	if (flags & OBSD41_IXON) oflags |= NIX_IXON;
	if (flags & OBSD41_IXOFF) oflags |= NIX_IXOFF;
	if (flags & OBSD41_IXANY) oflags |= NIX_IXANY;
	if (flags & OBSD41_IUCLC) oflags |= NIX_IUCLC;
	if (flags & OBSD41_IMAXBEL) oflags |= NIX_IMAXBEL;

	return (oflags);
}

static __inline obsd41_tcflag_t
nix_termios_iflag_to_obsd41_termios_iflag(nix_tcflag_t flags)
{
	obsd41_tcflag_t oflags = 0;

	if (flags == 0)
		return (0);

	if (flags & NIX_IGNBRK) oflags |= OBSD41_IGNBRK;
	if (flags & NIX_BRKINT) oflags |= OBSD41_BRKINT;
	if (flags & NIX_PARMRK) oflags |= OBSD41_PARMRK;
	if (flags & NIX_INPCK) oflags |= OBSD41_INPCK;
	if (flags & NIX_ISTRIP) oflags |= OBSD41_ISTRIP;
	if (flags & NIX_INLCR) oflags |= OBSD41_INLCR;
	if (flags & NIX_IGNCR) oflags |= OBSD41_IGNCR;
	if (flags & NIX_ICRNL) oflags |= OBSD41_ICRNL;
	if (flags & NIX_IXON) oflags |= OBSD41_IXON;
	if (flags & NIX_IXOFF) oflags |= OBSD41_IXOFF;
	if (flags & NIX_IXANY) oflags |= OBSD41_IXANY;
	if (flags & NIX_IUCLC) oflags |= OBSD41_IUCLC;
	if (flags & NIX_IMAXBEL) oflags |= OBSD41_IMAXBEL;

	return (oflags);
}

static __inline nix_tcflag_t
obsd41_termios_oflag_to_nix_termios_oflag(obsd41_tcflag_t flags)
{
	nix_tcflag_t oflags = 0;

	if (flags == 0)
	  return (0);

	if (flags & OBSD41_OPOST) oflags |= NIX_OPOST;
	if (flags & OBSD41_ONLCR) oflags |= NIX_ONLCR;
	if (flags & OBSD41_OXTABS) oflags |= NIX_OXTABS;
	if (flags & OBSD41_ONOEOT) oflags |= NIX_ONOEOT;
	if (flags & OBSD41_OCRNL) oflags |= NIX_OCRNL;
	if (flags & OBSD41_OLCUC) oflags |= NIX_OLCUC;
	if (flags & OBSD41_ONOCR) oflags |= NIX_ONOCR;
	if (flags & OBSD41_ONLRET) oflags |= NIX_ONLRET;

	return (oflags);
}

static __inline obsd41_tcflag_t
nix_termios_oflag_to_obsd41_termios_oflag(nix_tcflag_t flags)
{
	obsd41_tcflag_t oflags = 0;

	if (flags == 0)
	  return (0);

	if (flags & NIX_OPOST) oflags |= OBSD41_OPOST;
	if (flags & NIX_ONLCR) oflags |= OBSD41_ONLCR;
	if (flags & NIX_OXTABS) oflags |= OBSD41_OXTABS;
	if (flags & NIX_ONOEOT) oflags |= OBSD41_ONOEOT;
	if (flags & NIX_OCRNL) oflags |= OBSD41_OCRNL;
	if (flags & NIX_OLCUC) oflags |= OBSD41_OLCUC;
	if (flags & NIX_ONOCR) oflags |= OBSD41_ONOCR;
	if (flags & NIX_ONLRET) oflags |= OBSD41_ONLRET;

	return (oflags);
}

static __inline nix_tcflag_t
obsd41_termios_cflag_to_nix_termios_cflag(obsd41_tcflag_t flags)
{
	nix_tcflag_t oflags = 0;

	if (flags == 0)
	  return (0);

	switch (flags & OBSD41_CSIZE) {
		case OBSD41_CS5: oflags |= NIX_CS5; break;
		case OBSD41_CS6: oflags |= NIX_CS6; break;
		case OBSD41_CS7: oflags |= NIX_CS7; break;
		case OBSD41_CS8: oflags |= NIX_CS8; break;
	}
	if (flags & OBSD41_CIGNORE) oflags |= NIX_CIGNORE;
	if (flags & OBSD41_CSTOPB) oflags |= NIX_CSTOPB;
	if (flags & OBSD41_CREAD) oflags |= NIX_CREAD;
	if (flags & OBSD41_PARENB) oflags |= NIX_PARENB;
	if (flags & OBSD41_PARODD) oflags |= NIX_PARODD;
	if (flags & OBSD41_HUPCL) oflags |= NIX_HUPCL;
	if (flags & OBSD41_CLOCAL) oflags |= NIX_CLOCAL;
	if (flags & OBSD41_CRTSCTS) oflags |= NIX_CRTSCTS;
	if (flags & OBSD41_MDMBUF) oflags |= NIX_MDMBUF;

	return (oflags);
}

static __inline obsd41_tcflag_t
nix_termios_cflag_to_obsd41_termios_cflag(nix_tcflag_t flags)
{
	obsd41_tcflag_t oflags = 0;

	if (flags == 0)
		return (0);

	switch (flags & NIX_CSIZE) {
		case NIX_CS5: oflags |= OBSD41_CS5; break;
		case NIX_CS6: oflags |= OBSD41_CS6; break;
		case NIX_CS7: oflags |= OBSD41_CS7; break;
		case NIX_CS8: oflags |= OBSD41_CS8; break;
	}
	if (flags & NIX_CIGNORE) oflags |= OBSD41_CIGNORE;
	if (flags & NIX_CSTOPB) oflags |= OBSD41_CSTOPB;
	if (flags & NIX_CREAD) oflags |= OBSD41_CREAD;
	if (flags & NIX_PARENB) oflags |= OBSD41_PARENB;
	if (flags & NIX_PARODD) oflags |= OBSD41_PARODD;
	if (flags & NIX_HUPCL) oflags |= OBSD41_HUPCL;
	if (flags & NIX_CLOCAL) oflags |= OBSD41_CLOCAL;
	if (flags & NIX_CRTSCTS) oflags |= OBSD41_CRTSCTS;
	if (flags & NIX_MDMBUF) oflags |= OBSD41_MDMBUF;

	return (oflags);
}

static __inline nix_tcflag_t
obsd41_termios_lflag_to_nix_termios_lflag(obsd41_tcflag_t flags)
{
	nix_tcflag_t oflags = 0;

	if (flags == 0)
	  return (0);

	if (flags & OBSD41_ECHOKE) oflags |= NIX_ECHOKE;
	if (flags & OBSD41_ECHOE) oflags |= NIX_ECHOE;
	if (flags & OBSD41_ECHOK) oflags |= NIX_ECHOK;
	if (flags & OBSD41_ECHO) oflags |= NIX_ECHO;
	if (flags & OBSD41_ECHONL) oflags |= NIX_ECHONL;
	if (flags & OBSD41_ECHOPRT) oflags |= NIX_ECHOPRT;
	if (flags & OBSD41_ECHOCTL) oflags |= NIX_ECHOCTL;
	if (flags & OBSD41_ISIG) oflags |= NIX_ISIG;
	if (flags & OBSD41_ICANON) oflags |= NIX_ICANON;
	if (flags & OBSD41_ALTWERASE) oflags |= NIX_ALTWERASE;
	if (flags & OBSD41_IEXTEN) oflags |= NIX_IEXTEN;
	if (flags & OBSD41_EXTPROC) oflags |= NIX_EXTPROC;
	if (flags & OBSD41_TOSTOP) oflags |= NIX_TOSTOP;
	if (flags & OBSD41_FLUSHO) oflags |= NIX_FLUSHO;
	if (flags & OBSD41_XCASE) oflags |= NIX_XCASE;
	if (flags & OBSD41_NOKERNINFO) oflags |= NIX_NOKERNINFO;
	if (flags & OBSD41_PENDIN) oflags |= NIX_PENDIN;
	if (flags & OBSD41_NOFLSH) oflags |= NIX_NOFLSH;

	return (oflags);
}

static __inline obsd41_tcflag_t
nix_termios_lflag_to_obsd41_termios_lflag (nix_tcflag_t flags)
{
	obsd41_tcflag_t oflags = 0;

	if (flags == 0)
	  return (0);

	if (flags & NIX_ECHOKE) oflags |= OBSD41_ECHOKE;
	if (flags & NIX_ECHOE) oflags |= OBSD41_ECHOE;
	if (flags & NIX_ECHOK) oflags |= OBSD41_ECHOK;
	if (flags & NIX_ECHO) oflags |= OBSD41_ECHO;
	if (flags & NIX_ECHONL) oflags |= OBSD41_ECHONL;
	if (flags & NIX_ECHOPRT) oflags |= OBSD41_ECHOPRT;
	if (flags & NIX_ECHOCTL) oflags |= OBSD41_ECHOCTL;
	if (flags & NIX_ISIG) oflags |= OBSD41_ISIG;
	if (flags & NIX_ICANON) oflags |= OBSD41_ICANON;
	if (flags & NIX_ALTWERASE) oflags |= OBSD41_ALTWERASE;
	if (flags & NIX_IEXTEN) oflags |= OBSD41_IEXTEN;
	if (flags & NIX_EXTPROC) oflags |= OBSD41_EXTPROC;
	if (flags & NIX_TOSTOP) oflags |= OBSD41_TOSTOP;
	if (flags & NIX_FLUSHO) oflags |= OBSD41_FLUSHO;
	if (flags & NIX_XCASE) oflags |= OBSD41_XCASE;
	if (flags & NIX_NOKERNINFO) oflags |= OBSD41_NOKERNINFO;
	if (flags & NIX_PENDIN) oflags |= OBSD41_PENDIN;
	if (flags & NIX_NOFLSH) oflags |= OBSD41_NOFLSH;

	return (oflags);
}

static __inline void
obsd41_termios_cc_to_nix_termios_cc(obsd41_cc_t const *in,
									nix_cc_t          *out)
{
	out[NIX_VEOF]     = in[OBSD41_VEOF];
	out[NIX_VEOL]     = in[OBSD41_VEOL];
	out[NIX_VEOL2]    = in[OBSD41_VEOL2];
	out[NIX_VERASE]   = in[OBSD41_VERASE];
	out[NIX_VWERASE]  = in[OBSD41_VWERASE];
	out[NIX_VKILL]    = in[OBSD41_VKILL];
	out[NIX_VREPRINT] = in[OBSD41_VREPRINT];
	out[NIX_VINTR]    = in[OBSD41_VINTR];
	out[NIX_VQUIT]    = in[OBSD41_VQUIT];
	out[NIX_VSUSP]    = in[OBSD41_VSUSP];
	out[NIX_VDSUSP]   = in[OBSD41_VDSUSP];
	out[NIX_VSTART]   = in[OBSD41_VSTART];
	out[NIX_VSTOP]    = in[OBSD41_VSTOP];
	out[NIX_VLNEXT]   = in[OBSD41_VLNEXT];
	out[NIX_VDISCARD] = in[OBSD41_VDISCARD];
	out[NIX_VMIN]     = in[OBSD41_VMIN];
	out[NIX_VTIME]    = in[OBSD41_VTIME];
	out[NIX_VSTATUS]  = in[OBSD41_VSTATUS];
}

static __inline void
nix_termios_cc_to_obsd41_termios_cc(nix_cc_t const *in,
									obsd41_cc_t    *out)
{
	out[OBSD41_VEOF]     = in[NIX_VEOF];
	out[OBSD41_VEOL]     = in[NIX_VEOL];
	out[OBSD41_VEOL2]    = in[NIX_VEOL2];
	out[OBSD41_VERASE]   = in[NIX_VERASE];
	out[OBSD41_VWERASE]  = in[NIX_VWERASE];
	out[OBSD41_VKILL]    = in[NIX_VKILL];
	out[OBSD41_VREPRINT] = in[NIX_VREPRINT];
	out[OBSD41_VINTR]    = in[NIX_VINTR];
	out[OBSD41_VQUIT]    = in[NIX_VQUIT];
	out[OBSD41_VSUSP]    = in[NIX_VSUSP];
	out[OBSD41_VDSUSP]   = in[NIX_VDSUSP];
	out[OBSD41_VSTART]   = in[NIX_VSTART];
	out[OBSD41_VSTOP]    = in[NIX_VSTOP];
	out[OBSD41_VLNEXT]   = in[NIX_VLNEXT];
	out[OBSD41_VDISCARD] = in[NIX_VDISCARD];
	out[OBSD41_VMIN]     = in[NIX_VMIN];
	out[OBSD41_VTIME]    = in[NIX_VTIME];
	out[OBSD41_VSTATUS]  = in[NIX_VSTATUS];
}

static __inline nix_speed_t
obsd41_termios_speed_to_nix_termios_speed(obsd41_speed_t speed)
{
	return (speed);
}

static __inline nix_speed_t
nix_termios_speed_to_obsd41_termios_speed(obsd41_speed_t speed)
{
	return (speed);
}

void
obsd41_termios_to_nix_termios(xec_endian_t                 endian,
							  struct obsd41_termios const *in,
							  struct nix_termios          *out)
{
	out->c_iflag  = obsd41_termios_iflag_to_nix_termios_iflag(GE32(in->c_iflag));
	out->c_oflag  = obsd41_termios_oflag_to_nix_termios_oflag(GE32(in->c_oflag));
	out->c_cflag  = obsd41_termios_cflag_to_nix_termios_cflag(GE32(in->c_cflag));
	out->c_lflag  = obsd41_termios_lflag_to_nix_termios_lflag(GE32(in->c_lflag));
	out->c_ispeed = obsd41_termios_speed_to_nix_termios_speed(GE32(in->c_ispeed));
	out->c_ospeed = obsd41_termios_speed_to_nix_termios_speed(GE32(in->c_ospeed));
	obsd41_termios_cc_to_nix_termios_cc(in->c_cc, out->c_cc);
}

void
nix_termios_to_obsd41_termios(xec_endian_t              endian,
							  struct nix_termios const *in,
							  struct obsd41_termios    *out)
{
	out->c_iflag  = GE32(nix_termios_iflag_to_obsd41_termios_iflag(in->c_iflag));
	out->c_oflag  = GE32(nix_termios_oflag_to_obsd41_termios_oflag(in->c_oflag));
	out->c_cflag  = GE32(nix_termios_cflag_to_obsd41_termios_cflag(in->c_cflag));
	out->c_lflag  = GE32(nix_termios_lflag_to_obsd41_termios_lflag(in->c_lflag));
	out->c_ispeed = GE32(nix_termios_speed_to_obsd41_termios_speed(in->c_ispeed));
	out->c_ospeed = GE32(nix_termios_speed_to_obsd41_termios_speed(in->c_ospeed));
	obsd41_termios_cc_to_nix_termios_cc(in->c_cc, out->c_cc);
}

void
obsd41_fd_set_to_nix_fd_set(xec_endian_t         endian,
							obsd41_fd_set const *in,
							nix_fd_set          *out)
{
	size_t n;

	for (n = 0; n < sizeof(*in) / sizeof(obsd41_fd_mask_t) &&
		  n < sizeof(*out) / sizeof(nix_fd_mask_t); n++) {
		out->fds_bits[n] = GE32(in->fds_bits[n]);
	}
}

void
nix_fd_set_to_obsd41_fd_set(xec_endian_t      endian,
							nix_fd_set const *in,
							obsd41_fd_set    *out)
{
	size_t n;
  
	for (n = 0; n < sizeof(*in) / sizeof(nix_fd_mask_t) &&
		  n < sizeof(*out) / sizeof(obsd41_fd_mask_t); n++) {
		out->fds_bits[n] = GE32(in->fds_bits[n]);
	}
}
