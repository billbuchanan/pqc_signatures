/******************************************************************************
 * Helper functions for linear algebra over F_q and F_q^c
 ******************************************************************************
 * Copyright (C) 2023  The VOX team
 * License : MIT (see COPYING file for the full text)
 * Author  : Robin Larrieu
 *****************************************************************************/

#include "vox_arith.h"
#include <stddef.h>
#include <string.h>
#include <immintrin.h>


#if 0
#include <assert.h>
#define VOX_ENABLE_ASSERTIONS 1
#define VOX_ASSERT  assert
#else
#define VOX_ENABLE_ASSERTIONS 0
#define VOX_ASSERT(x)
#endif

/******************************************************************************
 * Local utility functions
 *
 * All linear algebra operations over F_q are of the form D = A + B*C
 * with matrices of size at most VOX_N
 *
 * All linear algebra operations over F_q^c are of the form
 *    C = A + T'*B where T' is a VOX_OC*VOX_VC block
 * or C = A + S'*B where S' is a VOX_T*(VOX_O-VOX_T) block over F_q
 * or C = A*V      where A is of size VOX_NC*VOX_NC
 *
 * We use these assumption to establish bounds on the intermediate results and
 * ensure there is no overflow on an uint32_t.
 * This lets us delay modular reduction (both by the modulus q and by the
 * defining polynomial of F_q^c) to improve performance.
 * Assertions may be enabled for debugging using the macro above; otherwise
 * they serve as documentation to help the reader
 *****************************************************************************/

/*
 * Arithmetic in modulo Q
 */

static const uint32_t VOX_Q2 = VOX_Q * ((uint32_t) VOX_Q);

static inline uint32_t long_mul(const Fq a, const Fq b) {
  return ((uint32_t) a) * ((uint32_t) b);
}

static inline Fq Fq_add(Fq a, Fq b) {
  /* Force lift in uint16_t as the addition would overflow if Q=251 and Fq = uint8_t */
  uint16_t tmp = (uint16_t) (a + ((uint16_t) b) - VOX_Q);
  uint16_t mask = - (tmp >> 15);
  return (Fq) (tmp + (VOX_Q & mask));
}

static inline Fq Fq_neg(Fq a) {
  uint16_t tmp = (uint16_t) (- ((uint16_t) a));
  uint16_t mask = - (tmp >> 15); /* 0 if a == 0, 0xFFFF otherwise */
  return (Fq) (tmp + (VOX_Q & mask));
}

static inline uint32_t Fq_addmul(uint32_t acc, const Fq a, const Fq b) {
  acc += long_mul(a,b);
  VOX_ASSERT(acc < VOX_Q + VOX_N*VOX_Q2);
  return acc;
}

/*
 * Barrett reduction of any 32-bit integer modulo Q
 */
static Fq Fq_reduce(uint32_t x) {
  /* Barrett inverse 2^32 / Q */
#if   (VOX_Q == 251)
#define VOX_QINV  UINT32_C(17111423)
#elif (VOX_Q == 1021)
#define VOX_QINV  UINT32_C(4206628)
#elif (VOX_Q == 4093)
#define VOX_QINV  UINT32_C(1049344)
#else
#error "Unsupported parameters"
#endif
  /* Compiler should be able to optimize this as a high multiplication
   * on 32-bit platforms */
  uint32_t m = (uint32_t) (((uint64_t) x * (uint64_t) VOX_QINV) >> 32);
  m = x - VOX_Q * m;
  /* Result is currently in [0, 2*Q) -> perform conditional subtraction */
  m -= VOX_Q;
  uint32_t mask = - (m >> 31);
  m += VOX_Q & mask;
  VOX_ASSERT(x % VOX_Q == m);
  return (Fq) m;
}

#define W1  ( (int) (sizeof(__m256i)  / sizeof(Fq)) )
#define W2  ( (int) (sizeof(uint32_t) / sizeof(Fq)) )

static void FqXW_loadW(__m256i out[W2], const Fq in[W1]) {
  __m256i a = _mm256_loadu_si256((const __m256i_u*) in);
  __m256i m = _mm256_set1_epi32((1U << VOX_Q_BITS)-1);
  for (int i=0; i<W2; i++) {
    out[i] = _mm256_and_si256(a, m);
    a = _mm256_srli_epi32(a, 8*sizeof(Fq));
  }
}

static void FqXW_store(Fq out[W1], const __m256i in[W2]) {
  __m256i a = in[0];
  for (int i=1; i<W2; i++) {
    a = _mm256_or_si256(a, _mm256_slli_epi32(in[i], 8*i*sizeof(Fq)));
  }
  _mm256_storeu_si256((__m256i_u *) out, a);
}

static void FqXW_scalar_addmul(__m256i acc[W2], Fq a, const Fq in[W1]) {
  __m256i axW = _mm256_set1_epi32(a);
  __m256i tmp = _mm256_loadu_si256((const __m256i_u*) in);
  __m256i m   = _mm256_set1_epi32((1U << VOX_Q_BITS)-1);
  for (int i=0; i<W2; i++) {
    __m256i u = _mm256_and_si256(tmp, m);
    acc[i]    = _mm256_add_epi32(acc[i], _mm256_mullo_epi32(axW, u));
    tmp       =  _mm256_srli_epi32(tmp, 8*sizeof(Fq));
  }
}

static __m256i FqX8_reduce(__m256i x) {
  __m256i qinv = _mm256_set1_epi64x(VOX_QINV);
  __m256i q  = _mm256_set1_epi32(VOX_Q);
  __m256i x2 = _mm256_srli_epi64(x, 32);
  __m256i m1 = _mm256_mul_epu32(x, qinv);
  __m256i m2 = _mm256_mul_epu32(x2, qinv);
  m1 = _mm256_blend_epi32(_mm256_srli_epi64(m1, 32), m2, 0xAA);
  m1 = _mm256_sub_epi32(x, _mm256_mullo_epi32(m1, q));
  m1 = _mm256_sub_epi32(m1, q);
  m2 = _mm256_and_si256(q, _mm256_srai_epi32(m1, 31));
  return _mm256_add_epi32(m1, m2);
}


