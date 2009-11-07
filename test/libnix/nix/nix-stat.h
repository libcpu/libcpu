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

#endif  /* !__nix_stat_h */
