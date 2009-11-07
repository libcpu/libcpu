#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

#include "nix.h"
#include "xec-debug.h"

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif

extern void *g_nix_log;

static void
nix_sockaddr_in_to_sockaddr_in(struct nix_sockaddr_in const *in,
							   struct sockaddr_in           *out)
{
#if defined (__APPLE__) || defined (__FreeBSD__) || defined (__NetBSD__) || defined (__OpenBSD__) || defined (__DragonFlyBSD__)
	out->sin_len         = sizeof (*out);
#endif
	out->sin_family      = in->sin_family;
	out->sin_port        = in->sin_port;
	out->sin_addr.s_addr = in->sin_addr;
}

static void
sockaddr_in_to_nix_sockaddr_in(struct sockaddr_in const *in,
							   struct nix_sockaddr_in   *out)
{
#if defined (__APPLE__) || defined (__FreeBSD__) || defined (__NetBSD__) || defined (__OpenBSD__) || defined (__DragonFlyBSD__)
	XEC_ASSERT(g_nix_log, in->sin_len == sizeof (*in));
#endif
	out->sin_family = in->sin_family;
	out->sin_port   = in->sin_port;
	out->sin_addr   = in->sin_addr.s_addr;
}

static void
nix_sockaddr_un_to_sockaddr_un(struct nix_sockaddr_un const *in,
							   struct sockaddr_un           *out)
{
#if defined (__APPLE__) || defined (__FreeBSD__) || defined (__NetBSD__) || defined (__OpenBSD__) || defined (__DragonFlyBSD__)
	// out->sun_len         = sizeof (*out);
#endif
	out->sun_family      = in->sun_family;
	strncpy(out->sun_path, in->sun_path, min(sizeof(in->sun_path), sizeof(out->sun_path)));
}

static void
sockaddr_un_to_nix_sockaddr_un(struct sockaddr_un const *in,
							   struct nix_sockaddr_un   *out)
{
#if defined (__APPLE__) || defined (__FreeBSD__) || defined (__NetBSD__) || defined (__OpenBSD__) || defined (__DragonFlyBSD__)
	XEC_ASSERT(g_nix_log, in->sun_len == sizeof (*in));
#endif
	out->sun_family = in->sun_family;
	strncpy(out->sun_path, in->sun_path, min(sizeof(in->sun_path), sizeof(out->sun_path)));
}

static int
nix_sockaddr_to_sockaddr(struct nix_sockaddr const *in,
						 nix_socklen_t              inlen,
						 struct sockaddr           *out,
						 socklen_t                 *outlen)
{
	switch (in->sa_family) {
		case NIX_AF_UNIX:
			XEC_ASSERT(g_nix_log, inlen == sizeof (struct nix_sockaddr_un));
			nix_sockaddr_un_to_sockaddr_un((struct nix_sockaddr_un const *)in,
			   (struct sockaddr_un *)out);
			*outlen = sizeof (struct sockaddr_un);
			return (1);

		case NIX_AF_INET:
			XEC_ASSERT(g_nix_log, inlen == sizeof (struct nix_sockaddr_in));
			nix_sockaddr_in_to_sockaddr_in((struct nix_sockaddr_in const *)in,
				(struct sockaddr_in *)out);
			*outlen = sizeof (struct sockaddr_in);
			return (1);
	}

	return (0);
}

static int
sockaddr_to_nix_sockaddr(struct sockaddr const *in,
						 socklen_t              inlen,
						 struct nix_sockaddr   *out,
						 nix_socklen_t         *outlen)
{
	switch (in->sa_family) {
		case NIX_AF_UNIX:
			XEC_ASSERT(g_nix_log, inlen >= sizeof (struct sockaddr_un));
			XEC_ASSERT(g_nix_log, *outlen >= sizeof (struct nix_sockaddr_un));
			sockaddr_un_to_nix_sockaddr_un((struct sockaddr_un const *)in,
				(struct nix_sockaddr_un *)out);
			*outlen = sizeof(struct nix_sockaddr_un);
			return (1);

		case NIX_AF_INET:
			XEC_ASSERT(g_nix_log, inlen >= sizeof (struct sockaddr_in));
			XEC_ASSERT(g_nix_log, *outlen >= sizeof (struct nix_sockaddr_in));
			sockaddr_in_to_nix_sockaddr_in((struct sockaddr_in const *)in,
				(struct nix_sockaddr_in *)out);
			*outlen = sizeof(struct nix_sockaddr_in);
			return (1);
	}

	return (0);
}

