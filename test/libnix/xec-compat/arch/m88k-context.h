
// XXX to be integrated with libcpu, remove me! for now identical layout

typedef struct _m88k_context {
  uint32_t gpr[32];
  uint32_t psr;
  uint32_t trapno;
  uint32_t sxip;
} m88k_context_t;

typedef uint32_t m88k_uintptr_t;

