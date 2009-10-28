typedef struct {
	union {
		struct {
			uint32_t r0;
			uint32_t r1;
			uint32_t r2;
			uint32_t r3;
			uint32_t r4;
			uint32_t r5;
			uint32_t r6;
			uint32_t r7;
			uint32_t r8;
			uint32_t r9;
			uint32_t r10;
			uint32_t r11;
			uint32_t r12;
			uint32_t r13;
			uint32_t r14;
			uint32_t r15;
		};
		uint32_t r[16];
	};
	uint32_t pc;
} reg_m68k_t;
