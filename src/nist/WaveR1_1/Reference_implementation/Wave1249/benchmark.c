/******************************************************************************
WAVE -- Code-Based Digital Signature Scheme
Copyright (c) 2023 The Wave Team
contact: wave-contact@inria.fr

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
******************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "api.h"
#include "cpucycles.h"
#include "prng/prng.h"

#define ITERATIONS 10

uint64_t timekey_f[ITERATIONS];
uint64_t time_sign_f[ITERATIONS];
uint64_t time_verify_f[ITERATIONS];

void swap(uint64_t *a, uint64_t *b) {
  uint64_t t = *a;
  *a = *b;
  *b = t;
}

// A function to implement bubble sort
void bubbleSort(uint64_t arr[], int n) {
  int i, j;
  for (i = 0; i < n - 1; i++)

    // Last i elements are already in place
    for (j = 0; j < n - i - 1; j++)
      if (arr[j] > arr[j + 1]) swap(&arr[j], &arr[j + 1]);
}

#define MSG_SIZE 32
#define SEED_SIZE 32
int main() {
  printf("Ahoy Mate\n");

  uint64_t start, end;
  uint8_t seed[SEED_SIZE];
  memset(seed, 0, SEED_SIZE);
  seed_prng(seed, SEED_SIZE);
  for (int i = 0; i < ITERATIONS; i++) {
    printf("Starting iteration %d\n", i);
    unsigned long long mlen = 32;
    unsigned long long output_sig = 0;
    uint8_t PK[CRYPTO_PUBLICKEYBYTES] = {0};
    uint8_t SK[CRYPTO_SECRETKEYBYTES] = {0};
    uint8_t sig[CRYPTO_BYTES + MSG_SIZE] = {0};

    uint8_t msg[MSG_SIZE] = {0};
    rng_bytes(msg, MSG_SIZE);

    start = cpucycles();
    crypto_sign_keypair(PK, SK);
    end = cpucycles();
    timekey_f[i] = end - start;

    start = cpucycles();
    crypto_sign(sig, &output_sig, msg, MSG_SIZE, SK);
    end = cpucycles();
    time_sign_f[i] = end - start;

    start = cpucycles();
    int res = crypto_sign_open(msg, &mlen, sig, output_sig, PK);
    end = cpucycles();
    time_verify_f[i] = end - start;
    if (res != 0) exit(res);
    printf("verification ok? %d\n", (res == 0));

    printf("Finishing iteration %d\n", i);
  }

  bubbleSort(timekey_f, ITERATIONS);
  bubbleSort(time_verify_f, ITERATIONS);
  bubbleSort(time_sign_f, ITERATIONS);

  uint64_t sign_avg = 0;
  uint64_t verify_avg = 0;
  uint64_t keygen_avg = 0;
  for (int i = 0; i < ITERATIONS; i++) {
    sign_avg += time_sign_f[i];
    verify_avg += time_verify_f[i];
    keygen_avg += timekey_f[i];
  }
  printf("keygen() MEDIAN  %12lu cycles \n", timekey_f[ITERATIONS / 2]);
  printf("keygen() AVERAGE %12lu cycles \n", (keygen_avg / ITERATIONS));
  printf("keygen() LOWEST  %12lu cycles \n", timekey_f[0]);
  printf("keygen() HIGHEST %12lu cycles \n", timekey_f[ITERATIONS - 1]);

  printf("sign()   MEDIAN  %12lu cycles\n", time_sign_f[ITERATIONS / 2]);
  printf("sign()   AVERAGE %12lu cycles \n", (sign_avg / ITERATIONS));
  printf("sign()   LOWEST  %12lu cycles \n", time_sign_f[0]);
  printf("sign()   HIGHEST %12lu cycles \n", time_sign_f[ITERATIONS - 1]);

  printf("verify() MEDIAN  %12lu cycles \n", time_verify_f[ITERATIONS / 2]);
  printf("verify() AVERAGE %12lu cycles \n", (verify_avg / ITERATIONS));
  printf("verify() LOWEST  %12lu cycles \n", time_verify_f[0]);
  printf("verify() HIGHEST %12lu cycles \n", time_verify_f[ITERATIONS - 1]);

  printf("Byte Mate\n");
  return 0;
}
