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
#include <sys/resource.h>

#include "api.h"
#include "prng/prng.h"

#define MSG_SIZE 32
#define SEED_SIZE 32
int main() {
  printf("Ahoy Mate\n");
  unsigned long long mlen = 32;
  unsigned long long output_sig = 0;
  uint8_t PK[CRYPTO_PUBLICKEYBYTES] = {0};
  uint8_t SK[CRYPTO_SECRETKEYBYTES] = {0};
  uint8_t sig[CRYPTO_BYTES + MSG_SIZE] = {0};

  uint8_t msg[MSG_SIZE] = {0};

  uint8_t seed[SEED_SIZE] = {0};
  seed_prng(seed, SEED_SIZE);

  crypto_sign_keypair(PK, SK);
  crypto_sign(sig, &output_sig, msg, MSG_SIZE, SK);
  int res = crypto_sign_open(msg, &mlen, sig, output_sig, PK);
  printf("verification ok? %s\n", (res == 0) ? "yes" : "no");

  printf("Byte Mate\n");

  close_prng();

  return 0;
}
