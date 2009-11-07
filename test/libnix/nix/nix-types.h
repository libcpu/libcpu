#ifndef __nix_types_h
#define __nix_types_h

#include "xec-base.h"

typedef uint32_t nix_dev_t;
typedef uint32_t nix_id_t;
typedef uint32_t nix_pid_t;
typedef uint32_t nix_uid_t;
typedef uint32_t nix_gid_t;
typedef uint32_t nix_clockid_t;
typedef uint32_t nix_mode_t;
typedef uint32_t nix_nfds_t;
typedef uint32_t nix_key_t;
typedef uint64_t nix_ino_t;
typedef uint64_t nix_fsid_t;
typedef uint64_t nix_time_t;
typedef uint64_t nix_fhandle_t;
typedef int64_t  nix_off_t;
typedef uint32_t nix_nlink_t;
typedef uint64_t nix_blkcnt_t;
typedef uint64_t nix_blksize_t;
typedef ssize_t  nix_ssize_t;

typedef uint32_t nix_fd_mask_t;

typedef struct nix_fd_set {
	nix_fd_mask_t fds_bits[256 >> 3]; /* FD_SETSIZE */
} nix_fd_set;

struct nix_timeval {
	nix_time_t tv_sec;
	uint64_t   tv_usec;
};

struct nix_timespec {
	nix_time_t tv_sec;
	uint64_t   tv_nsec;
};

struct nix_timezone {
	int32_t tz_minuteswest;
	int32_t tz_dsttime;
};

struct nix_iovec {
	void   *iov_base;
	size_t  iov_len;
};

struct nix_stat {
	nix_dev_t   st_dev;
	nix_ino_t   st_ino;
	nix_mode_t  st_mode;
	nix_nlink_t st_nlink;
	nix_uid_t   st_uid;
	nix_gid_t   st_gid;
	nix_dev_t   st_rdev;
	struct nix_timespec st_atimespec;
	struct nix_timespec st_mtimespec;
	struct nix_timespec st_ctimespec;
	struct nix_timespec st_btimespec;
	nix_off_t   st_size;
	nix_blkcnt_t st_blocks;
	nix_blksize_t st_blksize;
	uint64_t   st_flags;
	uint64_t   st_gen;
};

#define NIX_CLOCK_REALTIME  0
#define NIX_CLOCK_VIRTUAL   1
#define NIX_CLOCK_PROF      2
#define NIX_CLOCK_MONOTONIC 4

#define NIX_POLLIN      0x0001
#define NIX_POLLPRI     0x0002
#define NIX_POLLOUT     0x0004
#define NIX_POLLERR     0x0008
#define NIX_POLLHUP     0x0010
#define NIX_POLLNVAL    0x0020
#define NIX_POLLRDNORM  0x0040
#define NIX_POLLWRNORM  NIX_POLLOUT
#define NIX_POLLRDBAND  0x0080
#define NIX_POLLWRBAND  0x0100
#define NIX_POLLEXTEND  0x0200
#define NIX_POLLATTRIB  0x0400
#define NIX_POLLNLINK   0x0800
#define NIX_POLLWRITE   0x1000

struct nix_pollfd {
	int fd;
	int events;
	int revents;
};

struct nix_sembuf {
	uint32_t sem_num;
	int32_t  sem_op;
	int32_t  sem_flg;
};

typedef struct nix_siginfo {
	int dummy;
} nix_siginfo_t;

#define NIX_AF_LOCAL 0
#define NIX_AF_UNIX  NIX_AF_LOCAL
#define NIX_AF_INET  2
#define NIX_AF_INET6 24

#define NIX_RUSAGE_SELF     (0)
#define NIX_RUSAGE_CHILDREN (-1)

struct nix_rusage {
	struct nix_timeval ru_utime;
	struct nix_timeval ru_stime;
	int64_t            ru_maxrss;
	int64_t            ru_ixrss;
	int64_t            ru_idrss;
	int64_t            ru_isrss;
	int64_t            ru_minflt;
	int64_t            ru_majflt;
	int64_t            ru_nswap;
	int64_t            ru_inblock;
	int64_t            ru_oublock;
	int64_t            ru_msgsnd;
	int64_t            ru_msgrcv;
	int64_t            ru_nsignals;
	int64_t            ru_nvcsw;
	int64_t            ru_nivcsw;
};

typedef uint64_t nix_rlim_t;

#define NIX_RLIMIT_CORE    0
#define NIX_RLIMIT_CPU     1
#define NIX_RLIMIT_DATA    2
#define NIX_RLIMIT_FSIZE   3
#define NIX_RLIMIT_MEMLOCK 4
#define NIX_RLIMIT_NOFILE  5
#define NIX_RLIMIT_NPROC   6
#define NIX_RLIMIT_RSS     7
#define NIX_RLIMIT_STACK   8

struct nix_rlimit {
	nix_rlim_t rlim_cur;
	nix_rlim_t rlim_max;
};

#endif  /* !__nix_types_h */
