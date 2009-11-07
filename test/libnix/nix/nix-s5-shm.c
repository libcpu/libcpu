#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/shm.h>

#include "nix.h"

int
nix_s5_shmget(nix_key_t key, size_t size, int shmflg, nix_env_t *env)
{
	return (nix_nosys(env));
}

uintmax_t
nix_s5_shmat(int shmid, uintmax_t shmaddr, int shmflg, nix_env_t *env)
{
	return (nix_nosys(env));
}

int
nix_s5_shmdt(uintmax_t shmaddr, nix_env_t *env)
{
	return (nix_nosys(env));
}

int
nix_s5_shmctl(int shmid, int cmd, void *arg, nix_env_t *env)
{
	return (nix_nosys(env));
}
