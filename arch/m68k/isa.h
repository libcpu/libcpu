#include "types.h"

#define RAM16(a) (int16_t)(RAM[a] << 8 | RAM[a+1])
#define RAM32(a) (int32_t)(RAM[a] << 24 | RAM[a+1] << 16 | RAM[a+2] << 8 | RAM[a+3])

#define SGN8(a) ((a>0x7F)? '-' : '+')
#define ABS8(a) ((a>0x7F)? 0x100-a : a)
#define SGN16(a) ((a>0x7FFF)? '-' : '+')
#define ABS16(a) ((a>0x7FFF)? 0x10000-a : a)

#define ASSERT(x) \
	if (!(x)) { \
		printf("Disasm error at %s:%d: [$%04X] %02X\n", __FILE__, __LINE__, pc, opcode); \
		exit(1); \
	}


int32_t arch_disasm_get_disp(uint8_t *RAM, addr_t pc, uint16_t opcode);

extern const char sizechar[];

enum {
	SIZE_INVLD = 0,
	SIZE_B = 1,
	SIZE_L = 2,
	SIZE_W = 3
};

extern const char *condstr[];
extern const char *condstr_db[];

static inline unsigned int
bits(uint16_t word, unsigned int a, unsigned int b) {
	uint16_t mask;
	b++;
	mask = ((1 << b) - 1) - ((1 << a) - 1);
//	printf("(word = 0x%x) a = %d, b = %d, mask = 0x%x 0x%x 0x%x  ret=0x%x\n", word, a, b, mask, (1 << a) - 1, (1 << b) - 1, (word & mask) >> a);
	return (word & mask) >> a;
}

