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

#include "fq_arithmetic/vf2.h"
#include "fq_arithmetic/vf3.h"
#include "prng/prng.h"
#include "util/djbsort.h"

// Algorithm 14, page 15 of specs
/*
        randperm() draws a permutation by sorting:
        1 - draw random data r_i, 0 <= i < n
        2 - sort (r_i,i) according to r_i -> (x_i,pi(i)) with x_i ordered
        3 - the mapping (i -> pi(i)) is a permutation

        This permutation is uniformly random if all the r_i drawn at step 1
        are distinct. If not, to enforce uniformity, reject and start again.
*/
void randperm(uint16_t *sigma, int n, vf3_e *y, vf3_e *z) {
  int i, b = 0;
  int64_t *x = calloc(n, sizeof(int64_t));
  uint8_t *tr = calloc(n, sizeof(uint8_t));
  uint32_t *r = calloc(n, sizeof(uint32_t));

  for (i = 0; i < n; ++i) {
    tr[i] = vf3_get_element(i, y);
    tr[i] ^= vf3_get_element(i, z) << 2;
  }

  do {
    rng_bytes((uint8_t *)r, n * sizeof(uint32_t));
    for (i = 0; i < n; ++i) {
      // we assume the index i < n fits in 16 bits
      x[i] = (((int64_t)r[i]) << 24) | (i << 8) | tr[i];
    }
    int64_sort_4(x, n);
    for (i = 1, b = 0; i < n; ++i) {
      if ((x[i - 1] >> 24) == (x[i] >> 24)) {
        b = 1;
        break;
      }
    }
  } while (b);

  for (i = 0; i < n; ++i) {
    vf3_set_coeff(i, y, x[i] & 3);
    vf3_set_coeff(i, z, (x[i] >> 2) & 3);
    sigma[i] = (x[i] >> 8) & 0xFFFF;
  }

  free(x);
  free(tr);
  free(r);
}

// Algorithm 15, page 15 of specs
/*
        As randperm, but here we don't need a uniformly random permutation,
        we only need that the following partition of {0,...,n-1}:
          ({pi(0),...pi(k-1)}, {pi(k),...pi(k+t-1)}, {pi(k+t),...,pi(n-1)})
        is uniformly random among the partitions in three parts of size k,
        t, and n-k.  To ensure that, it is enough that x_{k-1}!=x_k and
        x_{k+t-1}!=x_{k+t}.
*/
void randpermV(uint16_t *sigma, int n, int k, int t, vf3_e *y) {
  int i, b;
  uint8_t *tr = calloc(n, sizeof(uint8_t));
  int64_t *x = calloc(n, sizeof(int64_t));
  uint32_t *r = calloc(n, sizeof(uint32_t));

  for (i = 0; i < n; ++i) {
    tr[i] = vf3_get_element(i, y);
  }

  do {
    rng_bytes((uint8_t *)r, n * sizeof(uint32_t));
    for (i = 0; i < n; ++i) {
      // we assume the index i < n fits in 16 bits
      x[i] = (((int64_t)r[i]) << 24) | (i << 8) | tr[i];
    }
    int64_sort_4(x, n);
    if ((x[k - 1] >> 24) == (x[k] >> 24))  // k is public
      continue;
    for (i = 0, b = 0; i < n - k; ++i) {  // t is secret
      b |= ((x[k + i - 1] >> 24) == (x[k + i] >> 24)) & (i == t);
    }
  } while (b);  // b = (x[k+t-1] == x[k+t])

  for (i = 0; i < n; ++i) {
    vf3_set_coeff(i, y, x[i] & 0x3);
    sigma[i] = (x[i] >> 8) & 0xFFFF;
  }

  free(x);
  free(tr);
  free(r);
}

// Algorithm 16, page 16 of specs
/*
        We want a random permutation pi such that exactly l positions of
        J={i,0<=i<n,s_i=1} (the support of s) lie in {pi(0),...,pi(k-1)}.

        The permutation is drawn by sorting, and we add a large constant
        before sorting to the elements such that s_i=1. After sorting in
        increasing order, the elements of the support of s come last. We
        swap the last l elements with elements in the first k positions.

        Here we don't need a uniformly random permutation, we only need that
        x_{k-l-1}!=x_{k-l} and x_{n-l-1}!=x_{n-l}.

*/
void randpermU(uint16_t *sigma, int n, int k, int l, vf3_e *y, vf3_e *v,
               vf2_e *s) {
  int i, b;
  uint64_t *tr = calloc(n, sizeof(uint64_t));
  int64_t *x = calloc(n, sizeof(int64_t));
  uint32_t *r = calloc(n, sizeof(uint32_t));

  for (i = 0; i < n; ++i) {
    tr[i] = vf3_get_element(i, y);
    tr[i] ^= vf3_get_element(i, v) << 2;
    tr[i] ^= ((int64_t)vf2_get_element(i, s)) << 56;
  }

  do {
    rng_bytes((uint8_t *)r, n * sizeof(uint32_t));
    for (i = 0; i < n; ++i) {
      // we assume the index i < n fits in 16 bits
      x[i] = (((int64_t)r[i]) << 24) | (i << 8) | tr[i];
    }
    int64_sort_4(x, n);
    for (i = 1, b = 0; i < k; ++i) {  // l is secret
      b |= (((x[k - i - 1] >> 24) == (x[k - i] >> 24)) |
            ((x[n - i - 1] >> 24) == (x[n - i] >> 24))) &
           (i == l);
    }
  } while (b);  // b = (x[k-l-1] == x[k-l]) | (x[n-l-1] == x[n-l])

  // swap x[k-i-1] and x[n-i-1] if (i < l) while keeping l secret
  for (i = 0; i < k; ++i) {
    b = (i < l);
    int64_t tmp = b * x[k - i - 1] + (1 - b) * x[n - i - 1];
    x[k - i - 1] = (1 - b) * x[k - i - 1] + b * x[n - i - 1];
    x[n - i - 1] = tmp;
  }

  for (i = 0; i < n; ++i) {
    vf3_set_coeff(i, y, x[i] & 3);
    vf3_set_coeff(i, v, (x[i] >> 2) & 3);
    vf2_set_coeff(i, s, (x[i] >> 56) & 1);
    sigma[i] = (x[i] >> 8) & 0xFFFF;
  }

  free(x);
  free(tr);
  free(r);
}

/*
        Permute a vector according to the inverse of pi
        compute the inverse permutation of pi, inplace
 */
void vectperminv(vf3_e *res, vf3_e *y, uint16_t *pi) {
  // assume size(pi) >= y->size and res->size >= y->size
  int i;
  int64_t *x = calloc(y->size, sizeof(int64_t));

  for (i = 0; i < y->size; ++i) {
    // assumes that indices fits in 16 bits (y->size <= 2^16)
    x[i] = (((int64_t)pi[i]) << 18) | (i << 2) | vf3_get_element(i, y);
  }
  int64_sort_4(x, y->size);
  for (i = 0; i < y->size; ++i) {
    vf3_set_coeff(i, res, x[i] & 3);
    pi[i] = (x[i] >> 2) & 0xFFFF;
  }

  free(x);
}
