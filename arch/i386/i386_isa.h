/*
 * libcpu: i386_isa.h
 *
 * instruction decoding (shared by diassembler and translator)
 */

#ifndef I386_ISA_H
#define I386_ISA_H

enum i386_instr_types {
#define DECLARE_INSTR(name,str) name,
#include "i386_instr.h"
#undef DECLARE_INSTR
};

#endif /* I386_ISA_H */
