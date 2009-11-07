#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/msg.h>

#include "nix.h"

int
nix_s5_msgget(nix_key_t key, int msgflg, nix_env_t *env)
{
	return (nix_nosys(env));
}

int
nix_s5_msgsnd(int msqid, void const *msgp, size_t msgsz, int msgflg, nix_env_t *env)
{
	return (nix_nosys(env));
}

int
nix_s5_msgrcv(int msqid, void *msgp, size_t *msgsz, long msgtyp, int msgflg, nix_env_t *env)
{
	return (nix_nosys(env));
}

int
nix_s5_msgctl(int msqid, int cmd, void *arg, nix_env_t *env)
{
	return (nix_nosys(env));
}