int
nix_socket(int family, int type, int protocol, nix_env_t *env)
{
	int fd;
	int gfd;

	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "family=%d, type=%d, protocol=%d", family, type, protocol);

	fd = socket(family, type, protocol);
	if (fd < 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	if ((gfd = nix_fd_alloc(fd, env)) < 0) {
		nix_env_set_errno(env, ENFILE);
		close (fd);
		return (-1);
	}

	return (gfd);
}

int
nix_socketpair(int family, int type, int protocol, int *sv, nix_env_t *env)
{
	int d;
	int rc;
	int rsv[2];

	d = 0;

	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "domain=%d, type=%d, protocol=%d, sv=%p", family, type, protocol, sv);

	if (sv == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	rc = socketpair(family, type, protocol, rsv);
	if (rc < 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	if ((sv[d] = nix_fd_alloc(rsv[d], env)) < 0)
	  goto errnfile;

	d++;
	if ((sv[d] = nix_fd_alloc(rsv[d], env)) < 0)
	  goto errnfile;

	return (0);

errnfile:
	nix_fd_release(sv[d], env);
	close(rsv[1]);
	close(rsv[0]);
	nix_env_set_errno(env, ENFILE);
	return (0);
}

int
nix_connect(int fd, struct nix_sockaddr const *sa, size_t salen, nix_env_t *env)
{
	struct sockaddr_storage ss;
	socklen_t               sslen;
	int                     rfd;
	int                     rc;
	socklen_t              *psalen = &sslen;
	struct sockaddr        *psa = (struct sockaddr *)&ss;

	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "fd=%d, sa=%p, salen=%zu", fd, sa, salen);

	if (sa == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	if ((rfd = nix_fd_get(fd)) < 0) {
		nix_env_set_errno(env, EBADF);
		return (-1);
	}

	rc = 0;

	__nix_try
	{
		if (!nix_sockaddr_to_sockaddr (sa, salen, psa, psalen)) {
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

	if (rc < 0)
		return (-1);

	if (connect(rfd, psa, *psalen) != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	return (0);
}

int
nix_bind(int fd, struct nix_sockaddr const *sa, size_t salen, nix_env_t *env)
{
	struct sockaddr_storage ss;
	socklen_t               sslen;
	int                     rfd;
	int                     rc;

	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "fd=%d, sa=%p, salen=%zu", fd, sa, salen);

	if (sa == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	if ((rfd = nix_fd_get(fd)) < 0) {
		nix_env_set_errno(env, EBADF);
		return (-1);
	}

	rc = 0;

	__nix_try
	{
		if (!nix_sockaddr_to_sockaddr(sa, salen, (struct sockaddr *)&ss, &sslen)) {
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

	if (rc < 0)
		return (-1);

	if (bind(rfd, (struct sockaddr *)&ss, sslen) < 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	return (0);
}

int
nix_accept(int fd, struct nix_sockaddr *sa, nix_socklen_t *salen, nix_env_t *env)
{
	int                     rfd;
	int                     afd;
	int                     gfd;
	struct sockaddr_storage ss;
	socklen_t               sslen = sizeof (ss);

	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "fd=%d, sa=%p, salen=%p", fd, sa, salen);

	if (sa == NULL || salen == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	if ((rfd = nix_fd_get(fd)) < 0) {
		nix_env_set_errno(env, EBADF);
		return (-1);
	}

	if ((afd = accept(rfd, (struct sockaddr *)&ss, &sslen)) < 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	if ((gfd = nix_fd_alloc(afd, env)) < 0) {
		close(afd);
		return (-1);
	}

	__nix_try
	{
		sockaddr_to_nix_sockaddr ( (struct sockaddr const *)&ss, sslen, sa, salen);
	}
	__nix_catch_any
	{
		nix_fd_release(gfd, env);
		close(afd);

		nix_env_set_errno(env, EFAULT);
		return (-1);
	}
	__nix_end_try

	return (0);
}

int
nix_listen(int fd, int backlog, nix_env_t *env)
{
	int rfd;

	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "fd=%d, backlog=%d", fd, backlog);

	if (backlog < 0) {
		nix_env_set_errno(env, EINVAL);
		return (-1);
	}

	if ((rfd = nix_fd_get(fd)) < 0) {
		nix_env_set_errno(env, EBADF);
		return (-1);
	}

	if (listen(rfd, backlog) != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	return (0);
}

int
nix_shutdown(int fd, int how, nix_env_t *env)
{
	int rfd;
  
	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "fd=%d, how=%d", fd, how);

	if ((rfd = nix_fd_get(fd)) < 0) {
		nix_env_set_errno(env, EBADF);
		return (-1);
	}

	if (shutdown(rfd, how) != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	return (0);
}

int
nix_getpeereid(int fd, nix_uid_t *euid, nix_gid_t *egid, nix_env_t *env)
{
	int   rfd;
	uid_t uid;
	gid_t gid;

	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "fd=%d, euid=%p, egid=%p", fd, euid, egid);

	if (euid == NULL || egid == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	if ((rfd = nix_fd_get(fd)) < 0) {
		nix_env_set_errno(env, EBADF);
		return (-1);
	}

	if (getpeereid(rfd, &uid, &gid) != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	__nix_try
	{
		*euid = uid;
		*egid = gid;
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
nix_getpeername(int fd, struct nix_sockaddr *sa, nix_socklen_t *salen,
	nix_env_t *env)
{
	int                     rfd;
	struct sockaddr_storage ss;
	socklen_t               sslen = sizeof(ss);

	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "fd=%d, sa=%p, salen=%p", fd, sa, salen);

	if (sa == NULL || salen == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	if ((rfd = nix_fd_get(fd)) < 0) {
		nix_env_set_errno(env, EBADF);
		return (-1);
	}

	if (getpeername(rfd, (struct sockaddr *)&ss, &sslen) != 0) {
		nix_env_set_errno (env, errno);
		return (-1);
	}

	__nix_try
	{
		sockaddr_to_nix_sockaddr((struct sockaddr const *)&ss, sslen,
			sa, salen);
	}
	__nix_catch_any
	{
		nix_env_set_errno(env, EFAULT);
		return(-1);
	}
	__nix_end_try

	return (0);
}

int
nix_getsockname(int fd, struct nix_sockaddr *sa, nix_socklen_t *salen, nix_env_t *env)
{
	int                     rfd;
	struct sockaddr_storage ss;
	socklen_t               sslen = sizeof(ss);

	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "fd=%d, sa=%p, salen=%p", fd, sa, salen);

	if (sa == NULL || salen == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	if ((rfd = nix_fd_get(fd)) < 0) {
		nix_env_set_errno(env, EBADF);
		return (-1);
	}

	if (getsockname(rfd, (struct sockaddr *)&ss, &sslen) != 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	__nix_try
	{
		sockaddr_to_nix_sockaddr((struct sockaddr const *)&ss, sslen,
			sa, salen);
	}
	__nix_catch_any
	{
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}
	__nix_end_try

	return (0);
}

nix_ssize_t
nix_recvfrom (int fd, void *buf, size_t len, int flags, struct nix_sockaddr *sa, nix_socklen_t *salen, nix_env_t *env)
{
	struct sockaddr_storage  ss;
	ssize_t                  rc;
	int                      rfd;
	socklen_t                sslen  = sizeof(ss);
	struct sockaddr         *psa    = sa != NULL ? (struct sockaddr *)&ss : NULL;
	socklen_t               *psalen = salen != NULL ? &sslen : NULL;

	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0,
		 "fd=%d, buf=%p, len=%zu, flags=%x, sa=%p, salen=%p",
		 fd, buf, len, flags, sa, salen);

	if (buf == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	if (psa != NULL && psalen == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	if ((rfd = nix_fd_get(fd)) < 0) {
		nix_env_set_errno(env, EBADF);
		return (-1);
	}

	if (len == 0)
		return (0);

	/*XXX convert flags */

	if ((rc = recvfrom(rfd, buf, len, flags, psa, psalen)) < 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	if (psa != NULL) {
		int rc = 0;

		__nix_try
		{
			if (!sockaddr_to_nix_sockaddr (psa, *psalen, sa, salen)) {
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

		if (rc < 0)
			return (-1);
	}

	return (rc);
}

#if 0
nix_ssize_t
nix_recvmsg(int fd, void *buf, size_t len, int flags, struct nix_sockaddr *sa,
	size_t *salen, nix_env_t *env)
{
	struct sockaddr_storage  ss;
	socklen_t                sslen  = sizeof(ss);
	struct sockaddr_storage *pss    = sa != NULL ? &ss : NULL;
	socklen_t               *psslen = salen != NULL ? &sslen : NULL;
	ssize_t                  rc;
	int                      rfd;

	if (buf == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	if (pss != NULL && psslen == NULL) {
		nix_env_set_errno(env, EFAULT);
		return (-1);
	}

	if ((rfd = nix_fd_get(fd)) < 0) {
		nix_env_set_errno(env, EBADF);
		return (-1);
	}

	if (len == 0)
		return (0);

	/*XXX convert flags */

	if ((rc = recvfrom(rfd, buf, len, flags,
		(struct sockaddr *)pss, psslen)) < 0) {
		nix_env_set_errno(env, errno);
		return (-1);
	}

	if (pss != NULL) {
		int rc = 0;

		__nix_try
		{
			if (!sockaddr_to_nix_sockaddr((struct sockaddr *)pss,
				*psslen, sa, salen)) {
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

		if (rc < 0)
			return (-1);
	  }

	return (rc);
}
#endif

nix_ssize_t
nix_sendto(int fd, void const *buf, size_t len, int flags,
	struct nix_sockaddr const *sa, nix_socklen_t salen, nix_env_t *env)
{
	struct sockaddr_storage  ss;
	int                      rfd;
	ssize_t                  rc;
	socklen_t                sslen = 0;
	socklen_t               *psalen = NULL;
	struct sockaddr         *psa    = NULL;

	XEC_LOG(g_nix_log, XEC_LOG_DEBUG, 0, "fd=%d, buf=%p, len=%zu, flags=%x, sa=%p, salen=%zu", fd, buf, len, flags, sa, salen);

	if (buf == NULL) {
		nix_env_set_errno(env, EFAULT);
		return ((-1));
	}

	if ((rfd = nix_fd_get(fd)) < 0) {
		nix_env_set_errno(env, EBADF);
		return (-1);
	}

	if (len == 0)
		return (0);

	if (sa != NULL) {
		int rc = 0;

		__nix_try
		{
			psa = (struct sockaddr *)&ss;
			psalen = &sslen;
			sslen = sizeof(ss);
			if (!nix_sockaddr_to_sockaddr(sa, salen, psa, psalen)) {
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

		if (rc < 0)
			return (-1);
	  }

	/*XXX convert flags */

	if ((rc = sendto (rfd, buf, len, flags, psa,
			psalen == NULL ? 0 : *psalen)) < 0) {
		nix_env_set_errno (env, errno);
		return (-1);
	}

	return (rc);
}
