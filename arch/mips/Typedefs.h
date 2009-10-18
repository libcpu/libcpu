#ifndef TYPEDEFS_H_
#define TYPEDEFS_H_

#include "stdincludes.h"

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef u64 TRegister;

typedef TRegister TRegisters[32];

typedef union {
	u32 ByIndex[32];
	struct {
		u32 Index;
		u32 Random; // alias NextFreeTLBEntry...?
		u32 EntryLo0;
		u32 EntryLo1;
		u32 Context;
		u32 PageMask;
		u32 Wired;
		u32 Res0;
		u32 BadVAddr;
		u32 Count;
		u32 EntryHi;
		u32 Compare;
		u32 Status;
		u32 Cause;
		u32 ExceptPC;
		u32 PRevID;
		u32 Config;
		u32 LLAddr;
		u32 WatchLo;
		u32 WatchHi;
		u32 XContext;
		u32 Res1;
		u32 Res2;
		u32 Res3;
		u32 Res4;
		u32 Res5;
		u32 PErr;
		u32 CacheErr;
		u32 TagLo;
		u32 TagHi;
		u32 ErrorEPC;
		u32 Res6;
	};
} TCOP0Registers;

typedef union {
	u32 R32[32];
	float F32[32];
	u64 R64[16];
	double F64[16];
} TCOP1Registers;

typedef union {
	struct {
		u32 lo;
		u32 hi;
	} R32;
	u64 R64;
} TCOP1Registers64[64];

typedef struct {
	u32 ll;
	u32 hl;
	u32 lh;
	u32 PageMask;
	u32 Valid0;
	u32 Valid1;
	u32 Global;
	u32 Dirty0;
	u32 Dirty1;
	u32 Asid;
	u32 VPN;
	u32 PFN0;
	u32 PFN1;
	u32 AddressVPNShift;
	u32 AddressVPNMask;
	u32 PFNShift;
	u32 OffsetMask;
} TTLB;

typedef struct {
	u32 MODE_REG;
	u32 CONFIG_REG;
	u32 CURRENT_LOAD_REG;
	u32 SELECT_REG;
	u32 REFRESH_REG;
	u32 LATENCY_REG;
	u32 RERROR_REG;
	u32 WERROR_REG;
} TRIReg;

typedef struct {
	u32 MEM_ADDR_REG;
	u32 DRAM_ADDR_REG;
	u32 RD_LEN_REG;
	u32 WR_LEN_REG;
	u32 STATUS_REG;
	u32 SEMAPHORE_REG;
	u32 PC_REG;
	u32 IBIST_REG;
} TSPReg;

typedef struct {
	u32 MODE_REG;
	u32 INTR_MASK_REG;
	u32 INTR_REG;
} TMIReg;

typedef struct {
	u32 DRAM_ADDR_REG;
	u32 DBUF_DRAM_ADDR_REG;
	u32 LEN_REG;
	u32 DBUF_LEN_REG;
	u32 CONTROL_REG;
	u32 STATUS_REG;
	u32 DACRATE_REG;
	u32 BITRATE_REG;
	u32 DBUF;
} TAIReg;

typedef struct {
	u32 DRAM_ADDR_REG;
} TSIReg;

typedef struct {
	u32 CONFIG_REG;
	u32 DEVICE_ID_REG;
	u32 DELAY_REG;
	u32 MODE_REG;
	u32 REF_INTERVAL_REG;
	u32 REF_ROW_REG;
	u32 RAS_INTERVAL_REG;
	u32 MIN_INTERVAL_REG;
	u32 ADDR_SELECT_REG;
	u32 DEVICE_MANUF_REG;
} TRDRAMReg;

typedef struct {
	u32 STATUS_REG;
	u32 DRAM_ADDR_REG;
	u32 H_WIDTH_REG;
	u32 V_INTR_REG;
	u32 V_CURRENT_LINE_REG;
	u32 TIMING_REG;
	u32 V_SYNC_REG;
	u32 H_SYNC_REG;
	u32 H_SYNC_LEAP_REG;
	u32 H_VIDEO_REG;
	u32 V_VIDEO_REG;
	u32 V_BURST_REG;
	u32 X_SCALE_REG;
	u32 Y_SCALE_REG;
} TVIReg;

