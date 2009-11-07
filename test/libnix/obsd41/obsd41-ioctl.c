#include <errno.h>

#include "openbsd41.h"

#include "xec-debug.h"
#include "xec-byte-order.h"

extern void *g_bsd_log;

#define GE16(x) ((endian) != XEC_ENDIAN_NATIVE ? xec_byte_swap_int16(x) : (x))
#define GE32(x) ((endian) != XEC_ENDIAN_NATIVE ? xec_byte_swap_int32(x) : (x))
#define GE64(x) ((endian) != XEC_ENDIAN_NATIVE ? xec_byte_swap_int64(x) : (x))

#define GELONG(x) ((sizeof(obsd41_long_t) == sizeof(uint64_t)) ? GE64(x) : GE32(x))

int
obsd41_ioctl_tty(nix_env_t        *env,
				 xec_endian_t      endian,
				 int               fd,
				 obsd41_ulong_t    request,
				 obsd41_uintptr_t  arg)
{
	xec_mem_if_t *mem = nix_env_get_memory(env);

	switch (request) {
		case OBSD41_TIOCGETA:
		{
			struct nix_termios     ios;
			struct obsd41_termios *oios;
			xec_mem_flg_t          mf = 0;

			/* arg is a pointer in the guest addres space. */
			oios = (struct obsd41_termios *)xec_mem_gtoh(mem, arg, &mf);
			if (mf != 0) {
				nix_env_set_errno(env, EFAULT);
				return (-1);
			}

			if (nix_ioctl(fd, NIX_TIOCGETA, &ios, env) != 0)
				return (-1);

			__nix_try
			{
				nix_termios_to_obsd41_termios (endian, &ios, oios);
			}
			__nix_catch_any
			{
				nix_env_set_errno (env, EFAULT);
				return (-1);
			}
			__nix_end_try

			return (0);
		}

		case OBSD41_TIOCSETA:
		case OBSD41_TIOCSETAW:
		case OBSD41_TIOCSETAF:
		{
			struct nix_termios     ios;
			struct obsd41_termios *oios;
			int                    req = 0;
			xec_mem_flg_t          mf  = 0;

			switch (request) {
				case OBSD41_TIOCSETA:  req = NIX_TIOCSETA;  break;
				case OBSD41_TIOCSETAW: req = NIX_TIOCSETAW; break;
				case OBSD41_TIOCSETAF: req = NIX_TIOCSETAF; break;
			}

			/* arg is a pointer in the guest addres space. */
			oios = (struct obsd41_termios *)xec_mem_gtoh(mem, arg, &mf);
			if (mf != 0) {
				nix_env_set_errno(env, EFAULT);
				return (-1);
			}

			__nix_try
			{
				obsd41_termios_to_nix_termios(endian, oios, &ios);
			}
			__nix_catch_any
			{
				nix_env_set_errno(env, EFAULT);
				return (-1);
			}
			__nix_end_try

			return (nix_ioctl(fd, req, &ios, env)); 
		}

		default:
			break;
	}

	nix_env_set_errno(env, EINVAL);
	return (-1);
}

int
obsd41_ioctl_file(nix_env_t        *env,
				  xec_endian_t      endian,
				  int               fd,
				  obsd41_ulong_t    request,
				  obsd41_uintptr_t  arg)
{
	xec_mem_if_t *mem = nix_env_get_memory(env);

	switch (request) {
		case OBSD41_FIONREAD:
		{
			size_t         bytes;
			uint32_t      *obytes;
			xec_mem_flg_t  mf = 0;

			/* arg is a pointer in the guest addres space. */
			obytes = (uint32_t *)xec_mem_gtoh(mem, arg, &mf);
			if (mf != 0) {
				nix_env_set_errno(env, EFAULT);
				return (-1);
			}

			if (nix_ioctl(fd, NIX_FIONREAD, &bytes, env) < 0)
				return (-1);

			__nix_try
			{
				*obytes = GE32(bytes);
			}
			__nix_catch_any
			{
				nix_env_set_errno(env, EFAULT);
				return (-1);
			}
			__nix_end_try

			return (0);
		}

		case OBSD41_TIOCSETA:
		case OBSD41_TIOCSETAW:
		case OBSD41_TIOCSETAF:
		{
			struct nix_termios     ios;
			struct obsd41_termios *oios;
			int                    req = 0;
			xec_mem_flg_t          mf  = 0;

			switch (request) {
				case OBSD41_TIOCSETA:  req = NIX_TIOCSETA;  break;
				case OBSD41_TIOCSETAW: req = NIX_TIOCSETAW; break;
				case OBSD41_TIOCSETAF: req = NIX_TIOCSETAF; break;
			}

			/* arg is a pointer in the guest addres space. */
			oios = (struct obsd41_termios *)xec_mem_gtoh(mem, arg, &mf);
			if (mf != 0) {
				nix_env_set_errno(env, EFAULT);
				return (-1);
			}

			__nix_try
			{
				obsd41_termios_to_nix_termios(endian, oios, &ios);
			}
			__nix_catch_any
			{
				nix_env_set_errno(env, EFAULT);
				return (-1);
			}
			__nix_end_try

			return (nix_ioctl(fd, req, &ios, env)); 
		}

		default:
			break;
	}

	nix_env_set_errno (env, EINVAL);
	return (-1);
}

int
obsd41_ioctl_dispatch(nix_env_t        *env,
					  xec_endian_t      endian,
					  int               fd,
					  obsd41_ulong_t    request,
					  obsd41_uintptr_t  arg)
{
	XEC_LOG (g_bsd_log, XEC_LOG_DEBUG, 0,
			 "group='%c' command=%u length=%u [%c%c%c]\n",
			 OBSD41_IOCGROUP (request),
			 request & 0xff,
			 OBSD41_IOCPARM_LEN(request),
			 (request & OBSD41_IOC_VOID) ? 'v' : '-',
			 (request & OBSD41_IOC_IN)   ? 'i' : '-',
			 (request & OBSD41_IOC_OUT)  ? 'o' : '-');

	nix_env_set_errno(env, 0);
	switch (OBSD41_IOCGROUP(request)) {
	case 't': return (obsd41_ioctl_tty(env, endian, fd, request, arg));
	case 'f': return (obsd41_ioctl_file(env, endian, fd, request, arg));
	}

	return (0); // XXX

	nix_env_set_errno (env, EINVAL);
	return (-1);
}
