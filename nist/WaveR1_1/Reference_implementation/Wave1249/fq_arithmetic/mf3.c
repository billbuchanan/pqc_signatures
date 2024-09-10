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

#include "mf3.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vf2.h"
#include "vf3.h"

mf3_e *mf3_alloc(size_t nr_rows, size_t nr_cols) {
  mf3_e *m = (mf3_e *)malloc(sizeof(mf3_e));
  m->n_row = nr_rows;
  m->n_col = nr_cols;
  m->rows = vf3_array_alloc(nr_cols, nr_rows);
  return m;
}

void mf3_free(mf3_e *m) {
  vf3_array_free(m->rows, m->n_row);
  free(m);
}

void mf3_random(mf3_e *e) {
  for (int i = 0; i < e->n_row; i++) {
    vf3_random(&e->rows[i]);
  }
}

void mf3_set(mf3_e *dest, mf3_e const *src) {
  for (size_t i = 0; i < src->n_row; i++) {
    vf3_copy(dest->rows + i, src->rows + i);
  }
}

mf3_e *mf3_copy(mf3_e const *src) {
  mf3_e *dest = mf3_alloc(src->n_row, src->n_col);
  mf3_set(dest, src);
  return dest;
}

void mf3_settozero(mf3_e *M) {
  for (int i = 0; i < M->n_row; i++) {
    vf3_set_to_zero(M->rows + i);
  }
}

void mf3_clean(mf3_e *M) {
  for (size_t i = 0; i < M->n_row; i++) vf3_trim(M->rows + i);
}

void mf3_stack_zeros(mf3_e *H, size_t nr_stack) {
  H->rows = realloc(H->rows, (H->n_row + nr_stack) * sizeof(vf3_e));
  for (int i = H->n_row; i < H->n_row + nr_stack; i++) {
    vf3_init(H->rows + i, H->n_col);
  }
  H->n_row = H->n_row + nr_stack;
}

void mf3_stack(mf3_e *H, mf3_e const *M) {
  // assume H->n_col = M->n_col
  H->rows = realloc(H->rows, (H->n_row + M->n_row) * sizeof(vf3_e));
  for (int i = H->n_row; i < H->n_row + M->n_row; i++) {
    vf3_init(H->rows + i, H->n_col);
    vf3_copy(H->rows + i, M->rows + (i - H->n_row));
  }
  H->n_row = H->n_row + M->n_row;
}

int mf3_equal(mf3_e const *H, mf3_e const *M) {
  // assume M->n_col = H->n_col and M->n_row >= H->n_row
  for (int i = 0; i < H->n_row; i++)
    if (!vf3_equal(H->rows + i, M->rows + i)) {
      return 0;
    }
  return 1;
}

void mf3_transpose(mf3_e *m) {
  mf3_e *tmp = mf3_alloc(m->n_col, m->n_row);
  for (int i = 0; i < m->n_row; ++i)
    for (int j = 0; j < m->n_col; ++j) {
      mf3_setcoeff(tmp, j, i, mf3_coeff(m, i, j));
    }

  mf3_free(m);
  *m = *tmp;
}

// transpose and copy

void mf3_transpose_and_copy(mf3_e *tr, mf3_e const *m) {
  for (int i = 0; i < m->n_row; ++i)
    for (int j = 0; j < m->n_col; ++j)
      mf3_setcoeff(tr, j, i, mf3_coeff(m, i, j));
}

// s = H * e^T
void mf3_product_matrix_vector(vf3_e *s, mf3_e const *H, vf3_e const *e) {
  vf3_e *tmp = vf3_alloc(H->n_col);
  for (int i = 0; i < H->n_row; i++) {
    vf3_vector_mul(tmp, &H->rows[i], e);
    int w = vf3_hamming_weight(tmp);
    w += vf3_number_of_coordinates_equal_to_two(tmp);
    vf3_set_coeff(i, s, w % 3);
  }
  vf3_free(tmp);
}

// y = x * g
void mf3_product_vector_matrix(vf3_e *y, vf3_e const *x, mf3_e const *g) {
  // assume x->size = g->n_row
  memset(y->r0, 0, y->alloc * sizeof(wave_word));
  memset(y->r1, 0, y->alloc * sizeof(wave_word));

  for (int i = 0; i < g->n_row; i++) {
    uint8_t a = vf3_get_element(i, x);
    vf3_vector_add_multiple_inplace(y, a, g->rows + i);
  }
}

void vf2_mf3_diag_support(vf2_e *d, mf3_e const *m) {
  // assume a->size >= m->n_row
  memset(d->x, 0, sizeof(wave_word) * d->alloc);
  for (int l = 0; l < m->n_row; ++l) {
    d->x[l / WORD_LENGTH] ^=
        m->rows[l].r0[l / WORD_LENGTH] & (((wave_word)1) << (l % WORD_LENGTH));
  }
}

void mf3_print(mf3_e *m) {
  for (int i = 0; i < m->n_row; i++) {
    vf3_print(&m->rows[i]);
    printf("\n");
  }
}
