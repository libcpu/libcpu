/***************************************************************************
                          endian.h  -  convert big to little endian
                             -------------------
    begin                : Thu Jan 29 2004
    copyright            : (C) 2004 by The SoftPear Project
    web                  : http://www.softpear.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef endian_h
#define endian_h

#define ARCH_LITTLEENDIAN
#ifdef ARCH_LITTLEENDIAN
static inline uint16_t BE16_toHost(uint16_t val) {
	return ((val & 0xFF00)>>8) | ((val & 0x00FF)<<8);
}
static inline uint32_t BE32_toHost(uint32_t val) {
	__asm__("bswap %0" : "=r" (val) : "0" (val));
	return val;
}
#define BE64_toHost ___swab64
#define Host_to_BE16 BE16_toHost
#define Host_to_BE32 BE32_toHost
#define Host_to_BE64 BE64_toHost
#else
#define BE16_toHost(a) (a)
#define BE32_toHost(a) (a)
#define BE64_toHost(a) (a)
#define Host_to_BE16(a) (a)
#define Host_to_BE32(a) (a)
#define Host_to_BE64(a) (a)
#endif


//#define INT32_EndianSwap(a) ((a<<24)|((a&0x0000ff00)<<8)|((a&0x00ff0000)>>8)|(a>>24))

#define ___swab16(x) \
({ \
        u_int16_t __x = (x); \
        ((u_int16_t)( \
                (((u_int16_t)(__x) & (u_int16_t)0x00ffU) << 8) | \
                (((u_int16_t)(__x) & (u_int16_t)0xff00U) >> 8) )); \
})

#define ___swab32(x) \
({ \
        u_int32_t __x = (x); \
        ((u_int32_t)( \
                (((u_int32_t)(__x) & (u_int32_t)0x000000ffUL) << 24) | \
                (((u_int32_t)(__x) & (u_int32_t)0x0000ff00UL) <<  8) | \
                (((u_int32_t)(__x) & (u_int32_t)0x00ff0000UL) >>  8) | \
                (((u_int32_t)(__x) & (u_int32_t)0xff000000UL) >> 24) )); \
})

#define ___swab64(x) \
({ \
        u_int64_t __x = (x); \
        ((u_int64_t)( \
                (u_int64_t)(((u_int64_t)(__x) & (u_int64_t)0x00000000000000ffULL) << 56) | \
                (u_int64_t)(((u_int64_t)(__x) & (u_int64_t)0x000000000000ff00ULL) << 40) | \
                (u_int64_t)(((u_int64_t)(__x) & (u_int64_t)0x0000000000ff0000ULL) << 24) | \
                (u_int64_t)(((u_int64_t)(__x) & (u_int64_t)0x00000000ff000000ULL) <<  8) | \
                (u_int64_t)(((u_int64_t)(__x) & (u_int64_t)0x000000ff00000000ULL) >>  8) | \
                (u_int64_t)(((u_int64_t)(__x) & (u_int64_t)0x0000ff0000000000ULL) >> 24) | \
                (u_int64_t)(((u_int64_t)(__x) & (u_int64_t)0x00ff000000000000ULL) >> 40) | \
                (u_int64_t)(((u_int64_t)(__x) & (u_int64_t)0xff00000000000000ULL) >> 56) )); \
})

typedef enum {big_endian, little_endian, unknown_endian} endianess;

#endif
