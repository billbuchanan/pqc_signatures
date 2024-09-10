#include <immintrin.h>

#include "p251.h"

/// Performs reduction modular-251 in 32-bit lanes.
static inline __m256i _mm256_mod251_epi32(__m256i a) {
  // Expand 32-bit lanes to 64-bit.
  __m256i low = _mm256_unpacklo_epi32(a, _mm256_setzero_si256());
  __m256i high = _mm256_unpackhi_epi32(a, _mm256_setzero_si256());

  // Compute xr, where r = 2^39 / 251.
  const __m256i R = _mm256_set1_epi64x(2190262207);
  __m256i low_mod = _mm256_mul_epu32(low, R);
  __m256i high_mod = _mm256_mul_epu32(high, R);

  // Compute floor(xr / 2^39).
  low_mod = _mm256_srli_epi64(low_mod, 39);
  high_mod = _mm256_srli_epi64(high_mod, 39);

  // Compute floor(xr / 2^39) * 251.
  const __m256i P = _mm256_set1_epi64x(251);
  low_mod = _mm256_mul_epu32(low_mod, P);
  high_mod = _mm256_mul_epu32(high_mod, P);

  // Compute x - floor(xr / 2^39) * 251.
  low_mod = _mm256_sub_epi64(low, low_mod);
  high_mod = _mm256_sub_epi64(high, high_mod);

  // Truncate, concat, and return.
  __m256i ll = _mm256_unpacklo_epi32(low_mod, high_mod);
  __m256i hh = _mm256_unpackhi_epi32(low_mod, high_mod);
  return _mm256_unpacklo_epi32(ll, hh);
}

/// Performs "z[16] += vx[m] * my[m][16]"
/// parallelizaion over m
EXPORT void p251_vec_mat16cols_muladd_avx2_ct(void *vz, void const *vx,
                                              void const *my, uint64_t m) {
  uint8_t const *vxx = (uint8_t const *)vx;
  __m128i const *const myy = (__m128i const *)my;
  __m128i *const vzz = (__m128i *)vz;

  __m256i vzz_epi16 = _mm256_cvtepu8_epi16(_mm_loadu_si128(&vzz[0]));
  __m256i RetL = _mm256_unpacklo_epi16(vzz_epi16, _mm256_setzero_si256());
  __m256i RetH = _mm256_unpackhi_epi16(vzz_epi16, _mm256_setzero_si256());

  for (size_t i = 0; i < m; ++i) {
    const __m256i VA = _mm256_set1_epi16(vxx[i]);
    const __m256i VB =
        _mm256_cvtepu8_epi16(_mm_loadu_si128((__m128i *)&myy[i]));
    const __m256i VM = _mm256_mullo_epi16(VA, VB);
    const __m256i VMLow = _mm256_unpacklo_epi16(VM, _mm256_setzero_si256());
    const __m256i VMHigh = _mm256_unpackhi_epi16(VM, _mm256_setzero_si256());
    RetL = _mm256_add_epi32(RetL, VMLow);
    RetH = _mm256_add_epi32(RetH, VMHigh);
  }

  RetL = _mm256_mod251_epi32(RetL);
  RetH = _mm256_mod251_epi32(RetH);

  const __m256i SM = _mm256_set_epi8(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 12, 8, 4, 0,
                                     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 12, 8, 4, 0);
  const __m256i RetLu8 = _mm256_shuffle_epi8(RetL, SM);
  const __m256i RetHu8 = _mm256_shuffle_epi8(RetH, SM);

  const __m128i Ret128 = _mm256_extracti128_si256(
      _mm256_permute4x64_epi64(_mm256_unpacklo_epi32(RetLu8, RetHu8),
                               0b11111000),
      0);

  _mm_storeu_si128((__m128i *)&vzz[0], Ret128);
}

