#include "obsd41-guest.h"
#include "m88k-context.h"
#include "xec-debug.h"
#include "xec-mem-if.h"
#include "xec-byte-order.h"

extern void *g_bsd_log;

/*
 * OpenBSD/m88k system call layout:
 *
 *   Trap: 128
 *
 *   r13 - System Call Number
 *
 *  From sys/arch/m88k/m88k/trap.c:
 *   
 *   For 88k, all the arguments are passed in the registers (r2-r12)
 *   For syscall (and __syscall), r2 (and r3) has the actual code.
 *   __syscall  takes a quad syscall number, so that other
 *   arguments are at their natural alignments.
 *
 */

void
obsd41_guest_get_syscall(obsd41_us_syscall_t *self,
						 xec_monitor_t *xmon,
						 int *scno)
{
	m88k_context_t *ctx = xec_monitor_get_context(xmon); 
	*scno = ctx->gpr[13];
}

int
obsd41_guest_get_next_param(void                *_self,
							xec_monitor_t       *xmon,
							unsigned             flags,
							xec_param_type_t     type,
							xec_param_t         *param)
{
	obsd41_us_syscall_t *self = (obsd41_us_syscall_t *)_self;
	xec_mem_if_t        *mem  = xec_monitor_get_memory(xmon);
	m88k_context_t      *ctx  = xec_monitor_get_context(xmon); 

	param->type = type;
	if (self->last_param < (10 - 2)) {
		switch (type) {
			case XEC_PARAM_BYTE:
				param->value.tnosign.u8 = ctx->gpr[2 + self->last_param];
				self->last_param++;
				break;

			case XEC_PARAM_HALF:
				param->value.tnosign.u16 = ctx->gpr[2 + self->last_param];
				self->last_param++;
				break;

			case XEC_PARAM_POINTER:
			case XEC_PARAM_WORD:
			case XEC_PARAM_INTPTR:
				param->value.tnosign.u32 = ctx->gpr[2 + self->last_param];
				self->last_param++;
				break;

			case XEC_PARAM_DWORD:
			case XEC_PARAM_DOUBLE:
				if (self->last_param + 2 < (10 - 2))
				  {
					param->value.tnosign.u64 = ctx->gpr[2 + self->last_param];
					param->value.tnosign.u64 <<= 32;
					self->last_param++;
				  }
				if (self->last_param + 2 < (10 - 2))
				  {
					param->value.tnosign.u64 |= ctx->gpr[2 + self->last_param];
					self->last_param++;
				  }                        
				break;

			default:
				XEC_BUGCHECK(g_bsd_log, 5010);
				return (-1);
		}
	} else {
		__nix_try
		{
			xec_mem_flg_t   mf = 0;
			m88k_uintptr_t *sp = (m88k_uintptr_t *)xec_mem_gtoh(mem, ctx->gpr[31], &mf);

			XEC_ASSERT (g_bsd_log, mf == 0);
			sp += (self->last_param - 8);

			switch (type) {
				case XEC_PARAM_BYTE:
					param->value.tnosign.u8 = xec_byte_swap_big_to_host32(*sp);
					sp++, self->last_param++;
					break;

				case XEC_PARAM_HALF:
					param->value.tnosign.u16 = xec_byte_swap_big_to_host32(*sp);
					sp++, self->last_param++;
					break;

				case XEC_PARAM_POINTER:
				case XEC_PARAM_WORD:
				case XEC_PARAM_INTPTR:
					param->value.tnosign.u32 = xec_byte_swap_big_to_host32(*sp);
					sp++, self->last_param++;
					self->last_param++;
					break;

				case XEC_PARAM_DWORD:
				case XEC_PARAM_DOUBLE:
					param->value.tnosign.u64 = xec_byte_swap_big_to_host32(*sp);
					param->value.tnosign.u64 <<= 32;
					sp++, self->last_param++;
					param->value.tnosign.u64 |= ctx->gpr[2 + self->last_param];
					sp++, self->last_param++;
					break;

				default:
					XEC_BUGCHECK(g_bsd_log, 5011);
					return (-1);
			}
		}
		__nix_catch_any
		{
			XEC_BUGCHECK(g_bsd_log, 5020);
			return (-1);
		}
		__nix_end_try
	}

	return (0);
}

void
obsd41_guest_set_result(void                *self,
						xec_monitor_t       *xmon,
						int                  error,
						xec_param_t const   *result)
{
	m88k_context_t *ctx = xec_monitor_get_context (xmon); 

	if (error != 0) {
		ctx->psr |= (1 << 28); /* Set Carry */
		ctx->gpr[2] = error;
	} else {
		uint32_t hi, lo;

		ctx->psr &= ~(1 << 28); /* Clear Carry */
      
		hi = lo = 0;

		if (result != NULL) {
			switch (result->type) {
				case XEC_PARAM_BYTE:
					lo = result->value.tnosign.u64 & 0xff;
					break;
				case XEC_PARAM_HALF:
					lo = result->value.tnosign.u64 & 0xffff;
					break;
				case XEC_PARAM_DWORD:
					lo = result->value.tnosign.u64 >> 32;
					hi = result->value.tnosign.u64 & 0xffffffff;
					break;
				case XEC_PARAM_POINTER:
				case XEC_PARAM_INTPTR:
				case XEC_PARAM_WORD:
					lo = result->value.tnosign.u64 & 0xffffffff;
					break;
				default:
					XEC_BUGCHECK (g_bsd_log, 5012);
			}
		}

		ctx->gpr[2] = lo;
		ctx->gpr[3] = hi;
	}
}
