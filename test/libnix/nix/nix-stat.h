#ifndef __nix_stat_h
#define __nix_stat_h

#include "nix-types.h"

/* nix_bsd_getfsstat */
#define NIX_MNT_WAIT   0
#define NIX_MNT_NOWAIT 1

/* nix_bsd_[f]statfs */
#define NIX_MFSNAMELEN 32
#define NIX_MNAMELEN   128

struct nix_statfs {
	uint32_t   f_type;
	uint64_t   f_flags;
	uint64_t   f_bsize;
	uint64_t   f_iosize;
	uint64_t   f_blocks;
	uint64_t   f_bfree;
	int64_t    f_bavail;
	uint64_t   f_files;
	int64_t    f_ffree;
	uint64_t   f_syncwrites;
	uint64_t   f_asyncwrites;
	uint64_t   f_syncreads;
	uint64_t   f_asyncreads;
	uint32_t   f_namemax;
	nix_uid_t  f_owner;
	nix_fsid_t f_fsid;
	struct nix_timespec f_ctime;
	char       f_fstypename[NIX_MFSNAMELEN];
	char       f_mntfromname[NIX_MNAMELEN];
	char       f_mntonname[NIX_MNAMELEN];
};

#define	NIX_S_IFMT		0170000
#define	NIX_S_IFIFO		0010000
#define	NIX_S_IFCHR		0020000
#define	NIX_S_IFDIR		0040000
#define	NIX_S_IFBLK		0060000
#define	NIX_S_IFREG		0100000
#define	NIX_S_IFLNK		0120000
#define	NIX_S_IFSOCK	0140000
#define	NIX_S_IFWHT		0160000

#define	NIX_S_ISUID		0004000
#define	NIX_S_ISGID		0002000
#define	NIX_S_ISVTX		0001000
#define	NIX_S_ISTXT		NIX_S_ISVTX

#endif  /* !__nix_stat_h */
