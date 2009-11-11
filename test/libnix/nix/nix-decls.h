#ifndef __nix_decls_h
#define __nix_decls_h

#include "nix-env.h"

/* nix-init.c */
void nix_init(size_t nfds, size_t nsigs);
/* nix-xcpt.c */
void nix_xcpt_record(jmp_buf *jb);
void nix_xcpt_forget(void);
/* nix-env.c */
nix_env_t *nix_env_create(xec_mem_if_t *mem);
void nix_env_set_errno(nix_env_t *env, intmax_t error);
intmax_t nix_env_get_errno(nix_env_t const *env);
xec_mem_if_t *nix_env_get_memory(nix_env_t const *env);
void nix_env_set_memory(nix_env_t *env, xec_mem_if_t *memif);
/* nix-common.c */
int nix_nosys(nix_env_t *env);
/* nix-fd.c */
int nix_fd_init(size_t count);
int nix_fd_alloc(int fd, nix_env_t *env);
int nix_fd_alloc_at(int gfd, int fd, nix_env_t *env);
int nix_fd_release(int fd, nix_env_t *env);
int nix_fd_get(int fd);
int nix_fd_get_nearest(nix_env_t *env, int fd, int dir);
int nix_getdtablesize(void);
/* nix-file.c */
nix_mode_t nix_umask(nix_mode_t mode, nix_env_t *env);
int nix_open(char const *path, int flags, int mode, nix_env_t *env);
int nix_access(char const *path, int mode, nix_env_t *env);
int nix_creat(char const *path, int mode, nix_env_t *env);
int nix_fsync(int fd, nix_env_t *env);
nix_off_t nix_lseek(int fd, nix_off_t offset, int whence, nix_env_t *env);
int nix_ftruncate(int fd, nix_off_t off, nix_env_t *env);
nix_ssize_t nix_pread(int fd, void *buf, size_t bufsiz, nix_off_t offset, nix_env_t *env);
nix_ssize_t nix_pwrite(int fd, void const *buf, size_t bufsiz, nix_off_t offset, nix_env_t *env);
int nix_flock(int fd, int op, nix_env_t *env);
int nix_fstat(int fd, struct nix_stat *sb, nix_env_t *env);
int nix_fchmod(int fd, int mode, nix_env_t *env);
int nix_fchown(int fd, nix_uid_t uid, nix_gid_t gid, nix_env_t *env);
int nix_futimes(int fd, struct nix_timeval const *times, nix_env_t *env);
long nix_fpathconf(int fd, int name, nix_env_t *env);
int nix_truncate(char const *path, nix_off_t off, nix_env_t *env);
int nix_stat(char const *path, struct nix_stat *sb, nix_env_t *env);
int nix_lstat(char const *path, struct nix_stat *sb, nix_env_t *env);
int nix_mknod(char const *path, int mode, nix_dev_t dev, nix_env_t *env);
int nix_mkfifo(char const *path, nix_mode_t mode, nix_env_t *env);
int nix_chmod(char const *path, int mode, nix_env_t *env);
int nix_chown(char const *path, nix_uid_t uid, nix_gid_t gid, nix_env_t *env);
int nix_lchown(char const *path, nix_uid_t uid, nix_gid_t gid, nix_env_t *env);
int nix_link(char const *name1, char const *name2, nix_env_t *env);
int nix_rename(char const *name1, char const *name2, nix_env_t *env);
int nix_unlink(char const *name, nix_env_t *env);
int nix_symlink(char const *name1, char const *name2, nix_env_t *env);
int nix_readlink(char const *path, char *buf, int bufsiz, nix_env_t *env);
int nix_utimes(char const *path, struct nix_timeval const *times, nix_env_t *env);
long nix_pathconf(char const *path, int name, nix_env_t *env);
/* nix-io.c */
int nix_sync(nix_env_t *env);
int nix_dup(int oldd, nix_env_t *env);
int nix_dup2(int oldd, int newd, nix_env_t *env);
int nix_close(int fd, nix_env_t *env);
nix_ssize_t nix_read(int fd, void *buf, size_t bufsiz, nix_env_t *env);
nix_ssize_t nix_readv(int fd, struct nix_iovec const *iov, int iovcnt, nix_env_t *env);
nix_ssize_t nix_write(int fd, void const *buf, size_t bufsiz, nix_env_t *env);
nix_ssize_t nix_writev(int fd, struct nix_iovec const *iov, int iovcnt, nix_env_t *env);
int nix_pipe(int *fds, nix_env_t *env);
int nix_ioctl(int fd, unsigned long request, void *data, nix_env_t *env);
int nix_fcntl(int fd, int cmd, int arg, nix_env_t *env);
int nix_select(int highestfd, nix_fd_set *rfds, nix_fd_set *wfds, nix_fd_set *xfds, struct nix_timeval const *tv, nix_env_t *env);
int nix_poll(struct nix_pollfd *fds, nix_nfds_t nfds, int timeout, nix_env_t *env);
/* nix-process.c */
void nix_exit(int exitcode);
nix_pid_t nix_getpid(nix_env_t *env);
nix_pid_t nix_getppid(nix_env_t *env);
nix_pid_t nix_fork(nix_env_t *env);
nix_pid_t nix_vfork(nix_env_t *env);
int nix_execve(char const *path, char const **argv, char const **envp, nix_env_t *env);
int nix_ptrace(int request, nix_pid_t pid, void *address, int data, nix_env_t *env);
int nix_getpriority(int which, int who, nix_env_t *env);
int nix_setpriority(int which, int who, int prio, nix_env_t *env);
int nix_getrusage(int who, struct nix_rusage *rusage, nix_env_t *env);
int nix_getrlimit(int resource, struct nix_rlimit *rlim, nix_env_t *env);
int nix_setrlimit(int resource, struct nix_rlimit *rlim, nix_env_t *env);
nix_pid_t nix_wait(nix_pid_t wpid, nix_env_t *env);
nix_pid_t nix_waitpid(nix_pid_t wpid, int options, nix_env_t *env);
nix_pid_t nix_wait3(nix_pid_t wpid, int *status, int options, nix_env_t *env);
int nix_nice(int incr, nix_env_t *env);
/* nix-time.c */
int nix_gettimeofday(struct nix_timeval *tp, struct nix_timezone *tzp, nix_env_t *env);
int nix_settimeofday(struct nix_timeval const *tp, struct nix_timezone const *tzp, nix_env_t *env);
int nix_nanosleep(struct nix_timespec const *rqtp, struct nix_timespec *rmtp, nix_env_t *env);
int nix_time(nix_time_t *tp, nix_env_t *env);
int nix_stime(nix_time_t t, nix_env_t *env);
/* nix-signal.c */
int nix_signal_init(size_t count);
int nix_kill(nix_pid_t pid, int signo, nix_env_t *env);
int nix_killpg(nix_pid_t pgrp, int signo, nix_env_t *env);
int nix_sigaction(int signo, struct nix_sigaction const *sa, struct nix_sigaction *osa, nix_env_t *env);
int nix_sigaltstack(int signo, struct nix_sigaltstack const *ss, struct nix_sigaltstack *oss, nix_env_t *env);
int nix_sigprocmask(int how, nix_sigset_t const *set, nix_sigset_t *oset, nix_env_t *env);
int nix_sigsuspend(nix_sigset_t const *set, nix_env_t *env);
int nix_sigpending(nix_sigset_t *set, nix_env_t *env);
int nix_sigtimedwait(nix_sigset_t const *set, nix_siginfo_t *info, struct nix_timespec const *timeout, nix_env_t *env);
int nix_setitimer(int timer, struct nix_itimerval const *value, struct nix_itimerval *ovalue, nix_env_t *env);
int nix_getitimer(int timer, struct nix_itimerval *value, nix_env_t *env);
int nix_alarm(nix_time_t secs, nix_env_t *env);
int nix_pause(nix_env_t *env);
uintmax_t nix_signal(int signo, uintmax_t handler, nix_env_t *env);
/* nix-dir.c */
int nix_fchdir(int fd, nix_env_t *env);
int nix_chdir(char const *path, nix_env_t *env);
int nix_mkdir(char const *path, nix_mode_t mode, nix_env_t *env);
int nix_rmdir(char const *path, nix_env_t *env);
int nix_chroot(char const *root, nix_env_t *env);
int nix_getcwd(char *buf, size_t bufsiz, nix_env_t *env);
/* nix-cred.c */
nix_pid_t nix_getpgrp(nix_env_t *env);
nix_pid_t nix_getpgid(nix_pid_t pid, nix_env_t *env);
int nix_setpgid(nix_pid_t pid, nix_pid_t pgrp, nix_env_t *env);
nix_pid_t nix_setsid(nix_env_t *env);
nix_pid_t nix_getsid(nix_pid_t pid, nix_env_t *env);
int nix_getuid(nix_env_t *env);
int nix_setuid(nix_uid_t uid, nix_env_t *env);
int nix_getgid(nix_env_t *env);
int nix_setgid(nix_gid_t gid, nix_env_t *env);
int nix_geteuid(nix_env_t *env);
int nix_seteuid(nix_uid_t uid, nix_env_t *env);
int nix_getegid(nix_env_t *env);
int nix_setegid(nix_gid_t gid, nix_env_t *env);
int nix_getreuid(nix_env_t *env);
int nix_setreuid(nix_uid_t uid, nix_uid_t euid, nix_env_t *env);
int nix_setregid(nix_gid_t gid, nix_gid_t egid, nix_env_t *env);
int nix_reboot(int howto, nix_env_t *env);
/* nix-hostinfo.c */
int nix_gethostname(char *buf, size_t bufsiz, nix_env_t *env);
int nix_sethostname(char const *hostname, size_t len, nix_env_t *env);
int nix_getdomainname(char *buf, size_t bufsiz, nix_env_t *env);
int nix_setdomainname(char const *domainname, size_t len, nix_env_t *env);
/* nix-mem.c */
uintmax_t nix_brk(uintmax_t ptr, nix_env_t *env);
uintmax_t nix_sbrk(int incr, nix_env_t *env);
uintmax_t nix_sstk(int incr, nix_env_t *env);
xec_gaddr_t nix_mmap(xec_gaddr_t gaddr, size_t len, int prot, int flags, int fd, off_t offset, nix_env_t *env);
int nix_munmap(uintmax_t addr, size_t len, nix_env_t *env);
int nix_mlock(uintmax_t addr, size_t len, nix_env_t *env);
int nix_munlock(uintmax_t addr, size_t len, nix_env_t *env);
int nix_msync(uintmax_t addr, size_t len, int flags, nix_env_t *env);
int nix_mlockall(int flags, nix_env_t *env);
int nix_munlockall(nix_env_t *env);
int nix_mprotect(uintmax_t addr, size_t len, int prot, nix_env_t *env);
int nix_madvise(uintmax_t addr, size_t len, int flags, nix_env_t *env);
int nix_mincore(uintmax_t addr, size_t len, char *vec, nix_env_t *env);
int nix_minherit(uintmax_t addr, size_t len, int inherit, nix_env_t *env);
/* nix-socket.c */
int nix_socket(int family, int type, int protocol, nix_env_t *env);
int nix_socketpair(int family, int type, int protocol, int *sv, nix_env_t *env);
int nix_connect(int fd, struct nix_sockaddr const *sa, size_t salen, nix_env_t *env);
int nix_bind(int fd, struct nix_sockaddr const *sa, size_t salen, nix_env_t *env);
int nix_accept(int fd, struct nix_sockaddr *sa, nix_socklen_t *salen, nix_env_t *env);
int nix_listen(int fd, int backlog, nix_env_t *env);
int nix_shutdown(int fd, int how, nix_env_t *env);
int nix_getpeereid(int fd, nix_uid_t *euid, nix_gid_t *egid, nix_env_t *env);
int nix_getpeername(int fd, struct nix_sockaddr *sa, nix_socklen_t *salen, nix_env_t *env);
int nix_getsockname(int fd, struct nix_sockaddr *sa, nix_socklen_t *salen, nix_env_t *env);
nix_ssize_t nix_recvfrom(int fd, void *buf, size_t len, int flags, struct nix_sockaddr *sa, nix_socklen_t *salen, nix_env_t *env);
nix_ssize_t nix_sendto(int fd, void const *buf, size_t len, int flags, struct nix_sockaddr const *sa, nix_socklen_t salen, nix_env_t *env);
/* nix-bsd-file.c */
int nix_bsd_revoke(char const *path, nix_env_t *env);
int nix_bsd_preadv(int fd, struct nix_iovec const *iov, int iovcnt, nix_off_t offset, nix_env_t *env);
int nix_bsd_pwritev(int fd, struct nix_iovec const *iov, int iovcnt, nix_off_t offset, nix_env_t *env);
int nix_bsd_fchflags(int fd, int flags, nix_env_t *env);
int nix_bsd_chflags(char const *path, int flags, nix_env_t *env);
/* nix-linux-file.c */
int nix_linux_fdatasync(int fd, nix_env_t *env);
/* nix-rt-time.c */
int nix_rt_clock_gettime(nix_clockid_t clockid, struct nix_timespec *tp, nix_env_t *env);
int nix_rt_clock_settime(nix_clockid_t clockid, struct nix_timespec const *tp, nix_env_t *env);
int nix_rt_clock_getres(nix_clockid_t clockid, struct nix_timespec *tp, nix_env_t *env);
/* nix-rt-process.c */
int nix_rt_sched_yield(nix_env_t *env);
int nix_rt_sched_setparam(nix_pid_t pid, struct nix_rt_sched_param const *param, nix_env_t *env);
int nix_rt_sched_getparam(nix_pid_t pid, struct nix_rt_sched_param *param, nix_env_t *env);
int nix_rt_sched_setscheduler(nix_pid_t pid, int policy, struct nix_rt_sched_param const *param, nix_env_t *env);
int nix_rt_sched_getscheduler(nix_pid_t pid, int policy, struct nix_rt_sched_param *param, nix_env_t *env);
int nix_rt_sched_get_priority_min(int policy, nix_env_t *env);
int nix_rt_sched_get_priority_max(int policy, nix_env_t *env);
int nix_rt_sched_rr_get_interval(nix_pid_t pid, struct nix_timespec *interval, nix_env_t *env);
/* nix-bsd-io.c */
int nix_bsd_closefrom(int fd, nix_env_t *env);
int nix_bsd_kqueue(nix_env_t *env);
int nix_bsd_kevent(int kq, struct nix_bsd_kevent const *changelist, int nchanges, struct nix_bsd_kevent *eventlist, int nevents, struct nix_timespec const *timeout, nix_env_t *env);
/* nix-bsd-signal.c */
int nix_bsd_sigblock(int mask, nix_env_t *env);
int nix_bsd_sigsetmask(int mask, nix_env_t *env);
int nix_bsd_sigvec(int signo, struct nix_bsd_sigvec const *sv, struct nix_bsd_sigvec *osv, nix_env_t *env);
int nix_bsd_sigstack(int signo, struct nix_bsd_sigstack const *ss, struct nix_bsd_sigstack *oss, nix_env_t *env);
int nix_bsd_sigreturn(struct nix_bsd_sigcontext *ctx, nix_env_t *env);
/* nix-linux-signal.c */
int nix_linux_sgetmask(nix_env_t *env);
int nix_linux_ssetmask(int sigmask, nix_env_t *env);
int nix_linux_sigqueueinfo(nix_pid_t pid, int signo, nix_siginfo_t *info, nix_env_t *env);
/* nix-bsd-process.c */
int nix_bsd_rfork(int flags, nix_env_t *env);
int nix_bsd_ktrace(char const *trfile, int ops, int tracefile, nix_pid_t pid, nix_env_t *env);
int nix_bsd_profil(char *samples, size_t size, unsigned long offset, unsigned int scale, nix_env_t *env);
nix_pid_t nix_bsd_wait4(nix_pid_t wpid, int *status, int options, struct nix_rusage *rusage, nix_env_t *env);
/* nix-linux-process.c */
int nix_linux_clone(uintmax_t sp, int flags, nix_env_t *env);
int nix_linux_idle(nix_env_t *env);
int nix_linux_uselib(char const *path, nix_env_t *env);
int nix_linux_capget(nix_linux_cap_user_handler_t handler, nix_linux_cap_user_data_t data, nix_env_t *env);
int nix_linux_capset(nix_linux_cap_user_handler_t handler, nix_linux_cap_user_data_t const data, nix_env_t *env);
int nix_linux_personality(uintmax_t personality, nix_env_t *env);
int nix_linux_prctl(int option, uintmax_t arg2, uintmax_t arg3, uintmax_t arg4, uintmax_t arg5, nix_env_t *env);
/* nix-bsd-time.c */
int nix_bsd_adjtime(struct nix_timeval const *delta, struct nix_timeval *odelta, nix_env_t *env);
int nix_bsd_adjfreq(struct nix_timeval const *delta, struct nix_timeval *odelta, nix_env_t *env);
/* nix-linux-time.c */
int nix_linux_adjtimex(struct nix_linux_timex *buf, nix_env_t *env);
/* nix-bsd-cred.c */
int nix_bsd_issetugid(nix_env_t *env);
int nix_bsd_getlogin(char *buf, size_t bufsiz, nix_env_t *env);
int nix_bsd_setlogin(char const *name, nix_env_t *env);
int nix_bsd_acct(char const *file, nix_env_t *env);
int nix_bsd_getgroups(int gidsetlen, nix_gid_t *gids, nix_env_t *env);
int nix_bsd_setgroups(int gidsetlen, nix_gid_t const *gids, nix_env_t *env);
/* nix-linux-cred.c */
nix_uid_t nix_linux_setfsuid(nix_uid_t uid, nix_env_t *env);
nix_uid_t nix_linux_setfsgid(nix_gid_t gid, nix_env_t *env);
/* nix-hpux-cred.c */
int nix_hpux_getresuid(nix_uid_t *ruid, nix_uid_t *euid, nix_uid_t *suid, nix_env_t *env);
int nix_hpux_setresuid(nix_uid_t ruid, nix_uid_t euid, nix_uid_t suid, nix_env_t *env);
int nix_hpux_getresgid(nix_gid_t *rgid, nix_gid_t *egid, nix_gid_t *sgid, nix_env_t *env);
int nix_hpux_setresgid(nix_gid_t rgid, nix_gid_t egid, nix_gid_t sgid, nix_env_t *env);
/* nix-bsd-mem.c */
uintmax_t nix_bsd_mquery(uintmax_t addr, size_t len, int prot, int flags, int fd, off_t offset, nix_env_t *env);
int nix_bsd_vadvise(int flags, nix_env_t *env);
/* nix-linux-mem.c */
uintmax_t nix_linux_mremap(uintmax_t oldaddr, size_t oldsize, size_t newsize, int flags, nix_env_t *env);
/* nix-bsd-fs.c */
int nix_bsd_fstatfs(int fd, struct nix_statfs *buf, nix_env_t *env);
int nix_bsd_statfs(char const *path, struct nix_statfs *buf, nix_env_t *env);
int nix_bsd_getfsstat(struct nix_statfs *buf, size_t bufsiz, int flags, nix_env_t *env);
int nix_bsd_mount(char const *type, char const *dir, int flags, void *data, nix_env_t *env);
int nix_bsd_unmount(char const *dir, int flags, nix_env_t *env);
/* nix-linux-fs.c */
int nix_linux_ustat(nix_dev_t dev, struct nix_linux_ustat *buf, nix_env_t *env);
int nix_linux_mount(char const *source, char const *target, char const *filesystemtype, uint32_t mountflags, void const *data, nix_env_t *env);
/* nix-bsd-system.c */
int nix_bsd_sysarch(int number, void *args, nix_env_t *env);
int nix_bsd_quotactl(char const *path, int cmd, nix_uid_t uid, void *arg, nix_env_t *env);
int nix_bsd_swapctl(int cmd, void const *arg, int misc, nix_env_t *env);
int nix_bsd_getkerninfo(int op, char *where, size_t *size, int arg, nix_env_t *env);
int nix_bsd_quota(char const *name, unsigned int current, unsigned int max, nix_env_t *env);
int nix_bsd_swapon(char const *name, nix_env_t *env);
int nix_bsd_gethostid(char *buf, size_t bufsiz, nix_env_t *env);
int nix_bsd_sethostid(char const *hostid, nix_env_t *env);
/* nix-linux-system.c */
int nix_linux_create_module(char const *name, int flags, nix_env_t *env);
int nix_linux_init_module(char const *name, struct nix_linux_module *module, nix_env_t *env);
int nix_linux_delete_module(char const *name, nix_env_t *env);
int nix_linux_get_kernel_sym(struct nix_linux_kernel_sym *ksym, nix_env_t *env);
int nix_linux_bdflush(int func, uintmax_t value, nix_env_t *env);
int nix_linux_sysfs(int a, uint32_t b, uint32_t c, nix_env_t *env);
int nix_linux_syslog(int type, char *bufp, size_t len, nix_env_t *env);
int nix_linux_sysinfo(struct nix_linux_sysinfo *si, nix_env_t *env);
int nix_linux_swapoff(char const *path, nix_env_t *env);
/* nix-bsd-obsolete.c */
int nix_bsd_vread(int fd, struct nix_iovec const *iov, int iovcnt, nix_env_t *env);
int nix_bsd_vwrite(int fd, struct nix_iovec const *iov, int iovcnt, nix_env_t *env);
int nix_bsd_vhangup(nix_env_t *env);
int nix_bsd_vlimit(nix_env_t *env);
int nix_bsd_vtimes(nix_env_t *env);
int nix_bsd_vtrace(nix_env_t *env);
int nix_bsd_resuba(nix_env_t *env);
/* nix-s5-msg.c */
int nix_s5_msgget(nix_key_t key, int msgflg, nix_env_t *env);
int nix_s5_msgsnd(int msqid, void const *msgp, size_t msgsz, int msgflg, nix_env_t *env);
int nix_s5_msgrcv(int msqid, void *msgp, size_t *msgsz, long msgtyp, int msgflg, nix_env_t *env);
int nix_s5_msgctl(int msqid, int cmd, void *arg, nix_env_t *env);
/* nix-s5-sem.c */
int nix_s5_semget(nix_key_t key, int nsems, int flag, nix_env_t *env);
int nix_s5_semop(int semid, struct nix_sembuf *buf, size_t nops, nix_env_t *env);
int nix_s5_semctl(int semid, int semnum, int cmd, void *arg, nix_env_t *env);
/* nix-s5-shm.c */
int nix_s5_shmget(nix_key_t key, size_t size, int shmflg, nix_env_t *env);
uintmax_t nix_s5_shmat(int shmid, uintmax_t shmaddr, int shmflg, nix_env_t *env);
int nix_s5_shmdt(uintmax_t shmaddr, nix_env_t *env);
int nix_s5_shmctl(int shmid, int cmd, void *arg, nix_env_t *env);
/* nix-bsd-nfs.c */
int nix_bsd_nfssvc(int flags, void *arg, nix_env_t *env);
int nix_bsd_getfh(char const *path, nix_fhandle_t *fh, nix_env_t *env);
int nix_bsd_fhstatfs(nix_fhandle_t const *fhp, struct nix_statfs *buf, nix_env_t *env);
int nix_bsd_fhstat(nix_fhandle_t const *fhp, struct nix_stat *buf, nix_env_t *env);
int nix_bsd_fhopen(nix_fhandle_t const *fhp, int flags, nix_env_t *env);
/* nix-linux-nfs.c */
int nix_linux_nfsservctl(int cmd, struct nix_linux_nfsctl_arg *arg, union nix_linux_nfsctl_res *resp, nix_env_t *env);
/* nix-linux-i386.c */
int nix_linux_i386_ioperm(uint32_t from, uint32_t num, int on, nix_env_t *env);
int nix_linux_i386_iopl(uint32_t perm, nix_env_t *env);
int nix_linux_i386_vm86(uint32_t cmd, void *arg, nix_env_t *env);
int nix_linux_i386_modify_ldt(int ldtno, void *arg, uint32_t val, nix_env_t *env);

#endif  /* !__nix_h */
