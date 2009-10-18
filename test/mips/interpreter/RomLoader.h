#ifndef ROMLOADER_H_
#define ROMLOADER_H_

#include "Typedefs.h"
#include "stdincludes.h"

class RomLoader {
public:
	static u8* LoadRomFile(std::string romFile, u32 &out_RomSize) {
		printf("Loading %s...", romFile.c_str());
		std::ifstream file(romFile.c_str(), std::ios::in);

		// Determine Size...
		long begin, end;
		begin = file.tellg();
		file.seekg(0, std::ios::end);
		end = file.tellg();
		file.seekg(0, std::ios::beg);
		out_RomSize = end - begin;

		// Read whole file
		u8* result = (u8*) malloc(out_RomSize);
		file.read((char*) &result[0], out_RomSize);
		file.close();
		printf("Done\n");

		// Make a 32 bit read and then fix the bytes by shifting to make that right
		// that is the only portable way. Do not use byte indexing below, as
		// that requires knowledge of endianness.
		u32 endian = ((u32*) result)[0];
		printf("Endian mark: %x\n", endian);
		// Flip the bytes into the correct byte order
		switch (endian) {
		case 0x80371240:
		case 0x80371241:
			// No flipping needed
			break;
			/*case 0x37804012:
			 case 0x37804112:
			 // Swap Bytes
			 for (u32 i=0;i<out_RomSize;i+=2)
			 {
			 u8 b = result[i];
			 result[i] = result[i+1];
			 result[i+1] = b;
			 }
			 break;*/
		case 0x12408037:
		case 0x12418037:
			// Swap words
			for (u32 i = 0; i < (out_RomSize >> 2); i++) {
				u32 data = ((u32*) result)[i];
				u32 flippedData = (data >> 16) | (data << 16);
				((u32*) result)[i] = flippedData;
			}
			break;
			/*case 0x40123780:
			 case 0x41123780:
			 // Swap bytes and words
			 for (u32 i=0;i<romsize;i+=4)
			 {
			 BSWAPValue(((U32*)(ROM + i)));
			 if (((i & 0xFFFF) == 0) && (ShowProgress)){
			 sprintf(&Title[0], "%d%%", (i>>2)*100/(romsize>>2));  // use >>2 against overflow on 512 mbit roms
			 SetStatusText(1, Title);
			 }
			 }
			 break;*/
		default:
			printf("This is most likely not a n64 rom. It has an invalid rom type\n");
		}
		u32 endianAfter = ((u32*) result)[0];
		printf("Endian mark after flip: %x\n", endianAfter);

		return result;
	}
};

#endif /* ROMLOADER_H_ */