static __m256i FqX8_reduce_neg(__m256i x) {
  __m256i qinv = _mm256_set1_epi64x(VOX_QINV);
  __m256i q  = _mm256_set1_epi32(VOX_Q);
  __m256i x2 = _mm256_srli_epi64(x, 32);
  __m256i m1 = _mm256_mul_epu32(x, qinv);
  __m256i m2 = _mm256_mul_epu32(x2, qinv);
  m1 = _mm256_blend_epi32(_mm256_srli_epi64(m1, 32), m2, 0xAA);
  m1 = _mm256_sub_epi32(x, _mm256_mullo_epi32(m1, q));
  m1 = _mm256_sub_epi32(q, m1);
  m2 = _mm256_and_si256(q, _mm256_srai_epi32(m1, 31));
  return _mm256_add_epi32(m1, m2);
}

/*
 * Arithmetic in F_q^c
 */

static void Fqc_accInit(uint32_t acc[VOX_C], const Fqc *a) {
  for (int i=0; i<VOX_C; i++)
    acc[i] = a->coeffs[i];
}

static void Fqc_addmul(uint32_t acc[2*VOX_C-1], const Fqc *a, const Fqc *b)
{
  int i,j;
  for (i=0; i<VOX_C; i++) {
    for (j=0; j<VOX_C; j++) {
      acc[i+j] += long_mul(a->coeffs[i], b->coeffs[j]);
    }
  }
#if VOX_ENABLE_ASSERTIONS
  for (i=0; i<2*VOX_C-1; i++)
    VOX_ASSERT(acc[i] < VOX_Q + VOX_C*VOX_NC*VOX_Q2);
#endif
}


/*
 * Reduce the accumulator modulo q, def_poly
 * Input is destroyed during the computation
 */
static void Fqc_reduce(Fqc *dst, uint32_t acc[2*VOX_C-1])
{
  int i;
#if   (VOX_Q == 251) && (VOX_C == 6) /* reduce modulo X^6 + x + 1 */
  static const uint32_t bound = VOX_Q + VOX_C*VOX_NC*VOX_Q2;
  for (i=0; i<VOX_C-1; i++) {
    VOX_ASSERT(acc[i] < 2*bound);
    VOX_ASSERT(acc[i+1] < bound);
    VOX_ASSERT(acc[i+VOX_C] < bound);
    acc[i]   += bound - acc[i+VOX_C];
    acc[i+1] += bound - acc[i+VOX_C];
  }
  /* now acc[0:C] = input mod x^6 + x + 1 with acc[i] < 3*bound < 2^32 */
#elif (VOX_Q == 1021) && (VOX_C == 7) /* reduce modulo X^7 + x + 5 */
  static const uint32_t bound = VOX_Q + VOX_C*VOX_NC*VOX_Q2;
  for (i=0; i<VOX_C-1; i++) {
    VOX_ASSERT(acc[i] < 2*bound);
    VOX_ASSERT(acc[i+1] < bound);
    VOX_ASSERT(acc[i+VOX_C] < bound);
    acc[i]   += 5*(bound - acc[i+VOX_C]);
    acc[i+1] += bound - acc[i+VOX_C];
  }
  /* now acc[0:C] = input mod x^7 + x + 5 with acc[i] < 7*bound < 2^32 */
#elif (VOX_Q == 4093) && (VOX_C == 8) /* reduce modulo X^8 + 2 */
  static const uint32_t bound1 = VOX_Q + VOX_C*VOX_NC*VOX_Q2;
  static const uint32_t bound2 = 4096 + 3*(bound1 >> 12);
  static const uint32_t bound3 = VOX_Q * ((bound2 + VOX_Q - 1)/VOX_Q);
  uint32_t tmp;
  for (i=0; i<VOX_C-1; i++) {
    VOX_ASSERT(acc[i] < bound1);
    VOX_ASSERT(acc[i+VOX_C] < bound1);
    /* 3*bound1 > 2^32, so we must do an intermediate reduction to avoid overflow. */
    tmp = (acc[i+VOX_C] & ((1U << 12) - 1)) + 3*(acc[i+VOX_C] >> 12);
    VOX_ASSERT(tmp < bound2);
    VOX_ASSERT(tmp % VOX_Q == acc[i+VOX_C] % VOX_Q);
    acc[i] += 2*bound3 - 2*tmp;
  }
  VOX_ASSERT(acc[VOX_C-1] < bound1);
  /* now acc[0:C] = input mod x^8 + 2 with acc[i] < bound1 + 2*bound3 < 2^32 */
#else
#error "Unsupported parameters"
#endif
  for (i=0; i<VOX_C; i++)
    dst->coeffs[i] = Fq_reduce(acc[i]);
}


static void FqcX8_load1(__m256i out[VOX_C], const Fqc *in) {
  for (int i=0; i<VOX_C; i++) {
    out[i] = _mm256_set1_epi32(in->coeffs[i]);
  }
}

static void FqcX8_store(Fqc out[8], const __m256i in[VOX_C]) {
  const uint32_t *inc = (const uint32_t *) in;
  for (int i=0; i<8; i++) {
    for (int j=0; j<VOX_C; j++) {
      out[i].coeffs[j] = (Fq) inc[j*8+i];
    }
  }
}


