#ifndef BLOCK_IMPL_AVX2_H
#define BLOCK_IMPL_AVX2_H

#include <immintrin.h>
#include <wmmintrin.h>
#include <string.h>

typedef __m128i block128;
typedef __m256i block256;
typedef struct
{
	block128 data[3];
} block384;
typedef struct
{
	block256 data[2];
} block512;

// Unfortunately, there's no alternative version of these that works on integers.
#define shuffle_2x4xepi32(x, y, i) \
	_mm_castps_si128(_mm_shuffle_ps(_mm_castsi128_ps(x), _mm_castsi128_ps(y), i))
#define permute_8xepi32(x, i) \
	_mm256_castps_si256(_mm256_permute_ps(_mm256_castsi256_ps(x), i))
#define shuffle_2x4xepi64(x, y, i) \
	_mm256_castpd_si256(_mm256_shuffle_pd(_mm256_castsi256_pd(x), _mm256_castsi256_pd(y), i))

inline block128 block128_xor(block128 x, block128 y) { return _mm_xor_si128(x, y); }
inline block256 block256_xor(block256 x, block256 y) { return _mm256_xor_si256(x, y); }
inline block128 block128_and(block128 x, block128 y) { return _mm_and_si128(x, y); }
inline block256 block256_and(block256 x, block256 y) { return _mm256_and_si256(x, y); }
inline block128 block128_set_zero() { return _mm_setzero_si128(); }
inline block256 block256_set_zero() { return _mm256_setzero_si256(); }
inline block128 block128_set_all_8(uint8_t x) { return _mm_set1_epi8(x); }
inline block256 block256_set_all_8(uint8_t x) { return _mm256_set1_epi8(x); }
inline block128 block128_set_low32(uint32_t x) { return _mm_setr_epi32(x, 0, 0, 0); }
inline block256 block256_set_low32(uint32_t x) { return _mm256_setr_epi32(x, 0, 0, 0, 0, 0, 0, 0); }
inline block128 block128_set_low64(uint64_t x) { return _mm_set_epi64x(0, x); }
inline block256 block256_set_low64(uint64_t x) { return _mm256_setr_epi64x(x, 0, 0, 0); }
inline block256 block256_set_128(block128 x0, block128 x1) { return _mm256_setr_m128i(x0, x1); }

inline block256 block256_set_low128(block128 x)
{
	return _mm256_inserti128_si256(_mm256_setzero_si256(), x, 0);
}

inline block384 block384_xor(block384 x, block384 y)
{
	block384 out;
	out.data[0] = block128_xor(x.data[0], y.data[0]);
	out.data[1] = block128_xor(x.data[1], y.data[1]);
	out.data[2] = block128_xor(x.data[2], y.data[2]);
	return out;
}
inline block512 block512_xor(block512 x, block512 y)
{
	block512 out;
	out.data[0] = block256_xor(x.data[0], y.data[0]);
	out.data[1] = block256_xor(x.data[1], y.data[1]);
	return out;
}

inline block384 block384_and(block384 x, block384 y)
{
	block384 out;
	out.data[0] = block128_and(x.data[0], y.data[0]);
	out.data[1] = block128_and(x.data[1], y.data[1]);
	out.data[2] = block128_and(x.data[2], y.data[2]);
	return out;
}
inline block512 block512_and(block512 x, block512 y)
{
	block512 out;
	out.data[0] = block256_and(x.data[0], y.data[0]);
	out.data[1] = block256_and(x.data[1], y.data[1]);
	return out;
}

inline block384 block384_set_zero()
{
	block384 out;
	out.data[0] = block128_set_zero();
	out.data[1] = block128_set_zero();
	out.data[2] = block128_set_zero();
	return out;
}
inline block512 block512_set_zero()
{
	block512 out;
	out.data[0] = block256_set_zero();
	out.data[1] = block256_set_zero();
	return out;
}

inline block384 block384_set_all_8(uint8_t x)
{
	block384 out;
	out.data[0] = block128_set_all_8(x);
	out.data[1] = block128_set_all_8(x);
	out.data[2] = block128_set_all_8(x);
	return out;
}
inline block512 block512_set_all_8(uint8_t x)
{
	block512 out;
	out.data[0] = block256_set_all_8(x);
	out.data[1] = block256_set_all_8(x);
	return out;
}

inline block384 block384_set_low32(uint32_t x)
{
	block384 out;
	out.data[0] = block128_set_low32(x);
	return out;
}
inline block512 block512_set_low32(uint32_t x)
{
	block512 out;
	out.data[0] = block256_set_low32(x);
	return out;
}
inline block384 block384_set_low64(uint64_t x)
{
	block384 out;
	out.data[0] = block128_set_low64(x);
	return out;
}
inline block512 block512_set_low64(uint64_t x)
{
	block512 out;
	out.data[0] = block256_set_low64(x);
	return out;
}

inline bool block128_any_zeros(block128 x)
{
	return _mm_movemask_epi8(_mm_cmpeq_epi8(x, _mm_setzero_si128()));
}

