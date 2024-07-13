/*
 * Functions to compute the last vector from a set of minors.
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
#include "fmpz.h"
#include "fmpz_mat.h"
#include "fplll.h"
#include "inner.h"
#include <fplll/defs.h>
#include <stdio.h>

// Notations from https://staff.itee.uq.edu.au/havas/1994mh.pdf
static void reduce_bezout_coeff(fmpz_t x1, fmpz_t x2, const fmpz_t a1,
                                const fmpz_t a2) {
  fmpz_t i, s;
  fmpz_init(i);
  fmpz_init(s);
  fmpz_ndiv_qr(i, s, x2, a1);

  fmpz_addmul(x1, a2, i);
  fmpz_submul(x2, a1, i);

  fmpz_clear(i);
  fmpz_clear(s);
}

// Implements https://staff.itee.uq.edu.au/havas/1994mh.pdf to get
// small bezout coefficients
extern "C" int Zf(compute_last_vector)(const fmpz *minors, fmpz *last_vector) {
  fmpz_t g1, g2;
  fmpz_init(g1), fmpz_init(g2);
  fmpz_t gcd_coeff[4];
  for (int l = 0; l < 4; l++) {
    fmpz_init(gcd_coeff[l]);
  }

  // Then compute gcd(0, 1) and gcd(2, 3)
  fmpz_xgcd_canonical_bezout(g1, gcd_coeff[0], gcd_coeff[1], &minors[0],
                             &minors[1]);
  fmpz_xgcd_canonical_bezout(g2, gcd_coeff[2], gcd_coeff[3], &minors[2],
                             &minors[3]);

  // We compute full gcd
  fmpz_t gcd;
  fmpz_init(gcd);
  fmpz_t x1, x2;
  fmpz_init(x1);
  fmpz_init(x2);

  fmpz_xgcd_canonical_bezout(gcd, x1, x2, g1, g2);
  if (!fmpz_is_pm1(gcd)) {
    fmpz_clear(g1);
    fmpz_clear(g2);
    for (int l = 0; l < 4; l++) {
      fmpz_clear(gcd_coeff[l]);
    }

    return 1; // failure, the minors are not coprime, restart
  }

  // Then, we want an equation equal to the target det so we multiply everything
  // by it
  fmpz_t target_det;
  fmpz_init_set_ui(target_det, 1);
  for (int i = 0; i < SQUIRRELS_NBPRIMES; i++) {
    fmpz_mul_ui(target_det, target_det, Zf(det_primes)[i]);
  }

  fmpz_mul(x1, x1, target_det);
  fmpz_mul(x2, x2, target_det);

  // if we have a gcd 1, then we compute reduced coefficients for x1*g1 and
  // x2*g2
  reduce_bezout_coeff(x1, x2, g1, g2);

  // First, x1*g1
  fmpz_mul(gcd_coeff[0], gcd_coeff[0], x1);
  fmpz_mul(gcd_coeff[1], gcd_coeff[1], x1);

  reduce_bezout_coeff(gcd_coeff[0], gcd_coeff[1], &minors[0], &minors[1]);

  // First, x2*g2
  fmpz_mul(gcd_coeff[2], gcd_coeff[2], x2);
  fmpz_mul(gcd_coeff[3], gcd_coeff[3], x2);

  reduce_bezout_coeff(gcd_coeff[2], gcd_coeff[3], &minors[2], &minors[3]);

  for (int l = 0; l < 4; l++) {
    fmpz_set(&last_vector[SQUIRRELS_D - 1 - l], gcd_coeff[l]);
    if (l % 2 == 0) {
      fmpz_neg(&last_vector[SQUIRRELS_D - 1 - l],
               &last_vector[SQUIRRELS_D - 1 - l]);
    }
  }

  // Reduce further with LLL
  int nbcols = 5;
  fplll::ZZ_mat<mpz_t> mat =
      fplll::ZZ_mat<mpz_t>(1 + (nbcols * (nbcols - 1)) / 2, nbcols);

  // First row, represent last vector
  mpz_t v;
  mpz_init(v);
  for (int l = 0; l < 4; l++) {
    fmpz_get_mpz(v, gcd_coeff[l]);
    mat(0, l) = v;
  }
  mpz_clear(v);

  mpz_t v_det;
  mpz_init(v_det);
  fmpz_get_mpz(v_det, target_det);
  mat(0, 4) = v_det;
  mpz_clear(v_det);

  // Then we input relations between minors
  int index = 1;
  fmpz_t pair_gcd;
  fmpz_init(pair_gcd);
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < i; j++) {
      fmpz_t mi_entry;
      fmpz_init(mi_entry);
      fmpz_t mj_entry;
      fmpz_init(mj_entry);

      fmpz_gcd(pair_gcd, &minors[i], &minors[j]);
      fmpz_divexact(mi_entry, &minors[j], pair_gcd);
      fmpz_divexact(mj_entry, &minors[i], pair_gcd);
      fmpz_neg(mj_entry, mj_entry);

      mpz_t mi_entry2;
      mpz_init(mi_entry2);
      fmpz_get_mpz(mi_entry2, mi_entry);
      mat(index, i) = mi_entry2;
      mpz_clear(mi_entry2);

      mpz_t mj_entry2;
      mpz_init(mj_entry2);
      fmpz_get_mpz(mj_entry2, mj_entry);
      mat(index, j) = mj_entry2;
      mpz_clear(mj_entry2);

      index++;
    }
  }

  fmpz_clear(g1);
  fmpz_clear(g2);
  for (int l = 0; l < 4; l++) {
    fmpz_clear(gcd_coeff[l]);
  }
  fmpz_clear(pair_gcd);
  fmpz_clear(target_det);

  // zero on success
  int status = lll_reduction(mat, fplll::LLL_DEF_DELTA, fplll::LLL_DEF_ETA,
                             fplll::LM_WRAPPER, fplll::FT_DEFAULT, 0,
                             fplll::LLL_DEFAULT);
  if (status != fplll::RED_SUCCESS) {
    std::cerr << "LLL reduction failed with error '"
              << fplll::get_red_status_str(status) << std::endl;
    return status;
  }

  if (mat(mat.get_rows() - 1, 4) ==
      v_det) { // If last row contains the det as expected

    // Set last vector
    for (int l = 0; l < 4; l++) {
      mpz_t v;
      mpz_init(v);
      mat(mat.get_rows() - 1, l).get_mpz(v);

      fmpz_set_mpz(&last_vector[SQUIRRELS_D - 1 - l], v);
      if (l % 2 == 0) {
        fmpz_neg(&last_vector[SQUIRRELS_D - 1 - l],
                 &last_vector[SQUIRRELS_D - 1 - l]);
      }
      mpz_clear(v);
    }
  }

  return 0;
}