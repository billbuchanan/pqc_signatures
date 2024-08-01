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
#include "fq_arithmetic/vf3.h"
#include "params.h"
#include "prng/prng.h"
#include "types.h"
#include "util/gauss.h"
#include "util/mf3permut.h"
#include "wave/randperm.h"

int wave_sk_alloc(wave_sk_t *sk) {
  // sk = malloc(sizeof(wave_sk_t));
  sk->b = vf3_alloc(N2);
  sk->c = vf3_alloc(N2);
  // sk->perm fixed size uint16_t array
  // sk->mk fixed size uint8_t array
  return 1;
}

int wave_pk_alloc(wave_pk_t *pk) {
  //*pk = malloc(sizeof(wave_pk_t));
  pk->R = mf3_alloc(N - K, K);

  return 1;
}

void wave_sk_free(wave_sk_t *sk) {
  vf3_free(sk->b);
  vf3_free(sk->c);
  memset(sk->mk, 0, MK_SIZE);
}

void wave_pk_free(wave_pk_t *pk) { mf3_free(pk->R); }

/* assume (all allocated):
 - R a (N-K) x K matrix, will contain the public key
 - HVt the parity check matrix of V, transposed
 - pi a permutation of N elements
 - c_u and c_v length N vectors of coefficients
 - mk the master key
 */
