/*
 * libcpu: x86_isa.h
 *
 * instruction decoding (shared by diassembler and translator)
 */

#ifndef X86_ISA_H
#define X86_ISA_H

enum x86_instr_types {
#define DECLARE_INSTR(name,str) name,
#include "x86_instr.h"
#undef DECLARE_INSTR
};

#endif /* X86_ISA_H */
