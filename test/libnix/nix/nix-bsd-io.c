#include "nix-config.h"

#include <sys/types.h>
#ifdef HAVE_SYS_EVENT_H
#include <sys/event.h>
#endif
#include <sys/time.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "nix.h"
#include "nix-fd.h"

int
nix_bsd_closefrom(int fd, nix_env_t *env)
{
#if 0
	int rfd;

	if ((rfd = nix_fd_get(fd)) < 0) {
		nix_env_set_errno(env, EBADF);
		return (-1);
	}

	if (rfd > 2) {
		if (close(rfd) != 0) {
			nix_env_set_errno (env, errno);
			return (-1);
		}
  
		if (!nix_fd_release(fd, env))
			return (-1);
	}

	return (0);
#endif
	return (nix_nosys(env));
}

int
nix_bsd_kqueue(nix_env_t *env)
{
#ifdef HAVE_KQUEUE
	int fd, gfd;

	fd = kqueue();
	if (fd < 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	gfd = nix_fd_alloc(fd, env);
	if (gfd < 0) {
		close(fd);
		return (-1);
	}
  
	return (gfd);
#else
	return (nix_nosys (env));
#endif
}

int
nix_bsd_kevent (int kq,
	struct nix_bsd_kevent const *changelist, int nchanges,
	struct nix_bsd_kevent *eventlist, int nevents,
	struct nix_timespec const *timeout,
	nix_env_t *env)
{
  return (nix_nosys(env));
}
