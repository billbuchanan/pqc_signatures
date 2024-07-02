// -----------------------------------------------------------------------------
// File Name   : field192.c
// Description : 
// SPDX-License-Identifier: MIT
// -----------------------------------------------------------------------------

#include "field192.h"
#include "portable_endian.h"
#include <immintrin.h>

void poly192_sqr(const __m128i a[2], __m128i c[3]);
void poly192_mul(const __m128i a[2], const __m128i b[2], __m128i c[3]);
void GF2_192_rdc(const __m128i a[3], __m128i c[2]);
inline __m256i m256_compute_mask(const uint64_t index, const size_t bit);

void GF2_192_set0(GF2_192 a)
{
  a[0] = 0;
  a[1] = 0;
  a[2] = 0;
  a[3] = 0;
}

void GF2_192_to_bytes(const GF2_192 in, uint8_t* out)
{
  uint64_t temp = htole64(in[0]);
  size_t num_bytes = sizeof(uint64_t);

  memcpy(out, (uint8_t*)(&temp), num_bytes);
  temp = htole64(in[1]);
  memcpy(out + num_bytes, (uint8_t*)(&temp), num_bytes);
  temp = htole64(in[2]);
  memcpy(out + 2 * num_bytes, (uint8_t*)(&temp), num_bytes);
}

void GF2_192_from_bytes(const uint8_t* in, GF2_192 out)
{
  uint64_t temp;
  size_t num_bytes = sizeof(uint64_t);

  memcpy((uint8_t*)(&temp), in, num_bytes);
  out[0] = le64toh(temp);
  memcpy((uint8_t*)(&temp), in + num_bytes, num_bytes);
  out[1] = le64toh(temp);
  memcpy((uint8_t*)(&temp), in + 2 * num_bytes, num_bytes);
  out[2] = le64toh(temp);
  out[3] = 0;
}

void GF2_192_copy(const GF2_192 in, GF2_192 out)
{
  out[0] = in[0];
  out[1] = in[1];
  out[2] = in[2];
  out[3] = 0;
}

void GF2_192_add(const GF2_192 a, const GF2_192 b, GF2_192 c)
{
  __m256i temp[2];

  temp[0] = _mm256_loadu_si256((const __m256i*)a);
  temp[1] = _mm256_loadu_si256((const __m256i*)b);

  temp[0] = _mm256_xor_si256(temp[0], temp[1]);
  _mm256_storeu_si256((__m256i*)c, temp[0]);
}

void poly192_sqr(const __m128i a[2], __m128i c[3])
{
  __m128i temp[2];
  __m128i table = _mm_set_epi64x(0x5554515045444140, 0x1514111005040100);
  __m128i mask  = _mm_set_epi64x(0x0F0F0F0F0F0F0F0F, 0x0F0F0F0F0F0F0F0F);

  temp[0] = _mm_and_si128(a[0], mask);
  temp[1] = _mm_srli_epi64(a[0], 4);
  temp[1] = _mm_and_si128(temp[1], mask);
  temp[0] = _mm_shuffle_epi8(table, temp[0]);
  temp[1] = _mm_shuffle_epi8(table, temp[1]);

  c[0] = _mm_unpacklo_epi8(temp[0], temp[1]);
  c[1] = _mm_unpackhi_epi8(temp[0], temp[1]);

  temp[0] = _mm_and_si128(a[1], mask);
  temp[1] = _mm_srli_epi64(a[1], 4);
  temp[1] = _mm_and_si128(temp[1], mask);
  temp[0] = _mm_shuffle_epi8(table, temp[0]);
  temp[1] = _mm_shuffle_epi8(table, temp[1]);

  c[2] = _mm_unpacklo_epi8(temp[0], temp[1]);
}

void poly192_mul(const __m128i a[2], const __m128i b[2], __m128i c[3])
{
  __m128i temp[3];

  c[0] = _mm_clmulepi64_si128(a[0], b[0], 0x00);
  c[1] = _mm_clmulepi64_si128(a[0], b[0], 0x11);
  c[2] = _mm_clmulepi64_si128(a[1], b[1], 0x00);
  c[1] = _mm_xor_si128(c[1], _mm_clmulepi64_si128(a[0], b[1], 0x00));
  c[1] = _mm_xor_si128(c[1], _mm_clmulepi64_si128(a[1], b[0], 0x00));

  temp[0] = _mm_clmulepi64_si128(a[0], b[0], 0x01);
  temp[1] = _mm_clmulepi64_si128(a[0], b[0], 0x10);

  temp[0] = _mm_xor_si128(temp[0], temp[1]);
  temp[1] = _mm_slli_si128(temp[0], 8);
  temp[2] = _mm_srli_si128(temp[0], 8);

  c[0] = _mm_xor_si128(c[0], temp[1]);
  c[1] = _mm_xor_si128(c[1], temp[2]);

  temp[0] = _mm_clmulepi64_si128(a[1], b[0], 0x10);
  temp[1] = _mm_clmulepi64_si128(a[0], b[1], 0x01);

  temp[0] = _mm_xor_si128(temp[0], temp[1]);
  temp[1] = _mm_slli_si128(temp[0], 8);
  temp[2] = _mm_srli_si128(temp[0], 8);

  c[1] = _mm_xor_si128(c[1], temp[1]);
  c[2] = _mm_xor_si128(c[2], temp[2]);
}

