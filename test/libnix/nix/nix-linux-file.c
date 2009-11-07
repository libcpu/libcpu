#include "nix-config.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "nix.h"

int
nix_linux_fdatasync(int fd, nix_env_t *env)
{
#ifdef HAVE_FDATASYNC
	int rfd;

	if ((rfd = nix_fd_get(fd)) < 0) {
		nix_env_set_errno (env, EBADF);
		return (-1);
	}

	if (fdatasync(rfd) != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	return (0);
#else
	return nix_fsync(fd, env);
#endif
}
