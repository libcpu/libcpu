#ifndef _LIBCPU_TYPES_H_
#define _LIBCPU_TYPES_H_

#include <sys/types.h>
#include "defines.h"

typedef signed char sint8_t;
typedef signed short sint16_t;
typedef signed int sint32_t;
typedef signed long long sint64_t;

#ifndef sun
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
#endif

typedef uint64_t addr_t;

typedef uint16_t tag_t;

#include <arch/6502/6502_types.h>
#include <arch/m68k/m68k_types.h>
#include <arch/mips/mips_types.h>
#include <arch/x86/x86_types.h>
#include <arch/fapra/fapra_types.h>

#endif
