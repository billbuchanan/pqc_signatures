/*
 * Contains the key generation procedure and related functions.
 *
 * ==========================(LICENSE BEGIN)============================
 *
 * Copyright (c) 2023  Squirrels Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * ===========================(LICENSE END)=============================
 *
 * @author   Guilhem Niot <guilhem.niot@gniot.fr>
 */

#include "flint.h"
#include "fmpq.h"
#include "fmpz.h"
#include "fmpz_lll.h"
#include "fmpz_mat.h"
#include "fmpz_vec.h"
#include "inner.h"
#include "keygen_lll.h"
#include <gmp.h>
#include <stdint.h>
#include <stdio.h>

#define SQUIRRELS_NBMINORS 4

static void gram_schmidt(const secret_key *sk, int size_basis,
                         continuous_vector *vec) {
  for (int i = 0; i < size_basis; i++) {
    fpr factor = fpr_div(Zf(cvector_dot)(vec, &sk->b_gm[i]),
                         Zf(cvector_dot)(&sk->b_gm[i], &sk->b_gm[i]));
    Zf(cvector_submul2)(vec, &sk->b_gm[i],
                        factor); // Projection of vec on the gram schmidt vector
  }
}

// Sample a vector in the orthogonal of the previously sampled vectors
static void sample_orthogonal(inner_shake256_context *rng, const secret_key *sk,
                              double min_norm, double max_norm, int size_basis,
                              continuous_vector *candidate) {
  // First, sample a gaussian vector of standard deviation g_{max}/sqrt(SQUIRRELS_D)
  Zf(normaldist)(rng, candidate);
  Zf(cvector_mul)(candidate, FPR(SQUIRRELS_GSNORM_MAX / sqrt(SQUIRRELS_D/2) / sqrt(SQUIRRELS_D)));

  continuous_vector candidate_orth;
  memcpy(&candidate_orth, candidate, sizeof(continuous_vector));

  // Sample a direction in B orthogonal, and a gaussian vector in B
  gram_schmidt(sk, size_basis, &candidate_orth);
  Zf(cvector_sub)(candidate, &candidate_orth);

  // Then, we sample its norm
  int dim_orthogonal = SQUIRRELS_D - size_basis;

  uint64_t u = get_rng_u64(rng);
  double seed = (double)(u & 0x1FFFFFFFFFFFFFul) *
                pow(2, -53); // Random double between 0 and 1
  double r =
      min_norm * pow(seed * (pow(max_norm / min_norm, dim_orthogonal) - 1) + 1,
                     1. / dim_orthogonal);

  Zf(cvector_mul)(&candidate_orth, fpr_div(FPR(r), Zf(cvector_norm(&candidate_orth))));

  Zf(cvector_add)(candidate, &candidate_orth);
}

static int reduce_bigvec(const secret_key *sk, const fmpz_mat_t mat, fmpz_t last_vec) {
  slong maxbits = -1, prevmaxbits = 0;

  // We reduce it using Babai Nearest Plane
  while (maxbits != prevmaxbits) {
    prevmaxbits = maxbits;
    maxbits = _fmpz_vec_max_bits(last_vec, SQUIRRELS_D);
    if (maxbits < 0) {
      maxbits = -maxbits;
    }

    continuous_vector approx_last_vec;
    for (int i = 0; i < SQUIRRELS_D; i++) {
      slong exp = 0;
      approx_last_vec.coeffs[i] = FPR(fmpz_get_d_2exp(&exp, &last_vec[i]));
      approx_last_vec.coeffs[i].v *=
          pow(2, (maxbits > 53 ? 53 : maxbits) - (maxbits - exp));
    }

    // Apply Babai Nearest Plane
    int64_t coeff;
    for (int i = SQUIRRELS_D - 2; i >= 0; i--) {
      fpr prod_s = fpr_div(Zf(cvector_dot)(&sk->b_gm[i], &approx_last_vec),
                           Zf(cvector_dot)(&sk->b_gm[i], &sk->b_gm[i]));
      coeff = fpr_rint(prod_s);

      Zf(cvector_submul)(&approx_last_vec, &sk->b[i], fpr_of(coeff));

      // Multiply by the i-th column of the basis in GMP representation
      _fmpz_vec_scalar_submul_si_2exp(last_vec, fmpz_mat_entry(mat, i, 0), SQUIRRELS_D, coeff,
                                      maxbits - (maxbits > 53 ? 53 : maxbits));
    }
  }

  return maxbits < 31;
}

/**
 * Compute the five first minors of the matrix using Bareiss algorithm:
 * https://en.wikipedia.org/wiki/Bareiss_algorithm
 */
static int compute_cocyclic_equation(fmpz_mat_t mat) {
  // Compute HNF
  fmpz_mat_hnf(mat, mat);

  // Verify that the basis is cocyclic
  int is_cocyclic = 1;
  for (int i = 0; i < SQUIRRELS_D; i++) {
    for (int j = 0; j < SQUIRRELS_D - 1; j++) {
      if (i == j) { // must be one
        is_cocyclic &= fmpz_is_one(fmpz_mat_entry(mat, i, j));
      } else {
        is_cocyclic &= fmpz_is_zero(fmpz_mat_entry(mat, i, j));
      }
    }
  }

  if (!is_cocyclic) {
    return 0;
  }

  // The equation vector is the last column of mat at this point

  return 1;
}

