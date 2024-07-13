/*
 * Implementation of the signature generation and verification procedures.
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
 * @author   Guilhem Niot <guilhem@gniot.fr>
 */

#include "api.h"
#include "inner.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

static int check_norm(const discrete_vector *sign_point) {
  return Zf(vector_dot)(sign_point, sign_point) <= SQUIRRELS_BETA_SQUARE;
}

// algo 2.6 https://tprest.github.io/pdf/pub/thesis-thomas-prest.pdf
static void klein_sampler(inner_shake256_context *rng, const unsigned char *sk,
                          discrete_vector *candidate_sign) {
  for (int i = SQUIRRELS_D - 1; i >= 0; i--) { // Randomized babai rounding
    // Compute <candidate_sign,  sk->b_gm[i]>
    fpr scal_candidate = FPR(0.);
    for (int j = 0; j < SQUIRRELS_D; j++) {
      scal_candidate =
          fpr_add(scal_candidate, fpr_mul(coeff_sk_b_gm(sk, i, j),
                                          fpr_of(candidate_sign->coeffs[j])));
    }
    // Compute <sk->b_gm[i], sk->b_gm[i]>
    fpr scal_gs = FPR(0.);
    for (int j = 0; j < SQUIRRELS_D; j++) {
      scal_gs = fpr_add(scal_gs, fpr_sqr(coeff_sk_b_gm(sk, i, j)));
    }

    fpr di = fpr_div(scal_candidate, scal_gs);
    fpr sigmai = fpr_div(FPR(SQUIRRELS_SIGMA), fpr_sqrt(scal_gs));

    // Sample a coordinate for vector i
    int32_t zi = Zf(sampler)(rng, di, sigmai, FPR(SQUIRRELS_SIGMIN));

    // candidate_sign -= sk->b[i] * zi
    for (int j = 0; j < SQUIRRELS_D; j++) {
      candidate_sign->coeffs[j] -= zi * coeff_sk_b(sk, i, j);
    }
  }
}

int Zf(sign)(inner_shake256_context *rng, const unsigned char *sk,
             discrete_vector *hm) {
  // Reduce the component belonging to the lattice from the hash using Klein
  // sampler
  klein_sampler(rng, sk, hm);

  return check_norm(hm);
}

int Zf(verify_raw)(const unsigned char *pk, const discrete_vector *sig,
                   const discrete_vector *hm) {
  // We check that the difference sig-hm belongs to the lattice
  int result = 1;
  // To do so, we compute over each prime which is part of the basis determinant
  for (int pi = 0; pi < SQUIRRELS_NBPRIMES; pi++) {
    int32_t p = Zf(det_primes)[pi]; // modulo

    int64_t s = 0;
    for (int k = 0; k < SQUIRRELS_D - 1; k++) {
      s += ((int64_t)sig->coeffs[k] - hm->coeffs[k]) *
           coeff_pk(pk, k, pi);
    }

    s -= sig->coeffs[SQUIRRELS_D - 1] - hm->coeffs[SQUIRRELS_D - 1];

    result &= (s % p) == 0;
  }

  if (!result) {
    return result;
  }

  return check_norm(sig);
}
