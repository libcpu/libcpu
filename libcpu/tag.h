typedef enum {					/* bitfield! */
	TAG_TYPE_UNKNOWN      = 0,	/* unused (or not yet discovered) code or data */
	TAG_TYPE_CODE         = 1,	/* there is a reachable executable instruction here */
	TAG_TYPE_BRANCH_TARGET= 2,	/* target of a (conditional or unconditional) jump/branch */
	TAG_TYPE_SUBROUTINE   = 4,	/* target of a (conditional or unconditional) subroutine call */
	TAG_TYPE_AFTER_CALL   = 8,	/* execution continues here after a subroutine call returns */
	TAG_TYPE_AFTER_BRANCH = 16,	/* execution continues here if a branch is not taken */
	TAG_TYPE_BRANCH		  = 32, /* this instruction is a (conditional or unconditional) jump/branch */
	TAG_TYPE_CALL		  = 64, /* this instruction is a (conditional or unconditional) subroutine call */
	TAG_TYPE_RET		  = 128,
	TAG_TYPE_ENTRY        = 8	/* the client wants to be able to start execution at this instruction */
								//XXX we map this on the same code as TAG_TYPE_AFTER_CALL for now,
								//XXX to avoid needing 16 bits instead of 8
};
/* the tagging code assumes this fits into a byte! */

tagging_type_t get_tagging_type(cpu_t *cpu, addr_t a);
bool is_code(cpu_t *cpu, addr_t a);
void cpu_tag(cpu_t *cpu, addr_t pc);
