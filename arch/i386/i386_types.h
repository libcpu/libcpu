/*
 * libcpu: i386_types.h
 *
 * the register file
 */

#define DEFINE_REG_32_SPLIT(_extended, _reg, _high, _low) \
	union {					\
		uint32_t		_extended; \
		struct {					\
			uint16_t		_reg;	\
			struct {			\
				uint8_t		_high;	\
				uint8_t		_low;	\
			};				\
		};				\
	};

#define DEFINE_REG_32(_extended, _reg)			\
	union {				\
		uint32_t		_extended; \
		uint16_t		_reg;	\
	};
	
#define DEFINE_REG_16(_reg)			\
	struct {				\
		uint16_t		_reg; \
	};


PACKED(struct reg_i386_s {
	/*General registers */
	DEFINE_REG_32_SPLIT(eax, ax, ah, al);
	DEFINE_REG_32_SPLIT(ebx, bx, bh, bl);
	DEFINE_REG_32_SPLIT(ecx, cx, ch, cl);
	DEFINE_REG_32_SPLIT(edx, dx, dh, dl);
	/* Index registers */
	DEFINE_REG_32(esi, si);
	DEFINE_REG_32(edi, di);
	/* Pointer registers */
	DEFINE_REG_32(ebp, bp);
	DEFINE_REG_32(esp, sp);
	/* Special purpose registers */
	DEFINE_REG_32(eip, ip);
	/* Flags register */
	DEFINE_REG_32(eflags, flags);
	/* Segment registers */
	DEFINE_REG_16(cs);
	DEFINE_REG_16(ds);
	DEFINE_REG_16(ss);
	DEFINE_REG_16(es);
	DEFINE_REG_16(fs);
	DEFINE_REG_16(gs);
});
typedef struct reg_i386_s reg_i386_t;
