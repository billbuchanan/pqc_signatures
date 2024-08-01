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

#include "fips202.h"
#include "fq_arithmetic/vf3.h"

shake256incctx shakectx;
void seed_prng(uint8_t *seed, int seed_len) {
  shake256_inc_init(&shakectx);
  shake256_inc_absorb(&shakectx, seed, seed_len);
  shake256_inc_finalize(&shakectx);
}

void prng_bytes(uint8_t *output, int outlen) {
  shake256_inc_squeeze(output, outlen, &shakectx);
}

void close_prng() {}

#ifdef NIST_KAT
#include "NIST-kat/rng.h"
void rng_bytes(uint8_t *output, int outlen) { randombytes(output, outlen); }
#else
void rng_bytes(uint8_t *output, int outlen) { prng_bytes(output, outlen); }
#endif

void vf3_random_from_seed(vf3_e *y, uint8_t const *seed, int seed_len,
                          uint32_t custom) {
  size_t input_len, output_len;
  uint8_t *shake_input;
  uint8_t *shake_output;

  input_len = seed_len + sizeof(custom);
  shake_input = calloc(input_len, sizeof(uint8_t));
  memcpy(shake_input, seed, seed_len);
  memcpy(shake_input + seed_len, &custom, sizeof(custom));

  /* vf3_trits_from_bits() requires a (uint64_t *) type as second
   * argument. In case sizeof(wave_word) < sizeof(uint64_t), we
   * round up the amount of randomness to the next multiple of
   * sizeof(uint64_t) */
  /* We assume that vf3_trits_from_bits() converts bits to trits
   * with a ratio of 2 bits for one trit, hence the factor 2 */
  output_len = 2 * (1 + (y->alloc * sizeof(wave_word) - 1) / sizeof(uint64_t)) *
               (sizeof(uint64_t));
  shake_output = calloc(output_len, sizeof(uint8_t));
  shake256(shake_output, output_len, shake_input, input_len);
  vf3_trits_from_bits(y, (uint64_t *)shake_output);

  free(shake_input);
  free(shake_output);
}
