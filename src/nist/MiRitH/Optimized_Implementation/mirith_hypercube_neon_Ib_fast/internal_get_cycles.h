#ifndef INTERNAL_GET_CYCLES_H
#define INTERNAL_GET_CYCLES_H

#include <inttypes.h>

// TODO
#define _MAC_OS_
#define _M1CYCLES_
#if defined (OFFLINE_CC)
extern uint64_t offline_cc;
extern uint64_t begin_offline;
#endif

uint64_t _get_cycles(void);
uint64_t get_cycles(void);

uint64_t get_cycles(void);
double average(const uint64_t *cc, const uint64_t n_bench);
int cmp(const void *arg1, const void *arg2);

#endif
