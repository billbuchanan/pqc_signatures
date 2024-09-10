#include <immintrin.h>

#include "gf256.h"

/** @brief naive multiplication in GF2p8 */
static uint8_t mul_8_8_naive(uint8_t x, uint8_t y) {
  static const uint16_t p = 0x11B; // 100011011
  uint16_t xx = x;
  uint16_t z = 0;
  for (uint64_t i = 0; i < 8; ++i) z ^= ((y >> i) & 1u) ? (xx << i) : 0;
  for (uint64_t j = 14; j >= 8; j--) {
    z ^= ((z >> j) & 1) ? (p << (j - 8)) : 0;
  }
  return z;
}

__m256i gf_tables_lo2_ct[8];
__m256i gf_tables_hi2_ct[8];
__m256i gf_tables_hilo2_ct[8];
typedef uint8_t PREC_TABLE_2X16[2][16];

EXPORT void gf256_init_avx2_polytable_ct() {
  PREC_TABLE_2X16* gf_tlow2 = (PREC_TABLE_2X16*)gf_tables_lo2_ct;
  PREC_TABLE_2X16* gf_thigh2 = (PREC_TABLE_2X16*)gf_tables_hi2_ct;
  PREC_TABLE_2X16* gf_thighlow = (PREC_TABLE_2X16*)gf_tables_hilo2_ct;
  for (uint64_t i = 0; i < 8; ++i) {
    for (uint64_t j = 0; j < 16; ++j) {
      uint8_t lo = mul_8_8_naive(1 << i, j);
      uint8_t hi = mul_8_8_naive(1 << i, j << 4);
      gf_tlow2[i][0][j] = lo;
      gf_tlow2[i][1][j] = lo;
      gf_thigh2[i][0][j] = hi;
      gf_thigh2[i][1][j] = hi;
      gf_thighlow[i][0][j] = lo;
      gf_thighlow[i][1][j] = hi;
    }
  }
}

