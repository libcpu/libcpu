#ifndef _LIBCPU_TYPES_H_
#define _LIBCPU_TYPES_H_

#include <sys/types.h>
#include "defines.h"

typedef uint64_t addr_t;

typedef uint16_t tag_t;

#include <arch/6502/6502_types.h>
#include <arch/m68k/m68k_types.h>
#include <arch/mips/mips_types.h>
#include <arch/x86/x86_types.h>
#include <arch/fapra/fapra_types.h>

#endif
