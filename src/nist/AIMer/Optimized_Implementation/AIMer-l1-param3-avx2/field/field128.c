// -----------------------------------------------------------------------------
// File Name   : field128.c
// Description : 
// SPDX-License-Identifier: MIT
// -----------------------------------------------------------------------------

#include "field128.h"
#include "portable_endian.h"
#include <immintrin.h>

void poly128_sqr(const __m128i a, __m128i c[2]);
void poly128_mul(const __m128i a, const __m128i b, __m128i c[2]);
void GF2_128_rdc(const __m128i a[2], __m128i c[1]);

void GF2_128_set0(GF2_128 a)
{
  a[0] = 0;
  a[1] = 0;
}

void GF2_128_to_bytes(const GF2_128 in, uint8_t* out)
{
  uint64_t temp = htole64(in[0]);
  size_t num_bytes = sizeof(uint64_t);

  memcpy(out, (uint8_t*)(&temp), num_bytes);
  temp = htole64(in[1]);
  memcpy(out + num_bytes, (uint8_t*)(&temp), num_bytes);
}

void GF2_128_from_bytes(const uint8_t* in, GF2_128 out)
{
  uint64_t temp;
  size_t num_bytes = sizeof(uint64_t);

  memcpy((uint8_t*)(&temp), in, num_bytes);
  out[0] = le64toh(temp);
  memcpy((uint8_t*)(&temp), in + num_bytes, num_bytes);
  out[1] = le64toh(temp);
}

void GF2_128_copy(const GF2_128 in, GF2_128 out)
{
  out[0] = in[0];
  out[1] = in[1];
}

void GF2_128_add(const GF2_128 a, const GF2_128 b, GF2_128 c)
{
  __m128i temp[2];

  temp[0] = _mm_loadu_si128((const __m128i*)a);
  temp[1] = _mm_loadu_si128((const __m128i*)b);

  temp[0] = _mm_xor_si128(temp[0], temp[1]);
  _mm_storeu_si128((__m128i*)c, temp[0]);
}

void poly128_sqr(const __m128i a, __m128i c[2])
{
  __m128i temp[2];
  __m128i table = _mm_set_epi64x(0x5554515045444140, 0x1514111005040100);
  __m128i mask  = _mm_set_epi64x(0x0F0F0F0F0F0F0F0F, 0x0F0F0F0F0F0F0F0F);

  temp[0] = _mm_and_si128(a, mask);
  temp[1] = _mm_srli_epi64(a, 4);
  temp[1] = _mm_and_si128(temp[1], mask);

  temp[0] = _mm_shuffle_epi8(table, temp[0]);
  temp[1] = _mm_shuffle_epi8(table, temp[1]);

  c[0] = _mm_unpacklo_epi8(temp[0], temp[1]);
  c[1] = _mm_unpackhi_epi8(temp[0], temp[1]);
}

void poly128_mul(const __m128i a, const __m128i b, __m128i c[2])
{
  __m128i temp[3];

  c[0] = _mm_clmulepi64_si128(a, b, 0x00);
  c[1] = _mm_clmulepi64_si128(a, b, 0x11);

  temp[0] = _mm_clmulepi64_si128(a, b, 0x01);
  temp[1] = _mm_clmulepi64_si128(a, b, 0x10);

  temp[0] = _mm_xor_si128(temp[0], temp[1]);
  temp[1] = _mm_slli_si128(temp[0], 8);
  temp[2] = _mm_srli_si128(temp[0], 8);

  c[0] = _mm_xor_si128(c[0], temp[1]);
  c[1] = _mm_xor_si128(c[1], temp[2]);
}

void GF2_128_rdc(const __m128i a[2], __m128i c[1])
{
  // irreducible polynomial f(x) = x^128 + x^7 + x^2 + x + 1
  __m128i irr = _mm_set_epi64x(0x0, 0x87);
  __m128i temp[3];

  temp[0] = _mm_clmulepi64_si128(a[1], irr, 0x01);
  temp[1] = _mm_slli_si128(temp[0], 8);
  temp[2] = _mm_srli_si128(temp[0], 8);
  temp[2] = _mm_xor_si128(temp[2], a[1]);

  temp[0] = _mm_clmulepi64_si128(temp[2], irr, 0x00);
  c[0] = _mm_xor_si128(a[0], temp[0]);
  c[0] = _mm_xor_si128(c[0], temp[1]);
}

void GF2_128_mul(const GF2_128 a, const GF2_128 b, GF2_128 c)
{
  __m128i temp[2];
  __m128i intermediate[2] = {_mm_setzero_si128(), _mm_setzero_si128()};

  temp[0] = _mm_loadu_si128((const __m128i*)a);
  temp[1] = _mm_loadu_si128((const __m128i*)b);

  poly128_mul(temp[0], temp[1], intermediate);
  GF2_128_rdc(intermediate, &temp[0]);

  _mm_storeu_si128((__m128i*)c, temp[0]);
}

void GF2_128_sqr(const GF2_128 a, GF2_128 c)
{
  __m128i temp;
  __m128i intermediate[2] = {_mm_setzero_si128(), _mm_setzero_si128()};

  temp = _mm_loadu_si128((const __m128i*)a);

  poly128_sqr(temp, intermediate);
  GF2_128_rdc(intermediate, &temp);

  _mm_storeu_si128((__m128i*)c, temp);
}

void GF2_128_transposed_matmul(const GF2_128 a, const GF2_128* b, GF2_128 c)
{
  unsigned int i, j;
  
  const uint32_t* a_ptr = (uint32_t *)a;

  __m256i temp[2] = {_mm256_setzero_si256(), _mm256_setzero_si256()};
  __m256i matrix_data[4];

  for (i = 0; i < 4; i++)
  {
    const __m256i shift = _mm256_set_epi32(0,2,4,6,1,3,5,7);
    __m256i index = _mm256_set1_epi32(a_ptr[i]);
    for (j = 32*(i+1); j > 32*i; j -= 8)
    {
      __m256i mask = _mm256_sllv_epi32(index, shift);
      matrix_data[0] = _mm256_loadu_si256((const __m256i*)&b[j-2]);
      matrix_data[1] = _mm256_loadu_si256((const __m256i*)&b[j-4]);
      matrix_data[2] = _mm256_loadu_si256((const __m256i*)&b[j-6]);
      matrix_data[3] = _mm256_loadu_si256((const __m256i*)&b[j-8]);

      temp[0] = _mm256_xor_si256(temp[0],
        _mm256_and_si256(matrix_data[0],
          _mm256_srai_epi32(_mm256_shuffle_epi32(mask, 0b11111111), 31)));
      temp[1] = _mm256_xor_si256(temp[1],
        _mm256_and_si256(matrix_data[1],
          _mm256_srai_epi32(_mm256_shuffle_epi32(mask, 0b10101010), 31)));
      temp[0] = _mm256_xor_si256(temp[0],
        _mm256_and_si256(matrix_data[2],
          _mm256_srai_epi32(_mm256_shuffle_epi32(mask, 0b01010101), 31)));
      temp[1] = _mm256_xor_si256(temp[1],
        _mm256_and_si256(matrix_data[3],
          _mm256_srai_epi32(_mm256_shuffle_epi32(mask, 0b00000000), 31)));
      index = _mm256_slli_epi32(index, 8);
    }
  }

  temp[0] = _mm256_xor_si256(temp[0], temp[1]);

  _mm_storeu_si128((__m128i*)c,
                  _mm_xor_si128(_mm256_extracti128_si256(temp[0], 0),
                  _mm256_extracti128_si256(temp[0], 1)));
}