typedef struct {
	u32 DRAM_ADDR_REG;
	u32 CART_ADDR_REG;
	u32 RD_LEN_REG;
	u32 WR_LEN_REG;
	u32 STATUS_REG;
	u32 BSD_DOM1_LAT_REG;
	u32 BSD_DOM1_PWD_REG;
	u32 BSD_DOM1_PGS_REG;
	u32 BSD_DOM1_RLS_REG;
	u32 BSD_DOM2_LAT_REG;
	u32 BSD_DOM2_PWD_REG;
	u32 BSD_DOM2_PGS_REG;
	u32 BSD_DOM2_RLS_REG;
} TPIReg;

typedef struct {
	u32 START_REG;
	u32 END_REG;
	u32 CURRENT_REG;
	u32 STATUS_REG;
	u32 CLOCK_REG;
	u32 BUFBUSY_REG;
	u32 PIPEBUSY_REG;
	u32 TMEM_REG;
} TDPCReg;

// PifCommand
typedef struct Comnd_s {
	int txmax, rxmax;
} Comnd;

typedef struct TController_s {
	bool PluggedIn;
	bool MemPack;
	bool HasRumble;
	bool Rumble;
	u32 KeyState;
} TController;

const int BitM1 = 0x1;
const int BitM2 = 0x3;
const int BitM3 = 0x7;
const int BitM4 = 0xF;
const int BitM5 = 0x1F;
const int BitM6 = 0x3F;
const int BitM7 = 0x7F;
const int BitM8 = 0xFF;
const int BitM9 = 0x1FF;
const int BitM10 = 0x3FF;
const int BitM11 = 0x7FF;
const int BitM12 = 0xFFF;
const int BitM13 = 0x1FFF;
const int BitM14 = 0x3FFF;
const int BitM15 = 0x7FFF;
const int BitM16 = 0xFFFF;
const int BitM17 = 0x1FFFF;
const int BitM18 = 0x3FFFF;
const int BitM19 = 0x7FFFF;
const int BitM20 = 0xFFFFF;
const int BitM21 = 0x1FFFFF;
const int BitM22 = 0x3FFFFF;
const int BitM23 = 0x7FFFFF;
const int BitM24 = 0xFFFFFF;
const int BitM25 = 0x1FFFFFF;
const int BitM26 = 0x3FFFFFF;
const int BitM27 = 0x7FFFFFF;
const int BitM28 = 0xFFFFFFF;
const int BitM29 = 0x1FFFFFFF;
const int BitM30 = 0x3FFFFFFF;
const int BitM31 = 0x7FFFFFFF;
const int BitM32 = 0xFFFFFFFF;

const int Bit1 = 0x1;
const int Bit2 = 0x2;
const int Bit3 = 0x4;
const int Bit4 = 0x8;
const int Bit5 = 0x10;
const int Bit6 = 0x20;
const int Bit7 = 0x40;
const int Bit8 = 0x80;
const int Bit9 = 0x100;
const int Bit10 = 0x200;
const int Bit11 = 0x400;
const int Bit12 = 0x800;
const int Bit13 = 0x1000;
const int Bit14 = 0x2000;
const int Bit15 = 0x4000;
const int Bit16 = 0x8000;
const int Bit17 = 0x10000;
const int Bit18 = 0x20000;
const int Bit19 = 0x40000;
const int Bit20 = 0x80000;
const int Bit21 = 0x100000;
const int Bit22 = 0x200000;
const int Bit23 = 0x400000;
const int Bit24 = 0x800000;
const int Bit25 = 0x1000000;
const int Bit26 = 0x2000000;
const int Bit27 = 0x4000000;
const int Bit28 = 0x8000000;
const int Bit29 = 0x10000000;
const int Bit30 = 0x20000000;
const int Bit31 = 0x40000000;
const int Bit32 = 0x80000000;

const int KB = 1024;
const int MB = (1024 * KB);
const int GB = (1024 * MB);

enum TComparison {
	GT, GE, LT, LE
};

// this has to be implemented by our concrete platform (e.g. android)
extern int logf(const char *format, ...);
extern int logef(const char *format, ...);
#endif   /* TYPEDEFS_H_ */
