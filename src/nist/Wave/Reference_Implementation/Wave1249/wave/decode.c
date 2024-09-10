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

#include "fq_arithmetic/mf3.h"
#include "fq_arithmetic/vf2.h"
#include "fq_arithmetic/vf3.h"
#include "params.h"
#include "randperm.h"
#include "reject.h"
#include "sample.h"
#include "types.h"
#include "util/gauss.h"

void mf3_genmatperm(mf3_e *H, uint16_t *pi, uint8_t *seed, int seed_len,
                    uint32_t domain) {
  mf3_e *tmp = mf3_alloc(H->n_col, H->n_row);

  for (size_t i = 0; i < tmp->n_row; ++i) {
    vf3_random_from_seed(tmp->rows + i, seed, seed_len, domain ^ pi[i]);
  }

  mf3_transpose_and_copy(H, tmp);

  mf3_free(tmp);
}

int decode_v(vf3_e *e_v, vf3_e *y_v, wave_sk_t *sk) {
  uint16_t t, pi[N2];

  // BEGIN alloc
  vf2_e *z;
  z = vf2_alloc(KV);

  vf3_e *y, *e, *x, *tmp;
  y = vf3_alloc(N2);
  e = vf3_alloc(N2);
  x = vf3_alloc(KV);
  tmp = vf3_alloc(KV);

  mf3_e *G = mf3_alloc(KV, N2);
  // END alloc

  t = sampleV();

  int nb_pivots;
  do {
    vf3_copy(y, y_v);
    randpermV(pi, N2, KV - D, t,
              y);  // sample a random permutation pi and permute y inplace
    mf3_genmatperm(G, pi, sk->mk, MK_SIZE,
                   DOMAIN_V);  // generate G = G_V^\pi from the master key
    nb_pivots = partial_gauss_elimination(G, D);  // partial Gauss elim inplace
  } while (nb_pivots != KV - D);
  /* repeat until no pivot fail, if no pivot failed, the first KV-D
   * columns of G contains a diagonal of 1 on top, all other
   * coefficients in those columns are null */

  vf2_vector_1t0(z, t);  // z = 1^t | 0^*
  vf3_random_non_zero(x);
  vf3_vector_mask(x, z);           // first t coordinates of x are random != 0
  vf3_vector_split_zero(tmp, y);   // (tmp | *) = y
  vf3_vector_sub_inplace(x, tmp);  // x = x - tmp

  mf3_product_vector_matrix(e, x, G);
  /*
   now e is a codeword which
   - differs from y in the first t positions
   - equals to y in the next KV-D-t coordinates (assumes t <= KV-D)
   - is randomly distributed trits in the last N2-KV+D coordinates
   */

  vf3_vector_add_inplace(e, y);
  /*
   now e is a word such that (y - e) is a codeword:
   - of weight t exactly on the first KV-D coordinates
   - with randomly distributed trits in the last N2-KV+D coordinates
   */

#ifdef CHECK
  int pivot[N];
  mf3_stack_zeros(G, 2);
  vf3_copy(G->rows + KV, y);
  vf3_copy(G->rows + (KV + 1), e);
  if (gauss_elimination(G, pivot) != KV + 1) printf("decode_v: wrong coset\n");
  int j = 0;
  for (int i = 0; i < KV - D; ++i)
    if (vf3_get_element(i, e) != 0) ++j;
  if (j != t) printf("decode_v: wrong weight\n");
#endif

  vectperminv(e_v, e, pi);

  /*
   now e_v is such that (y_v - e_v) is a codeword
   */

  // BEGIN free
  vf2_free(z);

  vf3_free(y);
  vf3_free(e);
  vf3_free(x);
  vf3_free(tmp);

  mf3_free(G);
  // END free

  return vf3_hamming_weight(e_v);
}

