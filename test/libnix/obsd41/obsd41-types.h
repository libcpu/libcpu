#ifndef __obsd41_types_h
#define __obsd41_types_h

#include "xec-base.h"
#include "obsd41-guest-types.h"

typedef int32_t  obsd41_dev_t;
typedef uint32_t obsd41_ino_t;
typedef uint32_t obsd41_mode_t;
typedef uint32_t obsd41_nlink_t;
typedef uint32_t obsd41_uid_t;
typedef uint32_t obsd41_gid_t;

typedef int64_t obsd41_off_t;

typedef struct
  {
    int32_t val[2];
  } __obsd41_guest_alignment obsd41_fsid_t;

struct obsd41_timespec
  {
    obsd41_time_t tv_sec;
    obsd41_long_t tv_nsec;
  } __obsd41_guest_alignment;

struct obsd41_timeval
  {
    obsd41_time_t tv_sec;
    obsd41_long_t tv_usec;
  } __obsd41_guest_alignment;

struct obsd41_timezone
  {
    int32_t tz_minuteswest;
    int32_t tz_dsttime;
  } __obsd41_guest_alignment;

struct obsd41_stat
  {
    obsd41_dev_t           st_dev;
    obsd41_ino_t           st_ino;
    obsd41_mode_t          st_mode;
    obsd41_nlink_t         st_nlink;
    obsd41_uid_t           st_uid;
    obsd41_gid_t           st_gid;
    obsd41_dev_t           st_rdev;
    int32_t                st_lspare0;
    struct obsd41_timespec st_atimespec;
    struct obsd41_timespec st_mtimespec;
    struct obsd41_timespec st_ctimespec;
    obsd41_off_t           st_size;
    int64_t                st_blocks;
    uint32_t               st_blksize;
    uint32_t               st_flags;
    uint32_t               st_gen;
    int32_t                st_lspare1;
    struct obsd41_timespec __st_birthtimespec;
    int64_t                st_qspare[2];
  } __obsd41_guest_alignment;

union obsd41_mount_info
  {
    char __align[160];
  } __obsd41_guest_alignment;

#define OBSD41_MNT_WAIT     1
#define OBSD41_MNT_NOWAIT   2

#define OBSD41_MFSNAMELEN   16
#define OBSD41_MNAMELEN     90

struct obsd41_statfs
  {
    uint32_t                f_flags;
    int32_t                 f_bsize;
    uint32_t                f_iosize;
    uint32_t                f_blocks;
    uint32_t                f_bfree;
    int32_t                 f_bavail;
    uint32_t                f_files;
    uint32_t                f_ffree;
    obsd41_fsid_t           f_fsid;
    obsd41_uid_t            f_owner;
    uint32_t                f_syncwrites;
    uint32_t                f_asyncwrites;
    uint32_t                f_ctime;
    uint32_t                f_spare[3];
    char                    f_fstypename[OBSD41_MFSNAMELEN];
    char                    f_mntonname[OBSD41_MNAMELEN];
    char                    f_mntfromname[OBSD41_MNAMELEN];
    union obsd41_mount_info mount_info;
  } __obsd41_guest_alignment;

struct obsd41_iovec32
  {
    uint32_t iov_base;
    uint32_t iov_len;
  } __obsd41_guest_alignment;

struct obsd41_sigaction32
  {
    uint32_t __sa_handler;
    uint32_t sa_flags;
    uint32_t sa_mask;
  } __obsd41_guest_alignment;

typedef uint32_t obsd41_sigset_t;
#define OBSD41_SIG_BLOCK   1
#define OBSD41_SIG_UNBLOCK 2
#define OBSD41_SIG_SETMASK 3

typedef int32_t  obsd41_socklen_t;
typedef uint8_t  obsd41_sa_family_t;
typedef uint16_t obsd41_in_port_t;
typedef uint32_t obsd41_in_addr_t;

struct obsd41_sockaddr
  {
    uint8_t            sa_len;
    obsd41_sa_family_t sa_family;
    char               sa_data[14];
  } __obsd41_guest_alignment;

