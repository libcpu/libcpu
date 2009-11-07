#ifndef __xec_byte_order_h
#define __xec_byte_order_h

#include "xec-base.h"

extern uint8_t const __xec_bit_swap_tbl[256];

#if defined (__GNUC__)

#define _ULL(x) x##ull

#if defined (__ppc__) || defined (__powerpc__) || defined (__ppc64__) || defined (__powerpc64__)

static __inline void __xec_byte_swap_int16_native (uint16_t *x)
{
  __asm__ __volatile__ ("lhbrx %0, %1, %2" : "=r" (*x) : "b%" (x), "r" (0) : "memory");
}

#define _XEC_SWAP16_NATIVE

static __inline void __xec_byte_swap_int32_native (uint32_t *x)
{
  __asm__ __volatile__ ("lwbrx %0, %1, %2" : "=r" (*x) : "b%" (x), "r" (0) : "memory");
}

#define _XEC_SWAP32_NATIVE

#if !defined (_XEC_LITTLE_ENDIAN) && !defined (_XEC_BIG_ENDIAN)
# if defined (__LITTLE_ENDIAN__)
#  define _XEC_LITTLE_ENDIAN 1
# elif defined (__BIG_ENDIAN__)
#  define _XEC_BIG_ENDIAN    1
# else
#  error "I don't know what endian you're running!"
# endif
#endif

#elif defined (__i386__) || defined (__x86_64__) || defined (__amd64__)
static __inline void __xec_byte_swap_int16_native (uint16_t *x)
{
  __asm__ __volatile__ ("xchgb %b0, %h0" : "+q" (*x));
}

#define _XEC_SWAP16_NATIVE

static __inline void __xec_byte_swap_int32_native (uint32_t *x)
{
#if defined (_XEC_PLATFORM_I386)
  /* i386 has no bswap! */
  __asm__ __volatile__ ("xchgb %h0, %b0\n\t"
                        "rorl $16, %0\n\t"
                        "xchgb %h0, %b0"
                        : "+q" (*x));
#else
  __asm__ __volatile__ ("bswapl %0" : "+r" (*x));
#endif
}

#define _XEC_SWAP32_NATIVE

#if defined (__x86_64__) || defined (__amd64__)
static __inline void __xec_byte_swap_int64_native (uint64_t *x)
{
  __asm__ __volatile__ ("bswapq %0" : "+r" (*x));
}

#define _XEC_SWAP64_NATIVE

#elif !defined (_XEC_PLATFORM_I386)
static __inline void __xec_byte_swap_int64_native (uint64_t *x)
{
  __asm__ __volatile__ ("bswapl %%eax\n\t"
                        "bswapl %%edx\n\t"
                        "xchgl  %%eax, %%edx\n\t"
                        : "+A" (*x));
}

#define _XEC_SWAP64_NATIVE

#endif

#ifndef _XEC_LITTLE_ENDIAN
#define _XEC_LITTLE_ENDIAN 1
#undef _XEC_BIG_ENDIAN
#endif

#define _XEC_SWAP_UNALIGNED

#endif

#else

#define _ULL(x) x

#if (defined (_MSC_VER) && (defined (_M_IX86) || defined (_M_X64))) \
    || defined (__BORLANDC__)               \
    || defined (__WATCOMC__)                \
    || defined (__DMC__)

static __inline void __xec_byte_swap_int32_native (uint32_t *x)
{
#if defined (_XEC_PLATFORM_I386)
  /* i386 has no bswap! */
  __asm
    {
      mov   edx, x
      mov   eax, [edx]
      xchg  ah, al
      ror   eax, 16
      xchg  ah, al
      mov   [edx], eax
    }
#else
  __asm
    {
      mov   edx, x
      mov   eax, [edx]
      bswap eax
      mov   [edx], eax
    }
#endif
}

#define _XEC_SWAP32_NATIVE

