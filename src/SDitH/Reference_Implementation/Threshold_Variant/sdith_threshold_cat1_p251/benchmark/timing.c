#include "timing.h"
#ifdef BENCHMARK_CYCLES
#include <x86intrin.h>
#endif

void btimer_init(btimer_t* timer) {
    if(timer != NULL) {
        timer->counter = 0;
        timer->nb_milliseconds = 0.;
        timer->nb_cycles = 0;
    }
}
void btimer_count(btimer_t *timer) {
    if(timer != NULL)
        timer->counter++;
}
void btimer_start(btimer_t *timer) {
    if(timer != NULL) {
        gettimeofday(&timer->start, NULL);
      #ifdef BENCHMARK_CYCLES
        timer->cstart = __rdtscp(&timer->garbage);
      #endif
    }
}
double btimer_diff(btimer_t *timer) {
    return ( (timer->stop.tv_sec - timer->start.tv_sec) * 1000000 + (timer->stop.tv_usec - timer->start.tv_usec) )/1000.;
}
uint64_t btimer_diff_cycles(btimer_t *timer) {
    return (timer->cstop - timer->cstart);
}
void btimer_end(btimer_t *timer) {
    if(timer != NULL) {
        gettimeofday(&timer->stop, NULL);
      #ifdef BENCHMARK_CYCLES
        timer->cstop = __rdtscp(&timer->garbage);
      #endif
        timer->nb_milliseconds += btimer_diff(timer);
      #ifdef BENCHMARK_CYCLES
        timer->nb_cycles += btimer_diff_cycles(timer);
      #endif
    }
}
double btimer_get(btimer_t *timer) {
    return timer->nb_milliseconds/timer->counter;
}
double btimer_get_cycles(btimer_t *timer) {
    return (double)timer->nb_cycles/timer->counter;
}
