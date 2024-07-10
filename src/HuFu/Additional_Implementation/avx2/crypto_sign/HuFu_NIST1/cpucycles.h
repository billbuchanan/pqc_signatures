#ifndef CPUCYCLES_H
#define CPUCYCLES_H

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef USE_RDPMC  /* Needs echo 2 > /sys/devices/cpu/rdpmc */

static inline uint64_t cpucycles(void) {
  const uint32_t ecx = (1U << 30) + 1;
  uint64_t result;

  __asm__ volatile ("rdpmc; shlq $32,%%rdx; orq %%rdx,%%rax"
    : "=a" (result) : "c" (ecx) : "rdx");

  return result;
}

#else

static inline uint64_t cpucycles(void) {
  uint64_t result;

  __asm__ volatile ("rdtsc; shlq $32,%%rdx; orq %%rdx,%%rax"
    : "=a" (result) : : "%rdx");

  return result;
}

#endif

uint64_t cpucycles_overhead(void);
void print_singleres(const char *s, uint64_t t0, uint64_t t1);
void print_results(const char *s, uint64_t *t0, uint64_t *t1, size_t tlen);

#endif