/* Load 8 Fqc elements into SIMD registers, with offsets given by idx */
static void FqcX8_load8_stride(__m256i out[VOX_C], const Fqc *in, __m256i idx) {
  __m256i m   = _mm256_set1_epi32((1U << VOX_Q_BITS)-1);
#if   (VOX_Q == 251) && (VOX_C == 6) /* 6 limbs of 8 bits each */
  __m256i a = _mm256_i32gather_epi32((const int *) &in[0].coeffs[0], idx, 1);
  __m256i b = _mm256_i32gather_epi32((const int *) &in[0].coeffs[2], idx, 1);
  out[0] = _mm256_and_si256(m, a);
  out[1] = _mm256_and_si256(m, _mm256_srli_epi32(a,  8));
  out[2] = _mm256_and_si256(m, _mm256_srli_epi32(a, 16));
  out[3] = _mm256_and_si256(m, _mm256_srli_epi32(a, 24));
  out[4] = _mm256_and_si256(m, _mm256_srli_epi32(b, 16));
  out[5] = _mm256_and_si256(m, _mm256_srli_epi32(b, 24));
#elif (VOX_Q == 1021) && (VOX_C == 7) /* 7 limbs of 16 bits each */
  __m256i a = _mm256_i32gather_epi32((const int *) &in[0].coeffs[0], idx, 1);
  __m256i b = _mm256_i32gather_epi32((const int *) &in[0].coeffs[2], idx, 1);
  __m256i c = _mm256_i32gather_epi32((const int *) &in[0].coeffs[4], idx, 1);
  __m256i d = _mm256_i32gather_epi32((const int *) &in[0].coeffs[5], idx, 1);
  out[0] = _mm256_and_si256(m, a);
  out[1] = _mm256_and_si256(m, _mm256_srli_epi32(a, 16));
  out[2] = _mm256_and_si256(m, b);
  out[3] = _mm256_and_si256(m, _mm256_srli_epi32(b, 16));
  out[4] = _mm256_and_si256(m, c);
  out[5] = _mm256_and_si256(m, _mm256_srli_epi32(c, 16));
  out[6] = _mm256_and_si256(m, _mm256_srli_epi32(d, 16));
#elif (VOX_Q == 4093) && (VOX_C == 8) /* 8 limbs of 16 bits each */
  __m256i a = _mm256_i32gather_epi32((const int *) &in[0].coeffs[0], idx, 1);
  __m256i b = _mm256_i32gather_epi32((const int *) &in[0].coeffs[2], idx, 1);
  __m256i c = _mm256_i32gather_epi32((const int *) &in[0].coeffs[4], idx, 1);
  __m256i d = _mm256_i32gather_epi32((const int *) &in[0].coeffs[6], idx, 1);
  out[0] = _mm256_and_si256(m, a);
  out[1] = _mm256_and_si256(m, _mm256_srli_epi32(a, 16));
  out[2] = _mm256_and_si256(m, b);
  out[3] = _mm256_and_si256(m, _mm256_srli_epi32(b, 16));
  out[4] = _mm256_and_si256(m, c);
  out[5] = _mm256_and_si256(m, _mm256_srli_epi32(c, 16));
  out[6] = _mm256_and_si256(m, d);
  out[7] = _mm256_and_si256(m, _mm256_srli_epi32(d, 16));
#else
#error "Invalid parameters"
#endif
}

/* Load 8 consecutive elements */
static void FqcX8_load8(__m256i out[VOX_C], const Fqc in[8]) {
  size_t stride = sizeof(Fqc);
  __m256i idx = _mm256_setr_epi32(0*stride, 1*stride, 2*stride, 3*stride,
                                  4*stride, 5*stride, 6*stride, 7*stride);
  FqcX8_load8_stride(out, in, idx);
}

/* Load 8 elements in the same column of T */
static void FqcX8_load8_colT(__m256i out[VOX_C], const Fqc *in) {
  size_t stride = VOX_VC*sizeof(Fqc);
  __m256i idx = _mm256_setr_epi32(0*stride, 1*stride, 2*stride, 3*stride,
                                  4*stride, 5*stride, 6*stride, 7*stride);
  FqcX8_load8_stride(out, in, idx);
}

static void FqcX8_addmul(__m256i acc[2*VOX_C-1], __m256i a[VOX_C], __m256i b[VOX_C]) {
  for (int i=0; i<VOX_C; i++) {
    for (int j=0; j<VOX_C; j++) {
      acc[i+j] = _mm256_add_epi32(acc[i+j], _mm256_mullo_epi32(a[i], b[j]));
    }
  }
}

static void FqcX8_reduce(__m256i acc[2*VOX_C-1])
{
  int i;
#if   (VOX_Q == 251) && (VOX_C == 6) /* reduce modulo X^6 + x + 1 */
  __m256i bound = _mm256_set1_epi32(VOX_Q + VOX_C*VOX_NC*VOX_Q2);
  for (i=0; i<VOX_C-1; i++) {
    __m256i m = _mm256_sub_epi32(bound, acc[i+VOX_C]);
    acc[i]   = _mm256_add_epi32(acc[i], m);
    acc[i+1] = _mm256_add_epi32(acc[i+1], m);
  }
  /* now acc[0:C] = input mod x^6 + x + 1 with acc[i] < 3*bound < 2^32 */
#elif (VOX_Q == 1021) && (VOX_C == 7) /* reduce modulo X^7 + x + 5 */
  __m256i bound = _mm256_set1_epi32(VOX_Q + VOX_C*VOX_NC*VOX_Q2);
  __m256i five  = _mm256_set1_epi32(5);
  for (i=0; i<VOX_C-1; i++) {
    __m256i m = _mm256_sub_epi32(bound, acc[i+VOX_C]);
    acc[i]   = _mm256_add_epi32(acc[i], _mm256_mullo_epi32(five, m));
    acc[i+1] = _mm256_add_epi32(acc[i+1], m);
  }
  /* now acc[0:C] = input mod x^7 + x + 5 with acc[i] < 7*bound < 2^32 */
#elif (VOX_Q == 4093) && (VOX_C == 8) /* reduce modulo X^8 + 2 */
  static const uint32_t bound1 = VOX_Q + VOX_C*VOX_NC*VOX_Q2;
  static const uint32_t bound2 = 4096 + 3*(bound1 >> 12);
  static const uint32_t bound3 = VOX_Q * ((bound2 + VOX_Q - 1)/VOX_Q);
  __m256i bound = _mm256_set1_epi32(bound3);
  __m256i mask  = _mm256_set1_epi32((1U << 12) - 1);
  __m256i three = _mm256_set1_epi32(3);
  for (i=0; i<VOX_C-1; i++) {
    /* 3*bound1 > 2^32, so we must do an intermediate reduction to avoid overflow. */
    /* tmp = (acc[i+VOX_C] & ((1U << 12) - 1)) + 3*(acc[i+VOX_C] >> 12); */
    __m256i tmp1 = _mm256_and_si256(acc[i+VOX_C], mask);
    __m256i tmp2 = _mm256_mullo_epi32(three, _mm256_srli_epi32(acc[i+VOX_C], 12));
    tmp1 = _mm256_sub_epi32(bound, _mm256_add_epi32(tmp1, tmp2));
    acc[i] = _mm256_add_epi32(acc[i], _mm256_slli_epi32(tmp1,1));
  }
  /* now acc[0:C] = input mod x^8 + 2 with acc[i] < bound1 + 2*bound3 < 2^32 */
#else
#error "Unsupported parameters"
#endif
  for (i=0; i<VOX_C; i++)
    acc[i] = FqX8_reduce(acc[i]);
}

