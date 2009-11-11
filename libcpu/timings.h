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

#endif  /* !__libcpu_timings_h */
