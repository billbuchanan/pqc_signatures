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

#include "mf3permut.h"

#include "djbsort.h"

typedef struct {
  uint16_t min, max;
} swap_t;

typedef struct {
  int len;
  swap_t *swap;
} perm_network_t;

/* from djbsort-20190516/h-internal/int32_minmax.c */
#define int32_MINMAX(a, b) \
  do {                     \
    int32_t ab = b ^ a;    \
    int32_t c = b - a;     \
    c ^= ab & (c ^ b);     \
    c >>= 31;              \
    c &= ab;               \
    a ^= c;                \
    b ^= c;                \
  } while (0)

/* from djbsort-20190516/int32/portable3/sort.c */
perm_network_t *djbsort_network(int n) {
  int top, p, q, r, i, l = 0;
  perm_network_t *sigma = (perm_network_t *)malloc(sizeof(perm_network_t));
  sigma->swap = (swap_t *)malloc(n * n * sizeof(swap_t));

  if (n < 2) return NULL;
  top = 1;
  while (top < n - top) top += top;

  for (p = top; p > 0; p >>= 1) {
    for (i = 0; i < n - p; ++i)
      if (!(i & p)) {
        sigma->swap[l].min = i;
        sigma->swap[l].max = i + p;
        l++;
      }
    i = 0;
    for (q = top; q > p; q >>= 1) {
      for (; i < n - q; ++i) {
        if (!(i & p)) {
          for (r = q; r > p; r >>= 1) {
            sigma->swap[l].min = i + p;
            sigma->swap[l].max = i + r;
            l++;
          }
        }
      }
    }
  }
  sigma->len = l;
  sigma->swap = (swap_t *)realloc(sigma->swap, l * sizeof(swap_t));

  return sigma;
}

void djbsort_network_free(perm_network_t *sigma) {
  free(sigma->swap);
  free(sigma);
}

void vf3_conditional_swap(vf3_e *x, vf3_e *y, int b) {
  wave_word z0, z1, mask = -b;

  for (int i = 0; i < x->alloc; i++) {
    z0 = (x->r0[i] ^ y->r0[i]) & mask;
    z1 = (x->r1[i] ^ y->r1[i]) & mask;
    y->r0[i] ^= z0;
    y->r1[i] ^= z1;
    x->r0[i] ^= z0;
    x->r1[i] ^= z1;
  }
}

void mf3_row_permute_old(mf3_e *M, uint16_t *pi) {  // not constant time
  // assumes pi contains a permutation of size M->n_row
  // compute the permutation network of size M->n_row
  perm_network_t *sigma = djbsort_network(M->n_row);
  uint16_t x[M->n_row];

  for (int i = 0; i < M->n_row; ++i) {
    x[pi[i]] = i;
  }
  for (int l = 0; l < sigma->len; ++l) {
    int i = sigma->swap[l].min;
    int j = sigma->swap[l].max;
    vf3_conditional_swap(M->rows + i, M->rows + j, (x[i] > x[j]));
    if (x[i] > x[j]) {
      uint16_t tmp = x[i];
      x[i] = x[j];
      x[j] = tmp;
    }
  }
  djbsort_network_free(sigma);
}

/*
        Oblivious computation of the inverse permutation of pi
 */
void perminv(uint16_t *pi_inv, uint16_t *pi, int n) {
  int i;
  int64_t *x = calloc(n, sizeof(int64_t));

  for (i = 0; i < n; ++i) {
    // assumes that indices fits in 16 bits (y->size <= 2^16)
    x[i] = (((int64_t)pi[i]) << 16) | i;
  }
  int64_sort_4(x, n);
  for (i = 0; i < n; ++i) {
    pi_inv[i] = x[i] & 0xFFFF;
  }

  free(x);
}

void mf3_row_permute(mf3_e *M, uint16_t *pi) {
  // assumes pi contains a permutation of size M->n_row
  // compute the permutation network of size M->n_row
  perm_network_t *sigma = djbsort_network(M->n_row);
  uint16_t *x = calloc(M->n_row, sizeof(uint16_t));

  perminv(x, pi, M->n_row);
  for (int l = 0; l < sigma->len; ++l) {
    int i = sigma->swap[l].min;
    int j = sigma->swap[l].max;
    vf3_conditional_swap(M->rows + i, M->rows + j, (x[i] > x[j]));
    int32_MINMAX(x[i], x[j]);
  }
  djbsort_network_free(sigma);
  free(x);
}
