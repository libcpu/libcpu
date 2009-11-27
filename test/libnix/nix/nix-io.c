#include "nix-config.h"

#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#ifdef HAVE_SYS_FILIO_H
#include <sys/filio.h>
#endif
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>
#include <termios.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "nix.h"
#include "nix-structs.h"
#include "xec-mem.h"
#include "xec-debug.h"

extern void *g_nix_log;

int
nix_sync(nix_env_t *env)
{
	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "invoked", 0);  

	errno = 0;
	sync();
	if (errno != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}
	return (0);
}

int
nix_dup(int oldd, nix_env_t *env)
{
	int roldfd;
	int rnewfd;
	int gnewfd;

	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "oldd=%d", oldd);  

	if ((roldfd = nix_fd_get(oldd)) < 0) {
		nix_env_set_errno(env, EBADF);
		return (-1);
	}

	rnewfd = dup(roldfd);
	if (rnewfd < 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	gnewfd = nix_fd_alloc(rnewfd, env);
	if (gnewfd < 0) {
		close(rnewfd);
		return (-1);
	}

	return (gnewfd);
}

int
nix_dup2(int oldd, int newd, nix_env_t *env)
{
	int rc;
	int roldfd;
	int rnewfd;
	int gnewfd;

	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "oldd=%d newd=%d", oldd, newd);  

	if (newd < 0) {
		nix_env_set_errno(env, EINVAL);
		return (-1);
	}

	if (oldd < 0 || (roldfd = nix_fd_get(oldd)) < 0) {
		nix_env_set_errno(env, EBADF);
		return (-1);
	}

	if (!((rnewfd = nix_fd_get(newd)) < 0)) {
		rc = dup2(roldfd, rnewfd);
		gnewfd = newd;
	} else {
		rc = rnewfd = dup(roldfd);
		gnewfd = -1;
	}

	if (rc < 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	if (gnewfd < 0)
		gnewfd = nix_fd_alloc_at(newd, rnewfd, env);

	if (gnewfd != newd) {
		close (rnewfd);
		return (-1);
	}

	return (gnewfd);
}

int
nix_close(int fd, nix_env_t *env)
{
	int rfd;

	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "fd=%d", fd);

	if ((rfd = nix_fd_get(fd)) < 0) {
		nix_env_set_errno (env, EBADF);
		return (-1);
	}

	if (rfd > 2) { // XXX HACK
		if (close(rfd) != 0) {
			nix_env_set_errno(env, errno);
			return (-1);
		}

		if (!nix_fd_release(fd, env))
		  return (-1);
	}

	return (0);
}

nix_ssize_t
nix_read(int fd, void *buf, size_t bufsiz, nix_env_t *env)
{
	int     rfd;
	ssize_t nb;

	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "fd=%d, buf=%p, len=%zu", fd, buf, bufsiz);

	if (buf == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	if ((rfd = nix_fd_get(fd)) < 0) {
		nix_env_set_errno(env, EBADF);
		return (-1);
	}

	if (bufsiz == 0)
		return (0);

	nb = read(rfd, buf, bufsiz);
	if (nb < 0)
		nix_env_set_errno(env, errno);

	return (nb);
}

nix_ssize_t
nix_readv(int fd, struct nix_iovec const *iov, int iovcnt, nix_env_t *env)
{
	int     rfd;
	ssize_t nb;

	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "fd=%d, iov=%p, iovcnt=%zu", fd, iov, iovcnt);

	if (iov == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	if (iovcnt < 0) {
		nix_env_set_errno(env, EINVAL);
		return (-1);
	}

	if ((rfd = nix_fd_get(fd)) < 0) {
		nix_env_set_errno(env, EBADF);
		return (-1);
	}

	if (iovcnt == 0)
		return 0;

	nb = readv(rfd, (struct iovec *)iov, iovcnt);
	if (nb < 0)
		nix_env_set_errno(env, errno);

	return (nb);
}

nix_ssize_t
nix_write(int fd, void const *buf, size_t bufsiz, nix_env_t *env)
{
	int     rfd;
	ssize_t nb;

	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "fd=%d, buf=%p, len=%zu", fd, buf, bufsiz);

	if (buf == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	if ((rfd = nix_fd_get(fd)) < 0) {
		nix_env_set_errno(env, EBADF);
		return (-1);
	}

	if (bufsiz == 0)
		return (0);

	nb = write(rfd, buf, bufsiz);
	if (nb < 0)
		nix_env_set_errno(env, errno);

	return (nb);
}

