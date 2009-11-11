#ifndef __nix_xcpt_h
#define __nix_xcpt_h

#include <setjmp.h>

#define __nix_try					\
do {								\
	jmp_buf __jb;				    \
	volatile int __xcpt = setjmp(__jb);		\
	if (__xcpt == 0)				\
		nix_xcpt_record(&__jb);		\
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

#ifdef __cplusplus
extern "C" {
#endif

void
nix_xcpt_record(jmp_buf *jb);

void
nix_xcpt_forget(void);

#ifdef __cplusplus
}
#endif

#endif  /* !__nix_xcpt_h */