/******************************************************************************
 * Auxiliary functions for key generation
 *****************************************************************************/

/*
 * Fold a (n x n) matrix into a lower triangular one
 */
static void Fqc_mat_ToLowTri(Fqc mat[], size_t n) {
  size_t i, j, k;
  Fqc *Aij, *Aji;
  for (i=1; i<n; i++) {
    for (j=0; j<i; j++) {
      Aij = &mat[i*n+j];
      Aji = &mat[j*n+i];
      for (k=0; k<VOX_C; k++) {
        Aij->coeffs[k] = Fq_add(Aij->coeffs[k], Aji->coeffs[k]);
        Aji->coeffs[k] = 0;
      }
    }
  }
}


/*
 * Compute the constrained parts of the VOX public key.
 */
void VOX_CompletePetzoldPubkey(Fqc **Pub, Fqc **Aux,
                               const Fq  S[VOX_T*(VOX_O-VOX_T)],
                               const Fqc T[VOX_OC*VOX_VC])
{
  int i, j, k, l;
  __m256i acc_x8[2*VOX_VC-1], a[VOX_C], b[VOX_C];
  for (i=0; i<VOX_O; i++) {
    /* Write Pub[i] = [ P0  0  ]
     *                [ P1  P2 ]
     * (notice that for i >= VOX_T, P0 == 0) */
    /* Pointers to blocks P0, P1, P2 as multi-dimensional arrays for readability */
    Fqc (*P0)[VOX_NC] = (Fqc (*)[VOX_NC]) &Pub[i][0];
    Fqc (*P1)[VOX_NC] = (Fqc (*)[VOX_NC]) &Pub[i][VOX_OC*VOX_NC];
    Fqc (*P2)[VOX_NC] = (Fqc (*)[VOX_NC]) &Pub[i][VOX_OC*VOX_NC + VOX_OC];
    /* Aux[i] = P0 + T*P1 */
    for (j=0; j<VOX_OC; j++) {
      for (k=0; k+7<VOX_OC; k+=8) {
        memset(acc_x8, 0, sizeof(acc_x8));
        if ((i < VOX_T) && (k <= j))
          FqcX8_load8(acc_x8, &P0[j][k]);
        for (l=0; l<VOX_VC; l++) {
          FqcX8_load1(a, &T[j*VOX_VC + l]);
          FqcX8_load8(b, &P1[l][k]);
          FqcX8_addmul(acc_x8, a, b);
        }
        FqcX8_reduce(acc_x8);
        FqcX8_store(&Aux[i][j*VOX_OC + k], acc_x8);
      }
      for (; k<VOX_OC; k++) {
        uint32_t acc[2*VOX_C-1] = {0};
        if ((i < VOX_T) && (k <= j))
          Fqc_accInit(acc, &P0[j][k]);
        for (l=0; l<VOX_VC; l++)
          Fqc_addmul(acc, &T[j*VOX_VC + l], &P1[l][k]);
        Fqc_reduce(&Aux[i][j*VOX_OC + k], acc);
      }
    }
    /* Aux[i] += T * P2 * tT */
    for (j=0; j<VOX_OC; j++) {
      /* row_j(T) * P2 */
      Fqc tmp[VOX_VC];
      for (k=0; k+7<VOX_VC; k+=8) {
        memset(acc_x8, 0, sizeof(acc_x8));
        for (l=k; l<VOX_VC; l++) { /* P2 is lower triangular */
          FqcX8_load1(a, &T[j*VOX_VC + l]);
          FqcX8_load8(b, &P2[l][k]);
          FqcX8_addmul(acc_x8, a, b);
        }
        FqcX8_reduce(acc_x8);
        FqcX8_store(&tmp[k], acc_x8);
      }
      for (; k<VOX_VC; k++) {
        uint32_t acc[2*VOX_C-1] = {0};
        for (l=k; l<VOX_VC; l++) /* P2 is lower triangular */
          Fqc_addmul(acc, &T[j*VOX_VC + l], &P2[l][k]);
        Fqc_reduce(&tmp[k], acc);
      }
      /* row_j(Aux[i]) += row_j(T) * P2 * tT */
      for (k=0; k+7<VOX_OC; k+=8) {
        memset(acc_x8, 0, sizeof(acc_x8));
        FqcX8_load8(acc_x8, &Aux[i][j*VOX_OC+k]);
        for (l=0; l<VOX_VC; l++) {
          FqcX8_load1(a, &tmp[l]);
          FqcX8_load8_colT(b, &T[k*VOX_VC + l]);
          FqcX8_addmul(acc_x8, a, b);
        }
        FqcX8_reduce(acc_x8);
        FqcX8_store(&Aux[i][j*VOX_OC+k], acc_x8);
      }
      for (; k<VOX_OC; k++) {
        uint32_t acc[2*VOX_C-1] = {0};
        Fqc_accInit(acc, &Aux[i][j*VOX_OC+k]);
        for (l=0; l<VOX_VC; l++)
          Fqc_addmul(acc, &tmp[l], &T[k*VOX_VC + l]);
        Fqc_reduce(&Aux[i][j*VOX_OC+k], acc);
      }
    }
    /* Fold the result as a lower triangluar matrix */
    Fqc_mat_ToLowTri(Aux[i], VOX_OC);
  }
  /*
   * Mix equations using matrix S to generate the missing parts of Pub
   * (cast S as a multi-dimensional array for readability)
   * Also for vectorization it is more convenient to view Aux
   * as a VOX_OC x VOX_O matrix over Fq, and similarly for Pub.
   */
  const Fq (*mS)[VOX_O-VOX_T] = (const Fq (*)[VOX_O-VOX_T]) S;
  const Fq **Aux2 = (const Fq **) Aux;
  Fq **Pub2 = (Fq **) Pub;
  for (i=VOX_T; i<VOX_O; i++) {
    /* Row j has (j+1)*VOX_C nonzero coefficients.
     * First handle the rows that can be split into chunks of length W1
     * without overlap, that is W1*ceil((j+1)*VOX_C/W1) <= VOX_O
     * or equivalently j <= floor(W1*floor(VOX_O/W1)/VOX_C) - 1 */
    for (j = 0; j < (W1*(VOX_O/W1))/VOX_C; j++) {
      for (k = 0; k < (j+1)*VOX_C; k+=W1) {
        __m256i acc[W2];
        FqXW_loadW(acc, &Aux2[i][j*VOX_O+k]);
        for (l=0; l<VOX_T; l++) {
          FqXW_scalar_addmul(acc, mS[l][i-VOX_T], &Aux2[l][j*VOX_O+k]);
        }
        for (l=0; l<W2; l++)
          acc[l] = FqX8_reduce_neg(acc[l]);
        FqXW_store(&Pub2[i][j*VOX_N+k], acc);
      }
    }
    /* Handle the remaining rows, using a mix of vector and scalar instructions */
    for (; j < VOX_OC; j++) {
      for (k = 0; k <= (j+1)*VOX_C-W1; k+=W1) {
        __m256i acc[W2];
        FqXW_loadW(acc, &Aux2[i][j*VOX_O+k]);
        for (l=0; l<VOX_T; l++) {
          FqXW_scalar_addmul(acc, mS[l][i-VOX_T], &Aux2[l][j*VOX_O+k]);
        }
        for (l=0; l<W2; l++)
          acc[l] = FqX8_reduce_neg(acc[l]);
        FqXW_store(&Pub2[i][j*VOX_N+k], acc);
      }
      for (; k < (j+1)*VOX_C; k++) {
        uint32_t acc = Aux2[i][j*VOX_O+k];
        for (l=0; l<VOX_T; l++) {
          acc = Fq_addmul(acc, mS[l][i-VOX_T], Aux2[l][j*VOX_O+k]);
        }
        Pub2[i][j*VOX_N+k] = Fq_neg(Fq_reduce(acc));
      }
    }
  }
}

