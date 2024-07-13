/*
 * Support functions for discrete and continuous vectors.
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

#include "fpr.h"
#include "inner.h"
#include <stdint.h>

void Zf(cvector_of)(continuous_vector *target, const discrete_vector *source) {
  for (int i = 0; i < SQUIRRELS_D; i++) {
    target->coeffs[i] = fpr_of(source->coeffs[i]);
  }
}

void Zf(cvector_rint)(discrete_vector *target,
                      const continuous_vector *source) {
  for (int i = 0; i < SQUIRRELS_D; i++) {
    target->coeffs[i] = (int32_t)fpr_rint(source->coeffs[i]);
  }
}

void Zf(cvector_mul)(continuous_vector *vec, fpr x) {
  for (int i = 0; i < SQUIRRELS_D; i++) {
    vec->coeffs[i] = fpr_mul(vec->coeffs[i], x);
  }
}

void Zf(cvector_submul)(continuous_vector *target, const discrete_vector *vec,
                        fpr x) {
  for (int i = 0; i < SQUIRRELS_D; i++) {
    target->coeffs[i] =
        fpr_sub(target->coeffs[i], fpr_mul(fpr_of(vec->coeffs[i]), x));
  }
}

void Zf(cvector_submul2)(continuous_vector *target,
                         const continuous_vector *vec, fpr x) {
  for (int i = 0; i < SQUIRRELS_D; i++) {
    target->coeffs[i] = fpr_sub(target->coeffs[i], fpr_mul(vec->coeffs[i], x));
  }
}

fpr Zf(cvector_norm)(const continuous_vector *vec) {
  fpr squarenorm = FPR(0.);

  for (int i = 0; i < SQUIRRELS_D; i++) {
    squarenorm = fpr_add(squarenorm, fpr_sqr(vec->coeffs[i]));
  }

  return fpr_sqrt(squarenorm);
}

fpr Zf(cvector_dot)(const continuous_vector *v1, const continuous_vector *v2) {
  fpr s = FPR(0.);

  for (int i = 0; i < SQUIRRELS_D; i++) {
    s = fpr_add(s, fpr_mul(v1->coeffs[i], v2->coeffs[i]));
  }

  return s;
}

fpr Zf(cvector_idot)(const continuous_vector *v1, const discrete_vector *v2) {
  fpr s = FPR(0.);

  for (int i = 0; i < SQUIRRELS_D; i++) {
    s = fpr_add(s, fpr_mul(v1->coeffs[i], fpr_of(v2->coeffs[i])));
  }

  return s;
}

void Zf(cvector_sub)(continuous_vector *v1, const continuous_vector *v2) {
  for (int i = 0; i < SQUIRRELS_D; i++) {
    v1->coeffs[i] = fpr_sub(v1->coeffs[i], v2->coeffs[i]);
  }
}

void Zf(cvector_add)(continuous_vector *v1, const continuous_vector *v2) {
  for (int i = 0; i < SQUIRRELS_D; i++) {
    v1->coeffs[i] = fpr_add(v1->coeffs[i], v2->coeffs[i]);
  }
}

fpr Zf(vector_norm)(const discrete_vector *vec) {
  uint64_t squarenorm = 0;

  for (int i = 0; i < SQUIRRELS_D; i++) {
    squarenorm += vec->coeffs[i] * vec->coeffs[i];
  }

  return fpr_sqrt(fpr_of(squarenorm));
}

int64_t Zf(vector_dot)(const discrete_vector *v1, const discrete_vector *v2) {
  int64_t s = 0;
  for (int i = 0; i < SQUIRRELS_D; i++) {
    s += v1->coeffs[i] * v2->coeffs[i];
  }

  return s;
}

void Zf(vector_submul)(discrete_vector *target, const discrete_vector *vec,
                       int32_t x) {
  for (int i = 0; i < SQUIRRELS_D; i++) {
    target->coeffs[i] -= vec->coeffs[i] * x;
  }
}

void Zf(vector_sub)(discrete_vector *target, const discrete_vector *vec) {
  for (int i = 0; i < SQUIRRELS_D; i++) {
    target->coeffs[i] -= vec->coeffs[i];
  }
}

void Zf(vector_add)(discrete_vector *target, const discrete_vector *vec) {
  for (int i = 0; i < SQUIRRELS_D; i++) {
    target->coeffs[i] += vec->coeffs[i];
  }
}
