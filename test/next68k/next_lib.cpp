#include "libcpu.h"
#include <stdio.h>
#include <string.h>

#define ExecBase -1 /* let's put Exec at -1, so the jump table is at a negative address; ("0" would be bad, as this is the error case of OpenLibrary) */
#define DOSBase -4096

/* exec.library */
#define _LVOSupervisor -0x1e 
#define _LVOInitCode -0x48 
#define _LVOInitStruct -0x4e 
#define _LVOMakeLibrary -0x54 
#define _LVOMakeFunctions -0x5a 
#define _LVOFindResident -0x60 
#define _LVOInitResident -0x66 
#define _LVOAlert -0x6c 
#define _LVODebug -0x72 
#define _LVODisable -0x78 
#define _LVOEnable -0x7e 
#define _LVOForbid -0x84 
#define _LVOPermit -0x8a 
#define _LVOSetSR -0x90 
#define _LVOSuperState -0x96 
#define _LVOUserState -0x9c 
#define _LVOSetIntVector -0xa2 
#define _LVOAddIntServer -0xa8 
#define _LVORemIntServer -0xae 
#define _LVOCause -0xb4 
#define _LVOAllocate -0xba 
#define _LVODeallocate -0xc0 
#define _LVOAllocMem -0xc6 
#define _LVOAllocAbs -0xcc 
#define _LVOFreeMem -0xd2 
#define _LVOAvailMem -0xd8 
#define _LVOAllocEntry -0xde 
#define _LVOFreeEntry -0xe4 
#define _LVOInsert -0xea 
#define _LVOAddHead -0xf0 
#define _LVOAddTail -0xf6 
#define _LVORemove -0xfc 
#define _LVORemHead -0x102 
#define _LVORemTail -0x108 
#define _LVOEnqueue -0x10e 
#define _LVOFindName -0x114 
#define _LVOAddTask -0x11a 
#define _LVORemTask -0x120 
#define _LVOFindTask -0x126 
#define _LVOSetTaskPri -0x12c 
#define _LVOSetSignal -0x132 
#define _LVOSetExcept -0x138 
#define _LVOWait -0x13e 
#define _LVOSignal -0x144 
#define _LVOAllocSignal -0x14a 
#define _LVOFreeSignal -0x150 
#define _LVOAllocTrap -0x156 
#define _LVOFreeTrap -0x15c 
#define _LVOAddPort -0x162 
#define _LVORemPort -0x168 
#define _LVOPutMsg -0x16e 
#define _LVOGetMsg -0x174 
#define _LVOReplyMsg -0x17a 
#define _LVOWaitPort -0x180 
#define _LVOFindPort -0x186 
#define _LVOAddLibrary -0x18c 
#define _LVORemLibrary -0x192 
#define _LVOOldOpenLibrary -0x198 
#define _LVOCloseLibrary -0x19e 
#define _LVOSetFunction -0x1a4 
#define _LVOSumLibrary -0x1aa 
#define _LVOAddDevice -0x1b0 
#define _LVORemDevice -0x1b6 
#define _LVOOpenDevice -0x1bc 
#define _LVOCloseDevice -0x1c2 
#define _LVODoIO -0x1c8 
#define _LVOSendIO -0x1ce 
#define _LVOCheckIO -0x1d4 
#define _LVOWaitIO -0x1da 
#define _LVOAbortIO -0x1e0 
#define _LVOAddResource -0x1e6 
#define _LVORemResource -0x1ec 
#define _LVOOpenResource -0x1f2 
#define _LVORawDoFmt -0x20a 
#define _LVOGetCC -0x210 
#define _LVOTypeOfMem -0x216 
#define _LVOProcure -0x21c 
#define _LVOVacate -0x222 
#define _LVOOpenLibrary -0x228 
#define _LVOInitSemaphore -0x22e 
#define _LVOObtainSemaphore -0x234 
#define _LVOReleaseSemaphore -0x23a 
#define _LVOAttemptSemaphore -0x240 
#define _LVOObtainSemaphoreList -0x246 
#define _LVOReleaseSemaphoreList -0x24c 
#define _LVOFindSemaphore -0x252 
#define _LVOAddSemaphore -0x258 
#define _LVORemSemaphore -0x25e 
#define _LVOSumKickData -0x264 
#define _LVOAddMemList -0x26a 
#define _LVOCopyMem -0x270 
#define _LVOCopyMemQuick -0x276 
#define _LVOCacheClearU -0x27c 
#define _LVOCacheClearE -0x282 
#define _LVOCacheControl -0x288 
#define _LVOCreateIORequest -0x28e 
#define _LVODeleteIORequest -0x294 
#define _LVOCreateMsgPort -0x29a 
#define _LVODeleteMsgPort -0x2a0 
#define _LVOObtainSemaphoreShared -0x2a6 
#define _LVOAllocVec -0x2ac 
#define _LVOFreeVec -0x2b2 
#define _LVOCreatePool -0x2b8 
#define _LVODeletePool -0x2be 
#define _LVOAllocPooled -0x2c4 
#define _LVOFreePooled -0x2ca 
#define _LVOAttemptSemaphoreShared -0x2d0 
#define _LVOColdReboot -0x2d6 
#define _LVOStackSwap -0x2dc 
#define _LVOChildFree -0x2e2 
#define _LVOChildOrphan -0x2e8 
#define _LVOChildStatus -0x2ee 
#define _LVOChildWait -0x2f4 
#define _LVOCachePreDMA -0x2fa 
#define _LVOCachePostDMA -0x300 
#define _LVOAddMemHandler -0x306 
#define _LVORemMemHandler -0x30c 
#define _LVOObtainQuickVector -0x312 

