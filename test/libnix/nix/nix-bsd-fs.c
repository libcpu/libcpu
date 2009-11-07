#include <sys/param.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/time.h>
#ifdef __linux__
#include <sys/vfs.h>
#endif
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include "nix.h"
#include "xec-mem.h"

static void
cvt_statfs(struct statfs const *native,
	struct nix_statfs *dest)
{
	memset(dest, 0, sizeof(*dest));

#ifdef __OpenBSD__
	dest->f_type   = 0;
#else
	dest->f_type   = native->f_type;
#endif
	dest->f_bsize  = native->f_bsize;
	dest->f_blocks = native->f_blocks;
	dest->f_bfree  = native->f_bfree;
	dest->f_bavail = native->f_bavail;
	dest->f_files  = native->f_files;
	dest->f_ffree  = native->f_ffree;
#ifdef __linux__
	/* FIXME: Find a way to fill these fields. */
	dest->f_fsid   = 0;
	dest->f_namemax = 0;
	dest->f_iosize = 0;
	dest->f_syncwrites = 0;
	dest->f_asyncwrites = 0;
	dest->f_syncreads = 0;
	dest->f_asyncreads = 0;
	dest->f_owner = 0;
	dest->f_type = 0;
#if 0
	switch (dest->f_type) {
    	case EXT3_SUPER_MAGIC:
			strcpy(dest->f_fstypename, "ext3");
			break;
		case EXT2_OLD_SUPER_MAGIC:
		case EXT2_SUPER_MAGIC:
			strcpy(dest->f_fstypename, "ext2");
			break;
		case EXT_SUPER_MAGIC:
			strcpy(dest->f_fstypename, "ext");
			break;
		case ISOFS_SUPER_MAGIC:
			strcpy(dest->f_fstypename, "isofs");
			break;
		case MINIX_SUPER_MAGIC:
		case MINIX2_SUPER_MAGIC:
		case NEW_MINIX_SUPER_MAGIC:
			strcpy(dest->f_fstypename, "minix");
			break;
		case MSDOS_SUPER_MAGIC:
			strcpy(dest->f_fstypename, "msdos");
			break;
		case NFS_SUPER_MAGIC:
			strcpy(dest->f_fstypename, "nfs");
			break;
		case PROC_SUPER_MAGIC:
			strcpy(dest->f_fstypename, "proc");
			break;
		case BFS_SUPER_MAGIC:
			strcpy(dest->f_fstypename, "bfs");
			break;
		case _XIAFS_SUPER_MAGIC:
			strcpy(dest->f_fstypename, "xiafs");
			break;
		case JFS_SUPER_MAGIC:
			strcpy (dest->f_fstypename, "jfs");
			break;
		case JFFS_SUPER_MAGIC:
			strcpy (dest->f_fstypename, "jffs");
			break;
		case JFFS2_SUPER_MAGIC:
			strcpy (dest->f_fstypename, "jffs2");
			break;
		case XFS_SUPER_MAGIC:
			strcpy (dest->f_fstypename, "xfs");
			break;
		case UFS_SUPER_MAGIC:
			strcpy(dest->f_fstypename, "ufs");
			break;
		case AFFS_SUPER_MAGIC:
			strcpy(dest->f_fstypename, "affs");
			break;
		case ADFS_SUPER_MAGIC:
			strcpy(dest->f_fstypename, "adfs");
			break;
		case HFS_SUPER_MAGIC:
			strcpy(dest->f_fstypename, "hfs");
			break;
		case HFSPLUS_SUPER_MAGIC:
			strcpy(dest->f_fstypename, "hfsplus");
			break;
		case HPFS_SUPER_MAGIC:
			strcpy(dest->f_fstypename, "hpfs");
			break;
		case NTFS_SUPER_MAGIC:
			strcpy(dest->f_fstypename, "ntfs");
			break;
		case REISERFS_SUPER_MAGIC:
			strcpy(dest->f_fstypename, "reiser");
			break;
		case REISER4FS_SUPER_MAGIC:
			strcpy(dest->f_fstypename, "reiser4");
			break;
		default:
			strcpy(dest->f_fstypename, "unknown");
			break;
	}
#endif
	/* XXX Fill in mount paths! */

#else
	dest->f_fsid   = ((uint64_t)native->f_fsid.val[0] << 32ULL) | native->f_fsid.val[1];
	dest->f_iosize = native->f_iosize;
#if defined (__OpenBSD__)
	dest->f_ctime.tv_sec  = native->f_ctime;
	dest->f_ctime.tv_nsec = 0;
#else
	dest->f_ctime.tv_sec  = 0;
	dest->f_ctime.tv_nsec = 0;
#endif
#if defined (__FreeBSD__) || defined (__OpenBSD__)
	dest->f_syncwrites = native->f_syncwrites;
	dest->f_asyncwrites = native->f_asyncwrites;
#else
	dest->f_syncwrites = 0;
	dest->f_asyncwrites = 0;
#endif
#ifdef __FreeBSD__
	dest->f_syncreads = native->f_syncreads;
	dest->f_asyncreads = native->f_asyncreads;
#else
	dest->f_syncreads = 0;
	dest->f_asyncreads = 0;
#endif
	dest->f_owner = native->f_owner;
	strncpy(dest->f_mntfromname, native->f_mntfromname, sizeof(dest->f_mntfromname));
	strncpy(dest->f_mntonname, native->f_mntonname, sizeof(dest->f_mntonname));
#endif
}

