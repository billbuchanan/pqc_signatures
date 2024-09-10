#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#if defined (OFFLINE_CC)
uint64_t offline_cc;
uint64_t begin_offline;
#endif




// uint64_t get_cycles(void) {
// #if defined(__GNUC__)
//   uint32_t lo, hi;
//   __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
//   return ((uint64_t)lo | ((uint64_t)hi << 32));
// #else
//   return 0; /* Undefined for now; should be obvious in the output */
// #endif
// }

int cmp(const void *arg1, const void *arg2)
{
	uint64_t v1, v2;

	v1 = *(const uint64_t *)arg1;
	v2 = *(const uint64_t *)arg2;
	if (v1 < v2) {
		return -1;
	} else if (v1 == v2) {
		return 0;
	} else {
		return 1;
	}
}

double average(const uint64_t *cc, const uint64_t n_bench)
{
    int i;
    uint64_t acc = 0;

    for (i = 0; i < n_bench; i++)
    {
        acc += cc[i];
    }

    return (double)acc / n_bench;
}
