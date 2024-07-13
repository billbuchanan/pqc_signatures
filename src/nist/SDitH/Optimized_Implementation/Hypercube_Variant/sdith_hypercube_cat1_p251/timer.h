#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>
#include <sys/time.h>

#ifdef __cplusplus
#define EXPORT extern "C"
#else
#define EXPORT
#endif


#define kScalingFactor (2.6/3.8)

typedef struct sdith_bench_timer_t {
  unsigned int counter;
  // gettimeofday
  double nb_milliseconds;
  struct timeval start, stop;
  // rdtscp
  uint64_t nb_cycles;
  unsigned int garbage;
  uint64_t cstart, cstop;
} sdith_bench_timer;

EXPORT void sdith_bench_timer_init(sdith_bench_timer* timer);
EXPORT void sdith_bench_timer_start(sdith_bench_timer* timer);
EXPORT void sdith_bench_timer_count(sdith_bench_timer* timer);
EXPORT void sdith_bench_timer_end(sdith_bench_timer* timer);
EXPORT double sdith_bench_timer_diff(sdith_bench_timer* timer);
EXPORT uint64_t sdith_bench_timer_diff_cycles(sdith_bench_timer* timer);
EXPORT double sdith_bench_timer_get(sdith_bench_timer* timer);
EXPORT double sdith_bench_timer_get_cycles(sdith_bench_timer* timer);

#ifdef __cplusplus
namespace bench {
using timer = sdith_bench_timer;
inline void timer_init(timer* timer) { return sdith_bench_timer_init(timer); }
inline void timer_start(timer* timer) { return sdith_bench_timer_start(timer); }
inline void timer_count(timer* timer) { return sdith_bench_timer_count(timer); }
inline void timer_end(timer* timer) { return sdith_bench_timer_end(timer); }
inline double timer_diff(timer* timer) { return sdith_bench_timer_diff(timer); }
inline uint64_t timer_diff_cycles(timer* timer) { return sdith_bench_timer_diff_cycles(timer); }
inline double timer_get(timer* timer) { return sdith_bench_timer_get(timer); }
inline double timer_get_cycles(timer* timer) { return sdith_bench_timer_get_cycles(timer); }
}  // namespace bench
#endif // C++


#endif
