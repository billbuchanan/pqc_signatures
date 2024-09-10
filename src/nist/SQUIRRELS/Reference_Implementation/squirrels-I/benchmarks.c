/*
 * Benchmarking script.
 *
 * ==========================(LICENSE BEGIN)============================
 *
 * Copyright (c) 2023  Squirrels Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * ===========================(LICENSE END)=============================
 *
 * @author   Guilhem Niot <guilhem.niot@gniot.fr>
 */

#include "../../KAT/generator/katrng.h"
#include "api.h"
#include "inner.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <x86intrin.h>

#define BENCH_ITER 2000
#define BENCH_ITER2 10

void benchmark_keygen() {
  /*
   * Initialize a RNG.
   */
  unsigned char seed[48];
  inner_shake256_context rng;
  randombytes(seed, sizeof seed);
  inner_shake256_init(&rng);
  inner_shake256_inject(&rng, seed, sizeof seed);
  inner_shake256_flip(&rng);

  unsigned char *sk = malloc(CRYPTO_SECRETKEYBYTES);
  unsigned char *pk = malloc(CRYPTO_PUBLICKEYBYTES);

  clock_t start = clock();
  uint64_t startc = __rdtsc();
  for (int i = 0; i < BENCH_ITER2; ++i) {
    clock_t start2 = clock();
    crypto_sign_keypair(pk, sk);
    printf("Keygen %d/%d (took %f s)\n", i + 1, BENCH_ITER2,
           (double)(clock() - start2) / CLOCKS_PER_SEC);
  }
  uint64_t stopc = __rdtsc();
  clock_t stop = clock();
  double delta = (double)(stop-start) / CLOCKS_PER_SEC;
  printf("Elapsed time for %d key generation: %f. (%f s per sampling, "
         "%ld cycles per sampling) \n"
         "keygens/sec = %f\n",
         BENCH_ITER2, delta, (delta / (BENCH_ITER2)),
         (stopc - startc) / (BENCH_ITER2), (BENCH_ITER2) / delta);

  free(sk);
  free(pk);
}

void benchmark_sign(const unsigned char *sk) {
  /*
   * Initialize a RNG.
   */
  unsigned char seed[48];
  inner_shake256_context rng;
  randombytes(seed, sizeof seed);
  inner_shake256_init(&rng);
  inner_shake256_inject(&rng, seed, sizeof seed);
  inner_shake256_flip(&rng);

  // Then measure signature perf
  clock_t start = clock();
  uint64_t startc = __rdtsc();
  for (int i = 0; i < BENCH_ITER; ++i) {
    unsigned char sig[CRYPTO_BYTES];
    unsigned long long siglen;
    unsigned char m[] = "test";
    crypto_sign(sig, &siglen, m, sizeof(m), sk);
  }
  uint64_t stopc = __rdtsc();
  clock_t stop = clock();
  double delta = (double)(stop - start) / CLOCKS_PER_SEC;
  printf("Elapsed time for %d signature generation: %f. (%f ms per sampling, "
         "%ld cycles per sampling) \n"
         "signatures/sec = %f\n",
         BENCH_ITER, delta, (delta / (BENCH_ITER)) * 1000,
         (stopc - startc) / (BENCH_ITER), (BENCH_ITER) / delta);
}

void benchmark_verif(unsigned char *sk, unsigned char *pk) {
  /*
   * Initialize a RNG.
   */
  unsigned char seed[48];
  inner_shake256_context rng;
  randombytes(seed, sizeof seed);
  inner_shake256_init(&rng);
  inner_shake256_inject(&rng, seed, sizeof seed);
  inner_shake256_flip(&rng);

  // Then measure signature perf
  unsigned char sig[CRYPTO_BYTES];
  unsigned char m[] = "test";
  unsigned long long siglen, mlen = sizeof(m);

  crypto_sign(sig, &siglen, m, mlen, sk);

  clock_t start = clock();
  uint64_t startc = __rdtsc();
  for (int i = 0; i < BENCH_ITER; ++i) {
    crypto_sign_open(m, &mlen, sig, siglen, pk);
  }
  uint64_t stopc = __rdtsc();
  clock_t stop = clock();
  double delta = (double)(stop-start) / CLOCKS_PER_SEC;
  printf("Elapsed time for %d signature verification: %f. (%f ms per sampling, "
         "%ld cycles per sampling) \n"
         "verification/sec = %f\n",
         BENCH_ITER, delta, (delta / (BENCH_ITER)) * 1000,
         (stopc - startc) / (BENCH_ITER), (BENCH_ITER) / delta);
}

void run_benchmarks() {
  static unsigned char entropy_input[48];
  for (int i = 0; i < 48; i++)
    entropy_input[i] = i;

  randombytes_init(entropy_input, NULL, 256);

  unsigned char *sk = malloc(CRYPTO_SECRETKEYBYTES);
  unsigned char *pk = malloc(CRYPTO_PUBLICKEYBYTES);
  if(crypto_sign_keypair(pk, sk) == -1) {
      printf("Fatal error in keygen\n");
      return;
  }

  benchmark_keygen();
  benchmark_sign(sk);
  benchmark_verif(sk, pk);

  free(sk);
  free(pk);
}

int main() {
  srand(time(0));
  printf("Hello world, signature is Squirrels %u\n", SQUIRRELS_LEVEL);

  printf("Speed\n");
  run_benchmarks();

  return 0;
}
