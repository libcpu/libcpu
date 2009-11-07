#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/sem.h>

#include "nix.h"

int
nix_s5_semget(nix_key_t key, int nsems, int flag, nix_env_t *env)
{
	return (nix_nosys(env));
}

int
nix_s5_semop(int semid, struct nix_sembuf *buf, size_t nops, nix_env_t *env)
{
	return (nix_nosys(env));
}

int
nix_s5_semctl(int semid, int semnum, int cmd, void *arg, nix_env_t *env)
{
	return (nix_nosys(env));
}