struct obsd41_sockaddr_storage
  {
    uint8_t            ss_len;
    obsd41_sa_family_t ss_family;
    uint8_t            __ss_pad1[6];
    uint64_t           __ss_pad2;
    uint8_t            __ss_pad3[240];
  } __obsd41_guest_alignment;

struct obsd41_sockaddr_in
  {
    uint8_t            sin_len;
    obsd41_sa_family_t sin_family;
    obsd41_in_port_t   sin_port;
    uint32_t           sin_addr;
    int8_t             sin_zero[8];
  } __obsd41_guest_alignment;

struct obsd41_sockaddr_un
  {
    uint8_t            sun_len;
    obsd41_sa_family_t sun_family;
    char               sun_path[104];
  } __obsd41_guest_alignment;

#define OBSD41_RUSAGE_SELF     (0)
#define OBSD41_RUSAGE_CHILDREN (-1)

struct obsd41_rusage
  {
    struct obsd41_timeval ru_utime;
    struct obsd41_timeval ru_stime;
    obsd41_long_t         ru_maxrss;
    obsd41_long_t         ru_ixrss;
    obsd41_long_t         ru_idrss;
    obsd41_long_t         ru_isrss;
    obsd41_long_t         ru_minflt;
    obsd41_long_t         ru_majflt;
    obsd41_long_t         ru_nswap;
    obsd41_long_t         ru_inblock;
    obsd41_long_t         ru_oublock;
    obsd41_long_t         ru_msgsnd;
    obsd41_long_t         ru_msgrcv;
    obsd41_long_t         ru_nsignals;
    obsd41_long_t         ru_nvcsw;
    obsd41_long_t         ru_nivcsw;
  } __obsd41_guest_alignment;

typedef uint64_t obsd41_rlim_t;

#define OBSD41_RLIMIT_CPU     0 
#define OBSD41_RLIMIT_FSIZE   1
#define OBSD41_RLIMIT_DATA    2
#define OBSD41_RLIMIT_STACK   3
#define OBSD41_RLIMIT_CORE    4
#define OBSD41_RLIMIT_RSS     5
#define OBSD41_RLIMIT_MEMLOCK 6
#define OBSD41_RLIMIT_NPROC   7
#define OBSD41_RLIMIT_NOFILE  8

struct obsd41_rlimit
  {
    obsd41_rlim_t rlim_cur;
    obsd41_rlim_t rlim_max;
  } __obsd41_guest_alignment;

#define OBSD41_POLLIN      0x0001
#define OBSD41_POLLPRI     0x0002
#define OBSD41_POLLOUT     0x0004
#define OBSD41_POLLERR     0x0008
#define OBSD41_POLLHUP     0x0010
#define OBSD41_POLLNVAL    0x0020
#define OBSD41_POLLRDNORM  0x0040
#define OBSD41_POLLRDBAND  0x0080
#define OBSD41_POLLWRBAND  0x0100

struct obsd41_pollfd
  {
    int32_t fd;
    int16_t events;
    int16_t revents;
  } __obsd41_guest_alignment;

/* Special Control Characters */
#define OBSD41_VEOF     0
#define OBSD41_VEOL     1
#define OBSD41_VEOL2    2
#define OBSD41_VERASE   3
#define OBSD41_VWERASE  4
#define OBSD41_VKILL    5
#define OBSD41_VREPRINT 6
#define OBSD41_VINTR    8
#define OBSD41_VQUIT    9
#define OBSD41_VSUSP    10
#define OBSD41_VDSUSP   11
#define OBSD41_VSTART   12
#define OBSD41_VSTOP    13
#define OBSD41_VLNEXT   14
#define OBSD41_VDISCARD 15
#define OBSD41_VMIN     16
#define OBSD41_VTIME    17
#define OBSD41_VSTATUS  18

#define OBSD41_NCCS     20

/* Input flags */
#define OBSD41_IGNBRK   0x00000001
#define OBSD41_BRKINT   0x00000002
#define OBSD41_IGNPAR   0x00000004
#define OBSD41_PARMRK   0x00000008
#define OBSD41_INPCK    0x00000010
#define OBSD41_ISTRIP   0x00000020
#define OBSD41_INLCR    0x00000040
#define OBSD41_IGNCR    0x00000080
#define OBSD41_ICRNL    0x00000100
#define OBSD41_IXON     0x00000200
#define OBSD41_IXOFF    0x00000400
#define OBSD41_IXANY    0x00000800
#define OBSD41_IUCLC    0x00001000
#define OBSD41_IMAXBEL  0x00002000