/*
 * From the public key and the private transforms S,T, compute the private key
 */
void VOX_ComposeST(Fqc **Pub, Fqc **Aux,
                   const Fq  S[VOX_T*(VOX_O-VOX_T)],
                   const Fqc T[VOX_OC*VOX_VC])
{
  int i, j, k, l;
  __m256i acc_x8[2*VOX_VC-1], a[VOX_C], b[VOX_C];
  /*
   * For i < VOX_T, the oil-oil block is in Aux[i] so just copy it.
   * For i >= VOX_T, the oil-oil block is zero so we skip it
   */
  for (i=0; i<VOX_T; i++) {
    for (j=0; j<VOX_OC; j++) {
      memcpy(&Pub[i][j*VOX_NC], &Aux[i][j*VOX_OC], (size_t) (j+1)*sizeof(Fqc));
    }
  }
  /*
   * The vinegar block is given by the following transform:
   *   ( P1  P2 ) -> ( P1+(P2+tP2)*tT  P2 )
   * So we need to compute P1 += (P2+tP2) * tT
   */
  for (i=0; i<VOX_O; i++) {
    /* Pointers to blocks P1, P2 as multi-dimensional arrays for readability */
    Fqc (*P1)[VOX_NC] = (Fqc (*)[VOX_NC]) &Pub[i][VOX_OC*VOX_NC];
    Fqc (*P2)[VOX_NC] = (Fqc (*)[VOX_NC]) &Pub[i][VOX_OC*VOX_NC + VOX_OC];
    for (j=0; j<VOX_VC; j++) {
      /* j-th row of P2 + tP2.
       * Since P2 is lower triangular, we do not really need to compute P2 + tP2.
       * We just need to double the diagonal coefficient and reduce it to prevent overflow */
      Fqc tmp;
      for (k=0; k<VOX_C; k++)
        tmp.coeffs[k] = Fq_add(P2[j][j].coeffs[k], P2[j][j].coeffs[k]);
      for (k=0; k+7<VOX_OC; k+=8) {
        memset(acc_x8, 0, sizeof(acc_x8));
        FqcX8_load8(acc_x8, &P1[j][k]);
        for (l=0; l<j; l++) { /* lower half -> P2 */
          FqcX8_load1(a, &P2[j][l]);
          FqcX8_load8_colT(b, &T[k*VOX_VC+l]);
          FqcX8_addmul(acc_x8, a, b);
        }
        FqcX8_load1(a, &tmp);
        FqcX8_load8_colT(b, &T[k*VOX_VC+j]);
        FqcX8_addmul(acc_x8, a, b);
        for (l=j+1; l<VOX_VC; l++) { /* upper half -> tP2 */
          FqcX8_load1(a, &P2[l][j]);
          FqcX8_load8_colT(b, &T[k*VOX_VC+l]);
          FqcX8_addmul(acc_x8, a, b);
        }
        FqcX8_reduce(acc_x8);
        FqcX8_store(&P1[j][k], acc_x8);
      }
      for (; k<VOX_OC; k++) {
        uint32_t acc[2*VOX_C-1] = {0};
        Fqc_accInit(acc, &P1[j][k]);
        for (l=0; l<j; l++) /* lower half -> P2 */
          Fqc_addmul(acc, &P2[j][l], &T[k*VOX_VC+l]);
        Fqc_addmul(acc, &tmp, &T[k*VOX_VC+j]); /* Diagonal coefficient */
        for (l=j+1; l<VOX_VC; l++) /* upper half -> tP2 */
          Fqc_addmul(acc, &P2[l][j], &T[k*VOX_VC+l]);
        Fqc_reduce(&P1[j][k], acc);
      }
    }
  }
  /*
   * Update the vinegar blocks by mixing equations using S
   * (cast S as a multi-dimensional array for readability)
   * Also for vectorization it is more convenient to view Pub
   * as a VOX_NC x VOX_N matrix.
   */
  const Fq (*mS)[VOX_O-VOX_T] = (const Fq (*)[VOX_O-VOX_T]) S;
  Fq **Pub2 = (Fq **) Pub;
  for (i=VOX_T; i<VOX_O; i++) {
    /* Row j has (j+1)*VOX_C nonzero coefficients.
     * First handle the rows that can be split into chunks of length W1
     * without overlap, that is W1*ceil((j+1)*VOX_C/W1) <= VOX_N
     * or equivalently j <= floor(W1*floor(VOX_N/W1)/VOX_C) - 1 */
    for (j = VOX_OC; j < (W1*(VOX_N/W1))/VOX_C; j++) {
      for (k = 0; k < (j+1)*VOX_C; k+=W1) {
        __m256i acc[W2];
        FqXW_loadW(acc, &Pub2[i][j*VOX_N+k]);
        for (l=0; l<VOX_T; l++) {
          FqXW_scalar_addmul(acc, mS[l][i-VOX_T], &Pub2[l][j*VOX_N+k]);
        }
        for (l=0; l<W2; l++)
          acc[l] = FqX8_reduce(acc[l]);
        FqXW_store(&Pub2[i][j*VOX_N+k], acc);
      }
    }
    /* Handle the remaining rows, using a mix of vector and scalar instructions.
     * Unlike with VOX_CompletePetzoldPubkey, the transformation is inplace with
     * matrices all of the same size, so read/write may safely overlap a row
     * boundary. */
    for (k = j*VOX_N; k <= VOX_NC*VOX_N-W1; k+=W1) {
      __m256i acc[W2];
      FqXW_loadW(acc, &Pub2[i][k]);
      for (l=0; l<VOX_T; l++) {
        FqXW_scalar_addmul(acc, mS[l][i-VOX_T], &Pub2[l][k]);
      }
      for (l=0; l<W2; l++)
        acc[l] = FqX8_reduce(acc[l]);
      FqXW_store(&Pub2[i][k], acc);
    }
    for (; k < VOX_NC*VOX_N; k++) {
      uint32_t acc = Pub2[i][k];
      for (l=0; l<VOX_T; l++) {
        acc = Fq_addmul(acc, mS[l][i-VOX_T], Pub2[l][k]);
      }
      Pub2[i][k] = Fq_reduce(acc);
    }
  }
}