void GF2_192_rdc(const __m128i a[3], __m128i c[2])
{
  // irreducible polynomial f(x) = x^192 + x^7 + x^2 + x + 1
  __m128i irr = _mm_set_epi64x(0x0, 0x87);
  __m128i temp[4];

  temp[0] = _mm_clmulepi64_si128(a[2], irr, 0x01);
  temp[3] = _mm_xor_si128(a[1], temp[0]);

  temp[0] = _mm_clmulepi64_si128(a[2], irr, 0x00);
  temp[1] = _mm_slli_si128(temp[0], 8);
  temp[2] = _mm_srli_si128(temp[0], 8);
  temp[3] = _mm_xor_si128(temp[3], temp[2]);

  temp[0] = _mm_clmulepi64_si128(temp[3], irr, 0x01);
  c[0] = _mm_xor_si128(temp[0], a[0]);
  c[0] = _mm_xor_si128(c[0], temp[1]);
  c[1] = _mm_and_si128(temp[3], _mm_set_epi64x(0x0, 0xFFFFFFFFFFFFFFFF));
}

void GF2_192_mul(const GF2_192 a, const GF2_192 b, GF2_192 c)
{
  __m128i temp_a[2], temp_b[2];
  __m128i intermediate[3] = {_mm_setzero_si128(), _mm_setzero_si128(), 
                             _mm_setzero_si128()};

  temp_a[0] = _mm_loadu_si128((const __m128i*)&a[0]);
  temp_a[1] = _mm_loadu_si128((const __m128i*)&a[2]);
  temp_b[0] = _mm_loadu_si128((const __m128i*)&b[0]);
  temp_b[1] = _mm_loadu_si128((const __m128i*)&b[2]);

  poly192_mul(temp_a, temp_b, intermediate);
  GF2_192_rdc(intermediate, temp_a);

  _mm_storeu_si128((__m128i*)&c[0], temp_a[0]);
  _mm_storeu_si128((__m128i*)&c[2], temp_a[1]);
}

void GF2_192_sqr(const GF2_192 a, GF2_192 c)
{
  __m128i temp_a[2];
  __m128i intermediate[3] = {_mm_setzero_si128(), _mm_setzero_si128(), 
                             _mm_setzero_si128()};

  temp_a[0] = _mm_loadu_si128((const __m128i*)&a[0]);
  temp_a[1] = _mm_loadu_si128((const __m128i*)&a[2]);

  poly192_sqr(temp_a, intermediate);
  GF2_192_rdc(intermediate, temp_a);

  _mm_storeu_si128((__m128i*)&c[0], temp_a[0]);
  _mm_storeu_si128((__m128i*)&c[2], temp_a[1]);
}

inline __m256i m256_compute_mask(const uint64_t index, const size_t bit)
{
  return _mm256_set1_epi64x(-((index >> bit) & 1));
}

void GF2_192_transposed_matmul(const GF2_192 a, const GF2_192* b, GF2_192 c)
{
  unsigned int i, j;
  const uint64_t* a_ptr = a;
  const GF2_192* b_ptr = b;
  __m256i matrix_data[4];

  __m256i temp[2] = {_mm256_setzero_si256(), _mm256_setzero_si256()};
  for (i = NUMWORDS_FIELD; i; --i, ++a_ptr)
  {
    uint64_t index = *a_ptr;
    for (j = NUMBITS_WORD; j; j -= 4, index >>= 4, b_ptr += 4)
    {
      matrix_data[0] = _mm256_loadu_si256((const __m256i*)&b_ptr[0]);
      matrix_data[1] = _mm256_loadu_si256((const __m256i*)&b_ptr[1]);
      matrix_data[2] = _mm256_loadu_si256((const __m256i*)&b_ptr[2]);
      matrix_data[3] = _mm256_loadu_si256((const __m256i*)&b_ptr[3]);

      temp[0] = _mm256_xor_si256(temp[0],
        _mm256_and_si256(matrix_data[0], m256_compute_mask(index, 0)));
      temp[1] = _mm256_xor_si256(temp[1],
        _mm256_and_si256(matrix_data[1], m256_compute_mask(index, 1)));
      temp[0] = _mm256_xor_si256(temp[0],
        _mm256_and_si256(matrix_data[2], m256_compute_mask(index, 2)));
      temp[1] = _mm256_xor_si256(temp[1],
        _mm256_and_si256(matrix_data[3], m256_compute_mask(index, 3)));
    }
  }

  _mm256_storeu_si256((__m256i*)c, _mm256_xor_si256(temp[0], temp[1]));
}
