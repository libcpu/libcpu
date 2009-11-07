#ifndef __nix_mem_h
#define __nix_mem_h

#include "nix-base.h"
#include "xec-mem-if.h"

#define NIX_PROT_NONE		0x0000
#define NIX_PROT_READ		0x0001
#define NIX_PROT_WRITE		0x0002
#define NIX_PROT_EXEC		0x0004

#define NIX_PROT_FLAGMASK	0x007

#define NIX_MAP_SHARED		0x0001
#define NIX_MAP_PRIVATE		0x0002

#define NIX_MAP_FIXED		0x0010
#define NIX_MAP_INHERIT		0x0080
#define NIX_MAP_NOEXTEND	0x0100
#define NIX_MAP_TRYFIXED	0x0400

#define NIX_MAP_FLAGMASK	0x0593

#endif /* !__nix_mem_h */
