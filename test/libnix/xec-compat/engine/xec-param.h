#ifndef __xec_param_h
#define __xec_param_h

#include "xec-base.h"

typedef enum _xec_param_type
  {
    XEC_PARAM_INVALID = 0,

    XEC_PARAM_BYTE,
    XEC_PARAM_HALF,
    XEC_PARAM_WORD,
    XEC_PARAM_DWORD,
    XEC_PARAM_SINGLE,
    XEC_PARAM_DOUBLE,
    XEC_PARAM_EXTENDED,
    XEC_PARAM_VECTOR,
    XEC_PARAM_POINTER,
    XEC_PARAM_INTPTR /* guest dependant */
  } xec_param_type_t;

typedef struct _xec_param
  {
    xec_param_type_t type;
    union
      {
        union
          {
#ifdef __LITTLE_ENDIAN__
            union { int64_t s8  : 8;  int64_t : 56; };
            union { int64_t s16 : 16; int64_t : 48; };
            union { int64_t s32 : 32; int64_t : 32; };
#else
            union { int64_t : 56; int64_t s8  : 8;  };
            union { int64_t : 48; int64_t s16 : 16; };
            union { int64_t : 32; int64_t s32 : 32; };
#endif
            int64_t s64;
          } tsign;

        union
          {
#ifdef __LITTLE_ENDIAN__
            union { uint64_t u8  : 8;  uint64_t : 56; };
            union { uint64_t u16 : 16; uint64_t : 48; };
            union { uint64_t u32 : 32; uint64_t : 32; };
#else
            union { uint64_t : 56; uint64_t u8  : 8;  };
            union { uint64_t : 48; uint64_t u16 : 16; };
            union { uint64_t : 32; uint64_t u32 : 32; };
#endif

            uint64_t u64;
          } tnosign;

        union
          {
            float s;
            double d;
            long double x;
          } fp;

        char  vector[16];
        void *pointer;
      } value;
  } xec_param_t;

#endif  /* !__xec_param_h */
