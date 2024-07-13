#include "timer.h"

#include <x86intrin.h>

void sdith_bench_timer_init(sdith_bench_timer* timer) {
  if (timer != NULL) {
    timer->counter = 0;
    timer->nb_milliseconds = 0.;
    timer->nb_cycles = 0;
  }
}
void sdith_bench_timer_count(sdith_bench_timer* timer) {
  if (timer != NULL) timer->counter++;
}
void sdith_bench_timer_start(sdith_bench_timer* timer) {
  if (timer != NULL) {
    gettimeofday(&timer->start, NULL);
    timer->cstart = __rdtscp(&timer->garbage);
  }
}
double sdith_bench_timer_diff(sdith_bench_timer* timer) {
  return ((timer->stop.tv_sec - timer->start.tv_sec) * 1000000 + (timer->stop.tv_usec - timer->start.tv_usec)) / 1000.;
}
uint64_t sdith_bench_timer_diff_cycles(sdith_bench_timer* timer) { return (timer->cstop - timer->cstart); }
void sdith_bench_timer_end(sdith_bench_timer* timer) {
  if (timer != NULL) {
    gettimeofday(&timer->stop, NULL);
    timer->cstop = __rdtscp(&timer->garbage);
    timer->nb_milliseconds += sdith_bench_timer_diff(timer);
    timer->nb_cycles += sdith_bench_timer_diff_cycles(timer);
  }
}
double sdith_bench_timer_get(sdith_bench_timer* timer) { return timer->nb_milliseconds * kScalingFactor / timer->counter; }
double sdith_bench_timer_get_cycles(sdith_bench_timer* timer) { return (double)timer->nb_cycles / timer->counter; }
