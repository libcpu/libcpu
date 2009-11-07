#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include "nix.h"
#include "nix-structs.h"
#include "xec-debug.h"

extern void *g_nix_log;

nix_mode_t
nix_umask(nix_mode_t mode, nix_env_t *env)
{
	mode_t oldmode;

	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "mode=%o", mode);  

	errno = 0;
	oldmode = umask(mode);
	if (errno != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	return (oldmode);
}

int
nix_open(char const *path, int flags, int mode, nix_env_t *env)
{
	int rfd;
	int gfd;

	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "path='%s' flags=%x mode=%o", path, flags, mode);  
	if (path == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	rfd = open(path, flags, mode);
	if (rfd < 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	gfd = nix_fd_alloc(rfd, env);
	if (gfd < 0) {
		nix_env_set_errno(env, ENFILE);
		close(rfd);
		return (-1);
	}

	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "path='%s' flags=%x mode=%o -> fd=%d", path, flags, mode, gfd);
	return (gfd);
}

int
nix_access(char const *path, int mode, nix_env_t *env)
{
	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "path='%s', mode=%o", path, mode);  

	if (path == NULL) {
    	nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	if (access(path, mode) != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	return (0);
}

int
nix_creat(char const *path, int mode, nix_env_t *env)
{
	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "path='%s', mode=%o", path, mode);  

	if (path == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	return (nix_open(path, O_WRONLY | O_CREAT | O_TRUNC, mode, env));
}

int
nix_fsync(int fd, nix_env_t *env)
{
	int rfd;

	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "fd=%d", fd);  

	if ((rfd = nix_fd_get(fd)) < 0) {
		nix_env_set_errno(env, EBADF);
		return (-1);
	}

	if (fsync(fd) != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	return (0);
}

nix_off_t
nix_lseek(int fd, nix_off_t offset, int whence, nix_env_t *env)
{
	off_t off;
	int   rfd;

	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "fd=%d, offset=%lld, whence=%d", fd, offset, whence);  

	if ((rfd = nix_fd_get(fd)) < 0) {
		nix_env_set_errno(env, EBADF);
		return (-1);
	}

	off = lseek(rfd, (off_t)offset, whence);
	if (off < 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	return (off);
}

int
nix_ftruncate(int fd, nix_off_t off, nix_env_t *env)
{
	int rfd;

	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "fd=%d, offset=%lld", fd, off);  

	if ((rfd = nix_fd_get(fd)) < 0) {
		nix_env_set_errno(env, EBADF);
		return (-1);
	}

	if (ftruncate (fd, off) != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	return (0);
}

nix_ssize_t
nix_pread(int fd, void *buf, size_t bufsiz, nix_off_t offset, nix_env_t *env)
{
	int     rfd;
	ssize_t nb;
  
	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "fd=%d, buf=%p, bufsiz=%zu, offset=%lld", fd, buf, bufsiz, offset);

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

	nb = pread(rfd, buf, bufsiz, offset);
	if (nb < 0)
		nix_env_set_errno(env, errno);

	return (nb);
}

nix_ssize_t
nix_pwrite(int fd, void const *buf, size_t bufsiz, nix_off_t offset, nix_env_t *env)
{
	int     rfd;
	ssize_t nb;
  
	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "fd=%d, buf=%p, bufsiz=%zu, offset=%lld", fd, buf, bufsiz, offset);

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

	nb = pwrite(rfd, buf, bufsiz, offset);
	if (nb < 0)
		nix_env_set_errno(env, errno);

	return (nb);
}

int
nix_flock(int fd, int op, nix_env_t *env)
{
	int rfd;

	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "fd=%d, op=%d", fd, op);

	if ((rfd = nix_fd_get(fd)) < 0) {
		nix_env_set_errno(env, EBADF);
		return (-1);
	}

	if (flock(rfd, op) != 0) {
		nix_env_set_errno (env, errno);
		return (-1);
	}

	return (0);
}

int
nix_fstat (int fd, struct nix_stat *sb, nix_env_t *env)
{
	struct stat nst;
	int         rfd;

	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "fd=%d, sb=%p", fd, sb);

	if ((rfd = nix_fd_get(fd)) < 0) {
		nix_env_set_errno(env, EBADF);
		return (-1);
	}

	if (fstat(rfd, &nst) < 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	stat_to_nix_stat(&nst, sb);
	return (0);
}

int
nix_fchmod(int fd, int mode, nix_env_t *env)
{
	int rfd;

	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "fd=%d, mode=%o", fd, mode);

	if ((rfd = nix_fd_get(fd)) < 0) {
		nix_env_set_errno(env, EBADF);
		return (-1);
	}

	if (fchmod(rfd, mode) != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	return (0);
}

int
nix_fchown(int fd, nix_uid_t uid, nix_gid_t gid, nix_env_t *env)
{
	int rfd;

	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "fd=%d, uid=%u, gid=%u", fd, uid, gid);

	if ((rfd = nix_fd_get(fd)) < 0) {
		nix_env_set_errno(env, EBADF);
		return (-1);
	}

	if (fchown(rfd, (uid_t)uid, (gid_t)gid) != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	return (0);
}

