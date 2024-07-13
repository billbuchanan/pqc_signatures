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

#include "fq_arithmetic/mf3.h"
#include "fq_arithmetic/types_f3.h"
#include "fq_arithmetic/vf3.h"

void normalize(vf3_e *e, int pos_l) {
  uint8_t tmp = vf3_get_element(pos_l, e);
  tmp = (4 + tmp - tmp * tmp) % 3;       // (1 + tmp - tmp^2) mod 3
  vf3_vector_scalarmul_inplace(e, tmp);  // e <- tmp * e
}

void reduce(vf3_e *x, vf3_e *y, int pos_l) {
  uint8_t tx = vf3_get_element(pos_l, x);
  uint8_t ty = vf3_get_element(pos_l, y);
  uint8_t a1 = (tx * tx) % 3;  // a1 = 0 or 1
  uint8_t a2 = 1 - a1;         // a2 = 0 or 1
  uint8_t a3 =
      (a2 - tx * ty + 6) %
      3;  // + 6, smallest multiple of 3 such that always a2 - tx * ty + 6 >= 0

  vf3_e *vec1 = vf3_alloc(x->size);
  vf3_e *vec2 = vf3_alloc(x->size);
  vf3_vector_scalarmul(vec1, a1, x);  // vec1 <- a1 * x
  vf3_vector_add_multiple_inplace(
      vec1, a2, y);                   // vec1 <- vec1 + a2 * y = a1 * x + a2 * y
  vf3_vector_scalarmul(vec2, a3, x);  // vec2 <- a3 * x
  vf3_vector_add_multiple_inplace(
      vec2, a1, y);  // vec2 <- vec2 + a1 * y = a3 * x + a1 * y

  vf3_copy(x, vec1);
  vf3_copy(y, vec2);

  vf3_free(vec1);
  vf3_free(vec2);
}

// x <- b * x + (1 - b) * y
// y <- a * x + b * y
// a ternary
// b binary
void adhoc_reduce(vf3_e *x, vf3_e *y, int pos_l) {
  uint8_t tx = vf3_get_element(pos_l, x);
  uint8_t ty = vf3_get_element(pos_l, y);
  uint8_t b = (tx * tx) % 3;  // b = 0 or 1
  uint8_t a =
      (1 - b - tx * ty + 6) % 3;  // + 6, smallest multiple of 3 such that
                                  // always 1 - b - tx * ty + 6 >= 0
  wave_word maskb = -(b > 0);
  wave_word maskcb = -(b == 0);
  wave_word maska0 = -(a > 0);
  wave_word maska1 = -(a > 1);
  wave_word x0, x1, y0, y1, z0, z1, tmp;

  for (int i = pos_l / WORD_LENGTH; i < x->alloc; i++) {
    // (x0, x1) <- i-th word of b * x
    x0 = x->r0[i] & maskb;
    x1 = x->r1[i] & x0;
    // (y0, y1) <- i-th word of (1 - b) * y
    //        y0 = y->r0[i] & (maskb ^ 0xFFFFFFFFFFFFFFFFUL);
    y0 = y->r0[i] & maskcb;
    y1 = y->r1[i] & y0;
    // (z0, z1) <- i-th word of b * x + (1 - b) * y
    // (z0, z1) <- (x0, x1) + (y_0, y1)
    tmp = x1 ^ y0;
    z0 = (x0 ^ y0) | (tmp ^ y1);
    z1 = (x0 ^ y1) & tmp;

    // (x0, x1) <- i-th word of a * x
    x0 = x->r0[i] & maska0;
    x1 = (x->r1[i] ^ maska1) & x0;
    // (y0, y1) <- i-th word of b * y
    y0 = y->r0[i] & maskb;
    y1 = y->r1[i] & y0;
    // i-th word of y <- i-th word of a * x + b * y
    // i-th word of y <- (x0, x1) + (y0, y1)
    tmp = x1 ^ y0;
    y->r0[i] = (x0 ^ y0) | (tmp ^ y1);
    y->r1[i] = (x0 ^ y1) & tmp;
    // i-th word of x <- i-th word of b * x + (1 - b) * y
    x->r0[i] = z0;
    x->r1[i] = z1;
  }
}

#define WORD_MASK(word_array, word_index, bit_index) \
  -((word_array[word_index] >> bit_index) & 1)
#define WORD_NEG_MASK(word_array, word_index, bit_index) \
  -(~(word_array[word_index] >> bit_index) & 1)