inline bool block256_any_zeros(block256 x)
{
	return _mm256_movemask_epi8(_mm256_cmpeq_epi8(x, _mm256_setzero_si256()));
}

inline bool block192_any_zeros(block192 x)
{
	block256 b = block256_set_zero();
	memcpy(&b, &x, sizeof(x));
	return _mm256_movemask_epi8(_mm256_cmpeq_epi8(b, _mm256_setzero_si256())) & 0x00ffffff;
}

inline block128 block128_byte_reverse(block128 x)
{
	block128 shuffle = _mm_setr_epi8(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);
	return _mm_shuffle_epi8(x, shuffle);
}

inline block256 block256_from_2_block128(block128 x, block128 y)
{
	return _mm256_setr_m128i(x, y);
}

#define VOLE_BLOCK_SHIFT 0
typedef block128 vole_block;
inline vole_block vole_block_set_zero() { return block128_set_zero(); }
inline vole_block vole_block_xor(vole_block x, vole_block y) { return block128_xor(x, y); }
inline vole_block vole_block_and(vole_block x, vole_block y) { return block128_and(x, y); }
inline vole_block vole_block_set_all_8(uint8_t x) { return block128_set_all_8(x); }
inline vole_block vole_block_set_low32(uint32_t x) { return block128_set_low32(x); }
inline vole_block vole_block_set_low64(uint64_t x) { return block128_set_low64(x); }

#ifndef HAVE_VCLMUL
#define POLY_VEC_LEN_SHIFT 0
typedef block128 clmul_block;
inline clmul_block clmul_block_xor(clmul_block x, clmul_block y) { return block128_xor(x, y); }
inline clmul_block clmul_block_and(clmul_block x, clmul_block y) { return block128_and(x, y); }
inline clmul_block clmul_block_set_all_8(uint8_t x) { return block128_set_all_8(x); }
inline clmul_block clmul_block_set_zero() { return block128_set_zero(); }

inline clmul_block clmul_block_clmul_ll(clmul_block x, clmul_block y)
{
	return _mm_clmulepi64_si128(x, y, 0x00);
}
inline clmul_block clmul_block_clmul_lh(clmul_block x, clmul_block y)
{
	return _mm_clmulepi64_si128(x, y, 0x10);
}
inline clmul_block clmul_block_clmul_hl(clmul_block x, clmul_block y)
{
	return _mm_clmulepi64_si128(x, y, 0x01);
}
inline clmul_block clmul_block_clmul_hh(clmul_block x, clmul_block y)
{
	return _mm_clmulepi64_si128(x, y, 0x11);
}

inline clmul_block clmul_block_shift_left_64(clmul_block x)
{
	return _mm_slli_si128(x, 8);
}
inline clmul_block clmul_block_shift_right_64(clmul_block x)
{
	return _mm_srli_si128(x, 8);
}
inline clmul_block clmul_block_mix_64(clmul_block x, clmul_block y) // output = y high, x low.
{
	return _mm_alignr_epi8(x, y, 8);
}
inline clmul_block clmul_block_broadcast_low64(clmul_block x)
{
	return _mm_broadcastq_epi64(x);
}

#else
#define POLY_VEC_LEN_SHIFT 1
typedef block256 clmul_block;
inline clmul_block clmul_block_xor(clmul_block x, clmul_block y) { return block256_xor(x, y); }
inline clmul_block clmul_block_and(clmul_block x, clmul_block y) { return block256_and(x, y); }
inline clmul_block clmul_block_set_all_8(uint8_t x) { return block256_set_all_8(x); }
inline clmul_block clmul_block_set_zero() { return block256_set_zero(); }

inline clmul_block clmul_block_clmul_ll(clmul_block x, clmul_block y)
{
	return _mm256_clmulepi64_epi128(x, y, 0x00);
}
inline clmul_block clmul_block_clmul_lh(clmul_block x, clmul_block y)
{
	return _mm256_clmulepi64_epi128(x, y, 0x10);
}
inline clmul_block clmul_block_clmul_hl(clmul_block x, clmul_block y)
{
	return _mm256_clmulepi64_epi128(x, y, 0x01);
}
inline clmul_block clmul_block_clmul_hh(clmul_block x, clmul_block y)
{
	return _mm256_clmulepi64_epi128(x, y, 0x11);
}

inline clmul_block clmul_block_shift_left_64(clmul_block x)
{
	return _mm256_slli_si256(x, 8);
}
inline clmul_block clmul_block_shift_right_64(clmul_block x)
{
	return _mm256_srli_si256(x, 8);
}
inline clmul_block clmul_block_mix_64(clmul_block x, clmul_block y) // output = y high, x low.
{
	return _mm256_alignr_epi8(x, y, 8);
}
inline clmul_block clmul_block_broadcast_low64(clmul_block x)
{
	return _mm256_shuffle_epi32(x, 0x44);
}
#endif

#endif
