#include <immintrin.h>
#include <stdint.h>
#include "gf256-avx2-polytable-ct.h"
#include "platform.h"

ALIGN(32) static uint8_t gf_tables_lo2_ct_v[8*2*16] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60, 0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60, 0, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96, 104, 112, 120, 0, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96, 104, 112, 120, 0, 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240, 0, 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240, 0, 32, 64, 96, 128, 160, 192, 224, 27, 59, 91, 123, 155, 187, 219, 251, 0, 32, 64, 96, 128, 160, 192, 224, 27, 59, 91, 123, 155, 187, 219, 251, 0, 64, 128, 192, 27, 91, 155, 219, 54, 118, 182, 246, 45, 109, 173, 237, 0, 64, 128, 192, 27, 91, 155, 219, 54, 118, 182, 246, 45, 109, 173, 237, 0, 128, 27, 155, 54, 182, 45, 173, 108, 236, 119, 247, 90, 218, 65, 193, 0, 128, 27, 155, 54, 182, 45, 173, 108, 236, 119, 247, 90, 218, 65, 193
};
ALIGN(32) static uint8_t gf_tables_hi2_ct_v[8*2*16] = {
    0, 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240, 0, 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240, 0, 32, 64, 96, 128, 160, 192, 224, 27, 59, 91, 123, 155, 187, 219, 251, 0, 32, 64, 96, 128, 160, 192, 224, 27, 59, 91, 123, 155, 187, 219, 251, 0, 64, 128, 192, 27, 91, 155, 219, 54, 118, 182, 246, 45, 109, 173, 237, 0, 64, 128, 192, 27, 91, 155, 219, 54, 118, 182, 246, 45, 109, 173, 237, 0, 128, 27, 155, 54, 182, 45, 173, 108, 236, 119, 247, 90, 218, 65, 193, 0, 128, 27, 155, 54, 182, 45, 173, 108, 236, 119, 247, 90, 218, 65, 193, 0, 27, 54, 45, 108, 119, 90, 65, 216, 195, 238, 245, 180, 175, 130, 153, 0, 27, 54, 45, 108, 119, 90, 65, 216, 195, 238, 245, 180, 175, 130, 153, 0, 54, 108, 90, 216, 238, 180, 130, 171, 157, 199, 241, 115, 69, 31, 41, 0, 54, 108, 90, 216, 238, 180, 130, 171, 157, 199, 241, 115, 69, 31, 41, 0, 108, 216, 180, 171, 199, 115, 31, 77, 33, 149, 249, 230, 138, 62, 82, 0, 108, 216, 180, 171, 199, 115, 31, 77, 33, 149, 249, 230, 138, 62, 82, 0, 216, 171, 115, 77, 149, 230, 62, 154, 66, 49, 233, 215, 15, 124, 164, 0, 216, 171, 115, 77, 149, 230, 62, 154, 66, 49, 233, 215, 15, 124, 164
};
ALIGN(32) static uint8_t gf_tables_hilo2_ct_v[8*2*16] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0, 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240, 0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 0, 32, 64, 96, 128, 160, 192, 224, 27, 59, 91, 123, 155, 187, 219, 251, 0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60, 0, 64, 128, 192, 27, 91, 155, 219, 54, 118, 182, 246, 45, 109, 173, 237, 0, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96, 104, 112, 120, 0, 128, 27, 155, 54, 182, 45, 173, 108, 236, 119, 247, 90, 218, 65, 193, 0, 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240, 0, 27, 54, 45, 108, 119, 90, 65, 216, 195, 238, 245, 180, 175, 130, 153, 0, 32, 64, 96, 128, 160, 192, 224, 27, 59, 91, 123, 155, 187, 219, 251, 0, 54, 108, 90, 216, 238, 180, 130, 171, 157, 199, 241, 115, 69, 31, 41, 0, 64, 128, 192, 27, 91, 155, 219, 54, 118, 182, 246, 45, 109, 173, 237, 0, 108, 216, 180, 171, 199, 115, 31, 77, 33, 149, 249, 230, 138, 62, 82, 0, 128, 27, 155, 54, 182, 45, 173, 108, 236, 119, 247, 90, 218, 65, 193, 0, 216, 171, 115, 77, 149, 230, 62, 154, 66, 49, 233, 215, 15, 124, 164
};

static __m256i* gf_tables_lo2_ct = (__m256i*) gf_tables_lo2_ct_v;
static __m256i* gf_tables_hi2_ct = (__m256i*) gf_tables_hi2_ct_v;
static __m256i* gf_tables_hilo2_ct = (__m256i*) gf_tables_hilo2_ct_v;
typedef uint8_t PREC_TABLE_2X16[2][16];