int decode_u(vf3_e *e_u, vf3_e *y_u, vf3_e *e_v, wave_sk_t *sk) {
  uint16_t t, l, i, j, pi[N2];

  // BEGIN alloc
  vf2_e *z0, *z1, *s, *s0, *s1;
  z0 = vf2_alloc(N2 - KU + D);
  z1 = vf2_alloc(KU - D);
  s = vf2_alloc(N2);
  s0 = vf2_alloc(N2 - KU + D);
  s1 = vf2_alloc(KU - D);

  vf3_e *y, *v, *e, *e0, *e1, *v0, *v1, *synd, *tmp;
  y = vf3_alloc(N2);
  e = vf3_alloc(N2);
  v = vf3_alloc(N2);
  e0 = vf3_alloc(N2 - KU + D);
  e1 = vf3_alloc(KU - D);
  v0 = vf3_alloc(N2 - KU + D);
  v1 = vf3_alloc(KU - D);
  synd = vf3_alloc(N2 - KU + D);
  tmp = vf3_alloc(N2);

  mf3_e *H;
  // END alloc

  t = vf3_hamming_weight(e_v);
  l = sampleU(t);

  int nb_pivots;
  while (1) {
    vf3_copy(y, y_u);  // y = y_u
    vf3_vector_sub(v, sk->c, sk->b);
    vf3_vector_mul_inplace(v, e_v);  // v = (c - b) * e_v
    vf2_vf3_support(s, e_v);  // s = supp(e_v) = e_v * e_v, a binary vector
    randpermU(pi, N2, N2 - KU + D, l, y, v,
              s);  // sample a random permutation pi and permute y,v,s inplace
    H = mf3_alloc(N2 - KU, N2);
    mf3_genmatperm(H, pi, sk->mk, MK_SIZE,
                   DOMAIN_U);  // generate H = H_U^\pi from the master key
    nb_pivots = extended_gauss_elimination(
        H, D);  // extended Gauss elim inplace, H increases by D rows
    if (nb_pivots == N2 - KU) break;
    mf3_free(H);
  }
  /* repeat until N2 - KU pivot succeed, when this happens, the first
   * N2-KU+D columns of H contains N2-KU '1' on its diagonal, all other
   * coefficients in those columns are null */

  vf2_mf3_diag_support(z0, H);  // z0 = diag(H)
  vf2_vector_split(s0, s1, s);  // (s0 | s1) <- s
  vf3_vector_split(v0, v1, v);  // (v0 | v1) <- v

  do {
    vf3_random(e0);
    vf3_vector_unmask(e0, z0);  // e0 = e0 * (1 - z0)

    vf3_random_non_zero(e1);
    vf3_vector_unmask(e1, s1);  // e1 = e1 * (1 - s1)
    // note that e1 and v1 have disjoint supports
    vf3_vector_add_inplace(e1, v1);  // e1 = v1 + e1

    vf3_vector_cat(e, e0, e1);  // e = (e0 | e1)

    /*
     now e is a vector with
     - in the first N2-KU+D positions (corresponding to e0):
     - random trits in D (non-pivot) positions (where z0 is null)
     - 0 on the other N2-KU (pivot) positions (where z0 is 1)
     - in the last KU-D positions (corresponding to e1):
     - random non-zero where s1 is null (positions where e_v is null)
     - coincide with v1 on the other positions
     */

    vf3_vector_sub(tmp, y, e);                // tmp = y - e
    mf3_product_matrix_vector(synd, H, tmp);  // synd = (y - e) * H^T
    // note that e0 and synd have disjoint supports
    vf3_vector_cat(tmp, synd, NULL);  // tmp = (synd | 0)
    vf3_vector_add_inplace(e, tmp);   // e = (e0 | e1) + (synd | 0)

    /*
     now e is a vector such that e * H^T = y * H^T
     */

    vf3_vector_add_inplace(e0, synd);
    vf3_vector_sub_inplace(e0, v0);
    j = vf3_hamming_weight(e0);
    vf3_vector_mask(e0, s0);
    i = vf3_hamming_weight(e0);
    j = N2 - KU + D - l - j + i;
    // i is the number of weight 1 matched pairs
    // j is the number of weight 0 matched pairs
    // the weigth default is 2 * j + i
  } while (2 * j + i != N - WEIGHT);

#ifdef CHECK
  mf3_product_matrix_vector(synd, H, e);
  mf3_product_matrix_vector(v0, H, e);
  if (!vf3_equal(synd, v0)) printf("decode_u: wrong coset\n");
#endif

  vectperminv(e_u, e, pi);

  /*
   now e_u is a vector such that e_u * H_U^T = y_u * H_U^T
   */

  // BEGIN free
  vf2_free(z0);
  vf2_free(z1);
  vf2_free(s);
  vf2_free(s0);
  vf2_free(s1);

  vf3_free(y);
  vf3_free(e);
  vf3_free(v);
  vf3_free(e0);
  vf3_free(e1);
  vf3_free(v0);
  vf3_free(v1);
  vf3_free(synd);
  vf3_free(tmp);

  mf3_free(H);
  // END free

  return j;
}

// decode(e, y, sk);
int decode(vf3_e *e, vf3_e *s, wave_sk_t *sk) {
  int t, j;
  uint16_t pi[N];

  // BEGIN alloc
  vf3_e *y, *y_u, *y_v, *e_u, *e_v, *tmp;
  y = vf3_alloc(N);
  y_u = vf3_alloc(N2);
  y_v = vf3_alloc(N2);
  e_u = vf3_alloc(N2);
  e_v = vf3_alloc(N2);
  tmp = vf3_alloc(N);
  // END alloc

  vf3_vector_cat_zero(tmp, s);                 // y = (s | 0)
  memcpy(pi, sk->perm, N * sizeof(uint16_t));  // or whatever is suitable
  vectperminv(y, tmp,
              pi);  // now y is y permuted and pi is the inverse of sk->perm

  vf3_vector_split(y_u, y_v, y);                // (y_u | y_v) <- y
  vf3_vector_sub_mul_inplace(y_v, sk->c, y_u);  // y_v = y_v - sk->c * y_u
  vf3_vector_sub_mul_inplace(y_u, sk->b, y_v);  // y_u = y_u - sk->b * y_v

  do {
    t = decode_v(e_v, y_v, sk);       // t the Hamming weight of e_v
    j = decode_u(e_u, y_u, e_v, sk);  // j the nb of (0,0) among matching pairs
  } while (!accept_instance(t, j));
  /*
   rejection is deterministic, uncommon values of (t,j) are
   rejected. The probability of occurrence is lower than 2^(-64),
   but rejection is required for the security proof (tailcut lemma)
   */

  vf3_vector_add_mul_inplace(e_u, sk->b, e_v);  // e_u = e_u + sk->b * e_v
  vf3_vector_add_mul_inplace(e_v, sk->c, e_u);  // e_v = e_v + sk->c * e_u
  vf3_vector_cat(tmp, e_u, e_v);                // tmp = (e_u | e_v)
  vectperminv(e, tmp, pi);  // apply the inverse of pi, that is sk->perm, to tmp

  // BEGIN free
  vf3_free(y);
  vf3_free(y_u);
  vf3_free(y_v);
  vf3_free(e_u);
  vf3_free(e_v);
  vf3_free(tmp);
  // END free
  return 1;
}