/******************************************************************************
 * Auxiliary functions for signature
 *****************************************************************************/


/* Precomputed table of [ Tr(X^i mod def_poly) for i < 3*C ] */
static const Fq Trace_Xi[3*VOX_C] = {
#if (VOX_Q == 251) && (VOX_C == 6)
  6, 0, 0, 0, 0, 246, 245, 0, 0, 0, 5, 11, 6, 0, 0, 246, 235, 234
#elif (VOX_Q == 1021) && (VOX_C == 7)
  7, 0, 0, 0, 0, 0, 1015, 986, 0, 0, 0, 0, 6, 65, 175, 0, 0, 0, 1015, 926, 521
#elif (VOX_Q == 4093) && (VOX_C == 8)
  8, 0, 0, 0, 0, 0, 0, 0, 4077, 0, 0, 0, 0, 0, 0, 0, 32, 0, 0, 0, 0, 0, 0, 0
#endif
};

/* Precompute Tr(a*X^i mod def_poly) for i < 2*VOX_C-1 */
static void Fqc_to_block_helper(Fq tab[2*VOX_C-1], const Fqc *a) {
  int i,j;
  uint32_t acc;
  /* TODO: use that Trace_Xi is sparse to save some operations ? */
  for (i=0; i<2*VOX_C-1; i++) {
    acc = 0;
    for (j=0; j<VOX_C; j++) /* C * Q^2 < 2^32 so there is no overflow */
      acc += long_mul(Trace_Xi[i+j], a->coeffs[j]);
    tab[i] = Fq_reduce(acc);
  }
}

/*
 * Expand an element of F_q^c into a c*c block over Fq such that
 * for any X1,X2, we have X1 * A * tX2 = Tr(a*x1*x2)
 */
void Fqc_to_block(Fq dst[], const Fqc *a, size_t ncols) {
  Fq tab[2*VOX_C-1];
  int i,j;
  Fqc_to_block_helper(tab, a);
  for (i=0; i<VOX_C; i++) {
    for (j=0; j<VOX_C; j++) {
      dst[i*ncols+j] = tab[i+j];
    }
  }
}

/*
 * Same as above but fold the result as a lower triangular block.
 * For any X, we have X * A * tX = Tr(a*x*x)
 */
void Fqc_to_LowTriBlock(Fq dst[], const Fqc *a, size_t ncols) {
  Fq tab[2*VOX_C-1];
  int i,j;
  Fqc_to_block_helper(tab, a);
  for (i=0; i<VOX_C; i++) {
    for (j=0; j<i; j++) {
      dst[i*ncols+j] = Fq_add(tab[i+j], tab[i+j]);
      dst[j*ncols+i] = 0;
    }
    dst[i*ncols+i] = tab[2*i];
  }
}

/*
 * Mix coefficients in the target vector using the matrix S to match
 * the VOX transformation between purely random equations and UOV equations
 */
void Fq_InjectS(Fq target[VOX_O], const Fq S[VOX_T*(VOX_O-VOX_T)]) {
  uint32_t acc;
  int i, j;

  for (i=VOX_T; i<VOX_O; i++) {
    acc = target[i];
    for (j=0; j<VOX_T; j++)
      acc = Fq_addmul(acc, target[j], S[j*(VOX_O-VOX_T) + i - VOX_T]);
    target[i] = Fq_reduce(acc);
  }
}

