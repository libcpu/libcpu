enum {
	FLOW_TYPE_ERR,		/* instruction that we can't handle (e.g. HLT, JMP ind.) */
	FLOW_TYPE_RET,		/* "return" (e.g. RET) */
	FLOW_TYPE_JUMP,		/* end of execution, new code elsewhere (i.e. JMP) */
	FLOW_TYPE_CALL,		/* new code elsewhere, will implicitly return to next instruction (i.e. CALL) */
	FLOW_TYPE_BRANCH,	/* execution branch */
	FLOW_TYPE_CONTINUE	/* continued execution */
};

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
