#ifndef __nix_socket_h
#define __nix_socket_h

typedef uint32_t nix_socklen_t;
typedef uint32_t nix_sa_family_t;

struct nix_sockaddr {
	nix_sa_family_t sa_family;
	char            sa_data[256 - sizeof (nix_sa_family_t)];
};

struct nix_sockaddr_storage {
	nix_sa_family_t ss_family;
	char            ss_data[256 - sizeof (nix_sa_family_t)];
};

struct nix_sockaddr_in {
	nix_sa_family_t sin_family;
	uint16_t        sin_port;
	uint32_t        sin_addr;
};

struct nix_sockaddr_un {
	nix_sa_family_t sun_family;
	char            sun_path[104];
};

struct nix_msghdr {
	void             *msg_name;
	nix_socklen_t     msg_namelen;
	struct nix_iovec *msg_iov;
	uint32_t          msg_iovlen;
	void             *msg_control;
	nix_socklen_t     msg_controllen;
	uint32_t          msg_flags;
};

struct nix_cmsghdr {
	nix_socklen_t cmsg_len;
	uint32_t      cmsg_level;
	uint32_t      cmsg_type;
	char          cmsg_data[1];
};

#endif  /* !__nix_socket_h */