static int draw_basis(inner_shake256_context *rng, secret_key *sk,
                      public_key *pk, fmpz_mat_t mat) {
  double drift = 0; // To check how far we are from the target det

  // Sample the n-1 first vectors
  fpr gs_norm;
  for (int k = 0; k < SQUIRRELS_D - 1; k++) {
    do {
      double alpha = SQUIRRELS_GS0NORM_MIN;
      double beta = SQUIRRELS_GS0NORM_MAX;
      if (drift > 0.01) { // Correction term to make sure we are not too far
                          // from the target det at the end of the sampling
        beta = (beta + 3 * alpha) / 4;
      } else if (drift < -0.01) {
        alpha = (3 * beta + alpha) / 4;
      }

      // Sample the k-th vector
      continuous_vector candidate;
      sample_orthogonal(rng, sk, alpha, beta, k, &candidate);

      // Round and add to basis
      Zf(cvector_rint)(&sk->b[k], &candidate);

      // Compute gram schmidt vector
      Zf(cvector_of)(&sk->b_gm[k], &sk->b[k]);
      gram_schmidt(sk, k, &sk->b_gm[k]);

      // We compute the norm of the gram schmidt vector, and reject in case it
      // is not in the defined boundaries
      gs_norm = Zf(cvector_norm)(&sk->b_gm[k]);
    } while (gs_norm.v < SQUIRRELS_GSNORM_MIN || gs_norm.v > SQUIRRELS_GSNORM_MAX);
    drift += log(gs_norm.v) - SQUIRRELS_LOG_DET_ON_DIM;
  }

  for (int i = 0; i < SQUIRRELS_D - 1; i++) {
    for (int j = 0; j < SQUIRRELS_D; j++) {
      fmpz_set_si(fmpz_mat_entry(mat, i, j), sk->b[i].coeffs[j]);
    }
  }

  // First, compute 4 minors
  fmpz *minors = _fmpz_vec_init(SQUIRRELS_NBMINORS);

  fmpz_mat_t matminors;
  fmpz_mat_window_init(matminors, mat, 0, 0, SQUIRRELS_D-1, SQUIRRELS_D);
  Zf(compute_minors)(minors, SQUIRRELS_NBMINORS, matminors);
  fmpz_mat_window_clear(matminors);

  int ret = Zf(compute_last_vector)(minors, fmpz_mat_entry(mat, SQUIRRELS_D-1, 0));
  _fmpz_vec_clear(minors, SQUIRRELS_NBMINORS);

  if (ret) {
    return 1;
  }

  if (!reduce_bigvec(sk, mat, fmpz_mat_entry(mat, SQUIRRELS_D-1, 0))) {
    return 1;
  }
  // We save it in the secret key structure
  for (int j = 0; j < SQUIRRELS_D; j++) { 
    sk->b[SQUIRRELS_D - 1].coeffs[j] = (int32_t)fmpz_get_si(fmpz_mat_entry(mat, SQUIRRELS_D-1, j));
  }

  // Compute gram schmidt vector
  Zf(cvector_of)(&sk->b_gm[SQUIRRELS_D - 1], &sk->b[SQUIRRELS_D - 1]);
  gram_schmidt(sk, SQUIRRELS_D - 1, &sk->b_gm[SQUIRRELS_D - 1]);

  // Check last vector norm
  gs_norm = Zf(cvector_norm)(&sk->b_gm[SQUIRRELS_D - 1]);
  if (gs_norm.v < SQUIRRELS_GSNORM_MIN || gs_norm.v > SQUIRRELS_GSNORM_MAX) {
    return 1;
  }

  // Compute cocyclic equation
  if (!compute_cocyclic_equation(mat)) {
    return 1;
  }

  // Then compute modulo each p_i for the public key
  fmpz_t tmp_eq;
  fmpz_init(tmp_eq);
  for (int l = 0; l < SQUIRRELS_NBPRIMES; l++) {
    for (int i = 0; i < SQUIRRELS_D - 1; i++) {
      pk->equation[l * (SQUIRRELS_D - 1) + i] =
          (int32_t)fmpz_mod_ui(tmp_eq, fmpz_mat_entry(mat, i, SQUIRRELS_D-1), (ulong)Zf(det_primes)[l]);
    }
  }
  fmpz_clear(tmp_eq);

  return 0;
}

int Zf(keygen)(inner_shake256_context *rng, secret_key *sk, public_key *pk) {
  // Will contain the basis, share memory between generation attempts
  fmpz_mat_t mat;
  fmpz_mat_init(mat, SQUIRRELS_D, SQUIRRELS_D);

  while (1) {
    int ret = draw_basis(rng, sk, pk, mat);
    if (ret == 0) {
      fmpz_mat_clear(mat);
      return 0; // success
    }

    // Otherwise restart
  }
}
