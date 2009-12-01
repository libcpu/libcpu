#include <sys/resource.h>
#include "libcpu.h"

void update_timing(cpu_t *cpu, int index, bool start)
{
	struct rusage r_usage;
	uint64_t usec;

	if ((cpu->flags_debug & CPU_DEBUG_PROFILE) == 0)
		return;

	getrusage(RUSAGE_SELF, &r_usage);
	usec = ((uint64_t)r_usage.ru_utime.tv_sec * 1000000) + r_usage.ru_utime.tv_usec;

	if (start)
		cpu->timer_start[index] = usec;
	else
		cpu->timer_total[index] += usec - cpu->timer_start[index];
}
