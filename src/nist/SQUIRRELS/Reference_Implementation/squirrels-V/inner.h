#ifndef SQUIRRELS_INNER_H__
#define SQUIRRELS_INNER_H__

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/*
 * Modified version of the inner.h file written by Thomas Pornin
 * in the reference implementation of Falcon.
 *
 * ==========================(LICENSE BEGIN)============================
 *
 * Copyright (c) 2023  Squirrels Project
 * Copyright (c) 2017-2019  Falcon Project
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
 */

/*
 * "Naming" macro used to apply a consistent prefix over all global
 * symbols.
 */
#ifndef SQUIRRELS_PREFIX
#define SQUIRRELS_PREFIX squirrels_inner
#endif
#define Zf(name) Zf_(SQUIRRELS_PREFIX, name)
#define Zf_(prefix, name) Zf__(prefix, name)
#define Zf__(prefix, name) prefix##_##name

#include "param.h"
extern const uint32_t Zf(det_primes)[];

/* Constant-time macros */
#define LSBMASK(c) (-((c)&1))
#define CMUX(x, y, c) (((x) & (LSBMASK(c))) ^ ((y) & (~LSBMASK(c))))
#define CZERO64(x) ((~(x) & ((x)-1)) >> 63)

/*
 * Some computations with floating-point elements, in particular
 * rounding to the nearest integer, rely on operations using _exactly_
 * the precision of IEEE-754 binary64 type (i.e. 52 bits). On 32-bit
 * x86, the 387 FPU may be used (depending on the target OS) and, in
 * that case, may use more precision bits (i.e. 64 bits, for an 80-bit
 * total type length); to prevent miscomputations, we define an explicit
 * function that modifies the precision in the FPU control word.
 *
 * set_fpu_cw() sets the precision to the provided value, and returns
 * the previously set precision; callers are supposed to restore the
 * previous precision on exit. The correct (52-bit) precision is
 * configured with the value "2". On unsupported compilers, or on
 * targets other than 32-bit x86, or when the native 'double' type is
 * not used, the set_fpu_cw() function does nothing at all.
 */
static inline unsigned set_fpu_cw(unsigned x) { return x; }

/*
 * MSVC 2015 does not know the C99 keyword 'restrict'.
 */
#if defined _MSC_VER && _MSC_VER
#ifndef restrict
#define restrict __restrict
#endif
#endif

/* ==================================================================== */
/*
 * SHAKE256 implementation (shake.c).
 *
 * API is defined to be easily replaced with the fips202.h API defined
 * as part of PQClean.
 */

typedef struct {
  union {
    uint64_t A[25];
    uint8_t dbuf[200];
  } st;
  uint64_t dptr;
} inner_shake256_context;

#define inner_shake256_init Zf(i_shake256_init)
#define inner_shake256_inject Zf(i_shake256_inject)
#define inner_shake256_flip Zf(i_shake256_flip)
#define inner_shake256_extract Zf(i_shake256_extract)

void Zf(i_shake256_init)(inner_shake256_context *sc);
void Zf(i_shake256_inject)(inner_shake256_context *sc, const uint8_t *in,
                           size_t len);
void Zf(i_shake256_flip)(inner_shake256_context *sc);
void Zf(i_shake256_extract)(inner_shake256_context *sc, uint8_t *out,
                            size_t len);

/*
 * Get a random 8-byte integer from a SHAKE-based RNG. This function
 * ensures consistent interpretation of the SHAKE output so that
 * the same values will be obtained over different platforms, in case
 * a known seed is used.
 */
static inline uint64_t get_rng_u64(inner_shake256_context *rng) {
  /*
   * We enforce little-endian representation.
   */

  uint8_t tmp[8];

  inner_shake256_extract(rng, tmp, sizeof tmp);
  return (uint64_t)tmp[0] | ((uint64_t)tmp[1] << 8) | ((uint64_t)tmp[2] << 16) |
         ((uint64_t)tmp[3] << 24) | ((uint64_t)tmp[4] << 32) |
         ((uint64_t)tmp[5] << 40) | ((uint64_t)tmp[6] << 48) |
         ((uint64_t)tmp[7] << 56);
}

