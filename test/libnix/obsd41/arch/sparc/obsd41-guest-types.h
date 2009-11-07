#ifndef __obsd41_sparc_types_h
#define __obsd41_sparc_types_h

#define OBSD41_MACHINE_NAME "sparc32"

typedef int32_t  obsd41_time_t;

typedef int32_t  obsd41_long_t;
typedef uint32_t obsd41_ulong_t;

typedef int32_t  obsd41_intptr_t;
typedef uint32_t obsd41_uintptr_t;

#ifdef __GNUC__
#define __obsd41_guest_alignment __attribute__ ((aligned (4)))
#else
#define __obsd41_guest_alignment
#endif

#endif  /* !__obsd41_sparc_types_h */