nix_ssize_t
nix_writev(int fd, struct nix_iovec const *iov, int iovcnt, nix_env_t *env)
{
	int     rfd;
	ssize_t nb;

	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "fd=%d, iov=%p, iovcnt=%zu", fd, iov, iovcnt);

	if (iov == NULL)
	  {
		nix_env_set_errno(env, EFAULT);
		return -1;
	  }

	if (iovcnt < 0)
	  {
		nix_env_set_errno(env, EINVAL);
		return -1;
	  }

	if ((rfd = nix_fd_get(fd)) < 0)
	  {
		nix_env_set_errno(env, EBADF);
		return -1;
	  }

	if (iovcnt == 0)
	  return 0;

	nb = writev(rfd, (struct iovec *)iov, iovcnt);
	if (nb < 0)
	  nix_env_set_errno(env, errno);

	return nb;
}

int
nix_pipe(int *fds, nix_env_t *env)
{
	int rfds[2];

	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "fds=%p", fds);

	if (fds == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	if (pipe(rfds) != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	fds[0] = nix_fd_alloc(rfds[0], env);
	if (fds[0] < 0)
	  goto fail;
	fds[1] = nix_fd_alloc(rfds[1], env);
	if (fds[1] < 0)
	  goto fail;

	return (0);

fail:
	close(rfds[1]);
	close(rfds[0]);
	fds[0] = fds[1] = 0;
	return (-1);
}

int
nix_ioctl(int fd, unsigned long request, void *data, nix_env_t *env)
{
	int rfd;

	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "fd=%d, request=%08x, data=%p", fd, request, data);

	if ((rfd = nix_fd_get(fd)) < 0) {
		nix_env_set_errno(env, EBADF);
		return (-1);
	}

	switch (request) {
		/* Terminal I/O */
		case NIX_TIOCGETA: /* Get Attributes */
		{
			struct termios nios;

			if (data == NULL) {
				nix_env_set_errno(env, EFAULT);
				return (-1);
			}

			if (tcgetattr(rfd, &nios) != 0) {
				nix_env_set_errno(env, EFAULT);
				return (-1);
			}

			termios_to_nix_termios(&nios, data);
			return (0);
		}

		case NIX_TIOCSETA:  /* Set Attributes */
		case NIX_TIOCSETAW: /* Set Attributes / Drain */
		case NIX_TIOCSETAF: /* Set Attributes / Drain / Flush */
		{
			struct termios nios;
			int            action = TCSANOW;

			if (data == NULL) {
				nix_env_set_errno(env, EFAULT);
				return (-1);
			}

			switch (request) {
				case NIX_TIOCSETA:  action = TCSANOW;   break;
				case NIX_TIOCSETAW: action = TCSADRAIN; break;
				case NIX_TIOCSETAF: action = TCSAFLUSH; break;
			}

			nix_termios_to_termios(data, &nios);

			if (tcsetattr(rfd, action, &nios) != 0) {
				nix_env_set_errno(env, errno);
				return (-1);
			}

			return (0);
		}

		case NIX_FIONREAD:
		{
			int bytes;

			if (data == NULL) {
				nix_env_set_errno(env, EFAULT);
				return (-1);
			}

			if (ioctl(rfd, FIONREAD, &bytes) != 0) {
				nix_env_set_errno(env, errno);
				return (-1);
			}

			__nix_try
			{
				*(size_t *)data = bytes;
			}
			__nix_catch_any
			{
				nix_env_set_errno(env, EFAULT);
				return (-1);
			}
			__nix_end_try

			return (0);
		}

		default:
	  		break;
	}

	nix_env_set_errno(env, EINVAL);
	return (-1);
}

int
nix_fcntl(int fd, int cmd, int arg, nix_env_t *env)
{
	int rc;
	int rfd;

	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "fd=%d, cmd=%08x, arg=%08x", fd, cmd, arg);

	if ((rfd = nix_fd_get(fd)) < 0) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	switch (cmd) {
		case F_DUPFD:
			return nix_dup(fd, env);

		case F_GETFL:
		case F_SETFL:
			rc = fcntl(rfd, cmd, arg);
			if (rc < 0) {
				nix_env_set_errno(env, errno);
				return (-1);
			}
			break;

		default:
			return (0); // XXX BAD BAD BAD, but helps.
			nix_env_set_errno(env, EINVAL);
			return (-1);
	}

	return (rc);
}