#if !defined (_XEC_PLATFORM_I386)
static __inline void __xec_byte_swap_int64_native (uint64_t *x)
{
#if defined (_M_X64)
  __asm
    {
      mov   rdx, x
      mov   rax, [rdx]
      bswap rax
      mov   [rdx], rax
    }
#else
  __asm
    {
      mov   edx, x
      mov   eax, [edx]
      mov   ecx, [edx+4]
      bswap eax
      bswap ecx
      mov   [edx+4], eax
      mov   [edx], ecx
    }
#endif
}

#define _XEC_SWAP64_NATIVE

#endif

#ifndef _XEC_LITTLE_ENDIAN
#define _XEC_LITTLE_ENDIAN 1
#undef _XEC_BIG_ENDIAN
#endif

#endif

#endif

/*
 * _consts swapping.
 */
#ifdef __GNUC__
#define __xec_byte_swap_is_const(x)     __builtin_constant_p (x)
#else
#define __xec_byte_swap_is_const(x)     (0)
#endif

#define __xec_byte_swap_int16_const(x)  ( (x) >> 8 | (x) << 8)
#define __xec_byte_swap_int32_const(x)  ( ( (x) & 0xff000000) >> 24 \
                                        | ( (x) & 0x00ff0000) >> 8  \
                                        | ( (x) & 0x0000ff00) << 8  \
                                        | ( (x) & 0x000000ff) << 24)
#define __xec_byte_swap_int64_const(x)  ( ( (x) & _ULL (0xff00000000000000)) >> 48 \
                                        | ( (x) & _ULL (0x00ff000000000000)) >> 32 \
                                        | ( (x) & _ULL (0x0000ff0000000000)) >> 24 \
                                        | ( (x) & _ULL (0x000000ff00000000)) >> 8  \
                                        | ( (x) & _ULL (0x00000000ff000000)) << 8  \
                                        | ( (x) & _ULL (0x0000000000ff0000)) << 24 \
                                        | ( (x) & _ULL (0x000000000000ff00)) << 32 \
                                        | ( (x) & _ULL (0x00000000000000ff)) << 48)
#if !defined (_XEC_SWAP16_NATIVE)    \
    && !defined (_XEC_SWAP32_NATIVE) \
    && !defined (_XEC_SWAP64_NATIVE)
#define _XEC_PORTABLE
#endif

static __inline void
__xec_byte_swap_int16_portable (uint16_t *x)
{
  uint8_t *b = (uint8_t *)x;
  b[0] ^= b[1];
  b[1] ^= b[0];
  b[0] ^= b[1];
}

static __inline uint16_t
__xec_byte_swap_int16_portable_inline (uint16_t x)
{
  __xec_byte_swap_int16_portable (&x);
  return x;
}

static __inline uint16_t
xec_byte_swap_int16_inplace (uint16_t *x)
{
#if defined (_XEC_PORTABLE) || !defined (_XEC_SWAP16_NATIVE)
  __xec_byte_swap_int16_portable (x);
#else
#if defined (_XEC_SWAP_UNALIGNED)
  if ( (uintptr_t)x & 1)
    __xec_byte_swap_int16_portable (x);
  else
#endif
    __xec_byte_swap_int16_native (x);
#endif
  return *x;
}

static __inline uint16_t
__xec_byte_swap_int16_inline (uint16_t x)
{
  return xec_byte_swap_int16_inplace (&x);
}

#define xec_byte_swap_int16(x) \
  (__xec_byte_swap_is_const (x) ? __xec_byte_swap_int16_const (x) : __xec_byte_swap_int16_inline (x))

static __inline void
__xec_byte_swap_int32_portable (uint32_t *x)
{
  uint8_t *b = (uint8_t *)x;
  b[0] ^= b[3];
  b[2] ^= b[1];
  b[3] ^= b[0];
  b[1] ^= b[2];
  b[0] ^= b[3];
  b[2] ^= b[1];
}

static __inline uint32_t
__xec_byte_swap_int32_portable_inline (uint32_t x)
{
  __xec_byte_swap_int32_portable (&x);
  return x;
}