int
nix_futimes(int fd, struct nix_timeval const *times, nix_env_t *env)
{
	struct timeval ntimes[2];
	int            rfd;
  
	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "fd=%d, times={ %llu, %llu }", fd, times->tv_sec, times->tv_usec);

	if (times == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	if ((rfd = nix_fd_get(fd)) < 0) {
		nix_env_set_errno(env, EBADF);
		return (-1);
	}

	ntimes[0].tv_sec  = times[0].tv_sec;
	ntimes[0].tv_usec = times[0].tv_usec;
	ntimes[1].tv_sec  = times[1].tv_sec;
	ntimes[1].tv_usec = times[1].tv_usec;

	if (futimes(rfd, ntimes) != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	return (0);
}

long
nix_fpathconf(int fd, int name, nix_env_t *env)
{
	long rv;
	int  rfd;

	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "fd=%d, name=%d", fd, name);

	if ((rfd = nix_fd_get(fd)) < 0) {
		nix_env_set_errno(env, EBADF);
		return (-1);
	}

	if ((rv = fpathconf(rfd, name)) < 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}
  
	return (rv);
}

int
nix_truncate(char const *path, nix_off_t off, nix_env_t *env)
{
	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "path='%s', offset=%lld", path, off);
  
	if (path == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	if (truncate(path, off) != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	return (0);
}

int
nix_stat(char const *path, struct nix_stat *sb, nix_env_t *env)
{
	struct stat nst;

	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "path='%s', sb=%p", path, sb);

	if (path == NULL || sb == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	if (stat(path, &nst) != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	stat_to_nix_stat(&nst, sb);
	return (0);
}

int
nix_lstat(char const *path, struct nix_stat *sb, nix_env_t *env)
{
	struct stat nst;

	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "path='%s', sb=%p", path, sb);

	if (path == NULL || sb == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	if (lstat(path, &nst) != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	stat_to_nix_stat(&nst, sb);
	return (0);
}

int
nix_mknod(char const *path, int mode, nix_dev_t dev, nix_env_t *env)
{
	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "path='%s', mode=%o dev=%x", path, mode, dev);

	if (path == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	if (mknod(path, mode, (dev_t)dev) != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
    }

  return (0);
}


int
nix_mkfifo (char const *path, nix_mode_t mode, nix_env_t *env)
{
	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "path='%s', mode=%o", path, mode);

	if (path == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	if (mkfifo(path, (mode_t)mode) != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	return (0);
}

int
nix_chmod(char const *path, int mode, nix_env_t *env)
{
	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "path='%s', mode=%o", path, mode);

	if (path == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	if (chmod(path, mode) != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	return (0);
}

int
nix_chown(char const *path, nix_uid_t uid, nix_gid_t gid, nix_env_t *env)
{
	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "path='%s', uid=%u, gid=%u", path, uid, gid);

	if (path == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	if (chown(path, (uid_t)uid, (gid_t)gid) != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	return (0);
}

int
nix_lchown(char const *path, nix_uid_t uid, nix_gid_t gid, nix_env_t *env)
{
	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "path='%s', uid=%u, gid=%u", path, uid, gid);

	if (path == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	if (lchown(path, (uid_t)uid, (gid_t)gid) != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	return (0);
}

int
nix_link(char const *name1, char const *name2, nix_env_t *env)
{
	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "name1='%s', name2='%s'", name1, name2);

	if (name1 == NULL || name2 == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	if (link(name1, name2) != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	return (0);
}

int
nix_rename(char const *name1, char const *name2, nix_env_t *env)
{
	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "name1='%s', name2='%s'", name1, name2);

	if (name1 == NULL || name2 == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	if (rename(name1, name2) != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	return (0);
}

int
nix_unlink(char const *name, nix_env_t *env)
{
	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "path='%s'", name);

	if (name == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
    }

	if (unlink (name) != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	return (0);
}

int
nix_symlink(char const *name1, char const *name2, nix_env_t *env)
{
	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "name1='%s', name2='%s'", name1, name2);

	if (name1 == NULL || name2 == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	if (symlink(name1, name2) != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	return (0);
}

int
nix_readlink(char const *path, char *buf, int bufsiz, nix_env_t *env)
{
	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "path='%s', buf=%p, bufsiz=%d", path, buf, bufsiz);

	if (path == NULL || buf == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	if (bufsiz == 0)
		return (0);

	if (readlink (path, buf, bufsiz) != 0) {
		nix_env_set_errno (env, errno);
		return (-1);
	}

	return (0);
}

int
nix_utimes(char const *path, struct nix_timeval const *times, nix_env_t *env)
{
	struct timeval ntimes[2];
  
	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "path='%s', times={ %llu, %llu }", path, times->tv_sec, times->tv_usec);

	if (path == NULL || times == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	ntimes[0].tv_sec  = times[0].tv_sec;
	ntimes[0].tv_usec = times[0].tv_usec;
	ntimes[1].tv_sec  = times[1].tv_sec;
	ntimes[1].tv_usec = times[1].tv_usec;

	if (utimes(path, ntimes) != 0) {
		nix_env_set_errno (env, errno);
		return (-1);
	}

	return (0);
}

long
nix_pathconf(char const *path, int name, nix_env_t *env)
{
	long rv;

	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "path='%s', name=%d", path, name);

	if (path == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
    }

	// XXX should convert `name'
	if ((rv = pathconf (path, name)) < 0) {
		nix_env_set_errno (env, errno);
		return (-1);
	}
  
	return (rv);
}
