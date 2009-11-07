#ifndef __obsd41_structs_h
#define __obsd41_structs_h

void
nix_timezone_to_obsd41_timezone(xec_endian_t               endian,
								struct nix_timezone const *in,
								struct obsd41_timezone    *out);

void
nix_timespec_to_obsd41_timespec(xec_endian_t               endian,
								struct nix_timespec const *in,
								struct obsd41_timespec    *out);

void
nix_timeval_to_obsd41_timeval(xec_endian_t              endian,
							  struct nix_timeval const *in,
							  struct obsd41_timeval    *out);

void
nix_stat_to_obsd41_stat(xec_endian_t           endian,
						struct nix_stat const *in,
						struct obsd41_stat    *out);

void
nix_sigaction_to_obsd41_sigaction32(xec_endian_t                endian,
									struct nix_sigaction const *out,
									struct obsd41_sigaction32  *in);

void
obsd41_timezone_to_nix_timezone(xec_endian_t                  endian,
								struct obsd41_timezone const *in,
								struct nix_timezone          *out);

void
obsd41_timespec_to_nix_timespec(xec_endian_t                  endian,
								struct obsd41_timespec const *in,
								struct nix_timespec          *out);

void
obsd41_timeval_to_nix_timeval(xec_endian_t                 endian,
							  struct obsd41_timeval const *in,
							  struct nix_timeval          *out);

void
obsd41_sigaction32_to_nix_sigaction(xec_endian_t                     endian,
									struct obsd41_sigaction32 const *in,
									struct nix_sigaction            *out);

void
nix_statfs_to_obsd41_statfs(xec_endian_t             endian,
							struct nix_statfs const *in,
							struct obsd41_statfs    *out);

int
nix_sockaddr_to_obsd41_sockaddr(xec_endian_t               endian,
								struct nix_sockaddr const *in,
								nix_socklen_t              inlen,
								struct obsd41_sockaddr    *out,
								obsd41_socklen_t          *outlen);

int
obsd41_sockaddr_to_nix_sockaddr(xec_endian_t                  endian,
								struct obsd41_sockaddr const *in,
								obsd41_socklen_t              inlen,
								struct nix_sockaddr          *out,
								nix_socklen_t                *outlen);

void
nix_rlimit_to_obsd41_rlimit(xec_endian_t             endian,
							struct nix_rlimit const *in,
							struct obsd41_rlimit    *out);

void
nix_rusage_to_obsd41_rusage(xec_endian_t             endian,
							struct nix_rusage const *in,
							struct obsd41_rusage    *out);

void
obsd41_termios_to_nix_termios(xec_endian_t                 endian,
							  struct obsd41_termios const *in,
							  struct nix_termios          *out);

void
nix_termios_to_obsd41_termios(xec_endian_t             endian,
							  struct nix_termios const *in,
							  struct obsd41_termios    *out);

void
obsd41_fd_set_to_nix_fd_set(xec_endian_t         endian,
							obsd41_fd_set const *in,
							nix_fd_set          *out);

void
nix_fd_set_to_obsd41_fd_set(xec_endian_t      endian,
							nix_fd_set const *in,
							obsd41_fd_set    *out);

void
obsd41_pollfd_to_nix_pollfd(xec_endian_t                endian,
							struct obsd41_pollfd const *in,
							struct nix_pollfd          *out);

void
nix_pollfd_to_obsd41_pollfd(xec_endian_t             endian,
							struct nix_pollfd const *in,
							struct obsd41_pollfd    *out);

#endif  /* !__obsd41_structs_h */