/* dos.library */
#define _LVOWriteChars -0x3ae

/* global program return address */
#define ADDR_EXIT (-1)

/* exec.library */
#define ADDR_OldOpenLibrary (-2)
#define ADDR_CloseLibrary (-3)
#define ADDR_AllocMem (-4)
#define ADDR_StackSwap (-5)
#define ADDR_SetSignal (-6)
#define ADDR_OpenLibrary (-7)

/* dos.library */
#define ADDR_WriteChars (-100)


//XXX
#define CAST_ADDR(a) ((a)&0xFFFF) /* HACK: limit address bus to 16 bit */
#define CAST32(a) (*(uint32_t*)(&(a)))
#define RAM32(a) (CAST32(RAM[CAST_ADDR(a)]))
#define STORE32(a,b) (RAM32(a) = htonl(b)) 

void
init_os(uint8_t *RAM, int argc, char **argv) {
	STORE32(4, ExecBase);
	STORE32(ExecBase+_LVOOldOpenLibrary, ADDR_OldOpenLibrary);
	STORE32(ExecBase+_LVOCloseLibrary, ADDR_CloseLibrary);
	STORE32(ExecBase+_LVOAllocMem, ADDR_AllocMem);
	STORE32(ExecBase+_LVOStackSwap, ADDR_StackSwap);
	STORE32(ExecBase+_LVOSetSignal, ADDR_SetSignal);
	STORE32(ExecBase+_LVOOpenLibrary, ADDR_OpenLibrary);

	STORE32(DOSBase+_LVOWriteChars, ADDR_WriteChars);

//	sp = 0xFFFC;
//	STORE32(sp, ADDR_EXIT);
}

void
WriteChars(char *s, int n) {
//	printf("WriteChars\n");
	int i;
	for (i=0; i<n; i++)
		putchar(*s++);
}

void
CloseLibrary(uint32_t handle) {
//	printf("CloseLibrary\n");
}

uint32_t
AllocMem(uint32_t byteSize, uint32_t requirements) {
	return 128*1024; /* XXX */
}

uint32_t
StackSwap(uint32_t newStack) {
	return 128*newStack; /* XXX */
}

uint32_t
SetSignal(uint32_t newSignals, uint32_t signalMask) {
	return 0;
}

uint32_t
OpenLibrary(char *libName, uint32_t version) {
	/* ignores version */
	if (!strcmp(libName, "dos.library"))
		return DOSBase;
	else
		return 0;
}

uint32_t
OldOpenLibrary(char *lib) {
//	printf("OldOpenLibrary %s\n", lib);
	return OpenLibrary(lib, 0);
}