int
nix_bsd_fstatfs(int fd, struct nix_statfs *buf, nix_env_t *env)
{
	struct statfs nsfs;
	int           rfd;

	if (buf == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	if ((rfd = nix_fd_get(fd)) < 0) {
		nix_env_set_errno (env, EBADF);
		return (-1);
	}

	if (fstatfs(rfd, &nsfs) != 0) {
		nix_env_set_errno (env, errno);
		return (-1);
	}

	__nix_try
	{
		cvt_statfs (&nsfs, buf);
	}
	__nix_catch_any
	{
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}
	__nix_end_try

	return (0);
}

int
nix_bsd_statfs(char const *path, struct nix_statfs *buf, nix_env_t *env)
{
	struct statfs nsfs;

	if (path == NULL || buf == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	if (statfs(path, &nsfs) != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	__nix_try
	{
		cvt_statfs(&nsfs, buf);
	}
	__nix_catch_any
	{
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}
	__nix_end_try

	return (0);
}

int
nix_bsd_getfsstat(struct nix_statfs *buf, size_t bufsiz, int flags,
	nix_env_t *env)
{
	struct statfs *nfs;
	int            n, maxcount;
	int            nflags = 0;
	int            count = 0;

#ifndef __linux__
	if (flags == NIX_MNT_NOWAIT)
		nflags = MNT_NOWAIT;
#endif
  
	if (buf == NULL) {
		count = getfsstat(NULL, 0, nflags);
		if (count < 0) {
			nix_env_set_errno(env, errno);
			return (-1);
		}
      
		return (count);
	}

	maxcount = bufsiz / sizeof(struct nix_statfs);
	if (maxcount == 0) {
		nix_env_set_errno(env, EINVAL);
		return (-1);
	}
  
	nfs = xec_mem_alloc_ntype(struct statfs, maxcount, 0);
	if (nfs == NULL) {
		nix_env_set_errno(env, ENOMEM);
		return (-1);
	}

	if ((count = getfsstat(nfs, maxcount * sizeof(struct statfs),
		  nflags)) < 0) {
		xec_mem_free(nfs);
		nix_env_set_errno(env, errno);
		return (-1);
	}
  
	for (n = 0; n < count; n++)
		cvt_statfs(&nfs[n], &buf[n]);

	xec_mem_free(nfs);

	return (count);
}

int
nix_bsd_mount(char const *type, char const *dir, int flags, void *data,
	nix_env_t *env)
{
	return (nix_nosys(env));
}

int
nix_bsd_unmount(char const *dir, int flags, nix_env_t *env)
{
	return (nix_nosys(env));
}