static inline uint64_t get_rng_u8(inner_shake256_context *rng) {
  uint8_t tmp;
  inner_shake256_extract(rng, &tmp, 1);
  return tmp;
}

/* ==================================================================== */
/*
 * Implementation of floating-point real numbers (fpr.h, fpr.c).
 */

/*
 * Real numbers are implemented by an extra header file, included below.
 * This is meant to support pluggable implementations. The default
 * implementation relies on the C type 'double'.
 *
 * The included file must define the following types, functions and
 * constants:
 *
 *   fpr
 *         type for a real number
 *
 *   fpr fpr_of(int64_t i)
 *         cast an integer into a real number; source must be in the
 *         -(2^63-1)..+(2^63-1) range
 *
 *   fpr fpr_scaled(int64_t i, int sc)
 *         compute i*2^sc as a real number; source 'i' must be in the
 *         -(2^63-1)..+(2^63-1) range
 *
 *   fpr fpr_ldexp(fpr x, int e)
 *         compute x*2^e
 *
 *   int64_t fpr_rint(fpr x)
 *         round x to the nearest integer; x must be in the -(2^63-1)
 *         to +(2^63-1) range
 *
 *   int64_t fpr_trunc(fpr x)
 *         round to an integer; this rounds towards zero; value must
 *         be in the -(2^63-1) to +(2^63-1) range
 *
 *   fpr fpr_add(fpr x, fpr y)
 *         compute x + y
 *
 *   fpr fpr_sub(fpr x, fpr y)
 *         compute x - y
 *
 *   fpr fpr_neg(fpr x)
 *         compute -x
 *
 *   fpr fpr_half(fpr x)
 *         compute x/2
 *
 *   fpr fpr_mul(fpr x, fpr y)
 *         compute x * y
 *
 *   fpr fpr_sqr(fpr x)
 *         compute x * x
 *
 *   fpr fpr_inv(fpr x)
 *         compute 1/x
 *
 *   fpr fpr_div(fpr x, fpr y)
 *         compute x/y
 *
 *   fpr fpr_sqrt(fpr x)
 *         compute the square root of x
 *
 *   uint64_t fpr_expm_p63(fpr x)
 *         return exp(x), assuming that 0 <= x < log(2). Returned value
 *         is scaled to 63 bits (i.e. it really returns 2^63*exp(-x),
 *         rounded to the nearest integer). Computation should have a
 *         precision of at least 45 bits.
 *
 *   const fpr fpr_p2_tab[]
 *         precomputed powers of 2 (by index, 0 to 10)
 *
 * Constants of type 'fpr':
 *
 *   fpr fpr_inv_2sqrsigma0    1/(2*(1.8205^2))
 *   fpr fpr_log2              log(2)
 *   fpr fpr_inv_log2          1/log(2)
 *   fpr fpr_ptwo63            2^63
 */
#include "fpr.h"

/**
 * vector.c
 */

typedef struct {
  fpr coeffs[SQUIRRELS_D];
} continuous_vector;

typedef struct {
  int32_t coeffs[SQUIRRELS_D];
} discrete_vector;

typedef discrete_vector basis[SQUIRRELS_D];
typedef continuous_vector basis_gm[SQUIRRELS_D];

typedef struct {
  basis b;
  basis_gm b_gm;
} secret_key;

typedef struct {
  uint32_t
      equation[(SQUIRRELS_D - 1) *
               SQUIRRELS_NBPRIMES]; // Stores the last column of the HNF of the
                                  // basis (except last coordinate which is the
                                  // fixed det), modulo each prime of the det
} public_key;

typedef struct {
  char header;
  char r[SQUIRRELS_K / 8];
  char s[SQUIRRELS_SIG_BYTESIZE];
} signature;

void Zf(cvector_of)(continuous_vector *target, const discrete_vector *source);
/* Note: the caller must make sure that rounded coordinates fit in 32-bit integers. */
void Zf(cvector_rint)(discrete_vector *target, const continuous_vector *source);
// Modifies vec to x*vec
void Zf(cvector_mul)(continuous_vector *vec, fpr x);
void Zf(cvector_submul)(continuous_vector *target, const discrete_vector *vec,
                        fpr x);