/* Output Flags */
#define OBSD41_OPOST    0x00000001
#define OBSD41_ONLCR    0x00000002
#define OBSD41_OXTABS   0x00000004
#define OBSD41_ONOEOT   0x00000008
#define OBSD41_OCRNL    0x00000010
#define OBSD41_OLCUC    0x00000020
#define OBSD41_ONOCR    0x00000040
#define OBSD41_ONLRET   0x00000080

/* Control Flags */
#define OBSD41_CIGNORE  0x00000001
#define OBSD41_CSIZE    0x00000300
#define OBSD41_CS5      0x00000000
#define OBSD41_CS6      0x00000100
#define OBSD41_CS7      0x00000200
#define OBSD41_CS8      0x00000300
#define OBSD41_CSTOPB   0x00000400
#define OBSD41_CREAD    0x00000800
#define OBSD41_PARENB   0x00001000
#define OBSD41_PARODD   0x00002000
#define OBSD41_HUPCL    0x00004000
#define OBSD41_CLOCAL   0x00008000
#define OBSD41_CRTSCTS  0x00010000
#define OBSD41_MDMBUF   0x00100000
#define OBSD41_CHWFLOW  (OBSD41_MDMBUF | OBSD41_CRTSCTS)

/* Local Flags */
#define OBSD41_ECHOKE     0x00000001
#define OBSD41_ECHOE      0x00000002
#define OBSD41_ECHOK      0x00000004
#define OBSD41_ECHO       0x00000008
#define OBSD41_ECHONL     0x00000010
#define OBSD41_ECHOPRT    0x00000020
#define OBSD41_ECHOCTL    0x00000040
#define OBSD41_ISIG       0x00000080
#define OBSD41_ICANON     0x00000100
#define OBSD41_ALTWERASE  0x00000200
#define OBSD41_IEXTEN     0x00000400
#define OBSD41_EXTPROC    0x00000800
#define OBSD41_TOSTOP     0x00400000
#define OBSD41_FLUSHO     0x00800000
#define OBSD41_XCASE      0x01000000
#define OBSD41_NOKERNINFO 0x02000000
#define OBSD41_PENDIN     0x20000000
#define OBSD41_NOFLSH     0x80000000

/* Standard speeds */
#define OBSD41_B0      0
#define OBSD41_B50     50
#define OBSD41_B75     75
#define OBSD41_B110    110
#define OBSD41_B134    134
#define OBSD41_B150    150
#define OBSD41_B200    200
#define OBSD41_B300    300
#define OBSD41_B600    600
#define OBSD41_B1200   1200
#define OBSD41_B1800   1800
#define OBSD41_B2400   2400
#define OBSD41_B4800   4800
#define OBSD41_B7200   7200
#define OBSD41_B9600   9600
#define OBSD41_B14400  14400
#define OBSD41_B19200  19200
#define OBSD41_B28800  28800
#define OBSD41_B38400  38400
#define OBSD41_B57600  57600
#define OBSD41_B76800  76800
#define OBSD41_B115200 115200
#define OBSD41_B230400 230400
#define OBSD41_EXTA    19200
#define OBSD41_EXTB    38400

typedef uint32_t obsd41_tcflag_t;
typedef uint32_t obsd41_speed_t;
typedef uint8_t  obsd41_cc_t;

struct obsd41_termios
  {
    obsd41_tcflag_t c_iflag;
    obsd41_tcflag_t c_oflag;
    obsd41_tcflag_t c_cflag;
    obsd41_tcflag_t c_lflag;
    obsd41_cc_t     c_cc[OBSD41_NCCS];
    obsd41_speed_t  c_ispeed;
    obsd41_speed_t  c_ospeed;
  };

typedef uint32_t obsd41_fd_mask_t;

typedef struct obsd41_fd_set
  {
    obsd41_fd_mask_t fds_bits[256 >> 3]; /* OBSD41_FD_SETSIZE */
  } obsd41_fd_set;

#endif  /* !__obsd41_types_h */