/// Performs "z[16] += vx[m] * my[m][16]"
/// parallelizaion over m
EXPORT void gf256_vec_mat16cols_muladd_polytable_avx2_ct(void* vz, void const* vx, void const* my, uint64_t m) {
  if (m%2) abort(); // TODO remove when done! currently, this function only supports even m
  const __m256i ONE = _mm256_set1_epi8(1);
  const __m256i MASK_LO = _mm256_set1_epi8(0xf);
  __m128i* const vzz = (__m128i*)vz;        // one row
  __m256i const* myy = (__m256i const*)my;  // contains 2 rows of m
  uint8_t const* vxx = (uint8_t*) vx;
  __m256i r = _mm256_setzero_si256();
  for (uint64_t k = 0; k < m; k += 2) {
    const __m256i vx_k = _mm256_set_m128i(_mm_set1_epi8(vxx[k + 1]), _mm_set1_epi8(vxx[k]));
    // clang-format off
    const __m256i x_A_lo = _mm256_xor_si256(
        _mm256_xor_si256(
            _mm256_xor_si256(
                _mm256_sign_epi8(_mm256_load_si256(gf_tables_lo2_ct + 0), _mm256_and_si256(ONE, vx_k)),
                _mm256_sign_epi8(_mm256_load_si256(gf_tables_lo2_ct + 1), _mm256_and_si256(ONE, _mm256_srli_epi16(vx_k, 1)))),
            _mm256_xor_si256(
                _mm256_sign_epi8(_mm256_load_si256(gf_tables_lo2_ct + 2), _mm256_and_si256(ONE, _mm256_srli_epi16(vx_k, 2))),
                _mm256_sign_epi8(_mm256_load_si256(gf_tables_lo2_ct + 3), _mm256_and_si256(ONE, _mm256_srli_epi16(vx_k, 3))))),
        _mm256_xor_si256(
            _mm256_xor_si256(
                _mm256_sign_epi8(_mm256_load_si256(gf_tables_lo2_ct + 4), _mm256_and_si256(ONE, _mm256_srli_epi16(vx_k, 4))),
                _mm256_sign_epi8(_mm256_load_si256(gf_tables_lo2_ct + 5), _mm256_and_si256(ONE, _mm256_srli_epi16(vx_k, 5)))),
            _mm256_xor_si256(
                _mm256_sign_epi8(_mm256_load_si256(gf_tables_lo2_ct + 6), _mm256_and_si256(ONE, _mm256_srli_epi16(vx_k, 6))),
                _mm256_sign_epi8(_mm256_load_si256(gf_tables_lo2_ct + 7), _mm256_and_si256(ONE, _mm256_srli_epi16(vx_k, 7))))));
    const __m256i x_A_hi = _mm256_xor_si256(
        _mm256_xor_si256(
            _mm256_xor_si256(
                _mm256_sign_epi8(_mm256_load_si256(gf_tables_hi2_ct + 0), _mm256_and_si256(ONE, vx_k)),
                _mm256_sign_epi8(_mm256_load_si256(gf_tables_hi2_ct + 1), _mm256_and_si256(ONE, _mm256_srli_epi16(vx_k, 1)))),
            _mm256_xor_si256(
                _mm256_sign_epi8(_mm256_load_si256(gf_tables_hi2_ct + 2), _mm256_and_si256(ONE, _mm256_srli_epi16(vx_k, 2))),
                _mm256_sign_epi8(_mm256_load_si256(gf_tables_hi2_ct + 3), _mm256_and_si256(ONE, _mm256_srli_epi16(vx_k, 3))))),
        _mm256_xor_si256(
            _mm256_xor_si256(
                _mm256_sign_epi8(_mm256_load_si256(gf_tables_hi2_ct + 4), _mm256_and_si256(ONE, _mm256_srli_epi16(vx_k, 4))),
                _mm256_sign_epi8(_mm256_load_si256(gf_tables_hi2_ct + 5), _mm256_and_si256(ONE, _mm256_srli_epi16(vx_k, 5)))),
            _mm256_xor_si256(
                _mm256_sign_epi8(_mm256_load_si256(gf_tables_hi2_ct + 6), _mm256_and_si256(ONE, _mm256_srli_epi16(vx_k, 6))),
                _mm256_sign_epi8(_mm256_load_si256(gf_tables_hi2_ct + 7), _mm256_and_si256(ONE, _mm256_srli_epi16(vx_k, 7))))));
    // clang-format on
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
  __m128i r2 = _mm_loadu_si128(vzz);
  r0 = _mm_xor_si128(r0, _mm_xor_si128(r1, r2));
  _mm_storeu_si128(vzz, r0);
}

/// Performs "z[n] += vx[m] * my[m][n]" where n is a multiple of 32
/// parallelization over n
EXPORT void gf256_vec_mat128cols_muladd_polytable_avx2_ct(void* vz, void const* vx, void const* my, uint64_t m) {
  // x_A H_A = [<x_A, H_A_0>, ..., <x_A, H_A_k-1>]
  const __m256i ONE = _mm256_set1_epi8(1);
  const __m256i MASK_LO = _mm256_set1_epi8(0xf);
  __m256i* const vzz = (__m256i*)vz;
  __m256i const* myy = (__m256i const*)my;
  uint8_t const* vxx = (uint8_t*) vx;
  __m256i x_B_0 = _mm256_loadu_si256(vzz);
  __m256i x_B_1 = _mm256_loadu_si256(vzz + 1);
  __m256i x_B_2 = _mm256_loadu_si256(vzz + 2);
  __m256i x_B_3 = _mm256_loadu_si256(vzz + 3);
  for (uint64_t i = 0; i < m; i++) {
    const __m256i vx_k = _mm256_set1_epi8(vxx[i]);
    // clang-format off
    const __m256i x_A_hilo = _mm256_xor_si256(
        _mm256_xor_si256(
            _mm256_xor_si256(
                _mm256_sign_epi8(_mm256_load_si256(gf_tables_hilo2_ct + 0), _mm256_and_si256(ONE, vx_k)),
                _mm256_sign_epi8(_mm256_load_si256(gf_tables_hilo2_ct + 1), _mm256_and_si256(ONE, _mm256_srli_epi16(vx_k, 1)))),
            _mm256_xor_si256(
                _mm256_sign_epi8(_mm256_load_si256(gf_tables_hilo2_ct + 2), _mm256_and_si256(ONE, _mm256_srli_epi16(vx_k, 2))),
                _mm256_sign_epi8(_mm256_load_si256(gf_tables_hilo2_ct + 3), _mm256_and_si256(ONE, _mm256_srli_epi16(vx_k, 3))))),
        _mm256_xor_si256(
            _mm256_xor_si256(
                _mm256_sign_epi8(_mm256_load_si256(gf_tables_hilo2_ct + 4), _mm256_and_si256(ONE, _mm256_srli_epi16(vx_k, 4))),
                _mm256_sign_epi8(_mm256_load_si256(gf_tables_hilo2_ct + 5), _mm256_and_si256(ONE, _mm256_srli_epi16(vx_k, 5)))),
            _mm256_xor_si256(
                _mm256_sign_epi8(_mm256_load_si256(gf_tables_hilo2_ct + 6), _mm256_and_si256(ONE, _mm256_srli_epi16(vx_k, 6))),
                _mm256_sign_epi8(_mm256_load_si256(gf_tables_hilo2_ct + 7), _mm256_and_si256(ONE, _mm256_srli_epi16(vx_k, 7))))));
    // clang-format on

    const __m256i x_A_lo = _mm256_broadcastsi128_si256(_mm256_extracti128_si256(x_A_hilo, 0));
    const __m256i x_A_hi = _mm256_broadcastsi128_si256(_mm256_extracti128_si256(x_A_hilo, 1));

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
