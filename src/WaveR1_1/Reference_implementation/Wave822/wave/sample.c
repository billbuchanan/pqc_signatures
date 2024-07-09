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
#include <stdlib.h>

#include "params.h"
#include "prng/prng.h"

typedef struct {
  int prec, size, offset;
  uint64_t *proba;
} distrib;

typedef struct {
  int size, offset;
  distrib *dist;
} fdistrib;

#include "distrib.h"

int sampleV() {
  int i, res;
  uint64_t p, mask;

  mask = (1UL << DV.prec) - 1;  // assumes DV.prec < 64
  rng_bytes((uint8_t *)&p, sizeof(p));
  p &= mask;

  for (i = DV.size - 2, res = 0; i >= 0; --i) {
    res += ((p >= DV.proba[i]) & (res == 0)) * (i + 1 + DV.offset);
  }
  res += (res == 0) * DV.offset;
  return res;
}

int sampleU(int t) {
  int i, j, res;
  uint64_t p, mask;

  rng_bytes((uint8_t *)&p, sizeof(p));

  t -= DU.offset;
  for (j = 0, res = 0; j < DU.size; ++j) {
    mask = (1UL << DU.dist[j].prec) - 1;  // assumes DU.dist[j].prec < 64
    for (i = DU.dist[j].size - 2; i >= 0; --i) {
      res += (((p & mask) >= DU.dist[j].proba[i]) & (res == 0) & (j == t)) *
             (i + 1 + DU.dist[j].offset);
    }
    res += ((res == 0) & (j == t)) * DU.dist[j].offset;
  }
  return res;
}