/// Performs "z[16] += vx[m] * my[m][16]"
/// parallelizaion over m
void gf256_vec_mat16cols_muladd_polytable_avx2_ct(void* vz, void const* vx, void const* my, uint64_t m, uint64_t scaling) {
  if (m%2) abort(); // TODO remove when done! currently, this function only supports even m
  const __m256i ONE = _mm256_set1_epi8(1);
  const __m256i MASK_LO = _mm256_set1_epi8(0xf);
  __m128i* const vzz = (__m128i*)vz;        // one row
  __m256i const* myy = (__m256i const*)my;  // contains 2 rows of m
  uint8_t const* vxx = (uint8_t*) vx;
  __m256i r = _mm256_setzero_si256();
  for (uint64_t k = 0; k < m; k += 2) {
    const __m256i vx_k = _mm256_set_m128i(_mm_set1_epi8((char) vxx[k + 1]), _mm_set1_epi8((char) vxx[k]));
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
    const __m256i m2rows;
    *((__m128i *)&m2rows) = _mm_loadu_si128((__m128i *)myy);
    *(((__m128i *)&m2rows)+1) = _mm_loadu_si128(((__m128i *)myy)+scaling);
    const __m256i m2rows_low = _mm256_and_si256(m2rows, MASK_LO);
    const __m256i m2rows_high = _mm256_and_si256(_mm256_srli_epi64(m2rows, 4), MASK_LO);
    r = _mm256_xor_si256(
        r, _mm256_xor_si256(_mm256_shuffle_epi8(x_A_lo, m2rows_low), _mm256_shuffle_epi8(x_A_hi, m2rows_high)));
    myy+=scaling;  // advance 2 rows
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
void gf256_vec_mat128cols_muladd_polytable_avx2_ct(void* vz, void const* vx, void const* my, uint64_t m, uint64_t scaling) {
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
    const __m256i vx_k = _mm256_set1_epi8((char) vxx[i]);
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

    //const __m256i H_a_0 = _mm256_loadu_si256(myy);
    const __m256i H_a_0;
    *((__m128i *)&H_a_0) = _mm_loadu_si128((__m128i *)myy);
    *(((__m128i *)&H_a_0)+1) = _mm_loadu_si128(((__m128i *)myy)+scaling);
    const __m256i H_a_0_low = _mm256_and_si256(H_a_0, MASK_LO);
    const __m256i H_a_0_high = _mm256_and_si256(_mm256_srli_epi64(H_a_0, 4), MASK_LO);
    x_B_0 = _mm256_xor_si256(
        x_B_0, _mm256_xor_si256(_mm256_shuffle_epi8(x_A_lo, H_a_0_low), _mm256_shuffle_epi8(x_A_hi, H_a_0_high)));

    //const __m256i H_a_1 = _mm256_loadu_si256(myy + 1);
    const __m256i H_a_1;
    *((__m128i *)&H_a_1) = _mm_loadu_si128(((__m128i *)myy)+2*scaling);
    *(((__m128i *)&H_a_1)+1) = _mm_loadu_si128(((__m128i *)myy)+3*scaling);
    const __m256i H_a_1_low = _mm256_and_si256(H_a_1, MASK_LO);
    const __m256i H_a_1_high = _mm256_and_si256(_mm256_srli_epi64(H_a_1, 4), MASK_LO);
    x_B_1 = _mm256_xor_si256(
        x_B_1, _mm256_xor_si256(_mm256_shuffle_epi8(x_A_lo, H_a_1_low), _mm256_shuffle_epi8(x_A_hi, H_a_1_high)));

    //const __m256i H_a_2 = _mm256_loadu_si256(myy + 2);
    const __m256i H_a_2;
    *((__m128i *)&H_a_2) = _mm_loadu_si128(((__m128i *)myy)+4*scaling);
    *(((__m128i *)&H_a_2)+1) = _mm_loadu_si128(((__m128i *)myy)+5*scaling);
    const __m256i H_a_2_low = _mm256_and_si256(H_a_2, MASK_LO);
    const __m256i H_a_2_high = _mm256_and_si256(_mm256_srli_epi64(H_a_2, 4), MASK_LO);
    x_B_2 = _mm256_xor_si256(
        x_B_2, _mm256_xor_si256(_mm256_shuffle_epi8(x_A_lo, H_a_2_low), _mm256_shuffle_epi8(x_A_hi, H_a_2_high)));

    //const __m256i H_a_3 = _mm256_loadu_si256(myy + 3);
    const __m256i H_a_3;
    *((__m128i *)&H_a_3) = _mm_loadu_si128(((__m128i *)myy)+6*scaling);
    *(((__m128i *)&H_a_3)+1) = _mm_loadu_si128(((__m128i *)myy)+7*scaling);
    const __m256i H_a_3_low = _mm256_and_si256(H_a_3, MASK_LO);
    const __m256i H_a_3_high = _mm256_and_si256(_mm256_srli_epi64(H_a_3, 4), MASK_LO);
    x_B_3 = _mm256_xor_si256(
        x_B_3, _mm256_xor_si256(_mm256_shuffle_epi8(x_A_lo, H_a_3_low), _mm256_shuffle_epi8(x_A_hi, H_a_3_high)));

    myy += 4*scaling;
  }
  _mm256_storeu_si256(vzz, x_B_0);
  _mm256_storeu_si256(vzz + 1, x_B_1);
  _mm256_storeu_si256(vzz + 2, x_B_2);
  _mm256_storeu_si256(vzz + 3, x_B_3);
  _mm256_zeroupper();
}
