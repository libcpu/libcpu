/* spmalloc.h */

#include <stdlib.h>

#ifndef SPMALLOC_H
#define SPMALLOC_H

int spMemInit();

void *spmalloc(size_t);
void spfree(void*);

#define SOFTPEAR_CODEBASE 0x00800000

#define SPMALLOCTABLESIZE 1000

typedef struct {
	void* ptr;
	size_t len;
} SpMallocTableEntry;

#endif