void pkgen(mf3_e *R, mf3_e *HVt, uint16_t *pi, vf3_e *c_u, vf3_e *c_v,
           uint8_t *mk) {
  int i, j, l, k;
  uint8_t coeff_u, coeff_v;
  uint16_t sigma[N] = {0};
  int pivot[N - K] = {0};

  vf3_e *col_u;
  col_u = vf3_alloc(N2 - KU);

  mf3_e *Hdbl = mf3_alloc(N, N2 - KV);
  for (i = 0; i < N2; ++i) {
    vf3_copy(Hdbl->rows + i, HVt->rows + i);
    vf3_copy(Hdbl->rows + (i + N2), HVt->rows + i);
  }
  mf3_row_permute(Hdbl, pi);

  mf3_e *H_tmp, *H;
  H_tmp = mf3_alloc(N, N - K);
  H = mf3_alloc(N - K, N);

  for (i = 0; i < N; ++i) {
    j = pi[i] % N2;
    vf3_random_from_seed(col_u, mk, MK_SIZE, DOMAIN_U ^ j);
    coeff_u = vf3_get_element(i, c_u);
    vf3_vector_scalarmul_inplace(col_u, coeff_u);
    coeff_v = vf3_get_element(i, c_v);
    vf3_vector_scalarmul_inplace(Hdbl->rows + i, coeff_v);
    vf3_vector_cat(H_tmp->rows + i, col_u, Hdbl->rows + i);
  }

  mf3_transpose_and_copy(H, H_tmp);  // should be (N-K) x N matrix now
  if (gauss_elimination(H, pivot) != N - K) {
    // should not happen, put a safeguard?
  }

  mf3_e *pk_t = mf3_alloc(N - K, K);
  mf3_e *m_t = mf3_alloc(K, N - K);

  // build the public key from the K non pivot columns of H_tmp
  // assumes pivot[] contains pivot indices in increasing order
  for (j = 0, i = 0, l = 0; j < N; ++j) {
    if ((i < N - K) && (pivot[i] == j)) {  // j the i-th pivot
      // identity part of the matrix -> skip
      ++i;
    } else {
      // copy j-th column of H_tmp into the l-th column of R
      for (k = 0; k < N - K; ++k) {
        mf3_setcoeff(pk_t, k, l, mf3_coeff(H, k, j));
      }
      ++l;
    }
  }

  mf3_transpose_and_copy(m_t, pk_t);

#if K % 2 == 1
  for (i = 0; i < (K - 1) / 2; i++) {
#else
  for (i = 0; i < (K) / 2; i++) {
#endif
    vf3_vector_add(&R->rows[2 * i], &m_t->rows[(2 * i)],
                   &m_t->rows[(2 * i) + 1]);
    vf3_vector_sub(&R->rows[2 * i + 1], &m_t->rows[2 * i],
                   &m_t->rows[(2 * i) + 1]);
  }

#if K % 2 == 1
  vf3_e *tmp = vf3_alloc(K);
  vf3_vector_constant(tmp, 2);
  vf3_vector_mul(&R->rows[K - 1], tmp, &m_t->rows[K - 1]);
  // f3_vector_neg_vector(&r_t->row[K - 1], &m_t->row[K - 1]);
#endif

  // adjust the permutation pi
  for (j = 0, i = 0, l = N - K; j < N; ++j) {
    if ((i < N - K) && (pivot[i] == j)) {  // j the i-th pivot
      sigma[i++] = pi[j];
    } else {  // j is not a pivot
      sigma[l++] = pi[j];
    }
  }
  memcpy(pi, sigma, N * sizeof(uint16_t));

  vf3_free(col_u);
  mf3_free(H_tmp);
  mf3_free(H);
  mf3_free(Hdbl);
  mf3_free(pk_t);
  mf3_free(m_t);
#if K % 2 == 1
  vf3_free(tmp);
#endif
}

void parity_check_transpose_v(mf3_e *Ht, uint8_t *mk) {
  int i, j;
  mf3_e *Gt, *G;
  Gt = mf3_alloc(Ht->n_row, Ht->n_row - Ht->n_col);
  G = mf3_alloc(Ht->n_row - Ht->n_col, Ht->n_row);
  do {
    rng_bytes((uint8_t *)mk, MK_SIZE);
    for (i = 0; i < Gt->n_row; ++i) {
      vf3_random_from_seed(Gt->rows + i, mk, MK_SIZE, DOMAIN_V ^ i);
    }
    mf3_transpose_and_copy(G, Gt);
  } while (gauss_elimination_with_abort(G) < G->n_row);
  mf3_settozero(Ht);
  for (i = 0; i < G->n_row; ++i) {
    for (j = 0; j < Ht->n_col; ++j) {
      mf3_setcoeff(Ht, i, j, mf3_coeff(G, i, j + G->n_row));
    }
  }
  for (i = G->n_row; i < Ht->n_row; ++i) {
    mf3_setcoeff(Ht, i, i - G->n_row, 2);  // 2 is -1 mod 3
  }
  /*
   * G = ( Id | R) size k x n
   *
   *      (  R  )
   * Ht = (-----) size n x (n - k)
   *      ( -Id )
   */
  mf3_free(G);
  mf3_free(Gt);
}

int keygen(wave_sk_t *sk, wave_pk_t *pk) {
  vf3_e *a, *b, *c, *d, *x, *y, *tmp;

  a = vf3_alloc(N2);
  tmp = vf3_alloc(N2);
  b = vf3_alloc(N2);
  c = vf3_alloc(N2);
  d = vf3_alloc(N2);
  x = vf3_alloc(N);
  y = vf3_alloc(N);

  wave_sk_alloc(sk);

  mf3_e *HVt;
  HVt = mf3_alloc(N2, N2 - KV);
  /*
   parity_check_transpose_v() will also select a master key sk->mk
   such that HV can be in systematic form (simpler for CCT)
   */
  parity_check_transpose_v(HVt, sk->mk);

  vf3_random(sk->b);
  vf3_random_non_zero(sk->c);

  vf3_vector_constant(a, 1);        // a = 1
  vf3_vector_constant(tmp, 1);      // a = 1
  vf3_vector_mul(d, sk->b, sk->c);  // d =  b * c
  vf3_vector_add_inplace(d, tmp);   // d = 1 + b * c
  vf3_vector_neg(b, sk->b);
  vf3_vector_neg(c, sk->c);
  vf3_vector_cat(x, d, b);  // x = (d | b) = (d | -sk->b)
  vf3_vector_cat(y, c, a);  // y = (c | 1) = (-sk->c | 1)

  randperm(sk->perm, N, x, y);
  /* sk->perm may change in the pkgen process: some values may be
           swapped to allow a systematic form. It must not be used before
           the call to pkgen
  */

  if (pk != NULL) {
    wave_pk_alloc(pk);
    pkgen(pk->R, HVt, sk->perm, x, y, sk->mk);
    // sk->perm was may have been modified in place and has now its definitive
    // value
  }

  vf3_free(a);
  vf3_free(b);
  vf3_free(c);
  vf3_free(d);
  vf3_free(x);
  vf3_free(y);
  vf3_free(tmp);
  mf3_free(HVt);

  return 1;
}