/// Performs "z[16] += vx[m] * my[m][16]"
/// parallelizaion over m
EXPORT void p251_vec_mat16cols_muladd_b16_avx2_ct(void *vz, void const *vx,
                                                  void const *my, uint64_t m) {
  uint8_t const *vxx = (uint8_t const *)vx;
  __m128i const *const myy = (__m128i const *)my;
  __m128i *const vzz = (__m128i *)vz;
  const __m256i MASK_LOW = _mm256_set1_epi16(0xFF);
  const __m256i FIVE = _mm256_set1_epi16(5);
  const __m256i TWENTYFIVE = _mm256_set1_epi16(25);
  const __m256i N_251 = _mm256_set1_epi16(251);
  __m256i RetL = _mm256_cvtepu8_epi16(_mm_loadu_si128(&vzz[0]));
  __m256i RetH = _mm256_setzero_si256();
  const uint64_t mm1 = m - 1;

  uint16_t taken = 1;
  for (size_t i = 0; i < mm1; i += 2) {
    // since mullo has a large latency, we interleave 2 iterations
    const __m256i VA_0 = _mm256_set1_epi16(vxx[i]);
    const __m256i VA_1 = _mm256_set1_epi16(vxx[i + 1]);
    const __m256i VB_0 = _mm256_cvtepu8_epi16(_mm_loadu_si128(&myy[i]));
    const __m256i VB_1 = _mm256_cvtepu8_epi16(_mm_loadu_si128(&myy[i + 1]));
    const __m256i VM_0 = _mm256_mullo_epi16(VA_0, VB_0);
    const __m256i VM_1 = _mm256_mullo_epi16(VA_1, VB_1);

    RetL = _mm256_add_epi16(RetL,
                            _mm256_add_epi16(_mm256_and_si256(MASK_LOW, VM_0),
                                             _mm256_and_si256(MASK_LOW, VM_1)));
    RetH = _mm256_add_epi16(RetH, _mm256_add_epi16(_mm256_srli_epi16(VM_0, 8),
                                                   _mm256_srli_epi16(VM_1, 8)));

    // if the accumulator becomes too large, reduce it (it never happens for
    // m<=256)
    if (taken > 259) {
      RetL =
          _mm256_add_epi16(_mm256_mullo_epi16(_mm256_srli_epi16(RetL, 8), FIVE),
                           _mm256_and_si256(RetL, MASK_LOW));
      RetH =
          _mm256_add_epi16(_mm256_mullo_epi16(_mm256_srli_epi16(RetH, 8), FIVE),
                           _mm256_and_si256(RetH, MASK_LOW));
      taken = 6;
    } else {
      taken += 2;
    }
  }
  if (m & 1) {
    // deal  with the last iteration if m is odd
    const __m256i VA_0 = _mm256_set1_epi16(vxx[mm1]);
    const __m256i VB_0 = _mm256_cvtepu8_epi16(_mm_loadu_si128(&myy[mm1]));
    const __m256i VM_0 = _mm256_mullo_epi16(VA_0, VB_0);

    RetL = _mm256_add_epi16(RetL, _mm256_and_si256(MASK_LOW, VM_0));
    RetH = _mm256_add_epi16(RetH, _mm256_srli_epi16(VM_0, 8));
  }
  // 25 * msb(retH) + 5 * (lsb(retH)+msb(retL)) + 1 * lsb(retL) : numbers in
  // [0,30*256]
  const __m256i low = _mm256_and_si256(RetL, MASK_LOW);
  const __m256i middle = _mm256_add_epi16(_mm256_srli_epi16(RetL, 8),
                                          _mm256_and_si256(RetH, MASK_LOW));
  const __m256i hi = _mm256_srli_epi16(RetH, 8);
  const __m256i t1 =
      _mm256_add_epi16(_mm256_add_epi16(_mm256_mullo_epi16(hi, TWENTYFIVE),
                                        _mm256_mullo_epi16(middle, FIVE)),
                       low);

  // 5 * msb(t1) + lsb(t1) : numbers in [0,150+256]
  const __m256i t2 =
      _mm256_add_epi16(_mm256_mullo_epi16(_mm256_srli_epi16(t1, 8), FIVE),
                       _mm256_and_si256(t1, MASK_LOW));

  // t2 - 251.(t2>=251) : numbers in [0,250]
  const __m256i t3 = _mm256_sub_epi16(
      t2, _mm256_andnot_si256(_mm256_cmpgt_epi16(N_251, t2), N_251));

  // permute and return
  const __m256i SM =
      _mm256_set_epi8(14, 12, 10, 8, 6, 4, 2, 0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                      0xFF, 0xFF, 14, 12, 10, 8, 6, 4, 2, 0);
  const __m256i final1 = _mm256_shuffle_epi8(t3, SM);
  const __m128i final2 = _mm_or_si128(_mm256_extracti128_si256(final1, 0),
                                      _mm256_extracti128_si256(final1, 1));
  _mm_storeu_si128((__m128i *)&vzz[0], final2);
}

/// Performs "z[n] += vx[m] * my[m][n]" where n is a multiple of 32
/// parallelization over n
EXPORT void p251_vec_mat128cols_muladd_avx2_ct(void *vz, void const *vx,
                                               void const *my, uint64_t m) {
  uint8_t const *vxx = (uint8_t const *)vx;
  __m128i const *const myy = (__m128i const *)my;
  __m128i *const vzz = (__m128i *)vz;

  __m256i x_B[8];
  __m256i RetL[8];
  __m256i RetH[8];
  for (size_t i = 0; i < 8; ++i) {
    x_B[i] = _mm256_cvtepu8_epi16(_mm_loadu_si128(&vzz[i]));
    RetL[i] = _mm256_unpacklo_epi16(x_B[i], _mm256_setzero_si256());
    RetH[i] = _mm256_unpackhi_epi16(x_B[i], _mm256_setzero_si256());
  }

  for (size_t i = 0; i < m; ++i) {
    const __m256i VA = _mm256_set1_epi16(vxx[i]);
    for (size_t j = 0; j < 8; ++j) {
      const __m256i VB =
          _mm256_cvtepu8_epi16(_mm_loadu_si128((__m128i *)&myy[i * 8 + j]));
      const __m256i VM = _mm256_mullo_epi16(VA, VB);
      const __m256i VMLow = _mm256_unpacklo_epi16(VM, _mm256_setzero_si256());
      const __m256i VMHigh = _mm256_unpackhi_epi16(VM, _mm256_setzero_si256());
      RetL[j] = _mm256_add_epi32(RetL[j], VMLow);
      RetH[j] = _mm256_add_epi32(RetH[j], VMHigh);
    }
  }

  const __m256i SM = _mm256_set_epi8(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 12, 8, 4, 0,
                                     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 12, 8, 4, 0);
  for (size_t i = 0; i < 8; ++i) {
    const __m256i RetLRed = _mm256_mod251_epi32(RetL[i]);
    const __m256i RetHRed = _mm256_mod251_epi32(RetH[i]);
    const __m256i RetLu8 = _mm256_shuffle_epi8(RetLRed, SM);
    const __m256i RetHu8 = _mm256_shuffle_epi8(RetHRed, SM);
    const __m256i RetV = _mm256_permute4x64_epi64(
        _mm256_unpacklo_epi32(RetLu8, RetHu8), 0b11111000);
    _mm_storeu_si128((__m128i *)&vzz[i], _mm256_extractf128_si256(RetV, 0));
  }
}