/*
 * Mix coefficients in the signature vector to hide oil and vinegar variables,
 * as usual with UOV
 */
void Fq_InjectT(Fq signature[VOX_N], const Fqc T[VOX_OC*VOX_VC]) {
  /* A vector of size n over F_q and a vector of size n/c over F_q^c have the same representation */
  Fqc *sig2 = (Fqc *) signature;
  int i,j;

  for (i=VOX_OC; i<VOX_NC; i++) {
    uint32_t acc[2*VOX_C-1] = {0};
    Fqc_accInit(acc, &sig2[i]);
    for (j=0; j<VOX_OC; j++)
      Fqc_addmul(acc, &sig2[j], &T[j*VOX_VC + i - VOX_OC]);
    Fqc_reduce(&sig2[i], acc);
  }
}

/*
 * Inject given vinegar variables into the quadratic equation to obtain an affine
 * relation that must be verified by the oil variables.
 */
void Fq_GetLinEqn(uint32_t dst[VOX_O+1], const Fq mat[VOX_V*VOX_N], const Fq V[VOX_V], const Fq target) {
  uint32_t acc1, acc2;
  int i, j;

  /* V * Mat_OxV */
  for (i=0; i<VOX_O; i++) {
    acc1 = 0;
    for (j=0; j<VOX_V; j++)
      acc1 = Fq_addmul(acc1, V[j], mat[j*VOX_N + i]);
    dst[i] = Fq_reduce(acc1);
  }

  /* V * Mat_VxV * tV - target */
  acc1 = (uint32_t) (VOX_Q - target);
  for (i=0; i<VOX_V; i++) {
    acc2 = 0;
    for (j=0; j<VOX_V; j++)
      acc2 = Fq_addmul(acc2, V[j], mat[j*VOX_N + VOX_O + i]);
    acc1 = Fq_addmul(acc1, V[i], Fq_reduce(acc2));
  }
  dst[VOX_O] = Fq_reduce(acc1);
}

#include "tables/invmod_table.h"

/* Eliminate the leading coefficient in row1 using pivot row2 */
static void apply_pivot(uint32_t row1[], const uint32_t row2[], size_t n) {
  if (n == 0)
    return;
  Fq a = (Fq) (VOX_Q - row1[0]);
  row1[0] = 0;
  for (size_t i=1; i<n; i++)
    row1[i] = Fq_addmul(row1[i], a, (Fq) row2[i]);
}

/*
 * Search a row with a nonzero coefficient in column idx.
 * If found,
 *   - move the row in first position
 *   - divide by the leading coefficient, and reduce modulo q
 *     (needed to prevent overflows in apply_pivot)
 *   - return 1
 * If not, return 0
 */
static int search_pivot(uint32_t rows[], int nrows, int ncols, int blocklen, int idx) {
  int found = 0;
  int i,j;
  uint32_t tmp;
  Fq a;

  for (i=0; i<nrows; i++) {
    if (rows[i*ncols + idx] != 0) {
      found = 1;
      break;
    }
  }
  if (!found)
    return 0;
  a = invmod_table[Fq_reduce(rows[i*ncols + idx])];
  if (i > 0) {
    rows[idx] = 1;
    rows[i*ncols + idx] = 0;
    for (j=idx+1; j<blocklen; j++) {
      tmp = long_mul(a, Fq_reduce(rows[i*ncols + j]));
      rows[i*ncols + j] = rows[j];
      rows[j] = Fq_reduce(tmp);
    }
  }
  else {
    rows[idx] = 1;
    for (j=idx+1; j<blocklen; j++) {
      tmp = long_mul(a, Fq_reduce(rows[j]));
      rows[j] = Fq_reduce(tmp);
    }
  }
  return 1;
}

/*
 * Reduced row echelon form
 */
int rref_with_pivots(uint32_t eqns[(VOX_O-VOX_T)*(VOX_O+1)], int pivots[VOX_O+1]) {
  int n_pivots = 0;
  int n_nonpivots = VOX_O+1;
  int n_rows = VOX_O - VOX_T;
  int i,j;
  /* Cast eqns as a multi-dimensional array for readability */
  uint32_t (*Eqns)[VOX_O+1] = (uint32_t (*)[VOX_O+1]) eqns;

  for (i=0; i<VOX_O+1; i++) {
    for (j=0; j<VOX_O-VOX_T; j++)
      Eqns[j][i] = Fq_reduce(Eqns[j][i]);
    uint32_t *ref_row = Eqns[n_pivots];
    if (search_pivot(ref_row, n_rows, VOX_O+1, VOX_O+1, i)) {
      for (j=0; j<n_pivots; j++)
        apply_pivot(Eqns[j] + i, ref_row + i, VOX_O+1-i);
      for (j=n_pivots+1; j<VOX_O-VOX_T; j++)
        apply_pivot(Eqns[j] + i, ref_row + i, VOX_O+1-i);
      pivots[n_pivots] = i;
      n_pivots += 1;
      n_rows -= 1;
    }
    else {
      pivots[n_nonpivots-1] = i;
      n_nonpivots --;
    }
  }
  return n_pivots;
}

int rref_block_strict(uint32_t mat[], int ncols, int i, int j, int k, int l)
{
  int i1, j1, j2;
  int n_rows = k;
  /* Cast eqns as a multi-dimensional array for readability */
  uint32_t (*block)[ncols] = (uint32_t (*)[ncols]) (mat + i*ncols + j);

  for (j1=0; j1<l; j1++) {
    for (i1=0; i1<k; i1++)
      block[i1][j1] = Fq_reduce(block[i1][j1]);
    uint32_t *ref_row = block[j1];
    if (search_pivot(ref_row, n_rows, ncols, l, j1)) {
      for (i1=0; i1<j1; i1++)
        apply_pivot(block[i1] + j1, ref_row + j1, l-j1);
      for (i1=j1+1; i1<k; i1++)
        apply_pivot(block[i1] + j1, ref_row + j1, l-j1);
      n_rows -= 1;
    }
    else { /* reduce remaining columns and abort */
      for (j2=j1+1; j2<l; j2++) {
        for (i1=0; i1<k; i1++)
          block[i1][j2] = Fq_reduce(block[i1][j2]);
      }
      return j1;
    }
  }
  return j1;
}