void Zf(cvector_submul2)(continuous_vector *target,
                         const continuous_vector *vec, fpr x);
void Zf(cvector_sub)(continuous_vector *v1, const continuous_vector *v2);
void Zf(cvector_add)(continuous_vector *v1, const continuous_vector *v2);
// Returns the norm of vec
fpr Zf(cvector_norm)(const continuous_vector *vec);
fpr Zf(cvector_dot)(const continuous_vector *v1, const continuous_vector *v2);
fpr Zf(cvector_idot)(const continuous_vector *v1, const discrete_vector *v2);

fpr Zf(vector_norm)(const discrete_vector *vec);
int64_t Zf(vector_dot)(const discrete_vector *v1, const discrete_vector *v2);
void Zf(vector_submul)(discrete_vector *target, const discrete_vector *vec,
                       int32_t x);
void Zf(vector_sub)(discrete_vector *target, const discrete_vector *vec);
void Zf(vector_add)(discrete_vector *target, const discrete_vector *vec);

/**
 * common.c
 */

/**
 * This function samples a vector in [0, q-1]^{n-1} x (0) as per the spec. It is
 * constant time.
 */
void Zf(hash_to_point)(inner_shake256_context *sc, discrete_vector *x);

/**
 * keygen.c
 */

int Zf(keygen)(inner_shake256_context *rng, secret_key *sk, public_key *pk);

/**
 * sign.c
 */

int Zf(sign)(inner_shake256_context *rng, const unsigned char *sk,
             discrete_vector *hm);
int Zf(verify_raw)(const unsigned char *pk, const discrete_vector *sig,
                   const discrete_vector *hm);

/**
 * codec.c
 */

int Zf(ui32_encode)(uint8_t *out, const uint32_t *in, size_t len);
inline uint32_t Zf(ui32_decode)(const uint8_t *in) {
  /*
   * We enforce little-endian representation.
   */
  return (uint32_t)in[0] | (uint32_t)(in[1] << 8) | (uint32_t)(in[2] << 16) |
         (uint32_t)(in[3] << 24);
}

int Zf(i32_encode)(uint8_t *out, const int32_t *in, size_t len);
inline int32_t Zf(i32_decode)(const uint8_t *in) {
  /*
   * We enforce little-endian representation.
   */
  int32_t n = ((int32_t)in[0] | (int32_t)(in[1] << 8) | (int32_t)(in[2] << 16) |
               (int32_t)((in[3] & 0x7F) << 24));

  if (in[3] & (1 << 7)) {
    n -= 1u << 31;
  }

  return n;
}

size_t Zf(comp_encode)(void *out, size_t max_out_len, const discrete_vector *x,
                       unsigned rate);
size_t Zf(comp_decode)(discrete_vector *x, const void *in, size_t max_in_len,
                       unsigned rate);

#define coeff_sk_b_gm(sk, i, j)                                                \
  FPR(*(double *)(sk + 4 * SQUIRRELS_D * SQUIRRELS_D + 8 * (SQUIRRELS_D * i + j)))
#define coeff_sk_b(sk, i, j) Zf(i32_decode)(sk + 4 * (SQUIRRELS_D * i + j))
#define coeff_pk(pk, i, pi) Zf(ui32_decode)(pk + 4 * ((SQUIRRELS_D - 1) * pi + i))

/**
 * sampler.c
 */

/*
 * Internal sampler engine. Exported for tests.
 *
 * sampler() takes as parameters:
 *   ctx      pointer to the sampler_context structure
 *   mu       center for the distribution
 *   isigma   inverse of the distribution standard deviation
 * It returns an integer sampled along the Gaussian distribution centered
 * on mu and of standard deviation sigma = 1/isigma.
 */

int Zf(sampler)(inner_shake256_context *rng, fpr mu, fpr isigma, fpr sigma_min);

/**
 * normaldist.c
 */

void Zf(normaldist)(inner_shake256_context *rng, continuous_vector *vec);

/**
 * minors.c
 */
#include "fmpz.h"
#include "fmpz_mat.h"
void Zf(compute_minors)(fmpz *minors, slong nb_minors, const fmpz_mat_t B);

#endif
