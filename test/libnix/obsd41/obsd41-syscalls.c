/* 
 * OpenBSD 4.1 System Calls Implementation (Reflector)
 * Copyright (C) 2007 Orlando Bassotto. All rights reserved.
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xec-mem.h"
#include "xec-byte-order.h"
#include "xec-mem-if.h"
#include "xec-debug.h"
#include "xec-us-syscall.h"
#include "xec-us-syscall-if.h"
#include "obsd41-syscalls.h"
#include "obsd41-us-syscall-priv.h"

#include "nix.h"
#include "nix-fd.h"
#include "openbsd41.h"
#include "arc4random.h"

#include "obsd41-args.h"
#include "obsd41-sysctl.h"
#include "obsd41-mman.h"

#include "xec-mmap.h" /* XXX */

#define GE32(gi, x) \
(((gi)->endian != XEC_ENDIAN_NATIVE) ? xec_byte_swap_int32(x) : (x))

void *g_bsd_log = NULL;

static __inline struct nix_iovec *
__obsd41_iovec32_copy_from(nix_env_t				*env,
						   xec_mem_if_t				*mem,
						   xec_guest_info_t const	*gi,
						   struct obsd41_iovec32	*iov,
						   size_t					 niov)
{
	struct nix_iovec *xiov = NULL;

	__nix_try
	{
		xiov = xec_mem_alloc_ntype(struct nix_iovec, niov, 0);
		if (xiov == NULL)
			nix_env_set_errno(env, ENOMEM);
		else {
			size_t n;

			for (n = 0; n < niov; n++) {
				uintptr_t	  pa;
				xec_mem_flg_t mf = 0;

				pa = xec_mem_gtoh(mem, GE32 (gi, iov[n].iov_base), &mf);
				if (mf != 0) {
					nix_env_set_errno(env, EFAULT);
					xec_mem_free(xiov);
					xiov = NULL;
					break;
				}
				xiov[n].iov_base = (void *)pa;
				xiov[n].iov_len	= GE32(gi, iov[n].iov_len);
			}
		}
	}
	__nix_catch_any
	{
		nix_env_set_errno(env, EFAULT);
		if (xiov != NULL)
			xec_mem_free(xiov);
		xiov = NULL;
	}
	__nix_end_try

	return (xiov);
}

static __inline struct nix_pollfd *
__obsd41_pollfd_copy_from(nix_env_t				 *env,
						  xec_guest_info_t const *gi,
						  struct obsd41_pollfd	 *fds,
						  size_t				  nfds)
{
	struct nix_pollfd *xfds = NULL;

	__nix_try
	{
		xfds = xec_mem_alloc_ntype(struct nix_pollfd, nfds, 0);
		if (xfds == NULL)
			nix_env_set_errno(env, ENOMEM);
		else {
			size_t n;

			for (n = 0; n < nfds; n++)
				obsd41_pollfd_to_nix_pollfd(gi->endian, fds + n, xfds + n);
		}
	}
	__nix_catch_any
	{
		nix_env_set_errno(env, EFAULT);
		if (xfds != NULL)
			xec_mem_free(xfds);
		xfds = NULL;
	}
	__nix_end_try

	return (xfds);
}

int obsd41_syscall(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_syscall_args_t const *args,
	obsd41_syscall_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	return (xec_us_syscall_redispatch(xus, xmon, args->arg0, args->ap, result));
}

int obsd41_exit(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_exit_args_t const *args,
	obsd41_exit_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_exit(args->arg0);
	return (0); /* Not reached (should)! */
}

int obsd41_fork(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_fork_args_t const *args,
	obsd41_fork_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_fork(env);
	return (nix_env_get_errno(env));
}

int obsd41_read(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_read_args_t const *args,
	obsd41_read_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_read(args->arg0, args->arg1, args->arg2, env);
	return (nix_env_get_errno(env));
}

int obsd41_write(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_write_args_t const *args,
	obsd41_write_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_write(args->arg0, args->arg1, args->arg2, env);
	return (nix_env_get_errno(env));
}

int obsd41_open(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_open_args_t const *args,
	obsd41_open_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_open((char const *)args->arg0, args->arg1, args->arg2, env);
	return (nix_env_get_errno(env));
}

int obsd41_close(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_close_args_t const *args,
	obsd41_close_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_close(args->arg0, env);
	return (nix_env_get_errno(env));
}

int obsd41_wait4(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_wait4_args_t const *args,
	obsd41_wait4_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_bsd_wait4(args->arg0, args->arg1, args->arg2, (struct nix_rusage *)(uintptr_t)args->arg3, env);
	return (nix_env_get_errno(env));
}

int obsd41_creat_compat43(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_creat_compat43_args_t const *args,
	obsd41_creat_compat43_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_creat(args->arg0, args->arg1, env);
	return (nix_env_get_errno(env));
}

int obsd41_link(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_link_args_t const *args,
	obsd41_link_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_link((char const *)args->arg0, (char const *)args->arg1, env);
	return (nix_env_get_errno(env));
}

int obsd41_unlink(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_unlink_args_t const *args,
	obsd41_unlink_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_unlink((char const *)args->arg0, env);
	return (nix_env_get_errno(env));
}

int obsd41_oexecve(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_oexecve_args_t const *args,
	obsd41_oexecve_result_t *result)
{
//	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_chdir(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_chdir_args_t const *args,
	obsd41_chdir_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_chdir((char const *)args->arg0, env);
	return (nix_env_get_errno(env));
}

int obsd41_fchdir(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_fchdir_args_t const *args,
	obsd41_fchdir_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_fchdir(args->arg0, env);
	return (nix_env_get_errno(env));
}

int obsd41_mknod(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_mknod_args_t const *args,
	obsd41_mknod_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);
	
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_mknod((char const *)args->arg0, args->arg1, args->arg2, env);
	return (nix_env_get_errno(env));
}

int obsd41_chmod(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_chmod_args_t const *args,
	obsd41_chmod_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	__nix_try
	{
		*result = nix_chmod((char const *)args->arg0, args->arg1, env);
	}
	__nix_catch_any
	{
		nix_env_set_errno(env, EFAULT);
		*result = -1;
	}
	__nix_end_try

	return (nix_env_get_errno(env));
}

int obsd41_chown(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_chown_args_t const *args,
	obsd41_chown_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	__nix_try
	{
		*result = nix_chown((char const *)args->arg0, args->arg1, args->arg2, env);
	}
	__nix_catch_any
	{
		nix_env_set_errno(env, EFAULT);
		*result = -1;
	}
	__nix_end_try

	return (nix_env_get_errno(env));
}

int obsd41_brk(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_brk_args_t const *args,
	obsd41_brk_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = (void *)(uintptr_t)nix_brk((uintmax_t)(uintptr_t)args->arg0, env);
	return (nix_env_get_errno(env));
}

