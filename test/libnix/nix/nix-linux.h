#ifndef __nix_linux_h
#define __nix_linux_h

#include "nix-types.h"

typedef struct _nix_linux_cap_user_handler *nix_linux_cap_user_handler_t;
typedef struct _nix_linux_cap_user_data *nix_linux_cap_user_data_t;

struct nix_linux_module {
	int dummy;
};

struct nix_linux_kernel_sym {
	int dummy;
};
 
struct nix_linux_ustat {
	uint64_t  f_tfree;
	nix_ino_t f_tinode;
	char      f_fname[6];
	char      f_fpack[6];
};

struct nix_linux_sysinfo {
	int dummy;
};

struct nix_linux_nfsctl_arg {
	int dummy;
};

union nix_linux_nfsctl_res {
	int dummy;
};

struct nix_linux_timex {
	int dummy;
};

#endif  /* !__nix_linux_h */
