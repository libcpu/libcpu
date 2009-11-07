#ifndef __obsd41_ioctl_h
#define __obsd41_ioctl_h

#include "obsd41-ioctl-defs.h"

/* Ioctls */

/* Terminal I/O */
#define OBSD41_TIOCGETA  _OBSD41_IOR('t', 19, struct obsd41_termios)
#define OBSD41_TIOCSETA  _OBSD41_IOW('t', 20, struct obsd41_termios)
#define OBSD41_TIOCSETAW _OBSD41_IOW('t', 21, struct obsd41_termios)
#define OBSD41_TIOCSETAF _OBSD41_IOW('t', 22, struct obsd41_termios)

/* File */
#define OBSD41_FIONREAD  _OBSD41_IOR('f', 127, int)

int
obsd41_ioctl_dispatch (nix_env_t        *env,
					   xec_endian_t      endian,
					   int               fd,
					   obsd41_ulong_t    request,
					   obsd41_uintptr_t  arg);

#endif  /* !__obsd41_ioctl_h */
