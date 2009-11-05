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
			uint32_t r16;
			uint32_t r17;
			uint32_t r18;
			uint32_t r19;
			uint32_t r20;
			uint32_t r21;
			uint32_t r22;
			uint32_t r23;
			uint32_t r24;
			uint32_t r25;
			uint32_t r26;
			uint32_t r27;
			uint32_t r28;
			uint32_t r29;
			uint32_t r30;
			uint32_t r31;
		};
		uint32_t gpr[32];
	};
	union {
		struct {
			uint64_t x0;
			uint64_t x1;
			uint64_t x2;
			uint64_t x3;
			uint64_t x4;
			uint64_t x5;
			uint64_t x6;
			uint64_t x7;
			uint64_t x8;
			uint64_t x9;
			uint64_t x10;
			uint64_t x11;
			uint64_t x12;
			uint64_t x13;
			uint64_t x14;
			uint64_t x15;
			uint64_t x16;
			uint64_t x17;
			uint64_t x18;
			uint64_t x19;
			uint64_t x20;
			uint64_t x21;
			uint64_t x22;
			uint64_t x23;
			uint64_t x24;
			uint64_t x25;
			uint64_t x26;
			uint64_t x27;
			uint64_t x28;
			uint64_t x29;
			uint64_t x30;
			uint64_t x31;
		};
		uint64_t xfr[32];
	};
	uint32_t sxip; /* Execution IP */
	uint32_t snip; /* Next IP */
	uint32_t sfip; /* Fetch IP */
} reg_m88k_t;
