#include <sys/types.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "nix.h"
#include "xec-mmap.h"
#include "xec-debug.h"

extern void *g_nix_log;

uintmax_t
nix_brk(uintmax_t ptr, nix_env_t *env)
{
	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "ptr=%llx", ptr);
	return (nix_nosys(env));
}

uintmax_t
nix_sbrk(int incr, nix_env_t *env)
{
	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "incr=%d", incr);
	return (nix_nosys(env));
}

uintmax_t
nix_sstk(int incr, nix_env_t *env)
{
	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "incr=%d", incr);
	return (nix_nosys(env));
}

xec_gaddr_t
nix_mmap(xec_gaddr_t gaddr, size_t len, int prot, int flags, int fd,
	off_t offset, nix_env_t *env) 
{
	xec_haddr_t   ha;
	xec_gaddr_t   ga;
	xec_mmap_t   *xm;
	xec_mem_if_t *mem = nix_env_get_memory (env);
	unsigned      xf  = 0;

	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "addr=%llx, len=%08zx, prot=%x, flags=%x, fd=%d, offset=%lld", 
		 (uint64_t)(uintmax_t)gaddr, len, prot, flags, fd, offset);

	if (prot & NIX_PROT_READ)
		xf |= XEC_MMAP_READ;
	if (prot & NIX_PROT_WRITE)
		xf |= XEC_MMAP_WRITE;

	if (flags & NIX_MAP_SHARED)
		xf |= XEC_MMAP_SHARED;

	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "nix prot = %x host flags = %x", prot, xf);

	if (flags & NIX_MAP_FIXED)
		XEC_BUGCHECK (g_nix_log, 5040);

	if (fd != -1)
		XEC_BUGCHECK (g_nix_log, 5050);

	xm = xec_mmap_create(len, xf);
	ha = (xec_haddr_t)xec_mmap_get_bytes(xm);
	ga = xec_mem_gmap(mem, ha, len, xf);

	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "mmap()'ed %d bytes at 0x%x w/ flags 0x%x\n", len, ga, flags);

	if (ga == (xec_gaddr_t)(-1)) {
		nix_env_set_errno (env, ENOMEM);
	}

	return (ga);
}

int
nix_munmap (uintmax_t addr, size_t len, nix_env_t *env)
{
    XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "addr=%llx, len=%08x", addr, len);

	return 0;//(nix_nosys(env));
}

int
nix_mlock (uintmax_t addr, size_t len, nix_env_t *env)
{
	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "addr=%llx, len=%08x", addr, len);

	return (nix_nosys(env));
}

int
nix_munlock (uintmax_t addr, size_t len, nix_env_t *env)
{
	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "addr=%llx, len=%08x", addr, len);

	return (nix_nosys(env));
}

int
nix_msync (uintmax_t addr, size_t len, int flags, nix_env_t *env)
{
	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "addr=%llx, len=%08x, flags=%x", addr, len, flags);

	return (nix_nosys(env));
}

int
nix_mlockall (int flags, nix_env_t *env)
{
	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "flags=%x", flags);

	return (nix_nosys(env));
}

int
nix_munlockall (nix_env_t *env)
{
	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "invoked", 0);

	return (nix_nosys(env));
}

int
nix_mprotect(uintmax_t addr, size_t len, int prot, nix_env_t *env)
{
	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "addr=%llx, len=%08x, prot=%x", addr, len, prot);

	if (mprotect ( (void *)(uintptr_t)addr, len, prot) != 0) { /*XXX*/
		nix_env_set_errno (env, errno);
		return (-1);
	}

	return (0);
}

int
nix_madvise(uintmax_t addr, size_t len, int flags, nix_env_t *env)
{
	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "addr=%llx, len=%08x, flags=%x", addr, len, flags);

	return (nix_nosys(env));
}

int
nix_mincore(uintmax_t addr, size_t len, char *vec, nix_env_t *env)
{
	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "addr=%llx, len=%08x, vec=%p", addr, len, vec);

	return (nix_nosys(env));
}

int
nix_minherit(uintmax_t addr, size_t len, int inherit, nix_env_t *env)
{
	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "addr=%llx, len=%08x, inherit=%d", addr, len, inherit);

	return (nix_nosys(env));
}
