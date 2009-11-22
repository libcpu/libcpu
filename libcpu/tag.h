enum {					/* bitfield! */
	TAG_TYPE_UNKNOWN      = 0,	/* unused (or not yet discovered) code or data */
	TAG_TYPE_CODE         = 1,	/* there is a reachable executable instruction here */
	TAG_TYPE_BRANCH_TARGET= 2,	/* target of a (conditional or unconditional) jump/branch */
	TAG_TYPE_SUBROUTINE   = 4,	/* target of a (conditional or unconditional) subroutine call */
	TAG_TYPE_AFTER_CALL   = 8,	/* execution continues here after a subroutine call returns */
	TAG_TYPE_AFTER_BRANCH = 16,	/* execution continues here if a branch is not taken */
	TAG_TYPE_BRANCH		  = 32, /* this instruction is a (conditional or unconditional) jump/branch */
	TAG_TYPE_CALL		  = 64, /* this instruction is a (conditional or unconditional) subroutine call */
	TAG_TYPE_RET		  = 128,/* this instruction is a return from a subroutine */
	TAG_TYPE_ENTRY        = 256,/* the client wants to be able to start execution at this instruction */
	TAG_TYPE_CONDITIONAL  = 512,/* this is a conditional instr, e.g. conditional move */
	TAG_TYPE_DELAY_SLOT   = 1024,/* there is a delay slot following */
	TAG_TYPE_TRAP         = 2048,/* this instruction is a software trap, e.g. system calls, soft interrupts */
	TAG_TYPE_AFTER_TRAP   = 4096/* execution continues here after a trap reenters translation unit */
};
/* the tagging code assumes this fits into a byte! */

tagging_type_t get_tagging_type(cpu_t *cpu, addr_t a);
bool is_code(cpu_t *cpu, addr_t a);
void cpu_tag(cpu_t *cpu, addr_t pc);
