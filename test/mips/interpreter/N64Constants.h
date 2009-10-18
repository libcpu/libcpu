#ifndef N64CONSTANTS_H_
#define N64CONSTANTS_H_

// TODO: It might be nice to convert those constants
// into real constants in here. but for now just use the defines
#include "rcp.h"
#include "r4300.h"

const int MAX_RAM_SIZE = 8 * MB;

const u32 PI_DOM1_ADDR2_END = 0x1FBFFFFF;

const u32 PIF_ROM_SIZE = PIF_ROM_END - PIF_ROM_START + 1;

const u32 PIF_RAM_SIZE = PIF_RAM_END - PIF_RAM_START + 1;

const u32 SP_DMEM_SIZE = SP_DMEM_END - SP_DMEM_START + 1;

const u32 SP_IMEM_SIZE = SP_IMEM_END - SP_IMEM_START + 1;

const u32 SP_MEM_SIZE = SP_DMEM_SIZE + SP_IMEM_SIZE;

// TODO: Evaluate whether we still need this
const u32 CountTime = 10000;

const s32 MaxMempackSize = 32768;

#endif /* N64CONSTANTS_H_ */
