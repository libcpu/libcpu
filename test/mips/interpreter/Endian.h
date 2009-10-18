#ifndef ENDIAN_H_
#define ENDIAN_H_

#include "Typedefs.h"
#include "stdincludes.h"

//#define BigEndian

class Endian {
public:
	inline static u32 Address8(u32 address) {
#ifdef BigEndian
		return address;
#else
		return address ^ 3;
#endif
	}

	inline static u32 Address16(u32 address) {
#ifdef BigEndian
		return address;
#else
		return address ^ 2;
#endif
	}

	inline static void Write8(u8* target, u32 address, u8 value) {
#ifdef BigEndian
		*(target + address) = value;
#else
		*(target + Address8(address)) = value;
#endif
	}

	inline static void Write16(u8* target, u32 address, u16 value) {
#ifdef BigEndian
		*(u16*) (target + address) = value;
#else
		*(u16*) (target + Address16(address)) = value;
#endif
	}

	inline static void Write32(u8* target, u32 address, u32 value) {
		*(u32*) (target + address) = value;
	}

	inline static u8 Read8(u8* target, u32 address) {
#ifdef BigEndian
		return *(target + address);
#else
		return *(target + Address8(address));
#endif
	}

	inline static u16 Read16(u8* target, u32 address) {
#ifdef BigEndian
		return *(u16*) (target + address);
#else
		return *(u16*) (target + Address16(address));
#endif
	}

	inline static u32 Read32(u8* target, u32 address) {
		return *(u32*) (target + address);
	}

	inline u32 Swapped32(u32 value) {
		return (((value & 0xff000000) >> 24) | ((value & 0x00ff0000) >> 8)
				| ((value & 0x0000ff00) << 8) | ((value & 0x000000ff) << 24));
	}

	inline u16 Swapped16(u16 value) {
		return ((value & BitM8) << 8) || ((value >> 8) & BitM8);
	}

	static void UnalignedMemCopy(void *target, const void *source, size_t size) {
#ifdef BigEndian
		memcpy(target, source, size);
		return;
#else
		if ((intptr_t) (target) & Bit2) {
			printf("Error in ua_memcpy: Target address not aligned\n");
		} else {
			if ((intptr_t) (source) & Bit2) {
				// unaligned source address (divisable by 2) but aligned target address
				for (u32 i = 0; i < (size >> 1); i++) {
					*(u16*) (Address16((intptr_t) ((u16*) target + i)))
							= *(u16*) (Address16((intptr_t) ((u16*) source + i)));
				}
				switch (size & BitM1) {
				case 1:
					*(u8*) (Address8((intptr_t) ((u8*) target + size - 1)))
							= *(u8*) (Address8((intptr_t) ((u8*) source + size
									- 1)));
					break;
				}
			} else {
				// aligned source address
				switch (size & BitM2) {
				case 0:
					memcpy(target, source, size);
					break;
				case 1:
					memcpy(target, source, size - 1);
					*(u8*) (Address8((intptr_t) ((u8*) target + size - 1)))
							= *(u8*) (Address8((intptr_t) ((u8*) source + size
									- 1)));
					break;
				case 2:
					memcpy(target, source, size - 2);
					*(u8*) (Address8((intptr_t) ((u8*) target + size - 2)))
							= *(u8*) (Address8((intptr_t) ((u8*) source + size
									- 2)));
					*(u8*) (Address8((intptr_t) ((u8*) target + size - 1)))
							= *(u8*) (Address8((intptr_t) ((u8*) source + size
									- 1)));
					break;
				case 3:
					memcpy(target, source, size - 3);
					*(u8*) (Address8((intptr_t) ((u8*) target + size - 3)))
							= *(u8*) (Address8((intptr_t) ((u8*) source + size
									- 3)));
					*(u8*) (Address8((intptr_t) ((u8*) target + size - 2)))
							= *(u8*) (Address8((intptr_t) ((u8*) source + size
									- 2)));
					*(u8*) (Address8((intptr_t) ((u8*) target + size - 1)))
							= *(u8*) (Address8((intptr_t) ((u8*) source + size
									- 1)));
					break;
				}
			}
		}
#endif
	}
};

#endif /* ENDIAN_H_ */