int
nix_select(int                       highestfd,
		   nix_fd_set               *rfds,
		   nix_fd_set               *wfds,
		   nix_fd_set               *xfds,
		   struct nix_timeval const *tv,
		   nix_env_t                *env)
{
	fd_set          fds[3];
	struct timeval  ntv;
	int             nfds;
	int             rc = -1;
	int             lastfd = 0;
	int             hfds[256]; /*XXX*/
	struct timeval *pntv = (tv != NULL) ? &ntv : NULL;
	fd_set         *pfds[3] = { NULL, NULL, NULL };

	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "highestfd=%d, rfds=%p, wfds=%p, xfds=%p, tv=%p",
		highestfd, rfds, wfds, xfds, tv);

	if (pntv != NULL)
		nix_timeval_to_timeval(tv, pntv);
  
	if (highestfd > 256) /*XXX FD_SETSIZE */
		nfds = 256; /*XXX FD_SETSIZE */
	else
		nfds = highestfd;

	nfds = nix_fd_get_nearest(env, highestfd, -1);
	if (nfds < 0) {
		nix_env_set_errno(env, EBADF);
		return (-1);
	}

	lastfd = 0;

	if (nfds != 0) {
		int n;

		/* Create host -> guest mappings */
		for (n = 0; n < 256; n++)
			hfds[n] = -1;

		if (rfds != NULL) {
			FD_ZERO(&fds[0]);
			pfds[0] = &fds[0];
			for (n = 0; n < highestfd; n++) {
				if (FD_ISSET(n, (fd_set *)rfds)) {
					int hfd = nix_fd_get(n);
					if (!(hfd < 0) && hfd < 256) {
						FD_SET(hfd, pfds[0]);
						if (hfds[hfd] < 0)
						  hfds[hfd] = n;
						if (hfd > lastfd)
						  lastfd = hfd + 1;
					}
				}
			}
		}
		if (wfds != NULL) {
			FD_ZERO(&fds[1]);
			pfds[1] = &fds[1];
			for (n = 0; n < highestfd; n++) {
				if (FD_ISSET(n, (fd_set *)wfds)) {
					int hfd = nix_fd_get(n);
					if (!(hfd < 0) && hfd < 256) {
						FD_SET(hfd, pfds[1]);
						if (hfds[hfd] < 0)
						  hfds[hfd] = n;
						if (hfd > lastfd)
						  lastfd = hfd + 1;
					}
				}
			}
		}
		if (xfds != NULL) {
			FD_ZERO(&fds[2]);
			pfds[2] = &fds[2];
			for (n = 0; n < highestfd; n++) {
				if (FD_ISSET(n, (fd_set *)wfds)) {
					int hfd = nix_fd_get(n);
					if (!(hfd < 0) && hfd < 256) {
						FD_SET(hfd, pfds[2]);
						if (hfds[hfd] < 0)
						  hfds[hfd] = n;
						if (hfd > lastfd)
						  lastfd = hfd + 1;
					}
				}
			}
		}
	}

	errno = 0;
	rc = select(lastfd, pfds[0], pfds[1], pfds[2], pntv);
	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "select returned %d [lastfd=%d]", rc, lastfd);
	if (rc <= 0) {
		nix_env_set_errno(env, errno);
		goto done;
	}

	__nix_try
	{
		if (rfds != NULL || wfds != NULL || xfds != NULL) {
			size_t n;

			for (n = 0; n < 256; n++) {
				int gfd = hfds[n];

				if (gfd == -1)
				  continue;

				if (pfds[0] != NULL) {
					XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "selecting read hfd %d gfd %d", n, gfd);
					if (FD_ISSET(n, pfds[0]))
					  FD_SET(gfd, (fd_set *)rfds);
					else
					  FD_CLR(gfd, (fd_set *)rfds);
				}

				if (pfds[1] != NULL) {
					XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "selecting write hfd %d gfd %d", n, gfd);
					if (FD_ISSET(n, pfds[1]))
					  FD_SET(gfd, (fd_set *)wfds);
					else
					  FD_CLR(gfd, (fd_set *)wfds);
				}

				if (pfds[2] != NULL) {
					XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "selecting except hfd %d gfd %d", n, gfd);
					if (FD_ISSET(n, pfds[2]))
					  FD_SET(gfd, (fd_set *)wfds);
					else
					  FD_CLR(gfd, (fd_set *)wfds);
				}
			}
		  }
	}
	__nix_catch_any
	{
		nix_env_set_errno(env, EFAULT);
		rc = -1;
	}
	__nix_end_try