static __inline uint32_t
xec_byte_swap_int32_inplace (uint32_t *x)
{
#if defined (_XEC_PORTABLE) \
    || (!defined (_XEC_SWAP32_NATIVE) && !defined (_XEC_SWAP16_NATIVE))
  __xec_byte_swap_int32_portable (x);
#else
#if !defined (_XEC_SWAP32_NATIVE) && defined (_XEC_SWAP16_NATIVE)
  uint16_t *t = (uint16_t *)x;
#endif
  
#if defined (_XEC_SWAP_UNALIGNED)
  if ( (uintptr_t)x & 3)
    __xec_byte_swap_int32_portable (x);
  else
#endif
    {
#if defined (_XEC_SWAP32_NATIVE)
      __xec_byte_swap_int32_native (x);
#else
#if defined (_XEC_LITTLE_ENDIAN)
      *x = ( (uint32_t)xec_byte_swap_int16 (t[0]) << 16) | xec_byte_swap_int16 (t[1]);
#else
      *x = ( (uint32_t)xec_byte_swap_int16 (t[1]) << 16) | xec_byte_swap_int16 (t[0]);
#endif
#endif
    }

#endif

  return *x;
}

static __inline uint32_t
__xec_byte_swap_int32_inline (uint32_t x)
{
  return xec_byte_swap_int32_inplace (&x);
}

#define xec_byte_swap_int32(x) \
  (__xec_byte_swap_is_const (x) ? __xec_byte_swap_int32_const (x) : __xec_byte_swap_int32_inline (x))

static __inline void
__xec_byte_swap_int64_portable (uint64_t *x)
{
  uint8_t *b = (uint8_t *)x;
  /*
   * Unrolled because GCC even under low optimizations
   * zaps out one of the words on PowerPC.
   */
  b[0] ^= b[7];
  b[1] ^= b[6];
  b[2] ^= b[5];
  b[3] ^= b[4];
  b[7] ^= b[0];
  b[6] ^= b[1];
  b[5] ^= b[2];
  b[4] ^= b[3];
  b[0] ^= b[7];
  b[1] ^= b[6];
  b[2] ^= b[5];
  b[3] ^= b[4];
}

static __inline uint64_t
__xec_byte_swap_int64_portable_inline (uint64_t x)
{
  __xec_byte_swap_int64_portable (&x);
  return x;
}

static __inline uint64_t
xec_byte_swap_int64_inplace (uint64_t *x)
{
#if defined (_XEC_PORTABLE) \
    || (!defined (_XEC_SWAP64_NATIVE) && !defined (_XEC_SWAP32_NATIVE))
  __xec_byte_swap_int64_portable (x);
#else
#if !defined (_XEC_SWAP64_NATIVE) && defined (_XEC_SWAP32_NATIVE)
  uint32_t *t = (uint32_t *)x;
#endif
  
#if defined (_XEC_SWAP_UNALIGNED)
  if ( (uintptr_t)x & 7)
    __xec_byte_swap_int64_portable (x);
  else
#endif
    {
#if defined (_XEC_SWAP64_NATIVE)
      __xec_byte_swap_int64_native (x);
#else
#if defined (_XEC_LITTLE_ENDIAN)
      *x = ( (uint64_t)xec_byte_swap_int32 (t[0]) << 32) | xec_byte_swap_int32 (t[1]);
#else
      *x = ( (uint64_t)xec_byte_swap_int32 (t[1]) << 32) | xec_byte_swap_int32 (t[0]);
#endif
#endif
    }

#endif
  return *x;
}

static __inline uint64_t
__xec_byte_swap_int64_inline (uint64_t x)
{
  return xec_byte_swap_int64_inplace (&x);
}

#define xec_byte_swap_int64(x) \
  (__xec_byte_swap_is_const (x) ? __xec_byte_swap_int64_const (x) : __xec_byte_swap_int64_inline (x))

/*
 * Big/Little Endian
 */
