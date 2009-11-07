#ifndef __nix_xcpt_h
#define __nix_xcpt_h

#include <setjmp.h>

#define __nix_try					\
do {								\
	sigjmp_buf __jb;				\
	int __xcpt = _setjmp(__jb);		\
	if (__xcpt == 0)				\
		nix_xcpt_record(__jb);		\
	else							\
    	nix_xcpt_forget();			\
	if (__xcpt == 0) {
  
#define __nix_catch_any				\
	} else {						\
		nix_xcpt_forget();

#define __nix_end_try				\
	}								\
	nix_xcpt_forget();				\
} while (0);

void
nix_xcpt_record(sigjmp_buf jb);

void
nix_xcpt_forget(void);

#endif  /* !__nix_xcpt_h */