done:
	return (rc);
}

static __inline int
nix_poll_events_to(int events)
{
	int nevents = 0;

	if (events & NIX_POLLIN)
		nevents |= POLLIN;
	if (events & NIX_POLLPRI)
		nevents |= POLLPRI;
	if (events & NIX_POLLOUT)
		nevents |= POLLOUT;
	if (events & NIX_POLLRDNORM)
		nevents |= POLLRDNORM;
	if (events & NIX_POLLRDBAND)
		nevents |= POLLRDBAND;
	if (events & NIX_POLLWRBAND)
		nevents |= POLLWRBAND;
	if (events & NIX_POLLERR)
		nevents |= POLLERR;
	if (events & NIX_POLLHUP)
		nevents |= POLLHUP;
	if (events & NIX_POLLNVAL)
		nevents |= POLLNVAL;
#ifdef POLLEXTEND
	if (events & NIX_POLLEXTEND)
		nevents |= POLLEXTEND;
#endif
#ifdef POLLATTRIB
	if (events & NIX_POLLATTRIB)
		nevents |= POLLATTRIB;
#endif
#ifdef POLLNLINK
	if (events & NIX_POLLNLINK)
		nevents |= POLLNLINK;
#endif
#ifdef POLLWRITE
	if (events & NIX_POLLWRITE)
		nevents |= POLLWRITE;
#endif

	return (nevents);
}

static __inline int
nix_poll_events_from(int events)
{
	int xevents = 0;

	if (events & POLLIN)
		xevents |= NIX_POLLIN;
	if (events & POLLPRI)
		xevents |= NIX_POLLPRI;
	if (events & POLLOUT)
		xevents |= NIX_POLLOUT;
	if (events & POLLRDNORM)
		xevents |= NIX_POLLRDNORM;
	if (events & POLLRDBAND)
		xevents |= NIX_POLLRDBAND;
	if (events & POLLWRBAND)
		xevents |= NIX_POLLWRBAND;
	if (events & POLLERR)
		xevents |= NIX_POLLERR;
	if (events & POLLHUP)
		xevents |= NIX_POLLHUP;
	if (events & POLLNVAL)
		xevents |= NIX_POLLNVAL;
#ifdef POLLEXTEND
	if (events & POLLEXTEND)
		xevents |= NIX_POLLEXTEND;
#endif
#ifdef POLLATTRIB
	if (events & POLLATTRIB)
		xevents |= NIX_POLLATTRIB;
#endif
#ifdef POLLNLINK
	if (events & POLLNLINK)
		xevents |= NIX_POLLNLINK;
#endif
#ifdef POLLWRITE
	if (events & POLLWRITE)
		xevents |= NIX_POLLWRITE;
#endif

	return (xevents);
}

int
nix_poll(struct nix_pollfd *fds,
		 nix_nfds_t         nfds,
		 int                timeout,
		 nix_env_t         *env)
{
	int            rc;
	size_t         n;
	struct pollfd *_fds;

	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "fds=%p, nfds=%u, timeout=%d", fds, nfds, timeout);

	if (fds == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	if (nfds == 0) {
		/* EINVAL? */
		nix_env_set_errno(env, EINVAL);
		return (-1);
	}

	_fds = xec_mem_alloc_ntype(struct pollfd, nfds, 0);
	if (_fds == NULL) {
		nix_env_set_errno(env, ENOMEM);
		return (-1);
	}

	for (n = 0; n < nfds; n++) {
		_fds[n].fd      = nix_fd_get(fds[n].fd);
		_fds[n].events  = nix_poll_events_to(fds[n].events);
		_fds[n].revents = 0;
	}

	rc = poll(_fds, nfds, timeout);
	if (rc < 0) {
		nix_env_set_errno(env, errno);
		goto done;
	}

	rc = 0;
	for (n = 0; n < nfds; n++) {
		fds[n].revents = nix_poll_events_from(_fds[n].revents);
		if (fds[n].revents)
			rc++;
	}

done:
	xec_mem_free(_fds);

	return (rc);
}