int obsd41_getfsstat_compat25(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_getfsstat_compat25_args_t const *args,
	obsd41_getfsstat_compat25_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_lseek_compat43(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_lseek_compat43_args_t const *args,
	obsd41_lseek_compat43_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_lseek(args->arg0, args->arg1, args->arg2, env);
	return (nix_env_get_errno(env));
}

int obsd41_getpid(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_getpid_args_t const *args,
	obsd41_getpid_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_getpid(env);
	return (nix_env_get_errno(env));
}

int obsd41_mount(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_mount_args_t const *args,
	obsd41_mount_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_bsd_mount((char const *)args->arg0,
							(char const *)args->arg1,
							args->arg2,
							args->arg3,
							env);
	return (nix_env_get_errno(env));
}

int obsd41_umount(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_umount_args_t const *args,
	obsd41_umount_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_bsd_unmount((char const *)args->arg0, args->arg1, env);
	return (nix_env_get_errno(env));
}

int obsd41_setuid(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_setuid_args_t const *args,
	obsd41_setuid_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_setuid(args->arg0, env);
	return (nix_env_get_errno(env));
}

int obsd41_getuid(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_getuid_args_t const *args,
	obsd41_getuid_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_getuid(env);
	return (nix_env_get_errno(env));
}

int obsd41_geteuid(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_geteuid_args_t const *args,
	obsd41_geteuid_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_geteuid(env);
	return (nix_env_get_errno(env));
}

int obsd41_ptrace(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_ptrace_args_t const *args,
	obsd41_ptrace_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_ptrace(args->arg0, args->arg1, args->arg2, args->arg3, env);
	return (nix_env_get_errno(env));
}

int obsd41_recvmsg(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_recvmsg_args_t const *args,
	obsd41_recvmsg_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_sendmsg(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_sendmsg_args_t const *args,
	obsd41_sendmsg_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_recvfrom(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_recvfrom_args_t const *args,
	obsd41_recvfrom_result_t *result)
{
	xec_guest_info_t	 gi;
	struct nix_sockaddr	 sa;
	int					 rc		= -1;
	nix_socklen_t		 salen	= sizeof (sa);
	struct nix_sockaddr *psa	= args->arg4 != NULL ? &sa : NULL;
	nix_socklen_t	    *psalen = args->arg4 != NULL ? &salen : NULL;
	nix_env_t			*env	= obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	xec_monitor_get_guest_info(xmon, &gi);

	if (args->arg1 == NULL)
		return (EFAULT);

	if (args->arg2 == 0) {
		*result = 0;
		return (0);
	}

	if (rc == 0) {
		nix_env_set_errno(env, 0);
		*result = nix_recvfrom (args->arg0, args->arg1, args->arg2, args->arg3, psa, psalen, env);
	} else {
		*result = -1;
	}

	if (psa != NULL) {
		__nix_try
		{
			if (!nix_sockaddr_to_obsd41_sockaddr (gi.endian, psa, *psalen,
					args->arg4, (obsd41_socklen_t *)args->arg5)) {
				nix_env_set_errno(env, EINVAL);
				rc = -1;
			}
		}
		__nix_catch_any
		{
			nix_env_set_errno(env, EFAULT);
			rc = -1;
		}
		__nix_end_try
	}

	return (nix_env_get_errno(env));
}

int obsd41_accept(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_accept_args_t const *args,
	obsd41_accept_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_getpeername(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_getpeername_args_t const *args,
	obsd41_getpeername_result_t *result)
{
	xec_guest_info_t	 gi;
	struct nix_sockaddr	 sa;
	nix_socklen_t		 salen = sizeof (sa);
	nix_env_t			 *env  = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	if (args->arg1 == NULL)
		return (EFAULT);

	xec_monitor_get_guest_info(xmon, &gi);

	nix_env_set_errno(env, 0);
	*result = nix_getpeername(args->arg0, &sa, &salen, env);
	if (nix_env_get_errno(env))
		return (nix_env_get_errno(env));

	__nix_try
	{
		if (!nix_sockaddr_to_obsd41_sockaddr (gi.endian, &sa, salen,
				args->arg1, args->arg2)) {
			nix_env_set_errno(env, EINVAL);
		}
	}
	__nix_catch_any
	{
		nix_env_set_errno(env, EFAULT);
	}
	__nix_end_try
	
	return (nix_env_get_errno(env));
}

int obsd41_getsockname(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_getsockname_args_t const *args,
	obsd41_getsockname_result_t *result)
{
	xec_guest_info_t	 gi;
	struct nix_sockaddr	 sa;
	nix_socklen_t		 salen = sizeof (sa);
	nix_env_t			*env   = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	if (args->arg1 == NULL)
		return (EFAULT);

	xec_monitor_get_guest_info(xmon, &gi);

	nix_env_set_errno(env, 0);
	*result = nix_getsockname (args->arg0, &sa, &salen, env);
	if (nix_env_get_errno (env))
		return (nix_env_get_errno(env));

	__nix_try
	{
		if (!nix_sockaddr_to_obsd41_sockaddr (gi.endian, &sa, salen, args->arg1, args->arg2)) {
			nix_env_set_errno(env, EINVAL);
		}
	}
	__nix_catch_any
	{
		nix_env_set_errno(env, EFAULT);
	}
	__nix_end_try
	
	return (nix_env_get_errno(env));
}

int obsd41_access(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_access_args_t const *args,
	obsd41_access_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_access((char const *)args->arg0, args->arg1, env);
	return (nix_env_get_errno(env));
}

int obsd41_chflags(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_chflags_args_t const *args,
	obsd41_chflags_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_bsd_chflags((char const *)args->arg0, args->arg1, env);
	return (nix_env_get_errno(env));
}

int obsd41_fchflags(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_fchflags_args_t const *args,
	obsd41_fchflags_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_bsd_fchflags(args->arg0, args->arg1, env);
	return (nix_env_get_errno(env));
}

int obsd41_sync(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_sync_args_t const *args,
	obsd41_sync_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_sync(env);
	return (nix_env_get_errno(env));
}

int obsd41_kill(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_kill_args_t const *args,
	obsd41_kill_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_kill(args->arg0, args->arg1, env);
	return (nix_env_get_errno(env));
}

int obsd41_stat_compat43(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_stat_compat43_args_t const *args,
	obsd41_stat_compat43_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_getppid(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_getppid_args_t const *args,
	obsd41_getppid_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_getppid(env);
	return (nix_env_get_errno(env));
}

int obsd41_lstat_compat43(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_lstat_compat43_args_t const *args,
	obsd41_lstat_compat43_result_t *result)
{

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_dup(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_dup_args_t const *args,
	obsd41_dup_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_dup(args->arg0, env);
	return (nix_env_get_errno(env));
}

int obsd41_opipe(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_opipe_args_t const *args,
	obsd41_opipe_result_t *result)
{
//	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	return (ENOSYS);
}

int obsd41_getegid(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_getegid_args_t const *args,
	obsd41_getegid_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_getegid(env);
	return (nix_env_get_errno(env));
}

int obsd41_profil(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_profil_args_t const *args,
	obsd41_profil_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_bsd_profil(args->arg0, args->arg1, args->arg2, args->arg3, env);
	return (nix_env_get_errno(env));
}

int obsd41_ktrace(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_ktrace_args_t const *args,
	obsd41_ktrace_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_bsd_ktrace(args->arg0, args->arg1, args->arg2, args->arg3, env);
	return (nix_env_get_errno(env));
}

int obsd41_sigaction(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_sigaction_args_t const *args,
	obsd41_sigaction_result_t *result)
{
	xec_guest_info_t	  gi;
	struct nix_sigaction  sa;
	struct nix_sigaction  osa;
	struct nix_sigaction *psa  = args->arg1 != NULL ? &sa  : NULL;
	struct nix_sigaction *posa = args->arg2 != NULL ? &osa : NULL;
	nix_env_t			 *env  = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	if (psa == NULL && posa == NULL)
		return (EINVAL);

	xec_monitor_get_guest_info(xmon, &gi);

	if (psa != NULL) {
		__nix_try
		{
			obsd41_sigaction32_to_nix_sigaction(gi.endian, args->arg1, psa);
		}
		__nix_catch_any
		{
			return (EFAULT);
		}
		__nix_end_try
	}

	*result = nix_sigaction(args->arg0, psa, posa, env);
	if (nix_env_get_errno(env) != 0)
		return (nix_env_get_errno(env));

	if (posa != NULL) {
		__nix_try
		{
			nix_sigaction_to_obsd41_sigaction32(gi.endian, posa, args->arg2);
		}
		__nix_catch_any
		{
			return (EFAULT);
		}
		__nix_end_try
	}
	
	return (0);
}

int obsd41_getgid(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_getgid_args_t const *args,
	obsd41_getgid_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_getgid(env);
	return (nix_env_get_errno(env));
}

int obsd41_sigprocmask(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_sigprocmask_args_t const *args,
	obsd41_sigprocmask_result_t *result)
{
	int			  howto;
	nix_sigset_t  oset;
	nix_sigset_t  nset = args->arg1;
	nix_env_t	 *env = obsd41_us_syscall_get_nix_env(xus);
	
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	switch (args->arg0) {
		case OBSD41_SIG_BLOCK:	 howto = NIX_SIG_BLOCK;   break;
		case OBSD41_SIG_UNBLOCK: howto = NIX_SIG_UNBLOCK; break;
		case OBSD41_SIG_SETMASK: howto = NIX_SIG_SETMASK; break;
		default:				 return (EINVAL);
	}

	nix_sigprocmask(howto, &nset, &oset, env);
	*result = oset;

	return (nix_env_get_errno(env));
}

int obsd41_getlogin(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_getlogin_args_t const *args,
	obsd41_getlogin_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_bsd_getlogin((char *)args->arg0, args->arg1, env);
	return (nix_env_get_errno(env));
}

int obsd41_setlogin(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_setlogin_args_t const *args,
	obsd41_setlogin_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_bsd_setlogin((char const *)args->arg0, env);
	return (nix_env_get_errno(env));
}

int obsd41_acct(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_acct_args_t const *args,
	obsd41_acct_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_bsd_acct((char const *)args->arg0, env);
	return (nix_env_get_errno(env));
}

int obsd41_sigpending(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_sigpending_args_t const *args,
	obsd41_sigpending_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = 0; /* XXX */
	return (nix_env_get_errno(env));
}

int obsd41_osigaltstack(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_osigaltstack_args_t const *args,
	obsd41_osigaltstack_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_ioctl(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_ioctl_args_t const *args,
	obsd41_ioctl_result_t *result)
{
	xec_guest_info_t  gi;
	nix_env_t		 *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	xec_monitor_get_guest_info(xmon, &gi);

	nix_env_set_errno(env, 0);
	*result = obsd41_ioctl_dispatch(env, gi.endian, args->arg0, args->arg1, args->arg2);
	return (nix_env_get_errno(env));
}

int obsd41_reboot(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_reboot_args_t const *args,
	obsd41_reboot_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_reboot(args->arg0, env);
	return (nix_env_get_errno(env));
}

int obsd41_revoke(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_revoke_args_t const *args,
	obsd41_revoke_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_bsd_revoke((char const *)args->arg0, env);
	return (nix_env_get_errno(env));
}

int obsd41_symlink(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_symlink_args_t const *args,
	obsd41_symlink_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_symlink((char const *)args->arg0, (char const *)args->arg1, env);
	return (nix_env_get_errno(env));
}

int obsd41_readlink(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_readlink_args_t const *args,
	obsd41_readlink_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_readlink((char *)args->arg0, args->arg1, args->arg2, env);
	return (nix_env_get_errno(env));
}

int obsd41_execve(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_execve_args_t const *args,
	obsd41_execve_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_umask(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_umask_args_t const *args,
	obsd41_umask_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_umask(args->arg0, env);
	return (nix_env_get_errno(env));
}

int obsd41_chroot(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_chroot_args_t const *args,
	obsd41_chroot_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_chroot((char const *)args->arg0, env);
	return (nix_env_get_errno(env));
}

int obsd41_fstat_compat43(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_fstat_compat43_args_t const *args,
	obsd41_fstat_compat43_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_getkerninfo_compat43(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_getkerninfo_compat43_args_t const *args,
	obsd41_getkerninfo_compat43_result_t *result)
{
//	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_getpagesize_compat43(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_getpagesize_compat43_args_t const *args,
	obsd41_getpagesize_compat43_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_msync_compat43(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_msync_compat43_args_t const *args,
	obsd41_msync_compat43_result_t *result)
{
//	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	return (ENOSYS);
}

int obsd41_vfork(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_vfork_args_t const *args,
	obsd41_vfork_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_vfork(env);
	return (nix_env_get_errno(env));
}

int obsd41_ovread(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_ovread_args_t const *args,
	obsd41_ovread_result_t *result)
{
//	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_ovwrite(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_ovwrite_args_t const *args,
	obsd41_ovwrite_result_t *result)
{
//	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_sbrk(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_sbrk_args_t const *args,
	obsd41_sbrk_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = (void *)(uintptr_t)nix_sbrk(args->arg0, env);
	return (nix_env_get_errno(env));
}

int obsd41_sstk(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_sstk_args_t const *args,
	obsd41_sstk_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = (void *)(uintptr_t)nix_sstk(args->arg0, env);
	return (nix_env_get_errno(env));
}

int obsd41_mmap_compat43(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_mmap_compat43_args_t const *args,
	obsd41_mmap_compat43_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_vadvise(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_vadvise_args_t const *args,
	obsd41_vadvise_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	nix_env_set_errno(env, 0);
	*result = nix_bsd_vadvise(args->arg0, env);
	return (nix_env_get_errno(env));
}

int obsd41_munmap(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_munmap_args_t const *args,
	obsd41_munmap_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_munmap((uintmax_t)(uintptr_t)args->arg0, args->arg1, env);
	return (nix_env_get_errno(env));
}

int obsd41_mprotect(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_mprotect_args_t const *args,
	obsd41_mprotect_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_mprotect((uintmax_t)(uintptr_t)args->arg0, args->arg1, args->arg2, env);
	return (nix_env_get_errno(env));
}

int obsd41_madvise(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_madvise_args_t const *args,
	obsd41_madvise_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_madvise((uintmax_t)(uintptr_t)args->arg0, args->arg1, args->arg2, env);
	return (nix_env_get_errno(env));
}

int obsd41_ovhangup(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_ovhangup_args_t const *args,
	obsd41_ovhangup_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_bsd_vhangup(env);
	return (nix_env_get_errno(env));
}

int obsd41_ovlimit(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_ovlimit_args_t const *args,
	obsd41_ovlimit_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_bsd_vlimit(env);
	return (nix_env_get_errno(env));
}

int obsd41_mincore(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_mincore_args_t const *args,
	obsd41_mincore_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_mincore((uintmax_t)(uintptr_t)args->arg0, args->arg1, (char *)args->arg2, env);
	return (nix_env_get_errno(env));
}

int obsd41_getgroups(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_getgroups_args_t const *args,
	obsd41_getgroups_result_t *result)
{
//	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	return (ENOSYS);
}

int obsd41_setgroups(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_setgroups_args_t const *args,
	obsd41_setgroups_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_getpgrp(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_getpgrp_args_t const *args,
	obsd41_getpgrp_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_getpgrp(env);
	return (nix_env_get_errno(env));
}

int obsd41_setpgid(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_setpgid_args_t const *args,
	obsd41_setpgid_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_setpgid(args->arg0, args->arg1, env);
	return (nix_env_get_errno(env));
}

int obsd41_setitimer(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_setitimer_args_t const *args,
	obsd41_setitimer_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_wait_compat43(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_wait_compat43_args_t const *args,
	obsd41_wait_compat43_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_wait(args->arg0, env);
	return (nix_env_get_errno(env));
}

int obsd41_swapon_compat25(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_swapon_compat25_args_t const *args,
	obsd41_swapon_compat25_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_bsd_swapon((char const *)args->arg0, env);
	return (nix_env_get_errno(env));
}

int obsd41_getitimer(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_getitimer_args_t const *args,
	obsd41_getitimer_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_gethostname_compat43(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_gethostname_compat43_args_t const *args,
	obsd41_gethostname_compat43_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_gethostname((char *)args->arg0, args->arg1, env);
	return (nix_env_get_errno(env));
}

int obsd41_sethostname_compat43(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_sethostname_compat43_args_t const *args,
	obsd41_sethostname_compat43_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_sethostname((char const *)args->arg0, args->arg1, env);
	return (nix_env_get_errno(env));
}

int obsd41_getdtablesize_compat43(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_getdtablesize_compat43_args_t const *args,
	obsd41_getdtablesize_compat43_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_getdtablesize();
	return (nix_env_get_errno(env));
}

int obsd41_dup2(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_dup2_args_t const *args,
	obsd41_dup2_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_dup2(args->arg0, args->arg1, env);
	return (nix_env_get_errno(env));
}

int obsd41_fcntl(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_fcntl_args_t const *args,
	obsd41_fcntl_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_fcntl(args->arg0, args->arg1, args->arg2, env);
	return (nix_env_get_errno(env));
}

int obsd41_select(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_select_args_t const *args,
	obsd41_select_result_t *result)
{
	xec_guest_info_t	   gi;
	struct nix_timeval	   ntv;
	nix_fd_set			   fds[3];
	struct obsd41_timeval *ptv     = args->arg4;
	struct nix_timeval	  *pntv    = ptv != NULL ? &ntv : NULL;
	nix_env_t			  *env     = obsd41_us_syscall_get_nix_env(xus);
	nix_fd_set			  *pfds[3] = { NULL, NULL, NULL };

	xec_monitor_get_guest_info(xmon, &gi);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	if (ptv != NULL)
		obsd41_timeval_to_nix_timeval(gi.endian, ptv, pntv);

	__nix_try
	{
		if (args->arg1 != NULL) {
			pfds[0] = &fds[0];
			obsd41_fd_set_to_nix_fd_set(gi.endian, args->arg1, pfds[0]);
		}

		if (args->arg2 != NULL) {
			pfds[1] = &fds[1];
			obsd41_fd_set_to_nix_fd_set(gi.endian, args->arg2, pfds[1]);
		}

		if (args->arg3 != NULL) {
			pfds[2] = &fds[2];
			obsd41_fd_set_to_nix_fd_set (gi.endian, args->arg3, pfds[2]);
		}
	}
	__nix_catch_any
		{
			*result = -1;
			return (EFAULT);
		}
	__nix_end_try

	nix_env_set_errno(env, 0);
	*result = nix_select(args->arg0, pfds[0], pfds[1], pfds[2], pntv, env);

	__nix_try
	{
		if (pfds[0] != NULL)
			nix_fd_set_to_obsd41_fd_set(gi.endian, pfds[0], args->arg1);
		if (pfds[1] != NULL)
			nix_fd_set_to_obsd41_fd_set(gi.endian, pfds[1], args->arg2);
		if (pfds[2] != NULL)
			nix_fd_set_to_obsd41_fd_set(gi.endian, pfds[2], args->arg3);
	}
	__nix_catch_any
	{
		*result = -1;
		nix_env_set_errno(env, EFAULT);
	}
	__nix_end_try

	return (nix_env_get_errno(env));
}

int obsd41_fsync(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_fsync_args_t const *args,
	obsd41_fsync_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_fsync(args->arg0, env);
	return (nix_env_get_errno(env));
}

int obsd41_setpriority(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_setpriority_args_t const *args,
	obsd41_setpriority_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_setpriority(args->arg0, args->arg1, args->arg2, env);
	return (nix_env_get_errno(env));
}

int obsd41_socket(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_socket_args_t const *args,
	obsd41_socket_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_socket(args->arg0, args->arg1, args->arg2, env);
	return (nix_env_get_errno(env));
}

int obsd41_connect(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_connect_args_t const *args,
	obsd41_connect_result_t *result)
{
	xec_guest_info_t	 gi;
	int					 rc;
	struct nix_sockaddr	 sa;
	nix_socklen_t		 salen = sizeof (sa);
	nix_env_t			*env   = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	if (args->arg1 == NULL)
		return (EFAULT);

	xec_monitor_get_guest_info (xmon, &gi);

	rc = 0;

	__nix_try
	{
		if (!obsd41_sockaddr_to_nix_sockaddr(gi.endian, args->arg1, args->arg2, &sa, &salen)) {
			nix_env_set_errno(env, EAFNOSUPPORT);
			rc = -1;
		}
	}
	__nix_catch_any
	{
		nix_env_set_errno(env, EFAULT);
		rc = -1;
	}
	__nix_end_try

	if (rc == 0)
	{
		nix_env_set_errno(env, 0);
		*result = nix_connect(args->arg0, &sa, salen, env);
	}
	else
	{
		*result = -1;
	}

	return (nix_env_get_errno(env));
}

int obsd41_accept_compat43(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_accept_compat43_args_t const *args,
	obsd41_accept_compat43_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_getpriority(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_getpriority_args_t const *args,
	obsd41_getpriority_result_t *result)
{

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_send_compat43(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_send_compat43_args_t const *args,
	obsd41_send_compat43_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_recv_compat43(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_recv_compat43_args_t const *args,
	obsd41_recv_compat43_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_sigreturn(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_sigreturn_args_t const *args,
	obsd41_sigreturn_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_bind(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_bind_args_t const *args,
	obsd41_bind_result_t *result)
{
	xec_guest_info_t	 gi;
	int					 rc;
	struct nix_sockaddr	 sa;
	nix_socklen_t		 salen = sizeof (sa);
	nix_env_t			*env   = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	if (args->arg1 == NULL)
		return (EFAULT);

	xec_monitor_get_guest_info (xmon, &gi);

	rc = 0;

	__nix_try
	{
		if (!obsd41_sockaddr_to_nix_sockaddr (gi.endian, args->arg1, args->arg2, &sa, &salen)) {
			nix_env_set_errno(env, EINVAL);
			rc = -1;
		}
	}
	__nix_catch_any
	{
		nix_env_set_errno(env, EFAULT);
		rc = -1;
	}
	__nix_end_try

	if (rc == 0) {
		nix_env_set_errno(env, 0);
		*result = nix_bind (args->arg0, &sa, salen, env);
	} else {
		*result = -1;
	}

	return (nix_env_get_errno(env));
}

int obsd41_setsockopt(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_setsockopt_args_t const *args,
	obsd41_setsockopt_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	*result = 0;
	return (0); //ENOSYS;
}

int obsd41_listen(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_listen_args_t const *args,
	obsd41_listen_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_listen(args->arg0, args->arg1, env);
	return (nix_env_get_errno(env));
}

int obsd41_ovtimes(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_ovtimes_args_t const *args,
	obsd41_ovtimes_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_bsd_vtimes(env);
	return (nix_env_get_errno(env));
}

int obsd41_sigvec_compat43(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_sigvec_compat43_args_t const *args,
	obsd41_sigvec_compat43_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_sigblock_compat43(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_sigblock_compat43_args_t const *args,
	obsd41_sigblock_compat43_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_sigsetmask_compat43(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_sigsetmask_compat43_args_t const *args,
	obsd41_sigsetmask_compat43_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_sigsuspend(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_sigsuspend_args_t const *args,
	obsd41_sigsuspend_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_sigstack_compat43(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_sigstack_compat43_args_t const *args,
	obsd41_sigstack_compat43_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_recvmsg_compat43(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_recvmsg_compat43_args_t const *args,
	obsd41_recvmsg_compat43_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_sendmsg_compat43(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_sendmsg_compat43_args_t const *args,
	obsd41_sendmsg_compat43_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_ovtrace(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_ovtrace_args_t const *args,
	obsd41_ovtrace_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);
 
	nix_env_set_errno(env, 0);
	*result = nix_bsd_vtrace(env);
	return (nix_env_get_errno(env));
}

int obsd41_gettimeofday(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_gettimeofday_args_t const *args,
	obsd41_gettimeofday_result_t *result)
{
	xec_guest_info_t	 gi;
	struct nix_timeval	 tv;
	struct nix_timezone	 tz;
	struct nix_timeval	*ptv = args->arg0 != NULL ? &tv : NULL;
	struct nix_timezone *ptz = args->arg1 != NULL ? &tz : NULL;
	nix_env_t			*env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	if (ptv == NULL && ptz == NULL)
		return (EINVAL);

	xec_monitor_get_guest_info(xmon, &gi);

	nix_env_set_errno(env, 0);
	__nix_try
	{
		*result = nix_gettimeofday(ptv, ptz, env);

		if (ptv != NULL)
			nix_timeval_to_obsd41_timeval(gi.endian, ptv, args->arg0);
		if (ptz != NULL)
			nix_timezone_to_obsd41_timezone(gi.endian, ptz, args->arg1);
	}
	__nix_catch_any
	{
		nix_env_set_errno(env, EFAULT);
	}
	__nix_end_try

	return (nix_env_get_errno(env));
}

int obsd41_getrusage(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_getrusage_args_t const *args,
	obsd41_getrusage_result_t *result)
{
	xec_guest_info_t   gi;
	struct nix_rusage  ru;
	struct nix_rusage *pru = args->arg1 != NULL ? &ru : NULL;
	nix_env_t		  *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	if (pru == NULL)
		return (EINVAL);

	xec_monitor_get_guest_info(xmon, &gi);

	nix_env_set_errno(env, 0);

	*result = nix_getrusage (args->arg0, pru, env);
	
	__nix_try
	{
		nix_rusage_to_obsd41_rusage(gi.endian, pru, args->arg1);
	}
	__nix_catch_any
	{
		nix_env_set_errno(env, EFAULT);
	}
	__nix_end_try

	return (nix_env_get_errno(env));
}

int obsd41_getsockopt(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_getsockopt_args_t const *args,
	obsd41_getsockopt_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */
	*result = 0;
	return (0);//ENOSYS;
}

int obsd41_oresuba(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_oresuba_args_t const *args,
	obsd41_oresuba_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_bsd_resuba(env);
	return (nix_env_get_errno(env));
}

int obsd41_readv(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_readv_args_t const *args,
	obsd41_readv_result_t *result)
{
	xec_guest_info_t  gi;
	struct nix_iovec *xiov;
	nix_env_t		 *env = obsd41_us_syscall_get_nix_env(xus);
	xec_mem_if_t	 *mem = xec_monitor_get_memory (xmon);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

#if 0	
	xec_monitor_get_guest_info (xmon, &gi);
#else
	/*XXX */
	gi.endian = XEC_ENDIAN_BIG;
#endif

	xiov = __obsd41_iovec32_copy_from(env, mem, &gi, args->arg1, args->arg2);
	if (xiov != NULL) {
		*result = nix_readv (args->arg0, xiov, args->arg2, env);
		xec_mem_free(xiov);
	}

	return (nix_env_get_errno(env));
}

int obsd41_writev(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_writev_args_t const *args,
	obsd41_writev_result_t *result)
{
	xec_guest_info_t  gi;
	struct nix_iovec *xiov;
	nix_env_t		 *env = obsd41_us_syscall_get_nix_env(xus);
	xec_mem_if_t	 *mem = xec_monitor_get_memory (xmon);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	xec_monitor_get_guest_info(xmon, &gi);

	xiov = __obsd41_iovec32_copy_from(env, mem, &gi, args->arg1, args->arg2);
	if (xiov != NULL) {
		*result = nix_writev(args->arg0, xiov, args->arg2, env);
		xec_mem_free(xiov);
	}

	return (nix_env_get_errno(env));
}

int obsd41_settimeofday(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_settimeofday_args_t const *args,
	obsd41_settimeofday_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_fchown(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_fchown_args_t const *args,
	obsd41_fchown_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_fchown(args->arg0, args->arg1, args->arg2, env);
	return (nix_env_get_errno(env));
}

int obsd41_fchmod(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_fchmod_args_t const *args,
	obsd41_fchmod_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_fchmod(args->arg0, args->arg1, env);
	return (nix_env_get_errno(env));
}

int obsd41_recvfrom_compat43(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_recvfrom_compat43_args_t const *args,
	obsd41_recvfrom_compat43_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_setreuid(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_setreuid_args_t const *args,
	obsd41_setreuid_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_setreuid(args->arg0, args->arg1, env);
	return (nix_env_get_errno(env));
}

int obsd41_setregid(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_setregid_args_t const *args,
	obsd41_setregid_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_setregid(args->arg0, args->arg1, env);
	return (nix_env_get_errno(env));
}

int obsd41_rename(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_rename_args_t const *args,
	obsd41_rename_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_rename((char const *)args->arg0, (char const *)args->arg1, env);
	return (nix_env_get_errno(env));
}

int obsd41_truncate_compat43(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_truncate_compat43_args_t const *args,
	obsd41_truncate_compat43_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_truncate((char const *)args->arg0, args->arg1, env);
	return (nix_env_get_errno(env));
}

int obsd41_ftruncate_compat43(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_ftruncate_compat43_args_t const *args,
	obsd41_ftruncate_compat43_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_ftruncate(args->arg0, args->arg1, env);
	return (nix_env_get_errno(env));
}

int obsd41_flock(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_flock_args_t const *args,
	obsd41_flock_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_flock(args->arg0, args->arg1, env);
	return (nix_env_get_errno(env));
}

int obsd41_mkfifo(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_mkfifo_args_t const *args,
	obsd41_mkfifo_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_mkfifo((char const *)args->arg0, args->arg1, env);
	return (nix_env_get_errno(env));
}

int obsd41_sendto(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_sendto_args_t const *args,
	obsd41_sendto_result_t *result)
{
	xec_guest_info_t	 gi;
	struct nix_sockaddr	 sa;
	int					 rc	   = -1;
	struct nix_sockaddr *psa   = args->arg4 != NULL ? &sa : NULL;
	nix_socklen_t		 salen = sizeof (sa);
	nix_env_t			*env   = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	if (args->arg1 == NULL)
		return (EFAULT);

	xec_monitor_get_guest_info (xmon, &gi);

	if (args->arg2 == 0) {
		*result = 0;
		return (0);
	}

	if (psa != NULL) {
		__nix_try
		{
			if (!obsd41_sockaddr_to_nix_sockaddr (gi.endian, args->arg4, args->arg5, &sa, &salen)) {
				nix_env_set_errno(env, EINVAL);
				rc = -1;
			}
		}
		__nix_catch_any
		{
			nix_env_set_errno(env, EFAULT);
			rc = -1;
		}
		__nix_end_try
	} else {
		rc = 0;
		salen = 0;
	}

	if (rc == 0) {
		nix_env_set_errno(env, 0);
		*result = nix_sendto(args->arg0, args->arg1, args->arg2, args->arg3, psa, salen, env);
	} else {
		*result = -1;
	}

	return (nix_env_get_errno(env));
}

int obsd41_shutdown(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_shutdown_args_t const *args,
	obsd41_shutdown_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_shutdown(args->arg0, args->arg1, env);
	return (nix_env_get_errno(env));
}

int obsd41_socketpair(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_socketpair_args_t const *args,
	obsd41_socketpair_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_mkdir(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_mkdir_args_t const *args,
	obsd41_mkdir_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_mkdir((char const *)args->arg0, args->arg1, env);
	return (nix_env_get_errno(env));
}

int obsd41_rmdir(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_rmdir_args_t const *args,
	obsd41_rmdir_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_rmdir((char const *)args->arg0, env);
	return (nix_env_get_errno(env));
}

int obsd41_utimes(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_utimes_args_t const *args,
	obsd41_utimes_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_sigreturn_compat42(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_sigreturn_compat42_args_t const *args,
	obsd41_sigreturn_compat42_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_adjtime(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_adjtime_args_t const *args,
	obsd41_adjtime_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_getpeername_compat43(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_getpeername_compat43_args_t const *args,
	obsd41_getpeername_compat43_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_gethostid_compat43(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_gethostid_compat43_args_t const *args,
	obsd41_gethostid_compat43_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_bsd_gethostid((char *)args->arg0, args->arg1, env);
	return (nix_env_get_errno(env));
}

int obsd41_sethostid_compat43(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_sethostid_compat43_args_t const *args,
	obsd41_sethostid_compat43_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_bsd_sethostid((char const *)args->arg0, env);
	return (nix_env_get_errno(env));
}

int obsd41_getrlimit_compat43(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_getrlimit_compat43_args_t const *args,
	obsd41_getrlimit_compat43_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */
	
	return (ENOSYS);
}

int obsd41_setrlimit_compat43(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_setrlimit_compat43_args_t const *args,
	obsd41_setrlimit_compat43_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_killpg_compat43(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_killpg_compat43_args_t const *args,
	obsd41_killpg_compat43_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_killpg(args->arg0, args->arg1, env);
	return (nix_env_get_errno(env));
}

int obsd41_setsid(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_setsid_args_t const *args,
	obsd41_setsid_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_setsid(env);
	return (nix_env_get_errno(env));
}

int obsd41_quotactl(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_quotactl_args_t const *args,
	obsd41_quotactl_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_quota_compat43(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_quota_compat43_args_t const *args,
	obsd41_quota_compat43_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_getsockname_compat43(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_getsockname_compat43_args_t const *args,
	obsd41_getsockname_compat43_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_nfssvc(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_nfssvc_args_t const *args,
	obsd41_nfssvc_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_getdirentries_compat43(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_getdirentries_compat43_args_t const *args,
	obsd41_getdirentries_compat43_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_statfs_compat25(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_statfs_compat25_args_t const *args,
	obsd41_statfs_compat25_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_statfs_compat26(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_statfs_compat26_args_t const *args,
	obsd41_statfs_compat26_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_getfh(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_getfh_args_t const *args,
	obsd41_getfh_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_getdomainname_compat09(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_getdomainname_compat09_args_t const *args,
	obsd41_getdomainname_compat09_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_getdomainname((char *)args->arg0, args->arg1, env);
	return (nix_env_get_errno(env));
}

int obsd41_setdomainname_compat09(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_setdomainname_compat09_args_t const *args,
	obsd41_setdomainname_compat09_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_setdomainname((char const *)args->arg0, args->arg1, env);
	return (nix_env_get_errno(env));
}

int obsd41_uname_compat09(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_uname_compat09_args_t const *args,
	obsd41_uname_compat09_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_sysarch(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_sysarch_args_t const *args,
	obsd41_sysarch_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_semsys_compat10(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_semsys_compat10_args_t const *args,
	obsd41_semsys_compat10_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_msgsys_compat10(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_msgsys_compat10_args_t const *args,
	obsd41_msgsys_compat10_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_shmsys_compat10(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_shmsys_compat10_args_t const *args,
	obsd41_shmsys_compat10_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_pread(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_pread_args_t const *args,
	obsd41_pread_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_pread(args->arg0, args->arg1, args->arg2, args->arg4, env);
	return (nix_env_get_errno(env));
}

int obsd41_pwrite(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_pwrite_args_t const *args,
	obsd41_pwrite_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_pwrite(args->arg0, args->arg1, args->arg2, args->arg4, env);
	return (nix_env_get_errno(env));
}

int obsd41_setgid(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_setgid_args_t const *args,
	obsd41_setgid_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_setgid(args->arg0, env);
	return (nix_env_get_errno(env));
}

int obsd41_setegid(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_setegid_args_t const *args,
	obsd41_setegid_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_setegid(args->arg0, env);
	return (nix_env_get_errno(env));
}

int obsd41_seteuid(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_seteuid_args_t const *args,
	obsd41_seteuid_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_seteuid(args->arg0, env);
	return (nix_env_get_errno(env));
}

int obsd41_lfs_bmapv(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_lfs_bmapv_args_t const *args,
	obsd41_lfs_bmapv_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_lfs_markv(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_lfs_markv_args_t const *args,
	obsd41_lfs_markv_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_lfs_segclean(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_lfs_segclean_args_t const *args,
	obsd41_lfs_segclean_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_lfs_segwait(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_lfs_segwait_args_t const *args,
	obsd41_lfs_segwait_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_stat_compat35(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_stat_compat35_args_t const *args,
	obsd41_stat_compat35_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_fstat_compat35(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_fstat_compat35_args_t const *args,
	obsd41_fstat_compat35_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_lstat_compat35(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_lstat_compat35_args_t const *args,
	obsd41_lstat_compat35_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_pathconf(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_pathconf_args_t const *args,
	obsd41_pathconf_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_pathconf((char const *)args->arg0, args->arg1, env);
	return (nix_env_get_errno(env));
}

int obsd41_fpathconf(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_fpathconf_args_t const *args,
	obsd41_fpathconf_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_fpathconf(args->arg0, args->arg1, env);
	return (nix_env_get_errno(env));
}

int obsd41_swapctl(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_swapctl_args_t const *args,
	obsd41_swapctl_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_getrlimit(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_getrlimit_args_t const *args,
	obsd41_getrlimit_result_t *result)
{
	xec_guest_info_t   gi;
	int				   resource;
	struct nix_rlimit  rl;
	nix_env_t		  *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	if (args->arg1 == NULL)
		return (EFAULT);

	xec_monitor_get_guest_info(xmon, &gi);

	nix_env_set_errno(env, 0);
	switch (args->arg0) {
		case OBSD41_RLIMIT_CPU:		resource = NIX_RLIMIT_CPU;		break;
		case OBSD41_RLIMIT_CORE:	resource = NIX_RLIMIT_CORE;		break;
		case OBSD41_RLIMIT_DATA:	resource = NIX_RLIMIT_DATA;		break;
		case OBSD41_RLIMIT_FSIZE:	resource = NIX_RLIMIT_FSIZE;	break;
		case OBSD41_RLIMIT_MEMLOCK: resource = NIX_RLIMIT_MEMLOCK;	break;
		case OBSD41_RLIMIT_NOFILE:	resource = NIX_RLIMIT_NOFILE;	break;
		case OBSD41_RLIMIT_NPROC:	resource = NIX_RLIMIT_NPROC;	break;
		case OBSD41_RLIMIT_RSS:		resource = NIX_RLIMIT_RSS;		break;
		case OBSD41_RLIMIT_STACK:	resource = NIX_RLIMIT_STACK;	break;
		default:					return (EINVAL);
		}

	*result = nix_getrlimit(resource, &rl, env);
	if (nix_env_get_errno(env))
		return (nix_env_get_errno(env));

	__nix_try
	{
		nix_rlimit_to_obsd41_rlimit(gi.endian, &rl, args->arg1);
	}
	__nix_catch_any
	{
		nix_env_set_errno(env, EFAULT);
	}
	__nix_end_try

	return (nix_env_get_errno(env));
}

int obsd41_setrlimit(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_setrlimit_args_t const *args,
	obsd41_setrlimit_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	*result = 0;
	return (0);//XXX ENOSYS;
}

int obsd41_getdirentries(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_getdirentries_args_t const *args,
	obsd41_getdirentries_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_mmap(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_mmap_args_t const *args,
	obsd41_mmap_result_t *result)
{
	int		   prot;
	int		   flags;
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/*
	 * NIX PROT and FLAGS values are based on OBSD4.1.
	 */
	prot = args->arg2 & NIX_PROT_FLAGMASK;
	flags = args->arg3 & NIX_MAP_FLAGMASK;
	nix_env_set_errno(env, 0);

	*result = (uintptr_t)nix_mmap(args->arg0, args->arg1, prot, flags,
								  args->arg4, args->arg5, env);
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "%x,%x -> %x,%x - res = %x",
			args->arg2, args->arg3, prot, flags, *result); 

	return (nix_env_get_errno(env));
}

int obsd41___syscall(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41___syscall_args_t const *args,
	obsd41___syscall_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);
	return (xec_us_syscall_redispatch(xus, xmon, args->arg0, args->ap, result));
}

int obsd41_lseek(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_lseek_args_t const *args,
	obsd41_lseek_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_lseek(args->arg0, args->arg2, args->arg3, env);
	return (nix_env_get_errno(env));
}

int obsd41_truncate(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_truncate_args_t const *args,
	obsd41_truncate_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_truncate((char const *)args->arg0, args->arg1, env);
	return (nix_env_get_errno(env));
}

int obsd41_ftruncate(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_ftruncate_args_t const *args,
	obsd41_ftruncate_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_ftruncate(args->arg0, args->arg1, env);
	return (nix_env_get_errno(env));
}

static void __str_copy (char *out, uint32_t *len, char const *in)
{
	size_t inlen = strlen (in);

	*len = XEC_MIN (inlen, *len);
	if (*len == 0)
		return;

	memcpy (out, in, *len);
	out[*len] = 0;
}

int obsd41___sysctl(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41___sysctl_args_t const *args,
	obsd41___sysctl_result_t *result)
{
	xec_guest_info_t  gi;
	uint32_t		  n;
	uint32_t		 *mib     = args->arg0;
	uint32_t		  miblen  = args->arg1;
	uint32_t		 *oldp	  = args->arg2;
	uint32_t		 *oldlenp = args->arg3;
//	uint32_t		 *newp    = args->arg4;
//	uint32_t		  newlen  = args->arg5;
	char			  buf[512], *p = buf;
	
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	xec_monitor_get_guest_info(xmon, &gi);

	for (n = 0; n < miblen; n++) {
		if (n > 0)
			*p++ = '.';
		p += sprintf(p, "%u", GE32(&gi, mib[n]));
	}

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "mib=%s", buf);
 
	*result = 0;
	switch (GE32(&gi, mib[0])) {
		case OBSD41_CTL_KERN:
			switch (GE32(&gi, mib[1])) {
				case OBSD41_KERN_OSTYPE:
					XEC_ASSERT (g_bsd_log, miblen == 2);
					__nix_try
					{
						__str_copy((char *)oldp, oldlenp, "OpenBSD");
					}
					__nix_catch_any
					{
						*result = -1;
						return (EFAULT);
					}
					__nix_end_try
					return (0);

				case OBSD41_KERN_OSRELEASE:
					XEC_ASSERT (g_bsd_log, miblen == 2);
					__nix_try
					{
						__str_copy((char *)oldp, oldlenp, "4.1");
					}
					__nix_catch_any
					{
						*result = -1;
						return (EFAULT);
					}
					__nix_end_try
				 	return (0);

				case OBSD41_KERN_ARGMAX:
					XEC_ASSERT (g_bsd_log, miblen == 2);
					__nix_try
					{
						if (*oldlenp != GE32 (&gi, 4))
							*result = -1;
									
						*(uint32_t *)oldp = GE32 (&gi, 256 * 1024);
					}
					__nix_catch_any
					{
						*result = -1;
						return (EFAULT);
					}
					__nix_end_try
					return (0);

				case OBSD41_KERN_OSVERSION:
					XEC_ASSERT (g_bsd_log, miblen == 2);
					__nix_try
					{
						__str_copy((char *)oldp, oldlenp, "GENERIC");
					}
					__nix_catch_any
					{
						*result = -1;
						return (EFAULT);
					}
					__nix_end_try
					return (0);

				case OBSD41_KERN_HOSTNAME:
				{
					char hostname[512];
					nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

					XEC_ASSERT(g_bsd_log, miblen == 2);

					if (nix_gethostname(hostname, sizeof(hostname), env))
						return (nix_env_get_errno(env));

					__nix_try
					{
						__str_copy((char *)oldp, oldlenp, hostname);
					}
					__nix_catch_any
					{
						*result = -1;
						return (EFAULT);
					}
					__nix_end_try

					return (0);
				}

				case OBSD41_KERN_DOMAINNAME:
				{
					char domainname[512];
					nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

					XEC_ASSERT(g_bsd_log, miblen == 2);

					if (nix_getdomainname(domainname, sizeof (domainname), env))
					  return (nix_env_get_errno(env));

					__nix_try
					{
						__str_copy ( (char *)oldp, oldlenp, domainname);
					}
					__nix_catch_any
					{
						*result = -1;
						return (EFAULT);
					}
					__nix_end_try

					return (0);
				}

				case OBSD41_KERN_ARND:
				{
					XEC_ASSERT (g_bsd_log, miblen == 2);

					__nix_try
					{
						uint8_t buf[256];

						if (GE32 (&gi, *oldlenp) > sizeof (buf))
							*oldlenp = GE32 (&gi, sizeof (buf));

						if (oldp != NULL) {
							char pbuf[1024], *p = pbuf;
							size_t x;

							bsd_arc4random_bytes (buf, GE32 (&gi, *oldlenp));

							for (x = 0; x < GE32 (&gi, *oldlenp); x++)
								p += sprintf (p, "%02x ", (uint8_t)buf[x]);

							XEC_LOG(g_bsd_log, XEC_LOG_INFO, 0, "Generated RND sequence = %s", pbuf);
							memcpy (oldp, buf, GE32 (&gi, *oldlenp));
						}
					}
					__nix_catch_any
					{
						*result = -1;
						return (EFAULT);
					}
					__nix_end_try

					return (0);
				}

				default:
					XEC_LOG(g_bsd_log, XEC_LOG_ERROR, 0, "mib(%s) [CTL_KERN] not implemented", buf);
					*result = -1;
					return (EINVAL);
			}
			break;

		case OBSD41_CTL_HW:
			switch (GE32(&gi, mib[1])) {
				case OBSD41_HW_MACHINE:
					XEC_ASSERT(g_bsd_log, miblen == 2);
					__nix_try
					{
						__str_copy((char *)oldp, oldlenp, OBSD41_MACHINE_NAME);
					}
					__nix_catch_any
					{
						*result = -1;
						return (EFAULT);
					}
					__nix_end_try
					return (0);

				case OBSD41_HW_NCPUS:
					XEC_ASSERT (g_bsd_log, miblen == 2);
					__nix_try
					{
						XEC_ASSERT(NULL, *oldlenp == GE32 (&gi, sizeof(uint32_t)));
						*oldp = GE32(&gi, 1);
					}
					__nix_catch_any
					{
						*result = -1;
						return (EFAULT);
					}
					__nix_end_try
					return (0);

				case OBSD41_HW_PAGESIZE:
					XEC_ASSERT (NULL, miblen == 2);
					__nix_try
					{
						XEC_ASSERT(NULL, *oldlenp == GE32 (&gi, sizeof(uint32_t)));
						*oldp = GE32(&gi, gi.page_size);
					}
					__nix_catch_any
					{
						*result = -1;
						return (EFAULT);
					}
					__nix_end_try
					return (0);

				default:
					XEC_LOG(g_bsd_log, XEC_LOG_ERROR, 0, "mib(%s) [CTL_HW] not implemented", buf);
					*result = -1;
					return (EINVAL);
			}
			break;

		default:
			XEC_LOG(g_bsd_log, XEC_LOG_ERROR, 0, "mib(%s) not implemented", buf);
			*result = -1;
			return (EINVAL);
	}

	return (0);
}

int obsd41_mlock(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_mlock_args_t const *args,
	obsd41_mlock_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_mlock((uintmax_t)(uintptr_t)args->arg0, args->arg1, env);
	return (nix_env_get_errno(env));
}

int obsd41_munlock(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_munlock_args_t const *args,
	obsd41_munlock_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_munlock((uintmax_t)(uintptr_t)args->arg0, args->arg1, env);
	return (nix_env_get_errno(env));
}

int obsd41_futimes(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_futimes_args_t const *args,
	obsd41_futimes_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_getpgid(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_getpgid_args_t const *args,
	obsd41_getpgid_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_getpgid(args->arg0, env);
	return (nix_env_get_errno(env));
}

int obsd41_xfspioctl(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_xfspioctl_args_t const *args,
	obsd41_xfspioctl_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_semctl_compat23(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_semctl_compat23_args_t const *args,
	obsd41_semctl_compat23_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_semget(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_semget_args_t const *args,
	obsd41_semget_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_semop_compat35(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_semop_compat35_args_t const *args,
	obsd41_semop_compat35_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_osys_semconfig(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_osys_semconfig_args_t const *args,
	obsd41_osys_semconfig_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_msgctl_compat23(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_msgctl_compat23_args_t const *args,
	obsd41_msgctl_compat23_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_msgget(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_msgget_args_t const *args,
	obsd41_msgget_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_msgsnd(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_msgsnd_args_t const *args,
	obsd41_msgsnd_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_msgrcv(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_msgrcv_args_t const *args,
	obsd41_msgrcv_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_shmat(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_shmat_args_t const *args,
	obsd41_shmat_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_shmctl_compat23(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_shmctl_compat23_args_t const *args,
	obsd41_shmctl_compat23_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_shmdt(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_shmdt_args_t const *args,
	obsd41_shmdt_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_shmget_compat35(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_shmget_compat35_args_t const *args,
	obsd41_shmget_compat35_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_clock_gettime(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_clock_gettime_args_t const *args,
	obsd41_clock_gettime_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_clock_settime(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_clock_settime_args_t const *args,
	obsd41_clock_settime_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_clock_getres(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_clock_getres_args_t const *args,
	obsd41_clock_getres_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_nanosleep(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_nanosleep_args_t const *args,
	obsd41_nanosleep_result_t *result)
{
	xec_guest_info_t	 gi;
	struct nix_timespec	 rqt;
	struct nix_timespec	 rmt;
	struct nix_timespec *rmtp = args->arg1 != NULL ? &rmt : NULL;
	nix_env_t			*env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	if (args->arg0 == NULL)
		return (EFAULT);

	xec_monitor_get_guest_info(xmon, &gi);

	nix_env_set_errno(env, 0);
	__nix_try
	{
		obsd41_timespec_to_nix_timespec(gi.endian, args->arg0, &rqt);

		*result = nix_nanosleep(&rqt, rmtp, env);

		if (rmtp != NULL)
			nix_timespec_to_obsd41_timespec(gi.endian, rmtp, args->arg1);
	}
	__nix_catch_any
	{
		nix_env_set_errno(env, EFAULT);
	}
	__nix_end_try

	return (nix_env_get_errno(env));
}

int obsd41_minherit(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_minherit_args_t const *args,
	obsd41_minherit_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_minherit((uintmax_t)(uintptr_t)args->arg0, args->arg1, args->arg2, env);
	return (nix_env_get_errno(env));
}

int obsd41_rfork(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_rfork_args_t const *args,
	obsd41_rfork_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_bsd_rfork(args->arg0, env);
	return (nix_env_get_errno(env));
}

int obsd41_poll(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_poll_args_t const *args,
	obsd41_poll_result_t *result)
{
	xec_guest_info_t	  gi;
	struct obsd41_pollfd *ofds = args->arg0;
	struct nix_pollfd	 *fds = NULL;
	nix_env_t			 *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	xec_monitor_get_guest_info(xmon, &gi);

	nix_env_set_errno(env, 0);
	fds = __obsd41_pollfd_copy_from(env, &gi, ofds, args->arg1);
	if (fds != NULL) {
		*result = nix_poll(fds, args->arg1, args->arg2, env);

		if (nix_env_get_errno (env) == 0) {
			/* Copy back the results */
			__nix_try
			{
				size_t n;

				for (n = 0; n < args->arg1; n++)
					nix_pollfd_to_obsd41_pollfd(gi.endian, fds + n, ofds + n);
			}
			__nix_catch_any
			{
				nix_env_set_errno(env, EFAULT);
				*result = -1;
			}
			__nix_end_try
		}

		xec_mem_free(fds);
	} else {
		*result = -1;
	}

	return (nix_env_get_errno(env));
}

int obsd41_issetugid(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_issetugid_args_t const *args,
	obsd41_issetugid_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_bsd_issetugid(env);
	return (nix_env_get_errno(env));
}

int obsd41_lchown(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_lchown_args_t const *args,
	obsd41_lchown_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_lchown((char const *)args->arg0, args->arg1, args->arg2, env);
	return (nix_env_get_errno(env));
}

int obsd41_getsid(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_getsid_args_t const *args,
	obsd41_getsid_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_getsid(args->arg0, env);
	return (nix_env_get_errno(env));
}

int obsd41_msync(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_msync_args_t const *args,
	obsd41_msync_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_msync((uintmax_t)(uintptr_t)args->arg0, args->arg1, args->arg2, env);
	return (nix_env_get_errno(env));
}

int obsd41_semctl_compat35(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_semctl_compat35_args_t const *args,
	obsd41_semctl_compat35_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_shmctl_compat35(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_shmctl_compat35_args_t const *args,
	obsd41_shmctl_compat35_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_msgctl_compat35(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_msgctl_compat35_args_t const *args,
	obsd41_msgctl_compat35_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_getfsstat(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_getfsstat_args_t const *args,
	obsd41_getfsstat_result_t *result)
{
	xec_guest_info_t	  gi;
	int					  flags;
	size_t				  maxcount;
	struct nix_statfs	 *fss;
	struct obsd41_statfs *ofss;
	nix_env_t			 *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	xec_monitor_get_guest_info(xmon, &gi);

	nix_env_set_errno(env, 0);

	flags = 0;
	if (args->arg2 == OBSD41_MNT_NOWAIT)
		flags = NIX_MNT_NOWAIT;

	if (args->arg0 == NULL) {
		*result = nix_bsd_getfsstat(NULL, 0, flags, env);
		return (nix_env_get_errno(env));
	}

	maxcount = args->arg1 / sizeof(struct obsd41_statfs);
	if (maxcount == 0)
		return (EINVAL);
	
	fss = xec_mem_alloc_ntype(struct nix_statfs, maxcount, 0);
	if (fss == NULL)
		return (ENOMEM);

	*result = nix_bsd_getfsstat(fss, maxcount * sizeof (struct nix_statfs), flags, env);
	if ( (int32_t)*result < 0) {
		xec_mem_free (fss);
		return (nix_env_get_errno(env));
	}

	__nix_try
	{
		size_t n;

		ofss = (struct obsd41_statfs *)args->arg0;
		for (n = 0; n < *result; n++)
			nix_statfs_to_obsd41_statfs(gi.endian, fss + n, ofss + n);
			
		xec_mem_free(fss);
	}
	__nix_catch_any
	{
		xec_mem_free(fss);
		return (EFAULT);
	}
	__nix_end_try

	return (nix_env_get_errno(env));
}

int obsd41_statfs(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_statfs_args_t const *args,
	obsd41_statfs_result_t *result)
{
	xec_guest_info_t   gi;
	struct nix_statfs  fs;
	nix_env_t		  *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	if (args->arg0 == NULL)
		return (EFAULT);

	xec_monitor_get_guest_info(xmon, &gi);

	nix_env_set_errno(env, 0);
	*result = nix_bsd_statfs((char const *)args->arg0, &fs, env);
	if (*result)
		return (nix_env_get_errno(env));

	__nix_try
	{
		nix_statfs_to_obsd41_statfs(gi.endian, &fs, args->arg1);
	}
	__nix_catch_any
	{
		return (EFAULT);
	}
	__nix_end_try

	return (nix_env_get_errno(env));
}

int obsd41_fstatfs(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_fstatfs_args_t const *args,
	obsd41_fstatfs_result_t *result)
{
	xec_guest_info_t   gi;
	struct nix_statfs  fs;
	nix_env_t		  *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	xec_monitor_get_guest_info (xmon, &gi);

	nix_env_set_errno(env, 0);
	*result = nix_bsd_fstatfs (args->arg0, &fs, env);
	if (*result)
		return (nix_env_get_errno(env));

	__nix_try
	{
		nix_statfs_to_obsd41_statfs (gi.endian, &fs, args->arg1);
	}
	__nix_catch_any
	{
		return (EFAULT);
	}
	__nix_end_try

	return (nix_env_get_errno(env));

}

int obsd41_pipe(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_pipe_args_t const *args,
	obsd41_pipe_result_t *result)
{
	int		   fds[2];
	int32_t	  *ofds;
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	if (args->arg0 == NULL)
		return (EFAULT);

	nix_env_set_errno(env, 0);
	*result = nix_pipe(fds, env);
	if (*result)
		return (nix_env_get_errno(env));

	__nix_try
	{
		ofds = (int32_t *)args->arg0;

		ofds[0] = fds[0];
		ofds[1] = fds[1];
	}
	__nix_catch_any
	{
		nix_close(fds[1], env);
		nix_close(fds[0], env);
		nix_env_set_errno(env, EFAULT);
	}
	__nix_end_try

	return (nix_env_get_errno(env));
}

int obsd41_fhopen(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_fhopen_args_t const *args,
	obsd41_fhopen_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_fhstat_compat35(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_fhstat_compat35_args_t const *args,
	obsd41_fhstat_compat35_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_fhstatfs(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_fhstatfs_args_t const *args,
	obsd41_fhstatfs_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_preadv(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_preadv_args_t const *args,
	obsd41_preadv_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_pwritev(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_pwritev_args_t const *args,
	obsd41_pwritev_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_kqueue(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_kqueue_args_t const *args,
	obsd41_kqueue_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_bsd_kqueue(env);
	return (nix_env_get_errno(env));
}

int obsd41_kevent(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_kevent_args_t const *args,
	obsd41_kevent_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_mlockall(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_mlockall_args_t const *args,
	obsd41_mlockall_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_mlockall(args->arg0, env);
	return (nix_env_get_errno(env));
}

int obsd41_munlockall(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_munlockall_args_t const *args,
	obsd41_munlockall_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_munlockall(env);
	return (nix_env_get_errno(env));
}

int obsd41_getpeereid(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_getpeereid_args_t const *args,
	obsd41_getpeereid_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_getresuid(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_getresuid_args_t const *args,
	obsd41_getresuid_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_setresuid(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_setresuid_args_t const *args,
	obsd41_setresuid_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_hpux_setresuid(args->arg0, args->arg1, args->arg2, env);
	return (nix_env_get_errno(env));
}

int obsd41_getresgid(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_getresgid_args_t const *args,
	obsd41_getresgid_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_setresgid(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_setresgid_args_t const *args,
	obsd41_setresgid_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_hpux_setresgid(args->arg0, args->arg1, args->arg2, env);
	return (nix_env_get_errno(env));
}

int obsd41_osys_mquery(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_osys_mquery_args_t const *args,
	obsd41_osys_mquery_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	return (ENOSYS);
}

int obsd41_mquery(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_mquery_args_t const *args,
	obsd41_mquery_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = (void *)(uintptr_t)nix_bsd_mquery((uintmax_t)(uintptr_t)args->arg0, args->arg1, args->arg2, args->arg3, args->arg4, args->arg5, env);
	return (nix_env_get_errno(env));
}

int obsd41_closefrom(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_closefrom_args_t const *args,
	obsd41_closefrom_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_bsd_closefrom(args->arg0, env);
	return (nix_env_get_errno(env));
}

int obsd41_sigaltstack(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_sigaltstack_args_t const *args,
	obsd41_sigaltstack_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_shmget(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_shmget_args_t const *args,
	obsd41_shmget_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_semop(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_semop_args_t const *args,
	obsd41_semop_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_stat(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_stat_args_t const *args,
	obsd41_stat_result_t *result)
{
	xec_guest_info_t  gi;
	struct nix_stat	  st;
	nix_env_t		 *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	if (args->arg1 == NULL)
		return (EFAULT);

	xec_monitor_get_guest_info(xmon, &gi);

	nix_env_set_errno(env, 0);
	__nix_try
	{
		*result = nix_stat((char const *)args->arg0, &st, env);
	}
	__nix_catch_any
	{
		return (EFAULT);
	}
	__nix_end_try

	if (nix_env_get_errno(env) != 0)
		return (nix_env_get_errno(env));
	
	__nix_try
	{
		nix_stat_to_obsd41_stat(gi.endian, &st, args->arg1);
	}
	__nix_catch_any
	{
		return (EFAULT);
	}
	__nix_end_try

	return (0);
}

int obsd41_fstat(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_fstat_args_t const *args,
	obsd41_fstat_result_t *result)
{
	xec_guest_info_t  gi;
	struct nix_stat	  st;
	nix_env_t		 *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	if (args->arg1 == NULL)
		return (EFAULT);

	xec_monitor_get_guest_info (xmon, &gi);

	nix_env_set_errno(env, 0);
	*result = nix_fstat(args->arg0, &st, env);

	if (nix_env_get_errno(env) != 0)
		return (nix_env_get_errno(env));
	
	__nix_try
	{
		nix_stat_to_obsd41_stat (gi.endian, &st, args->arg1);
	}
	__nix_catch_any
	{
		return (EFAULT);
	}
	__nix_end_try

	return (0);
}

int obsd41_lstat(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_lstat_args_t const *args,
	obsd41_lstat_result_t *result)
{
	xec_guest_info_t  gi;
	struct nix_stat	  st;
	nix_env_t		 *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	if (args->arg1 == NULL)
		return (EFAULT);

	xec_monitor_get_guest_info(xmon, &gi);

	nix_env_set_errno(env, 0);
	__nix_try
	{
		*result = nix_lstat((char const *)args->arg0, &st, env);
	}
	__nix_catch_any
	{
		return (EFAULT);
	}
	__nix_end_try

	if (nix_env_get_errno(env) != 0)
		return (nix_env_get_errno(env));
	
	__nix_try
	{
		nix_stat_to_obsd41_stat(gi.endian, &st, args->arg1);
	}
	__nix_catch_any
	{
		return (EFAULT);
	}
	__nix_end_try

	return (0);
}

int obsd41_fhstat(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_fhstat_args_t const *args,
	obsd41_fhstat_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41___semctl(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41___semctl_args_t const *args,
	obsd41___semctl_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_shmctl(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_shmctl_args_t const *args,
	obsd41_shmctl_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_msgctl(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_msgctl_args_t const *args,
	obsd41_msgctl_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_sched_yield(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_sched_yield_args_t const *args,
	obsd41_sched_yield_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_rt_sched_yield(env);
	return (nix_env_get_errno(env));
}

int obsd41_getthrid(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_getthrid_args_t const *args,
	obsd41_getthrid_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_thrsleep(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_thrsleep_args_t const *args,
	obsd41_thrsleep_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_thrwakeup(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_thrwakeup_args_t const *args,
	obsd41_thrwakeup_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_threxit(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_threxit_args_t const *args,
	obsd41_threxit_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41_thrsigdivert(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_thrsigdivert_args_t const *args,
	obsd41_thrsigdivert_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}

int obsd41___getcwd(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41___getcwd_args_t const *args,
	obsd41___getcwd_result_t *result)
{
	nix_env_t *env = obsd41_us_syscall_get_nix_env(xus);

	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	nix_env_set_errno(env, 0);
	*result = nix_getcwd((char *)args->arg0, args->arg1, env);
	return (nix_env_get_errno(env));
}

int obsd41_adjfreq(
	xec_us_syscall_if_t *xus,
	xec_monitor_t *xmon,
	obsd41_adjfreq_args_t const *args,
	obsd41_adjfreq_result_t *result)
{
	XEC_LOG(g_bsd_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	/* Insert here implementation code */

	return (ENOSYS);
}
