/*
 * libcpu: 6502_interface.h
 *
 * flags exported to the client
 */

#define CPU_6502_BRK_TRAP   (1<<0)
#define CPU_6502_XXX_TRAP   (1<<1)
#define CPU_6502_I_TRAP     (1<<2)
#define CPU_6502_D_TRAP     (1<<3)
#define CPU_6502_D_IGNORE   (1<<4)
#define CPU_6502_V_IGNORE   (1<<5)
