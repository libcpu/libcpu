#include "nix-config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#ifdef HAVE_BITSTRING_H
#include <bitstring.h>
#else
#include "bsd-bitstring.h"
#endif

#include "nix.h"
#include "xec-mem.h"
#include "xec-debug.h"

extern void    *g_nix_log;

static size_t    nix_fdtable_size = 0;
static size_t    nix_usedfd       = 0;
static int      *nix_fdtable      = NULL;
static bitstr_t *nix_fdslots      = NULL;

#define NIX_FD_LOCK()
#define NIX_FD_UNLOCK()


// XXX THIS STUFF SHOULD BE IN THE nix_env!

int
nix_fd_init(size_t count)
{
	size_t n;

	XEC_ASSERT(g_nix_log, count > 3);

	nix_fdtable_size = count;
	nix_fdtable = xec_mem_alloc_ntype(int, count, 0);
	XEC_ASSERT(g_nix_log, nix_fdtable != NULL);
	nix_fdslots = bit_alloc(count);
	XEC_ASSERT(g_nix_log, nix_fdslots != NULL);

	for (n = 0; n < count; n++)
		nix_fdtable[n] = -1;

	/* Init defaults. */
	nix_fdtable[0] = STDIN_FILENO;
	nix_fdtable[1] = STDOUT_FILENO;
	nix_fdtable[2] = STDERR_FILENO;
	bit_nset(nix_fdslots, 0, 2);
	nix_usedfd = 3;

	return (1);
}

int
nix_fd_alloc(int fd, nix_env_t *env)
{
	int rc = -1;

	NIX_FD_LOCK();
	if (nix_usedfd >= nix_fdtable_size)
		nix_env_set_errno(env, ENFILE);
	else {
		bit_ffc(nix_fdslots, nix_fdtable_size, &rc);
      
		XEC_ASSERT(g_nix_log, rc != -1);
      
		bit_set(nix_fdslots, rc);

		nix_fdtable[rc] = fd;

		nix_usedfd++;
	}
	NIX_FD_UNLOCK();

	return (rc);
}

int
nix_fd_alloc_at(int gfd, int fd, nix_env_t *env)
{
	int rc = -1;

	NIX_FD_LOCK();
	if (nix_usedfd >= nix_fdtable_size)
		nix_env_set_errno(env, ENFILE);
	else {
		if (bit_test(nix_fdslots, gfd)) {
			nix_env_set_errno(env, EMFILE);
			return (-1);
        }
      
		bit_set(nix_fdslots, gfd);

		nix_fdtable[gfd] = fd;

		nix_usedfd++;
	}
	NIX_FD_UNLOCK ();

	return (rc);
}

int
nix_fd_release(int fd, nix_env_t *env)
{
	int rc = 0;

	NIX_FD_LOCK();
	if (fd < 0 || fd >= (int)nix_fdtable_size || nix_usedfd == 0)
		nix_env_set_errno(env, EBADF);
	else {
		if (!bit_test(nix_fdslots, fd))
			nix_env_set_errno(env, EBADF);
		else {
			nix_fdtable[fd] = -1;

			bit_clear(nix_fdslots, fd);

			--nix_usedfd;

			rc = 1;
		}
	}
	NIX_FD_UNLOCK ();

	return (rc);
}

int
nix_fd_get(int fd)
{
	if (fd < 0 || fd >= (int)nix_fdtable_size)
		return (-1);
	else
		return (nix_fdtable[fd]);
}

int
nix_fd_get_nearest(nix_env_t *env, int fd, int dir)
{
	int nfd = -1;

	if (fd < 0)
		return (-1);
	else if (fd > (int)nix_fdtable_size)
		fd = nix_fdtable_size;

	NIX_FD_LOCK ();
	if (dir < 0) {
		ssize_t n;

		for (n = fd; n >= 0; n--) {
			if (bit_test(nix_fdslots, n)) {
				nfd = n;
				break;
			}
		}
	} else {
		size_t n;

		for (n = fd; n < nix_fdtable_size; n++) {
			if (bit_test(nix_fdslots, n)) {
				nfd = n;
				break;
			}
		}
	}
	NIX_FD_UNLOCK ();

	return (nfd);
}

int
nix_getdtablesize(void)
{
	return (nix_fdtable_size);
}
