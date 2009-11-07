#include "xec-mem.h"
#include "xec-debug.h"
#include "xec-param.h"

#include "obsd41-guest.h"

extern void *g_bsd_log;

void
obsd41_init(void)
{
	if (g_bsd_log == NULL)
		g_bsd_log = xec_log_register("openbsd41");

	nix_init(256, 32);
}

static size_t
obsd41_us_syscall_get_max_params(void *_self)
{
	return 10;
}

extern xec_us_syscall_desc_t const *
obsd41_us_syscall_get(int scno);

static bool
obsd41_us_syscall_find(void *_self, int scno,
	xec_us_syscall_desc_t const **desc)
{
	*desc = obsd41_us_syscall_get(scno);
	if (*desc == NULL)
		return (false);

	return (true);
}

static bool
obsd41_us_syscall_extract (void *_self,
						   xec_monitor_t *xmon,
						   xec_us_syscall_desc_t const **desc)
{
	int                  scno;
	obsd41_us_syscall_t *self = (obsd41_us_syscall_t *)_self;

	obsd41_guest_get_syscall(self, xmon, &scno);

	*desc = obsd41_us_syscall_get(scno);
	if (*desc == NULL) {
		XEC_LOG (g_bsd_log, XEC_LOG_FATAL, 0,
			"system call %d descriptor not found", scno);
		return (false);
	}

	/* Reset last param */
	self->last_param = 0;

	return (true);
}

static xec_param_type_t
obsd41_us_syscall_get_retype(void *_self)
{
	obsd41_us_syscall_t *self = (obsd41_us_syscall_t *)_self;
	return self->retype;
}

void
obsd41_us_syscall_set_retype(void *_self, xec_param_type_t type)
{
	obsd41_us_syscall_t *self = (obsd41_us_syscall_t *)_self;
	self->retype = type;
}

static xec_us_syscall_if_vtbl_t const obsd41_us_syscall_impl_vtbl = {
	obsd41_us_syscall_get_max_params,
	obsd41_us_syscall_find,
	obsd41_us_syscall_extract,
	obsd41_guest_get_next_param,
	obsd41_guest_set_result,
	obsd41_us_syscall_get_retype,
	obsd41_us_syscall_set_retype
};

xec_us_syscall_if_t *
obsd41_us_syscall_create(xec_mem_if_t *memif)
{
	obsd41_us_syscall_t *sc;

	obsd41_init ();

	sc = xec_mem_alloc_type(obsd41_us_syscall_t, 0);
	if (sc != NULL) {
		sc->vtbl = &obsd41_us_syscall_impl_vtbl;
		sc->env = nix_env_create (memif);
		sc->last_param = 0;
		sc->retype = XEC_PARAM_INVALID;
	}
	return (xec_us_syscall_if_t *)sc;
}
