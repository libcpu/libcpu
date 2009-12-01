#ifndef __libcpu_timings_h
#define __libcpu_timings_h

#ifdef __MACH__
#include <mach/mach_time.h>

#define abs_time() mach_absolute_time()
#endif

#ifdef sun
#include <sys/time.h>

#define abs_time() gethrtime()
#endif

#ifdef linux
#warning High precission timing currently n/a on Linux

// HACK
#define abs_time() 0
#endif

#ifdef _WIN32
typedef unsigned long       DWORD;
#define WINAPI      __stdcall
extern "C" DWORD WINAPI GetTickCount(void);
#define abs_time() GetTickCount()
#endif

#endif  /* !__libcpu_timings_h */
