
typedef union _fp32_reg { 
	uint32_t i;
	float f;
} fp32_reg_t;

typedef union _fp64_reg { 
	union {
		uint32_t i32[2];
		uint64_t i64;
	} i;
	union {
		float  f32[2];
		double f64;
	} f;
} fp64_reg_t;

typedef union _fp80_reg {
#if defined(__i386__) || defined(__x86_64__)
	long double f;
#endif
	struct {
#ifdef __LITTLE_ENDIAN__
		uint64_t lo;
		uint16_t hi;
		uint16_t _unused_2;
		uint32_t _unused_1;
#else
		uint32_t _unused_1;
		uint16_t _unused_2;
		uint16_t hi;
		uint64_t lo;
#endif
	} i;
} __attribute__((aligned(16))) fp80_reg_t;

typedef struct _fp128_reg {
#ifdef __LITTLE_ENDIAN__
	uint64_t lo;
	uint64_t hi;
#else
	uint64_t hi;
	uint64_t lo;
#endif
} fp128_reg_t;

