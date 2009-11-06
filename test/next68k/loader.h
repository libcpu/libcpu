/* loader.h */

#include <stdio.h>
#include <mach-o/loader.h>

#ifndef loader_h
#define loader_h

/* Loader types */
typedef unsigned long LdrMagic_t;
typedef struct {
	void *base;
	unsigned long entrypoint;
	} LdrResult_t;
typedef struct LdrLLFunctions {
	char *name;
	unsigned long offset;
	struct LdrLLFunctions *next;
	} LdrLLFunctions;

/* Indicator to signal no entry point found in Mach-O binary 
 * In the original loader code, the indicator was a return value of 0
 * Prehistoric NeXT/68k binaries (from NeXTstep 3.0), however, 
 * actually have a valid entry point 0!
 * So, this constant, which is unlikely to be a valid entry point,
 * now indicates failure to find an entry point. Ouch.
 */
#define MACHO_NO_ENTRY_POINT ((LdrResult_t *)0xFFFFFFFFUL)

/* Function declarations */
LdrResult_t*	LdrLoadMachO (char *, char*, char*);
int		LdrCheckHdrCpuType(cpu_type_t, unsigned long, FILE*);
unsigned long	LdrGetArchOffset(cpu_type_t, FILE*);
LdrMagic_t	LdrValidateFile(FILE*);
int		LdrGetMachHeader(struct mach_header*, unsigned long, FILE*);
int		LdrLoadSegment(struct segment_command*, void**, unsigned long*, FILE*, unsigned long, unsigned long);
int		LdrLoadDylib(struct dylib_command*, void **, unsigned long*);
uint32_t 	LdrGetEntrypoint(struct symtab_command*, char*, unsigned long, FILE*);

void		LdrLibGetSymbolsSymtab(struct symtab_command*, unsigned long, unsigned long, FILE*);
void		LdrLLLibFunctionsAppend(const char*, unsigned long);
void 		LdrLLLibFunctionsClearList(void);
void 		LdrLLLibFunctionsPrint();

struct nlist*	LdrLoadSymtab(struct symtab_command*, unsigned long, FILE*);
char*		LdrLoadStrtab(struct symtab_command*, unsigned long, FILE*);
uint32_t*	LdrLoadIdrSymtab(struct dysymtab_command*, unsigned long, FILE*);

int		LdrLoadNativeLibJumps(void**, unsigned long*);
int		LdrBindSegment(struct segment_command*, struct nlist*, char*, uint32_t*, void*, unsigned long);
unsigned long   LdrGetTextText(struct segment_command*, void*);

void		LdrFakeDyld(struct segment_command*, void*, struct nlist*, char*, uint32_t*);
void		LdrFakeDyld2(struct symtab_command*, void*, unsigned long, FILE*);

void		LdrDump(void*, unsigned long, unsigned long);

#endif
