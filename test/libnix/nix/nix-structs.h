#ifndef __nix_structs_h
#define __nix_structs_h

#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <termios.h>

void
timeval_to_nix_timeval(struct timeval const *in,
					   struct nix_timeval   *out);

void
nix_timeval_to_timeval(struct nix_timeval const *in,
					   struct timeval           *out);

void
timespec_to_nix_timespec(struct timespec const *in,
						 struct nix_timespec   *out);

void
nix_timespec_to_timespec(struct nix_timespec const *in,
						 struct timespec           *out);

void
timezone_to_nix_timezone(struct timezone const *in,
						 struct nix_timezone   *out);

void
nix_timezone_to_timezone(struct nix_timezone const *in,
						 struct timezone           *out);

void
time_to_nix_timespec(time_t               secs,
					 uint32_t             nsecs,
					 struct nix_timespec *out);

void
stat_to_nix_stat(struct stat const *in,
				 struct nix_stat   *out);

void
rusage_to_nix_rusage(struct rusage const *in,
					 struct nix_rusage   *out);

void
rlimit_to_nix_rlimit(struct rlimit const *in,
					 struct nix_rlimit   *out);

void
termios_to_nix_termios(struct termios const *in,
					   struct nix_termios   *out);

void
nix_termios_to_termios(struct nix_termios const *in,
					   struct termios           *out);

#endif  /* !__nix_structs_h */
