/* XXX: high and low byte endianess! */
#define DEFINE_SPLIT_REG(_reg, _high, _low) \
	union {					\
		uint16_t		_reg;	\
		struct {			\
			uint8_t		_high;	\
			uint8_t		_low;	\
		};				\
	};

#define DEFINE_REG(_reg)			\
	struct {				\
		uint16_t		_reg;	\
	}

PACKED(struct reg_8086_s {
	/*General registers */
	DEFINE_SPLIT_REG(ax, ah, al);
	DEFINE_SPLIT_REG(bx, bh, bl);
	DEFINE_SPLIT_REG(cx, ch, cl);
	DEFINE_SPLIT_REG(dx, dh, dl);
	/* Index registers */
	DEFINE_REG(si);
	DEFINE_REG(di);
	/* Pointer registers */
	DEFINE_REG(bp);
	DEFINE_REG(sp);
	/* Special purpose registers */
	DEFINE_REG(ip);
	/* Flags register */
	DEFINE_REG(flags);
	/* Segment registers */
	DEFINE_REG(cs);
	DEFINE_REG(ds);
	DEFINE_REG(ss);
	DEFINE_REG(es);
});
typedef struct reg_8086_s reg_8086_t;
