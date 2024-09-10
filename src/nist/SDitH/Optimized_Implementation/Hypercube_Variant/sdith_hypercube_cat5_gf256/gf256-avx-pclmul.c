#include <immintrin.h>

#include "gf256.h"

static __m128i TIMES_B[2];
static __m128i TIMES_TWO[2];

// here, we precompute the 2x16 bytes that correspond to the multiplication by B=0x1b
// we could actually hardcode this constant.
EXPORT void gf256_init_pclmul_ct() {
  uint8_t (*t)[16] = (uint8_t (*)[16]) TIMES_B;
  uint8_t (*u)[16] = (uint8_t (*)[16]) TIMES_TWO;
  for (uint64_t i = 0; i<16; ++i) {
    t[0][i] = mul_gf256_naive(0x1b, i);
    t[1][i] = mul_gf256_naive(0x1b, i<<4);
    u[0][i] = mul_gf256_naive(0x02, i);
    u[1][i] = mul_gf256_naive(0x02, i<<4);
  }
}


EXPORT void gf256_vec_mat16cols_muladd_pclmul_ct(void* vz, void const* vx, void const* my, uint64_t m) {
  __m128i* const vzz = (__m128i*)vz;        // one row
  __m128i const* myy = (__m128i const*)my;  // one row of m
  uint8_t const* vxx = (uint8_t*) vx;

  __m128i resCL0 = _mm_setzero_si128();
  __m128i resCL1 = _mm_setzero_si128();

//#pragma unroll(2)
  for (size_t i = 0; i < m; i++) {
    const __m128i coeff = _mm_set_epi64x(0, (uint64_t)vxx[i]);
    const __m128i row = _mm_loadu_si128(myy + i);
    const __m128i VecVA = _mm_cvtepu8_epi16(row); // 8 lsb coeffs extended
    const __m128i VecVB = _mm_cvtepu8_epi16(
        _mm_shuffle_epi32(row, 0b00001110)); // 8 last coeffs extended

    const __m128i MulA0 = _mm_clmulepi64_si128(coeff, VecVA, 0x00); // 4 first
    const __m128i MulA1 = _mm_clmulepi64_si128(coeff, VecVA, 0x10); // 4 last
    const __m128i MulB0 = _mm_clmulepi64_si128(coeff, VecVB, 0x00); // 4 first
    const __m128i MulB1 = _mm_clmulepi64_si128(coeff, VecVB, 0x10); // 4 last

    resCL0 = _mm_xor_si128(resCL0, _mm_unpacklo_epi64(MulA0, MulA1)); // 8 first
    resCL1 = _mm_xor_si128(resCL1, _mm_unpacklo_epi64(MulB0, MulB1)); // 8 last
  }

  const __m128i SMask = _mm_set_epi8(15,13,11,9,7,5,3,1,14,12,10,8,6,4,2,0);
  const __m128i hilo_first = _mm_shuffle_epi8(resCL0, SMask);
  const __m128i hilo_last = _mm_shuffle_epi8(resCL1, SMask);
  const __m128i all_lo = _mm_unpacklo_epi64(hilo_first, hilo_last); // 8 lsbs
  const __m128i all_hi = _mm_unpackhi_epi64(hilo_first, hilo_last); // 8 msbs

  // return lo + B * hi
  const __m128i MASK_LO = _mm_set1_epi8(0xf);
  const __m128i TIMES_B_HI = _mm_load_si128(TIMES_B+1); // TODO: make it really constant
  const __m128i TIMES_B_LO = _mm_load_si128(TIMES_B+0); // TODO: make it really constant

  const __m128i r1 = _mm_shuffle_epi8(TIMES_B_LO, _mm_and_si128(MASK_LO, all_hi));
  const __m128i r2 = _mm_shuffle_epi8(TIMES_B_HI, _mm_and_si128(MASK_LO, _mm_srli_epi16(all_hi, 4)));
  const __m128i orig = _mm_loadu_si128(vzz);
  _mm_storeu_si128(vzz, _mm_xor_si128(_mm_xor_si128(orig, all_lo),_mm_xor_si128(r1, r2)));
}

EXPORT uint32_t gf2p24_mul_pclmul_ct(uint32_t x, uint32_t y) {
  const __m128i MASK_LO = _mm_set1_epi8(0xf);
  const __m128i TIMES_B_HI = _mm_load_si128(TIMES_B+1); // TODO: make it really constant?
  const __m128i TIMES_B_LO = _mm_load_si128(TIMES_B+0); // TODO: make it really constant?
  const __m128i TIMES_TWO_HI = _mm_load_si128(TIMES_TWO+1); // TODO: make it really constant?
  const __m128i TIMES_TWO_LO = _mm_load_si128(TIMES_TWO+0); // TODO: make it really constant?
  // first multiply x by y, each input byte is set on even positions 0,2,4
  const __m128i xx = _mm_cvtepu8_epi16(_mm_set1_epi64x(x & 0x00FFFFFFu));  // the mask here are not be needed if inputs are clean
  const __m128i yy = _mm_cvtepu8_epi16(_mm_set1_epi64x(y & 0x00FFFFFFu));
  const __m128i prod = _mm_clmulepi64_si128(xx, yy, 0x00); // 4 first
  // multiply by b  (i.e. bprod = prod[even terms] ^ B * prod[odd_terms])
  const __m128i r1 = _mm_shuffle_epi8(TIMES_B_LO, _mm_and_si128(MASK_LO, prod));
  const __m128i r2 = _mm_shuffle_epi8(TIMES_B_HI, _mm_and_si128(MASK_LO, _mm_srli_epi16(prod, 4)));
  const __m128i bprod = _mm_xor_si128(prod, _mm_srli_epi16(_mm_xor_si128(r1, r2), 8));
  // multiply by 2 (i.e. final[0,2,4] = bprod[0,2,4] ^ 2 * bprod[6,8])
  const __m128i s1 = _mm_shuffle_epi8(TIMES_TWO_LO, _mm_and_si128(MASK_LO, bprod));
  const __m128i s2 = _mm_shuffle_epi8(TIMES_TWO_HI, _mm_and_si128(MASK_LO, _mm_srli_epi16(bprod, 4)));
  const __m128i final = _mm_xor_si128(bprod, _mm_srli_si128(_mm_xor_si128(s1, s2), 6)); // 6 is in bytes in this instr.!
  // store final[0,2,4] and discard all the other bytes
  uint32_t res[4];
  const __m128i POS = _mm_set_epi8(-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,4,2,0);
  _mm_storeu_si128((__m128i*) res, _mm_shuffle_epi8(final, POS));
  return res[0];
}

//TODO implement the 128x128? (maybe with avx2-vpclmul to avoid register starvation?)