/*
 * Compute a quadratic equation in VOX_T variables (grevlex order)
 */
void Fq_GetMQEqn(Fq dst[MQ_T_TERMS], const Fq mat[VOX_N*VOX_N],
                 const Fq V[VOX_V], const Fq Ker[VOX_O*(VOX_T+1)], const Fq target)
{
  int i,j,k;
  uint32_t acc;
  const Fq (*mKer)[VOX_T+1] = (const Fq (*)[VOX_T+1]) Ker;
  /* tKer * mat_OxO * Ker */
  {
    Fq tmp[VOX_O];
    Fq tmp2[VOX_T+1][VOX_T+1];
    for (i=0; i<VOX_T+1; i++) {
      /* tmp = mat_OxO * col_i(Ker) ; note that mat_OxO is lower triangluar. */
      for (j=0; j<VOX_O; j++) {
        acc = 0;
        for (k=0; k<=j; k++)
          acc = Fq_addmul(acc, mat[j*VOX_N+k], mKer[k][i]);
        tmp[j] = Fq_reduce(acc);
      }
      /* Compute tKer * tmp and fold it into a lower triangular result */
      for (j=0; j<VOX_T+1; j++) {
        acc = 0;
        for (k=0; k<VOX_O; k++)
          acc = Fq_addmul(acc, mKer[k][j], tmp[k]);
        if (i <= j)
          tmp2[j][i] = Fq_reduce(acc);
        else
          tmp2[i][j] = Fq_reduce(acc + tmp2[i][j]);
      }
    }
    /* Write system in grevlex order */
    k = 0;
    for (i=0; i<VOX_T+1; i++) {
      for (j=0; j<=i; j++) {
        dst[k] = tmp2[i][j];
        k++;
      }
    }
    k -= VOX_T+1;
  }
  /* Add the affine part from the OxV and VxV blocks */
  {
    uint32_t tmp[VOX_O+1];
    Fq_GetLinEqn(tmp, mat+VOX_O*VOX_N, V, target);
    /* dst[k] += tmp[0:VOX_O]*Ker + (0,...,0,tmp[VOX_O]) */
    for (i=0; i<VOX_T+1; i++) {
      acc = dst[k+i];
      if (i==VOX_T)
        acc += tmp[VOX_O];
      for (j=0; j<VOX_O; j++)
        acc = Fq_addmul(acc, tmp[j], mKer[j][i]);
      dst[k+i] = Fq_reduce(acc);
    }
  }
}


/*
 * From a solution of a system in VOX_T variables, reconstruct the VOX_O oil variables
 */
void Fq_GetOilVars(Fq dst[VOX_O], const Fq Ker[VOX_O*(VOX_T+1)], const Fq X[VOX_T]) {
  uint32_t acc;
  int i,j;
  for (i=0; i<VOX_O; i++) {
    acc = Ker[i*(VOX_T+1)+VOX_T];
    for (j=0; j<VOX_T; j++) {
      acc = Fq_addmul(acc, Ker[i*(VOX_T+1)+j], X[j]);
    }
    dst[i] = Fq_reduce(acc);
  }
}


/******************************************************************************
 * Auxiliary function for verification
 *****************************************************************************/
/*
 * Evaluate a multivariate quadratic system: Tr(eqn(Phi(vars)))
 */
Fq Fqc_EvalMQSystem(const Fqc eqn[VOX_NC*VOX_NC], const Fq vars[VOX_N]) {
  uint32_t tr;
  int i, j, k;
  /* A vector of size n over F_q and a vector of size n/c over F_q^c have the same representation */
  Fqc *vars2 = (Fqc *) vars;
  Fqc Fqc_acc;

  __m256i acc1x8[2*VOX_C-1], acc2x8[2*VOX_C-1], V_j[VOX_C], eqn_ji[VOX_C];
  /* Compute sum(V * col_i(eqn) * V_i) in a vectorized fashion */
  memset(acc1x8, 0, sizeof(acc1x8));
  for (i=0; i+7<VOX_NC; i+=8) {
    memset(acc2x8, 0, sizeof(acc2x8));
    for (j=i; j<VOX_NC; j++) {
      FqcX8_load1(V_j, &vars2[j]);
      FqcX8_load8(eqn_ji, &eqn[VOX_NC*j+i]);
      FqcX8_addmul(acc2x8, V_j, eqn_ji);
    }
    FqcX8_load8(V_j, &vars2[i]);
    FqcX8_reduce(acc2x8);
    FqcX8_addmul(acc1x8, acc2x8, V_j);
  }
  /* collapse the accumulator on one lane, and finish computation sequentially */
  uint32_t acc1[2*VOX_C-1];
  uint32_t *acc1x8_ptr = (uint32_t *) acc1x8;
  for (j=0; j<2*VOX_C-1; j++) {
    acc1[j] = acc1x8_ptr[8*j];
    for (k=1; k<8; k++)
      acc1[j] += acc1x8_ptr[8*j+k];
  }
  for (; i<VOX_NC; i++) {
    uint32_t acc2[2*VOX_C-1] = {0};
    for (j=i; j<VOX_NC; j++) {
      Fqc_addmul(acc2, &vars2[j], &eqn[j*VOX_NC+i]);
    }
    Fqc_reduce(&Fqc_acc, acc2);
    Fqc_addmul(acc1, &Fqc_acc, &vars2[i]);
  }
  /* Compute the trace of the result */
  Fqc_reduce(&Fqc_acc, acc1);
  tr = 0;
  for (i=0; i<VOX_C; i++) {
    tr = Fq_addmul(tr, Trace_Xi[i], Fqc_acc.coeffs[i]);
  }
  return Fq_reduce(tr);
}
