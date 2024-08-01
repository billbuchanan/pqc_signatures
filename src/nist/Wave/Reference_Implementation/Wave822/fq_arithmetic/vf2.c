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

#include "vf2.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

vf2_e *vf2_alloc(size_t length) {
  vf2_e *e = malloc(sizeof(vf2_e));
  e->size = length;
  e->alloc = 1 + ((length - 1) / WORD_LENGTH);

  e->x = (wave_word *)calloc(e->alloc, sizeof(wave_word));
  return e;
}

void vf2_free(vf2_e *v) {
  free(v->x);
  free(v);
}

void vf2_vector_split(vf2_e *a, vf2_e *b, vf2_e const *x) {
  // assume a->size + b->size = x->size
  memset(a->x, 0, sizeof(wave_word) * a->alloc);
  memset(b->x, 0, sizeof(wave_word) * b->alloc);

  memcpy(a->x, x->x, sizeof(wave_word) * a->alloc);
  wave_word shift = a->size % WORD_LENGTH;
  if (shift > 0) {
    a->x[a->alloc - 1] &= (1UL << shift) - 1;
  }

  if (shift == 0) {
    memcpy(b->x, x->x + a->alloc, sizeof(wave_word) * b->alloc);
  } else {
    int i, j;

    for (i = 0, j = a->alloc - 1; j + 1 < x->alloc; ++i, ++j) {
      b->x[i] ^= x->x[j] >> shift;
      b->x[i] ^= x->x[j + 1] << (WORD_LENGTH - shift);
    }
    // j = x->alloc - 1
    // i could be b->alloc or b->alloc - 1
    // if i = b->alloc - 1, we are not done yet
    if (i < b->alloc) {
      b->x[i] = x->x[j] >> shift;
    }
  }
}

// need a cct implementation hiding the value of t
void vf2_vector_1t0(vf2_e *a, int t) {
  int wl = WORD_LENGTH;  // macro might give a size_t which is unsigned
  for (int i = 0; i < a->alloc; ++i, t -= wl) {
    a->x[i] = (t >= wl) * 0xFFFFFFFFFFFFFFFF +
              ((t > 0) & (t < wl)) * (0xFFFFFFFFFFFFFFFF >> (wl - t));
  }
}

void vf2_vf3_support(vf2_e *a, vf3_e const *e) {
  // assume e->alloc = a->alloc
  memcpy(a->x, e->r0, sizeof(wave_word) * a->alloc);
}

uint8_t vf2_get_element(size_t i, vf2_e const *a) {
  uint32_t pos, shift;
  pos = i / WORD_LENGTH;
  shift = i % WORD_LENGTH;
  return ((a->x[pos] >> shift) & 1);
}

void vf2_set_coeff(size_t j, vf2_e *x, uint8_t const a) {
  // asume a in {0, 1}
  size_t i;
  i = j / WORD_LENGTH;
  j %= WORD_LENGTH;
  wave_word z = ((wave_word)1) << j;
  wave_word a0 = ((wave_word)(a > 0)) << j;
  x->x[i] &= ~z;
  x->x[i] ^= a0;
}
