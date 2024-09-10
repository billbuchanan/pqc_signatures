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

#include "hash.h"

/**
 * Function:  expand
 * --------------------
 * expand the input to the correct size in trits.
 * It is algorithm 3 from https://eprint.iacr.org/2021/1432.pdf
 *
 *  input: binary input
 *  len_input: size of input
 *
 * 	h3: structure that will receive the trits
 *
 */

void expand(unsigned char *input, size_t len_input, size_t len_out, vf3_e *p) {
  uint8_t buf[168] = {0};
  shake256incctx sha3;
  shake256_inc_init(&sha3);

  shake256_inc_absorb(&sha3, input, len_input);
  shake256_inc_squeeze(buf, 168, &sha3);
  size_t len_bytes = 167;
  size_t len = 0;
  while (len < len_out) {
    if (len_bytes == 0) {
      shake256_inc_squeeze(buf, 168, &sha3);
      len_bytes = 167;
    }
    if (buf[len_bytes] < 243) {
      uint8_t to_convert = buf[len_bytes];
      for (int i = 0; i < 8; i = i + 2) {
        uint8_t a = to_convert & 1;
        a = (a | (to_convert & 2)) % 3;
        to_convert /= 3;
        if (len == len_out) break;
        vf3_set_coeff(len, p, ((a - 1) + 3) % 3);
        // p[len] = ((a - 1) + 3) % 3;
        len++;
      }
    }
    len_bytes--;
  }
  shake256_inc_ctx_release(&sha3);
}




/**
 * Function:  convert_to_trits
 * --------------------
 * convertion from bits to trits
 * It is algorithm 2 from https://eprint.iacr.org/2021/1432.pdf
 *
 *  input: binary input
 *
 * 	h3: structure that will receive the trits
 *
 */

 #define CHUNKS (HASH_SIZE/8) / 4
 // i.e. CHUNKS = number of uint32_ts in dividend

uint8_t divmod3(uint32_t * dividend){
   uint64_t trit = 0;
   for(int i = CHUNKS - 1; i >= 0; i--){
      uint32_t u = dividend[i];   // <- holds a normal chunk
      uint64_t x = trit << 32;   // <- big enough to hold inevitable overflow
      x += u;
      dividend[i] = x / 3;  // x / 3 will be 32 bits again
      trit = x % 3;
    }
    return trit;
 }

void convert_to_trits(unsigned char *input, vf3_e *h3) {
    for (int i = 0; i < h3->size; i++) {
        uint8_t trit = divmod3((uint32_t *)input);
        vf3_set_coeff(i, h3, trit);
    }
}

void hash_message(vf3_e *m_hash, const uint8_t *message, const size_t mlen,
                  const uint8_t *salt, const size_t msalt) {
  uint8_t input[mlen + msalt];
  memcpy(input, salt, msalt * sizeof(uint8_t));
  memcpy(input + msalt, message, mlen * sizeof(uint8_t));

  uint8_t h2[HASH_SIZE] = {0};
  sha3_512(h2, input, mlen + msalt);
  vf3_e *h3 = vf3_alloc(Mu);
  convert_to_trits(h2, h3);
  vf3_e *p = vf3_alloc(N - K - Mu);
  expand(h2, HASH_SIZE, N - K - Mu, p);

  vf3_vector_cat(m_hash, h3, p);

  vf3_free(h3);
  vf3_free(p);
}
