/*
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

#include "api.h"
#include "inner.h"
#include <math.h>

/* Store a double-precision centered normal vector in vec.
 * Computation using Box-Muller. SQUIRRELS_D is assumed to be even.
 * Standard deviation is sqrt(SQUIRRELS_D/2).
 */
void Zf(normaldist)(inner_shake256_context *rng, continuous_vector *vec) {
  uint64_t u[SQUIRRELS_D / 2], v[SQUIRRELS_D / 2], e[SQUIRRELS_D];
  double uf[SQUIRRELS_D / 2], vf[SQUIRRELS_D / 2];
  int geom[SQUIRRELS_D / 2];

  for (int i = 0; i < SQUIRRELS_D / 2; i++) {
    u[i] = get_rng_u64(rng);
    v[i] = get_rng_u64(rng);
    e[i] = get_rng_u64(rng);
    e[i + SQUIRRELS_D / 2] = get_rng_u64(rng);
  }

  for (int i = 0; i < SQUIRRELS_D / 2; i++) {
    uf[i] = 2 * M_PI * (double)(u[i] & 0x1FFFFFFFFFFFFFul) * pow(2, -53);
    vf[i] = 0.5 + (double)(v[i] & 0x1FFFFFFFFFFFFFul) * pow(2, -54);

    geom[i] =
        CMUX(63 + ffsll(e[2 * i + 1]), ffsll(e[2 * i]) - 1, CZERO64(e[2 * i]));

    vf[i] = sqrt(SQUIRRELS_D * (M_LN2 * geom[i] - log(vf[i])));
  }

  for (int i = 0; i < SQUIRRELS_D / 2; i++) {
    vec->coeffs[2 * i].v = vf[i] * cos(uf[i]);
    vec->coeffs[2 * i + 1].v = vf[i] * sin(uf[i]);
  }
}