static __inline uint16_t
xec_byte_swap_little_to_host16 (uint16_t x)
{
#if defined (_XEC_LITTLE_ENDIAN)
  return x;
#else
  return xec_byte_swap_int16 (x);
#endif
}

static __inline uint16_t
xec_byte_swap_host_to_little16 (uint16_t x)
{
#if defined (_XEC_LITTLE_ENDIAN)
  return x;
#else
  return xec_byte_swap_int16 (x);
#endif
}

static __inline uint32_t
xec_byte_swap_little_to_host32 (uint32_t x)
{
#if defined (_XEC_LITTLE_ENDIAN)
  return x;
#else
  return xec_byte_swap_int32 (x);
#endif
}

static __inline uint32_t
xec_byte_swap_host_to_little32 (uint32_t x)
{
#if defined (_XEC_LITTLE_ENDIAN)
  return x;
#else
  return xec_byte_swap_int32 (x);
#endif
}

static __inline uint64_t
xec_byte_swap_little_to_host64 (uint64_t x)
{
#if defined (_XEC_LITTLE_ENDIAN)
  return x;
#else
  return xec_byte_swap_int64 (x);
#endif
}

static __inline uint64_t
xec_byte_swap_host_to_little64 (uint64_t x)
{
#if defined (_XEC_LITTLE_ENDIAN)
  return x;
#else
  return xec_byte_swap_int64 (x);
#endif
}

static __inline uint16_t
xec_byte_swap_big_to_host16 (uint16_t x)
{
#if defined (_XEC_BIG_ENDIAN)
  return x;
#else
  return xec_byte_swap_int16 (x);
#endif
}

static __inline uint16_t
xec_byte_swap_host_to_big16 (uint16_t x)
{
#if defined (_XEC_BIG_ENDIAN)
  return x;
#else
  return xec_byte_swap_int16 (x);
#endif
}

static __inline uint32_t
xec_byte_swap_big_to_host32 (uint32_t x)
{
#if defined (_XEC_BIG_ENDIAN)
  return x;
#else
  return xec_byte_swap_int32 (x);
#endif
}

static __inline uint32_t
xec_byte_swap_host_to_big32 (uint32_t x)
{
#if defined (_XEC_BIG_ENDIAN)
  return x;
#else
  return xec_byte_swap_int32 (x);
#endif
}

static __inline uint64_t
xec_byte_swap_big_to_host64 (uint64_t x)
{
#if defined (_XEC_BIG_ENDIAN)
  return x;
#else
  return xec_byte_swap_int64 (x);
#endif
}

static __inline uint64_t
xec_byte_swap_host_to_big64 (uint64_t x)
{
#if defined (_XEC_BIG_ENDIAN)
  return x;
#else
  return xec_byte_swap_int64 (x);
#endif
}

/*
 * Bit swaps
 */
static __inline uint8_t
xec_bit_swap_inplace (uint8_t  *x)
{
  return (*x = __xec_bit_swap_tbl[*x]);
}

static __inline uint8_t
__xec_bit_swap_inline (uint8_t x)
{
  return xec_bit_swap_inplace (&x);
}

#define ___xec_bit_swap_const_aa55(x) ( ( ( (x) & 0xaa) >> 1) | ( ( (x) & 0x55) << 1) )
#define ___xec_bit_swap_const_cc33(x) ( ( ( (x) & 0xcc) >> 2) | ( ( (x) & 0x33) << 2) )
#define ___xec_bit_swap_const_f00f(x) ( ( ( (x) & 0xf0) >> 4) | ( ( (x) & 0x0f) << 4) )

#define __xec_bit_swap_const(x) \
  ___xec_bit_swap_const_f00f (___xec_bit_swap_const_cc33 (___xec_bit_swap_const_aa55 (x)))

#define xec_bit_swap(x) \
  (__xec_bit_swap_is_const(x) ? __xec_bit_swap_const (x) : __xec_bit_swap_inline (x))

#endif  /* !__xec_byte_order_h */
