#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "nix.h"
#include "nix-fd.h"

int
nix_fchdir(int fd, nix_env_t *env)
{
	int rfd;

	if ((rfd = nix_fd_get(fd)) < 0) {
		nix_env_set_errno(env, EBADF);
		return (-1);
	}

	if (fchdir(rfd) != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
  }

	return (0);
}

int
nix_chdir(char const *path, nix_env_t *env)
{
	if (path == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	if (chdir(path) != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	return (0);
}

int
nix_mkdir(char const *path, nix_mode_t mode, nix_env_t *env)
{
	if (path == NULL) {
		nix_env_set_errno (env, EFAULT);
		return (-1);
	}

	if (mkdir (path, mode) != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	return (0);
}

int
nix_rmdir(char const *path, nix_env_t *env)
{
	if (path == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	if (rmdir (path) != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	return (0);
}

int
nix_chroot(char const *root, nix_env_t *env)
{
	if (root == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	if (chroot(root) != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	return (0);
}

int
nix_getcwd(char *buf, size_t bufsiz, nix_env_t *env)
{
	if (buf == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	if (bufsiz == 0)
		return (0);

	if (getcwd(buf, bufsiz) == NULL) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	return (0);
}
