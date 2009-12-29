/*
 * libcpu: timings.cpp
 *
 * Implementations of timing functions for certain architectures.
 */

#include "timings.h"

#ifdef HAVE_LIBRT

#include <time.h>
#include <stdint.h>

uint64_t abs_time() {
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts) < 0)
    return 0;
  return ts.tv_sec * 1000000000 + ts.tv_nsec;
}

#endif
