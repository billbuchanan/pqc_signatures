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


#ifndef WAVE2_MF3_H
#define WAVE2_MF3_H

#include <stdint.h>

#include "types_f3.h"
#include "vf3.h"

/**
 * Allocate a new ternary matrix.  All entries will be set to 0.
 * @param nr_rows number of rows of the matrix
 * @param nr_cols number of columns of the matrix
 * @return a pointer to the new matrix
 */
mf3_e *mf3_alloc(size_t nr_rows, size_t nr_cols);

/**
 * Deallocate a ternary matrix.
 * @param m a valid pointer to a matrix (which will be freed)
 */
void mf3_free(mf3_e *m);

/**
 * Copy the entries from one matrix into another.
 * @param dest a valid pointer to the destination matrix (which will be
 * modified)
 * @param src a valid pointer to the source matrix
 */
void mf3_set(mf3_e *dest, mf3_e const *src);

/**
 * Allocate and fill a new copy of a matrix.
 * @param M a valid pointer to the matrix to be copied
 * @return a pointer to the new matrix (with the same dimensions and entries as
 * M)
 */
mf3_e *mf3_copy(mf3_e const *M);

/**
 * Set any extra entries (beyond the official size) of a matrix to 0.
 * @param M a valid pointer to a matrix
 */
void mf3_clean(mf3_e *M);

/**
 * Fill a matrix with random trits.
 * @param e a valid pointer to a matrix (must be already allocated)
 */
void mf3_random(mf3_e *e);

/**
 * Fill a matrix with random nonzero trits.
 * @param e a valid pointer to a matrix (must be already allocated)
 */
void mf3_random_non_zero(mf3_e *e);

/**
 * Replace a matrix with its transpose.
 * @param m a valid pointer to a matrix (which will be freed and replaced with
 * the transpose)
 */
void mf3_transpose(mf3_e *m);

/**
 * Fill a matrix with the entries of the transpose of another matrix.
 * Assumes tr->n_row = m->n_col and tr->n_col = m->n_row
 * @param tr a valid pointer to a matrix that can hold the transpose
 * @param m  a valid pointer to the matrix to be transposed
 */
void mf3_transpose_and_copy(mf3_e *tr, mf3_e const *m);

/**
 * Compute a matrix-vector product: s^t = He^t.
 * Assumes s->size = H->n_row and e->size = H->n_col.
 * @param s a valid pointer to a vector to hold the result
 * @param H a valid pointer to a matrix
 * @param e a valid pointer to a vector
 */
void mf3_product_matrix_vector(vf3_e *s, mf3_e const *H, vf3_e const *e);

/**
 * Compute a vector-matrix product: y = xG.
 * Assumes y->size = G->n_col and x->size = G->n_row
 * @param y a valid pointer to a vector to hold the result
 * @param x a valid pointer to a vector
 * @param g a valid pointer to a matrix
 */
void mf3_product_vector_matrix(vf3_e *y, vf3_e const *x, mf3_e const *G);

/**
 * Extend a matrix downwards with rows of zeroes.
 * The matrix is reallocated, and the new rows are allocated and zeroed.
 * @param H a pointer to a valid matrix
 * @param nr_stack number of new zero rows to be added
 */
void mf3_stack_zeros(mf3_e *H, size_t nr_stack);

/**
 * Extend a matrix H downwards with a copy of the rows of another matrix M.
 * Assumes H->n_col = M->n_col.
 * H will be reallocated, and copies of the rows of M will be allocated.
 * @param H a valid pointer to the matrix to be extended
 * @param M a valid pointer to the matrix to be stacked under H
 */
void mf3_stack(mf3_e *H, mf3_e const *M);

/**
 * Check equality of (entries of) two matrices.
 * @param H a pointer to a valid matrix
 * @param M a pointer to a valid matrix
 * @return 1 if the matrices are equal, otherwise 0.
 */
int mf3_equal(mf3_e const *H, mf3_e const *M);

/**
 * Set all entries of a matrix to zero.
 * @param M a valid pointer to a matrix
 */
void mf3_settozero(mf3_e *M);

/**
 * Compute a binary vector containing the support of the diagonal of a ternary
 * matrix. Assumes d->size = m->n_row
 * @param d a valid pointer to a binary vector (that will hold the result)
 * @param M a valid pointer to a matrix
 */
void vf2_mf3_diag_support(vf2_e *d, mf3_e const *M);

/**
 * Print the contents of the vectors in a matrix.
 * @param M a valid pointer to a matrix
 */
void mf3_print(mf3_e *M);

/**
 * Set the (i,j)-th entry of a matrix to a given trit value (represented as an
 * integer in {0,1,2}) Assumes 0 <= i < M->n_row and 0 <= j < M->n_col
 * @param M a valid pointer to a matrix
 * @param i an integer (row index)
 * @param j an integer (column index)
 * @param a an integer in {0,1,2} representing a single trit
 */
static inline void mf3_setcoeff(mf3_e *M, int i, int j, int8_t a) {
  vf3_set_coeff(j, &M->rows[i], a);
}

/**
 * Add a given trit value (represented as an integer in {0,1,2}) to the (i,j)-th
 * entry of a matrix. Assumes 0 <= i < M->n_row and 0 <= j < M->n_col
 * @param M a valid pointer to a matrix
 * @param i an integer (row index)
 * @param j an integer (column index)
 * @param a an integer in {0,1,2} representing a single trit
 */
static inline void mf3_addcoeff(mf3_e *M, int i, int j, int8_t a) {
  vf3_add_coeff(j, &M->rows[i], a);
}

/**
 * Get the (i,j)-th entry of a matrix, represented as an integer in {0,1,2}
 * Assumes 0 <= i < M->n_row and 0 <= j < M->n_col
 * @param M a valid pointer to a matrix
 * @param i an integer (row index)
 * @param j an integer (column index)
 * @return a an integer in {0,1,2} representing a single trit
 */
static inline uint8_t mf3_coeff(mf3_e const *M, int i, int j) {
  return vf3_get_element(j, &M->rows[i]);
}

#endif  // WAVE2_MF3_H
