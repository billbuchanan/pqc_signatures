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

#include "fq_arithmetic/vf3.h"
#include "params.h"
#include "prng/prng.h"
#include "types.h"
#include "util/hash.h"
#include "wave/decode.h"

int encode_signature(uint8_t *output, vf3_e *e) {
  int i, j, l;
  uint8_t x;

  for (i = N - K, l = 0; i < N; ++l) {
    for (j = 0, x = 0; (j < 5) && (i < N); ++j, ++i) {
      x *= 3;
      x += vf3_get_element(i, e);
    }
    output[l] = x;
  }

  return l;
}

int sign(vf3_e *sig, uint8_t *salt, uint8_t const *message, size_t mlen,
         wave_sk_t *sk) {
  vf3_e *syndrome, *e, *y;
  syndrome = vf3_alloc(N - K);
  e = vf3_alloc(N);
  y = vf3_alloc(N);

  rng_bytes(salt, SALT_SIZE * sizeof(uint8_t));
  hash_message(syndrome, message, mlen, salt, SALT_SIZE);

  vf3_vector_cat_zero(y, syndrome);  // y <- (syndrome | 0)
  decode(e, y, sk);

  int idx = 0;
  for (int i = N - K; i < N; i++) {
    vf3_set_coeff(idx, sig, vf3_get_element(i, e));
    idx++;
  }

#ifdef BATCH_SIGN
  extern FILE *sign_file_out;
  vf3_write(e, sign_file_out);
#endif

  vf3_free(syndrome);
  vf3_free(e);
  vf3_free(y);

  return 1;
}
