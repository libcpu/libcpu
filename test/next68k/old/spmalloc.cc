/* spmalloc.c */

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>

#include "debug.h"
#include "spmalloc.h"

/*

	This is the idea of the memory layout:
	
	0x00000000
		PPC application
	SOFTPEAR_CODEBASE (0x00800000)
		SoftPear code
	somwhere behind that
		Space used by spmalloc (currently just redirects to malloc)

*/

/* Globals */
SpMallocTableEntry spMallocTable[SPMALLOCTABLESIZE];

/* Initialize the applications memory according to the planning above */
int spMemInit() {

	debug_printf("spMemInit statistics:\n");
	
	debug_printf("\tSOFTPEAR_CODEBASE: %#lx\n", SOFTPEAR_CODEBASE);
	
	debug_printf("\tspMemInit located at: %#lx\n", &spMemInit);
	if((unsigned long)&spMemInit < SOFTPEAR_CODEBASE) {
		printf("spMemInit not at expected position, aborting");
		// XXX return 0;
	}
	
	void *ppccode = mmap((void*)0x0, SOFTPEAR_CODEBASE, PROT_READ|PROT_WRITE, MAP_ANON | MAP_FIXED | MAP_SHARED, -1, 0 );
	debug_printf("\tppccode: %#lx\n", (unsigned long)ppccode);
	if(ppccode == (void*)-1) {
		debug_printf("MMap failed because ");
		switch(errno) {
			case EACCES:
				debug_printf("EACCES\n");
				break;
			case EBADF:
				debug_printf("EBADF\n");
				break;
			case EINVAL:
				debug_printf("EINVAL\n");
				break;
			case ENOMEM:
				debug_printf("ENOMEM\n");
				break;
		}
		return 0;
	}

	/* Clear the memory */
	bzero((void*)0x0, SOFTPEAR_CODEBASE);
	
	return 1;
}

void *spmalloc(size_t size) {
	/* Reserve a block of size bytes in memory */
	void *buf = mmap(0, size, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_ANON|MAP_SHARED, -1, 0);

	/* Go through the table and find a place to insert the entry */
	int i;
	for(i=0; i<SPMALLOCTABLESIZE; i++)
		if(spMallocTable[i].len == 0)
			break;
	if(i<SPMALLOCTABLESIZE) {
		spMallocTable[i].ptr = buf;
		spMallocTable[i].len = size;
		return buf;
	}
	else {
		return NULL;
	}
}

void spfree(void *ptr) {
	/* Find ptr in the table */
	int i;
	
	for(i=0; i<SPMALLOCTABLESIZE; i++)
		if(spMallocTable[i].ptr == ptr)
			break;
	
	if(i<SPMALLOCTABLESIZE) {
		munmap(ptr, spMallocTable[i].len);
		spMallocTable[i].len = 0;
		}
	else {
		debug_printf("Trying to spfree space that has never been spmalloc'ed!\n");
		}
}
