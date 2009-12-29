#ifndef __libcpu_timings_h
#define __libcpu_timings_h

#include "config.h"

#ifdef __MACH__
#include <mach/mach_time.h>
#define abs_time() mach_absolute_time()

#elif defined(sun)
#include <sys/time.h>
#define abs_time() gethrtime()

#elif defined(_WIN32)
typedef unsigned long       DWORD;
#define WINAPI      __stdcall
extern "C" DWORD WINAPI GetTickCount(void);
#define abs_time() GetTickCount()

#elif defined(HAVE_LIBRT)
#include <stdint.h>
uint64_t abs_time();

#else
#error "No timing functions defined for your platform"

#endif

#endif  /* !__libcpu_timings_h */
