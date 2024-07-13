#ifdef __GFNI__
#include <immintrin.h>

#include "gf256.h"

EXPORT void gf256_init_gfni() { }

/// Performs "z[16] += vx[m] * my[m][16]"
/// parallelizaion over m
EXPORT void gf256_vec_mat16cols_muladd_gfni_ct(void* vz, void const* vx, void const* my, uint64_t m) {
  __m128i* const vzz = (__m128i*)vz;        // one row
  __m256i const* myy = (__m256i const*)my;  // contains 2 rows of m
  uint8_t const* vxx = (uint8_t*) vx;
  __m256i r = _mm256_setzero_si256();
  const uint64_t mm1 = m-1;
  for (uint64_t k = 0; k < mm1; k += 2) {
    const __m128i x_A0 = _mm_set1_epi8(vxx[k]);
    const __m128i x_A1 = _mm_set1_epi8(vxx[k + 1]);
    const __m256i x_A = _mm256_set_m128i(x_A1, x_A0);
    const __m256i m2rows = _mm256_loadu_si256(myy);
    r = _mm256_xor_si256(r, _mm256_gf2p8mul_epi8(x_A, m2rows));
    myy++;  // advance 2 rows
  }
  __m128i r1 = _mm256_extracti128_si256(r, 1);
  __m128i r0 = _mm256_castsi256_si128(r);
  _mm256_zeroupper();
  __m128i r2 = _mm_loadu_si128(vzz);
  r0 = _mm_xor_si128(r0, _mm_xor_si128(r1, r2));
  if (m&1) {
    // deal with the last row if m is even
    const __m128i x_A0 = _mm_set1_epi8(vxx[mm1]);
    const __m128i m1rows = _mm_loadu_si128((__m128i*) myy);
    r0 = _mm_xor_si128(r0, _mm_gf2p8mul_epi8(x_A0, m1rows));
  }
  _mm_storeu_si128(vzz, r0);
}

/// Performs "z[n] += vx[m] * my[m][n]" where n is a multiple of 32
/// parallelization over n
EXPORT void gf256_vec_mat128cols_muladd_gfni_ct(void* vz, void const* vx, void const* my, uint64_t m) {
  // x_A H_A = [<x_A, H_A_0>, ..., <x_A, H_A_k-1>]
  __m256i* const vzz = (__m256i*)vz;
  __m256i const* myy = (__m256i const*)my;
  uint8_t const* vxx = (uint8_t*) vx;
  __m256i x_B_0 = _mm256_loadu_si256(vzz);
  __m256i x_B_1 = _mm256_loadu_si256(vzz + 1);
  __m256i x_B_2 = _mm256_loadu_si256(vzz + 2);
  __m256i x_B_3 = _mm256_loadu_si256(vzz + 3);
  for (uint64_t i = 0; i < m; i++) {
    // gf256_muladd_mem(&x[params::x_A_size], share.x_A[i], &pk.H_a[i * params::syndrome_size], params::syndrome_size);
    const __m256i x_A = _mm256_set1_epi8(vxx[i]);

    const __m256i H_a_0 = _mm256_loadu_si256(myy);
    x_B_0 = _mm256_xor_si256(x_B_0, _mm256_gf2p8mul_epi8(x_A, H_a_0));

    const __m256i H_a_1 = _mm256_loadu_si256(myy + 1);
    x_B_1 = _mm256_xor_si256(x_B_1, _mm256_gf2p8mul_epi8(x_A, H_a_1));

    const __m256i H_a_2 = _mm256_loadu_si256(myy + 2);
    x_B_2 = _mm256_xor_si256(x_B_2, _mm256_gf2p8mul_epi8(x_A, H_a_2));

    const __m256i H_a_3 = _mm256_loadu_si256(myy + 3);
    x_B_3 = _mm256_xor_si256(x_B_3, _mm256_gf2p8mul_epi8(x_A, H_a_3));

    myy += 4;
  }
  _mm256_storeu_si256(vzz, x_B_0);
  _mm256_storeu_si256(vzz + 1, x_B_1);
  _mm256_storeu_si256(vzz + 2, x_B_2);
  _mm256_storeu_si256(vzz + 3, x_B_3);
  _mm256_zeroupper();
}
#endif // __GFNI__