// extended Gaussian elimination, Algorithm 21, page 19
int extended_gauss_elimination(mf3_e *H, int gap) {
  int pivot_count = 0;
  mf3_stack_zeros(H, gap);

  for (int l = 0; l < H->n_row; ++l) {
    vf3_e *x = H->rows + l;
    uint32_t l_word_index = l / WORD_LENGTH;
    uint32_t l_bit_index = l % WORD_LENGTH;
    // Reduce from row l down:
    for (int i = l + 1; i < H->n_row; ++i) {
      vf3_e *y = H->rows + i;
      wave_word x_is_pivot = WORD_MASK(x->r0, l_word_index, l_bit_index);
      wave_word subtract =
          x_is_pivot & WORD_MASK(y->r0, l_word_index, l_bit_index);
      wave_word xnormalize = WORD_MASK(x->r1, l_word_index, l_bit_index);
      wave_word ynormalize = WORD_MASK(y->r1, l_word_index, l_bit_index);
      for (int j = l_word_index; j < x->alloc; j++) {
        // 0. Load and normalize x
        wave_word x_0 = x->r0[j];
        wave_word x_1 = x->r1[j] ^ (xnormalize & x_0);
        // 1. Load and normalize y
        wave_word y_0 = y->r0[j];
        wave_word y_1 = y->r1[j] ^ (ynormalize & y_0);
        // 2. Mask x for inclusion (or not) in subtraction
        wave_word mx_0 = subtract & x_0;
        wave_word mx_1 = subtract & x_1;
        // 3. Subtract (masked) x from y in-place
        wave_word tmp = y_0 ^ mx_0;
        y_0 = tmp | (y_1 ^ mx_1);
        tmp ^= mx_1;
        y_1 = tmp & (y_1 ^ mx_0);
        // 4. Store with conditional swap (x,y) :-> (y,x) if not pivot
        tmp = x_is_pivot & (x_0 ^ y_0);
        x->r0[j] = tmp ^ y_0;
        y->r0[j] = tmp ^ x_0;
        tmp = x_is_pivot & (x_1 ^ y_1);
        x->r1[j] = tmp ^ y_1;
        y->r1[j] = tmp ^ x_1;
      }
    }
    // patch for last diagonal element NS May 20 2023
    {
      wave_word xnormalize = WORD_MASK(x->r1, l_word_index, l_bit_index);
      for (int j = l_word_index; j < x->alloc; j++) {
        // Just normalize x
        // x->r0[j] = x->r0[j];
        x->r1[j] = x->r1[j] ^ (xnormalize & x->r0[j]);
      }
    }
    // Note that the l-th row (x) is now normalized
    // Count pivots:
    pivot_count += (x->r0[l_word_index] >> l_bit_index) & 1;
    // Clear above the l-th row:
    for (int i = 0; i < l; ++i) {
      vf3_e *y = H->rows + i;
      wave_word mask0 = WORD_MASK(y->r0, l_word_index, l_bit_index);
      wave_word mask1 = WORD_NEG_MASK(y->r1, l_word_index, l_bit_index);

      for (int j = l_word_index; j < x->alloc; j++) {
        wave_word x_0 = x->r0[j] & mask0;
        wave_word x_1 = (x->r1[j] ^ mask1) & x_0;
        wave_word tmp = y->r1[j] ^ x_0;
        y->r1[j] = (y->r0[j] ^ x_1) & tmp;
        y->r0[j] = (y->r0[j] ^ x_0) | (tmp ^ x_1);
      }
    }
  }

  return pivot_count;
}

// partial Gaussian elimination, Algorithm 20, page 19
int partial_gauss_elimination(mf3_e *G, int gap) {
  int l, i, w = 0;

  for (l = 0; l < G->n_row - gap; ++l) {
    for (i = l + 1; i < G->n_row; ++i) {
      adhoc_reduce(G->rows + l, G->rows + i, l);
    }
    normalize(G->rows + l, l);
    w += mf3_coeff(G, l, l);
    for (i = 0; i < l; ++i) {
      uint8_t a = mf3_coeff(G, i, l);
      a = (3 - a) % 3;  // a <- -a
      // vf3_vector_add_multiple_inplace(G->rows + i, a, G->rows + l);
      vf3_vector_slice_add_multiple_inplace(G->rows + i, a, G->rows + l, l);
    }
  }

  return w;
}

// NS 27/04/23 Gaussian elimination for KeyGen, Algorithm 18, page 18
// The pivot positions are not secret and can be revealed
int gauss_elimination(mf3_e *H, int *pivot) {
  int i, j, l;

  for (l = 0, j = 0; (l < H->n_row) && (j < H->n_col); ++j) {
    for (i = l + 1; i < H->n_row; ++i) {
      adhoc_reduce(H->rows + l, H->rows + i, j);
    }
    if (mf3_coeff(H, l, j) == 0) {
      continue;
    }
    normalize(H->rows + l, j);
    for (i = 0; i < l; ++i) {
      uint8_t a = mf3_coeff(H, i, j);
      a = (3 - a) % 3;  // a <- -a
      // vf3_vector_add_multiple_inplace(H->rows + i, a, H->rows + l);
      vf3_vector_slice_add_multiple_inplace(H->rows + i, a, H->rows + l, l);
    }
    pivot[l] = j;
    ++l;
  }

  return l;
}

// NS 27/04/23 Gaussian elimination with abort for KeyGen, Algorithm 19, page 18
int gauss_elimination_with_abort(mf3_e *H) {
  int i, j;

  for (j = 0; j < H->n_row; ++j) {
    for (i = j + 1; i < H->n_row; ++i) {
      adhoc_reduce(H->rows + j, H->rows + i, j);
    }
    if (mf3_coeff(H, j, j) == 0) {
      break;
    }
    normalize(H->rows + j, j);
    for (i = 0; i < j; ++i) {
      uint8_t a = mf3_coeff(H, i, j);
      a = (3 - a) % 3;  // a <- -a
      vf3_vector_add_multiple_inplace(H->rows + i, a, H->rows + j);
    }
  }

  return j;
}
