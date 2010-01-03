#ifndef RINT_H_INCLUDED
#define RINT_H_INCLUDED

#ifdef _MSC_VER

static __inline __int64 llrint(double x)
{
	__int64 res;
	__asm
	{
		fld x
		fistp res
	}
	return res;
}

#else /* _MSC_VER */

#ifdef __cplusplus
#include <cmath>
#else
#include <math.h>
#endif

#endif /* _MSC_VER */

#endif /* RINT_H_INCLUDED */
