#include "nix-config.h"

#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "nix.h"
#include "nix-fd.h"

int
nix_bsd_revoke(char const *path, nix_env_t *env)
{
	if (path == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}
#ifdef HAVE_REVOKE
	if (revoke (path) != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	return (0);
#else
  /* XXX: What about this ? */
  return (nix_nosys(env));
#endif
}

int
nix_bsd_preadv(int fd, struct nix_iovec const *iov, int iovcnt, nix_off_t offset, nix_env_t *env)
{
	return (nix_nosys(env));
}

int
nix_bsd_pwritev(int fd, struct nix_iovec const *iov, int iovcnt, nix_off_t offset, nix_env_t *env)
{
	return (nix_nosys(env));
}

int
nix_bsd_fchflags(int fd, int flags, nix_env_t *env)
{
#ifdef HAVE_FCHFLAGS
	int rfd;

	if ((rfd = nix_fd_get(fd)) < 0) {
		nix_env_set_errno(env, EBADF);
		return (-1);
    }
  
	if (fchflags(rfd, flags) != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
    }

	return (0);
#else
	return (nix_nosys(env));
#endif
}

int
nix_bsd_chflags (char const *path, int flags, nix_env_t *env)
{
#ifdef HAVE_CHFLAGS
	if (path == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}
  
	if (chflags(path, flags) != 0) {
		nix_env_set_errno (env, errno);
		return (-1);
	}

	return (0);
#else
	return (nix_nosys(env));
#endif
}
