#include <immintrin.h>

#include "gf256.h"

// TODO: proofread this code
// TODO: unittest this code

__m128i gf_tables_lo[256];
__m128i gf_tables_hi[256];

EXPORT int gf256_init_avx2() {
  uint8_t(*const gf_tlow)[16] = (uint8_t(*)[16])gf_tables_lo;
  for (uint64_t i = 0; i < 256; ++i) {
    for (uint64_t j = 0; j < 16; ++j) {
      gf_tlow[i][j] = mul_gf256_naive(i, j);
    }
  }
  uint8_t(*const gf_thigh)[16] = (uint8_t(*)[16])gf_tables_hi;
  for (uint64_t i = 0; i < 256; ++i) {
    for (uint64_t j = 0; j < 16; ++j) {
      gf_thigh[i][j] = mul_gf256_naive(i, j << 4);
    }
  }
  return 0;
}

/// Performs "z[16] += vx[m] * my[m][16]"
/// parallelizaion over m
EXPORT void gf256_vec_mat16cols_muladd_avx2(void* vz, void const* vx, void const* my, uint64_t m) {
  const __m256i MASK_LO = _mm256_set1_epi8(0xf);
  __m128i* const vzz = (__m128i*)vz;        // one row
  __m256i const* myy = (__m256i const*)my;  // contains 2 rows of m
  uint8_t const* vxx = (uint8_t*) vx;
  __m256i r = _mm256_setzero_si256();
  const uint64_t mm1 = m-1;
  for (uint64_t k = 0; k < mm1; k += 2) {
    const __m128i x_A0_lo = _mm_load_si128(gf_tables_lo + vxx[k]);
    const __m128i x_A0_hi = _mm_load_si128(gf_tables_hi + vxx[k]);
    const __m128i x_A1_lo = _mm_load_si128(gf_tables_lo + vxx[k + 1]);
    const __m128i x_A1_hi = _mm_load_si128(gf_tables_hi + vxx[k + 1]);
    const __m256i x_A_lo = _mm256_set_m128i(x_A1_lo, x_A0_lo);
    const __m256i x_A_hi = _mm256_set_m128i(x_A1_hi, x_A0_hi);
    const __m256i m2rows = _mm256_loadu_si256(myy);
    const __m256i m2rows_low = _mm256_and_si256(m2rows, MASK_LO);
    const __m256i m2rows_high = _mm256_and_si256(_mm256_srli_epi64(m2rows, 4), MASK_LO);
    r = _mm256_xor_si256(
        r, _mm256_xor_si256(_mm256_shuffle_epi8(x_A_lo, m2rows_low), _mm256_shuffle_epi8(x_A_hi, m2rows_high)));
    myy++;  // advance 2 rows
  }
  __m128i r1 = _mm256_extracti128_si256(r, 1);
  __m128i r0 = _mm256_castsi256_si128(r);
  _mm256_zeroupper();
  __m128i r2 = _mm_load_si128(vzz);
  r0 = _mm_xor_si128(r0, _mm_xor_si128(r1, r2));
  if (m&1) {
    const __m128i MASK1_LO = _mm_set1_epi8(0xf);
    const __m128i x_A0_lo = _mm_load_si128(gf_tables_lo + vxx[mm1]);
    const __m128i x_A0_hi = _mm_load_si128(gf_tables_hi + vxx[mm1]);
    const __m128i m1rows = _mm_loadu_si128((__m128i*) myy);
    const __m128i m1rows_low = _mm_and_si128(m1rows, MASK1_LO);
    const __m128i m1rows_high = _mm_and_si128(_mm_srli_epi64(m1rows, 4), MASK1_LO);
    r0 = _mm_xor_si128(
        r0, _mm_xor_si128(_mm_shuffle_epi8(x_A0_lo, m1rows_low), _mm_shuffle_epi8(x_A0_hi, m1rows_high)));
  }
  _mm_store_si128(vzz, r0);
}

/// Performs "z[n] += vx[m] * my[m][n]" where n is a multiple of 32
/// parallelization over n
EXPORT void gf256_vec_mat128cols_muladd_avx2(void* vz, void const* vx, void const* my, uint64_t m) {
  // x_A H_A = [<x_A, H_A_0>, ..., <x_A, H_A_k-1>]
  const __m256i MASK_LO = _mm256_set1_epi8(0xf);
  __m256i* const vzz = (__m256i*)vz;
  __m256i const* myy = (__m256i const*)my;
  uint8_t const* vxx = (uint8_t*) vx;
  __m256i x_B_0 = _mm256_loadu_si256(vzz);
  __m256i x_B_1 = _mm256_loadu_si256(vzz + 1);
  __m256i x_B_2 = _mm256_loadu_si256(vzz + 2);
  __m256i x_B_3 = _mm256_loadu_si256(vzz + 3);
  for (uint64_t i = 0; i < m; i++) {
    const __m256i x_A_lo = _mm256_broadcastsi128_si256(_mm_load_si128(gf_tables_lo + vxx[i]));
    const __m256i x_A_hi = _mm256_broadcastsi128_si256(_mm_load_si128(gf_tables_hi + vxx[i]));

    const __m256i H_a_0 = _mm256_loadu_si256(myy);
    const __m256i H_a_0_low = _mm256_and_si256(H_a_0, MASK_LO);
    const __m256i H_a_0_high = _mm256_and_si256(_mm256_srli_epi64(H_a_0, 4), MASK_LO);
    x_B_0 = _mm256_xor_si256(
        x_B_0, _mm256_xor_si256(_mm256_shuffle_epi8(x_A_lo, H_a_0_low), _mm256_shuffle_epi8(x_A_hi, H_a_0_high)));

    const __m256i H_a_1 = _mm256_loadu_si256(myy + 1);
    const __m256i H_a_1_low = _mm256_and_si256(H_a_1, MASK_LO);
    const __m256i H_a_1_high = _mm256_and_si256(_mm256_srli_epi64(H_a_1, 4), MASK_LO);
    x_B_1 = _mm256_xor_si256(
        x_B_1, _mm256_xor_si256(_mm256_shuffle_epi8(x_A_lo, H_a_1_low), _mm256_shuffle_epi8(x_A_hi, H_a_1_high)));

    const __m256i H_a_2 = _mm256_loadu_si256(myy + 2);
    const __m256i H_a_2_low = _mm256_and_si256(H_a_2, MASK_LO);
    const __m256i H_a_2_high = _mm256_and_si256(_mm256_srli_epi64(H_a_2, 4), MASK_LO);
    x_B_2 = _mm256_xor_si256(
        x_B_2, _mm256_xor_si256(_mm256_shuffle_epi8(x_A_lo, H_a_2_low), _mm256_shuffle_epi8(x_A_hi, H_a_2_high)));

    const __m256i H_a_3 = _mm256_loadu_si256(myy + 3);
    const __m256i H_a_3_low = _mm256_and_si256(H_a_3, MASK_LO);
    const __m256i H_a_3_high = _mm256_and_si256(_mm256_srli_epi64(H_a_3, 4), MASK_LO);
    x_B_3 = _mm256_xor_si256(
        x_B_3, _mm256_xor_si256(_mm256_shuffle_epi8(x_A_lo, H_a_3_low), _mm256_shuffle_epi8(x_A_hi, H_a_3_high)));

    myy += 4;
  }
  _mm256_storeu_si256(vzz, x_B_0);
  _mm256_storeu_si256(vzz + 1, x_B_1);
  _mm256_storeu_si256(vzz + 2, x_B_2);
  _mm256_storeu_si256(vzz + 3, x_B_3);
  _mm256_zeroupper();
}
