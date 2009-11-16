enum {
	FLOW_TYPE_ERR         = 0,	/* unknown/bad/trap - we won't reach next instr */
	FLOW_TYPE_CALL        = 1,	/* subroutine call */
	FLOW_TYPE_RETURN      = 2,	/* return from subroutine */
	FLOW_TYPE_BRANCH      = 3,	/* conditional or unconditional jump/branch */
	FLOW_TYPE_CONTINUE    = 4,	/* control will flow into the next instr */
	FLOW_TYPE_CONDITIONAL = 128	/* FLAG: this instruction is conditional */
};
#define FLOW_TYPE_COND_BRANCH (FLOW_TYPE_BRANCH|FLOW_TYPE_CONDITIONAL)

/*
 * NEW_PC_NONE states that the destination of a call is unknown.
 * If the bitness of the guest is less than that of the host, this
 * is an invalid address for the guest, otherwise it's the top of
 * the address space minus 1, which should be the most unlikely
 * address for code. The tagging code assumes that NEW_PC_NONE
 * is outside of code_start and code_end.
 */
#define NEW_PC_NONE (addr_t)-1

int arch_tag_instr(uint8_t* RAM, addr_t pc, int *flow_type, addr_t *new_pc);
