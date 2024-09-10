#ifndef POLYNOMIALS_IMPL_H
#define POLYNOMIALS_IMPL_H

#include <inttypes.h>
#include <string.h>
#include <immintrin.h>
#include <wmmintrin.h>

#include "transpose.h"

// The single bit in each polynomial is duplicated for a whole byte.
#if POLY_VEC_LEN == 1
typedef uint8_t poly1_vec;
#elif POLY_VEC_LEN == 2
typedef uint16_t poly1_vec;
#elif POLY_VEC_LEN == 4
typedef uint32_t poly1_vec;
#endif

// This is twice as long as it needs to be. It is treated as a poly128_vec, with the high half of
// each 128-bit polynomial ignored.
typedef clmul_block poly64_vec;

typedef clmul_block poly128_vec;
typedef struct
{
	poly128_vec data[2]; // Striped in 128-bit chunks. Highest 64 bits of each poly are ignored.
} poly192_vec;
typedef struct
{
	poly128_vec data[2]; // Striped in 128-bit chunks.
} poly256_vec;
typedef struct
{
	poly128_vec data[3]; // Striped in 128-bit chunks. Highest 64 bits of each poly are ignored.
} poly320_vec;
typedef struct
{
	poly128_vec data[3]; // Striped in 128-bit chunks.
} poly384_vec;
typedef struct
{
	poly128_vec data[4]; // Striped in 128-bit chunks.
} poly512_vec;

inline poly1_vec poly1_set_all(uint8_t x)
{
#if POLY_VEC_LEN == 1
	return x;
#elif POLY_VEC_LEN == 2
	uint16_t out = x;
	out += (out << 8);
	return out;
#endif
}

inline poly1_vec poly1_load(unsigned long x, unsigned int bit_offset)
{
	x = (x >> bit_offset) & ((1 << POLY_VEC_LEN) - 1);

	// Split each bit out into its own byte.
	poly1_vec poly = x * (((1UL << 7 * POLY_VEC_LEN) - 1) / 0x7f);
	poly &= ((1UL << 8 * POLY_VEC_LEN) - 1) / 0xff;
	poly *= 0xff;
	return poly;
}

inline poly1_vec poly1_load_offset8(const void* s, unsigned int bit_offset)
{
	poly1_vec poly;
	memcpy(&poly, s, sizeof(poly));
	poly >>= bit_offset;
	poly &= ((1UL << 8 * POLY_VEC_LEN) - 1) / 0xff;
	poly *= 0xff;
	return poly;
}

inline poly64_vec poly64_load(const void* s)
{
#if POLY_VEC_LEN == 1
	uint64_t in;
#elif POLY_VEC_LEN == 2
	block128 in;
#endif
	memcpy(&in, s, sizeof(in));

	poly64_vec out;
#if POLY_VEC_LEN == 1
	out = _mm_cvtsi64_si128(in);
#elif POLY_VEC_LEN == 2
	out = _mm256_inserti128_si256(_mm256_setzero_si256(), in, 0);
	out = transpose2x2_64(out);
#endif

	return out;
}

inline poly128_vec poly128_load(const void* s)
{
	poly128_vec out;
	memcpy(&out, s, sizeof(out));
	return out;
}

inline poly192_vec poly192_load(const void* s)
{
	poly192_vec out;

#if POLY_VEC_LEN == 1
	memcpy(&out.data[0], s, sizeof(block128));
	out.data[1] = _mm_loadu_si64(((char*) s) + sizeof(block128));

#elif POLY_VEC_LEN == 2
	block256 a0a1a2b0;
	block128 b1b2;
	memcpy(&a0a1a2b0, p, sizeof(block256));
	memcpy(&b1b2, ((unsigned char*) p) + sizeof(block256), sizeof(block128));

	block256 b1b2a2b0 = _mm256_blend_epi32(a0a1a2b0, block256_set_low128(b1b2), 0x0f);
	block256 a2b0b2b1 = _mm256_permute4x64_epi64(b1b2a2b0, 0x1e);
	block256 a0a2b0b1 = shuffle_2x4xepi64(a0a1a2b0, a2b0b2b1, 0x0c);
	block256 a0a1b0b1 = _mm256_blend_epi32(a0a1a2b0, a0a2b0b1, 0xf0);
	out.data[0] = a0a1b0b1;
	out.data[1] = a2b0b2b1;
#endif

	return out;
}

inline poly256_vec poly256_load(const void* s)
{
	poly256_vec out;

#if POLY_VEC_LEN == 1
	memcpy(&out, s, sizeof(out));

#elif POLY_VEC_LEN == 2
	block256 in[2];
	memcpy(&in[0], s, sizeof(in));
	transpose2x2_128(&out.data[0], in[0], in[1]);
#endif

	return out;
}

inline poly320_vec poly320_load(const void* s)
{
	poly320_vec out;

#if POLY_VEC_LEN == 1
	memcpy(&out, s, 40);
	return out;
#elif POLY_VEC_LEN == 2
#error "not implemented"
#endif
}

inline poly384_vec poly384_load(const void* s)
{
	poly384_vec out;

#if POLY_VEC_LEN == 1
	memcpy(&out, s, sizeof(out));
	return out;
#elif POLY_VEC_LEN == 2
#error "not implemented"
#endif
}

inline poly512_vec poly512_load(const void* s)
{
	poly512_vec out;

#if POLY_VEC_LEN == 1
	memcpy(&out, s, sizeof(out));

#elif POLY_VEC_LEN == 2
	block256 in[4];
	memcpy(&in[0], s, sizeof(in));
	transpose2x2_128(&out.data[0], in[0], in[2]);
	transpose2x2_128(&out.data[2], in[1], in[3]);
#endif

	return out;
}

inline poly64_vec poly64_load_dup(const void* s)
{
	uint64_t in;
	memcpy(&in, s, sizeof(in));

#if POLY_VEC_LEN == 1
	return _mm_cvtsi64_si128(in);
#elif POLY_VEC_LEN == 2
	block128 x = _mm_cvtsi64_si128(in);
	return _mm256_inserti128_si256(_mm256_castsi128_si256(x), x, 1);
#endif
}

inline poly128_vec poly128_load_dup(const void* s)
{
	block128 input;
	memcpy(&input, s, sizeof(input));

#if POLY_VEC_LEN == 1
	return input;
#elif POLY_VEC_LEN == 2
	return _mm256_inserti128_si256(_mm256_castsi128_si256(x), x, 1);
#endif
}

inline poly192_vec poly192_load_dup(const void* s)
{
	block128 in[2];

	memcpy(&in[0], s, sizeof(block128));
	in[1] = _mm_loadu_si64(((char*) s) + sizeof(block128));

	poly192_vec out;
#if POLY_VEC_LEN == 1
	out.data[0] = in[0];
	out.data[1] = in[1];

#elif POLY_VEC_LEN == 2
	out.data[0] = _mm256_inserti128_si256(_mm256_castsi128_si256(in[0]), in[0], 1);
	out.data[1] = _mm256_inserti128_si256(_mm256_castsi128_si256(in[1]), in[1], 1);
#endif

	return out;
}

inline poly256_vec poly256_load_dup(const void* s)
{
	block128 in[2];

	memcpy(&in[0], s, 2 * sizeof(block128));

	poly256_vec out;
#if POLY_VEC_LEN == 1
	out.data[0] = in[0];
	out.data[1] = in[1];

#elif POLY_VEC_LEN == 2
	out.data[0] = _mm256_inserti128_si256(_mm256_castsi128_si256(in[0]), in[0], 1);
	out.data[1] = _mm256_inserti128_si256(_mm256_castsi128_si256(in[1]), in[1], 1);
#endif

	return out;
}

inline void poly64_store(void* d, poly64_vec s)
{
#if POLY_VEC_LEN == 1
	uint64_t out = _mm_cvtsi128_si64(s);
#elif POLY_VEC_LEN == 2
	__m128i out = _mm256_castsi256_si128(transpose2x2_64(s));
#endif

	memcpy(d, &out, sizeof(out));
}

inline void poly128_store(void* d, poly128_vec s)
{
	memcpy(d, &s, sizeof(s));
}

inline void poly192_store(void* d, poly192_vec s)
{
#if POLY_VEC_LEN == 1
	memcpy(d, &s, 24);

#elif POLY_VEC_LEN == 2
	// s.data[0] = a0a1b0b1
	// s.data[1] = a2  b2
	block256 a0b0b1b1 = _mm256_permute4x64_epi64(s.data[0], 0xf8);
	block256 a2a2b2b2 = permute_8xepi32(s.data[1], 0x44);
	block256 a2b0b1b2 = _mm256_blend_epi32(a0b0b1b1, a2a2b2b2, 0xc3);

	memcpy(p, &s.data[0], sizeof(block128));
	memcpy(((unsigned char*) p) + sizeof(block128), &a2b0b1b2, sizeof(block256));
#endif
}

inline void poly256_store(void* d, poly256_vec s)
{
#if POLY_VEC_LEN == 1
	memcpy(d, &s, sizeof(s));

#elif POLY_VEC_LEN == 2
	block256 out[2];
	transpose2x2_128(&out[0], s.data[0], s.data[1]);
	memcpy(d, &out[0], sizeof(out));
#endif
}

inline void poly320_store(void* d, poly320_vec s)
{
#if POLY_VEC_LEN == 1
	memcpy(d, &s, 40);
#elif POLY_VEC_LEN == 2
#error "not implemented"
#endif
}

inline void poly384_store(void* d, poly384_vec s)
{
#if POLY_VEC_LEN == 1
	memcpy(d, &s, sizeof(s));
#elif POLY_VEC_LEN == 2
#error "not implemented"
#endif
}

inline void poly512_store(void* d, poly512_vec s)
{
#if POLY_VEC_LEN == 1
	memcpy(d, &s, sizeof(s));

#elif POLY_VEC_LEN == 2
	block256 out[4];
	transpose2x2_128(&out[0], s.data[0], s.data[1]);
	block256 tmp = out[1];
	out[1] = out[2];
	out[2] = tmp;
	memcpy(d, &out[0], sizeof(out));
#endif
}

inline void poly128_store1(void* d, poly128_vec s)
{
	memcpy(d, &s, 16);
}

inline void poly192_store1(void* d, poly192_vec s)
{
	memcpy(d, &s.data[0], 16);
	memcpy(((unsigned char*) d) + 16, &s.data[1], 8);
}

inline void poly256_store1(void* d, poly256_vec s)
{
	memcpy(d, &s.data[0], 16);
	memcpy(((unsigned char*) d) + 16, &s.data[1], 16);
}

inline poly128_vec poly128_from_64(poly64_vec x)
{
	// set the unused bits to zero
#if POLY_VEC_LEN == 1
	return _mm_insert_epi64(x, 0, 1);
#elif POLY_VEC_LEN == 2
	return _mm256_insert_epi64(_mm256_insert_epi64(x, 0, 1), 0, 3);
#endif
}

inline poly192_vec poly192_from_128(poly128_vec x)
{
	poly192_vec out;
	out.data[0] = x;
	out.data[1] = clmul_block_set_zero();
	return out;
}

inline poly256_vec poly256_from_128(poly128_vec x)
{
	poly256_vec out;
	out.data[0] = x;
	out.data[1] = clmul_block_set_zero();
	return out;
}

inline poly256_vec poly256_from_192(poly192_vec x)
{
	poly256_vec out;
	out.data[0] = x.data[0];
#if POLY_VEC_LEN == 1
	out.data[1] = _mm_insert_epi64(x.data[1], 0, 1);
#elif POLY_VEC_LEN == 2
	out.data[1] =  _mm256_insert_epi64(_mm256_insert_epi64(x.data[1], 0, 1), 0, 3);
#endif
	return out;
}

inline poly384_vec poly384_from_192(poly192_vec x)
{
	poly384_vec out;
	out.data[0] = x.data[0];
#if POLY_VEC_LEN == 1
	out.data[1] = _mm_insert_epi64(x.data[1], 0, 1);
#elif POLY_VEC_LEN == 2
	out.data[1] =  _mm256_insert_epi64(_mm256_insert_epi64(x.data[1], 0, 1), 0, 3);
#endif
	out.data[2] = clmul_block_set_zero();
	return out;
}

inline poly320_vec poly320_from_256(poly256_vec x)
{
	poly320_vec out;
	out.data[0] = x.data[0];
	out.data[1] = x.data[1];
	out.data[2] = clmul_block_set_zero();
	return out;
}

inline poly512_vec poly512_from_256(poly256_vec x)
{
	poly512_vec out;
	out.data[0] = x.data[0];
	out.data[1] = x.data[1];
	out.data[2] = clmul_block_set_zero();
	out.data[3] = clmul_block_set_zero();
	return out;
}

inline void add_clmul_block_vectors(clmul_block* x, const clmul_block* y, size_t n)
{
	for (size_t i = 0; i < n; ++i)
		x[i] = clmul_block_xor(x[i], y[i]);
}

inline poly64_vec poly64_add(poly64_vec x, poly64_vec y)
{
	return clmul_block_xor(x, y);
}
inline poly128_vec poly128_add(poly128_vec x, poly128_vec y)
{
	return clmul_block_xor(x, y);
}
inline poly192_vec poly192_add(poly192_vec x, poly192_vec y)
{
	add_clmul_block_vectors(&x.data[0], &y.data[0], 2);
	return x;
}
inline poly256_vec poly256_add(poly256_vec x, poly256_vec y)
{
	add_clmul_block_vectors(&x.data[0], &y.data[0], 2);
	return x;
}
inline poly320_vec poly320_add(poly320_vec x, poly320_vec y)
{
	add_clmul_block_vectors(&x.data[0], &y.data[0], 3);
	return x;
}
inline poly384_vec poly384_add(poly384_vec x, poly384_vec y)
{
	add_clmul_block_vectors(&x.data[0], &y.data[0], 3);
	return x;
}
inline poly512_vec poly512_add(poly512_vec x, poly512_vec y)
{
	add_clmul_block_vectors(&x.data[0], &y.data[0], 4);
	return x;
}

inline poly128_vec poly64_mul(poly64_vec x, poly64_vec y)
{
	return clmul_block_clmul_ll(x, y);
}

// Karatsuba multiplication, but end right after the multiplications, and use a different pair of
// vectors as the inputs for the sum of x and the sum of y.
inline void karatsuba_mul_128_uninterpolated_other_sum(
	poly128_vec x, poly128_vec y, poly128_vec x_for_sum, poly128_vec y_for_sum, poly128_vec* out)
{
	poly128_vec x0y0 = clmul_block_clmul_ll(x, y);
	poly128_vec x1y1 = clmul_block_clmul_hh(x, y);
	clmul_block x1_cat_y0 = clmul_block_mix_64(y_for_sum, x_for_sum);
	clmul_block xsum = clmul_block_xor(x_for_sum, x1_cat_y0); // Result in low.
	clmul_block ysum = clmul_block_xor(y_for_sum, x1_cat_y0); // Result in high.
	poly128_vec xsum_ysum = clmul_block_clmul_lh(xsum, ysum);

	out[0] = x0y0;
	out[1] = xsum_ysum;
	out[2] = x1y1;
}

// Karatsuba multiplication, but end right after the multiplications.
inline void karatsuba_mul_128_uninterpolated(poly128_vec x, poly128_vec y, poly128_vec* out)
{
	karatsuba_mul_128_uninterpolated_other_sum(x, y, x, y, out);
}

// Karatsuba multiplication, but don't combine the 3 128-bit polynomials into a 256-bit polynomial.
inline void karatsuba_mul_128_uncombined(poly128_vec x, poly128_vec y, poly128_vec* out)
{
	karatsuba_mul_128_uninterpolated(x, y, out);
	out[1] = poly128_add(poly128_add(out[0], out[2]), out[1]);
}

// Given a sequence of n (for n >= 2) 128-bit polynomials p_i, compute the sum of x^(64*i) p_i as a
// sequence of n / 2 + 1 128-bit polynomials.
inline void combine_poly128s(poly128_vec* out, const poly128_vec* in, size_t n)
{
	out[0] = poly128_add(in[0], clmul_block_shift_left_64(in[1]));
	for (size_t i = 1; i < n / 2; ++i)
		out[i] = poly128_add(in[2*i], clmul_block_mix_64(in[2*i + 1], in[2*i - 1]));
	if (n % 2)
		out[n / 2] = poly128_add(in[n - 1], clmul_block_shift_right_64(in[n - 2]));
	else
		out[n / 2] = clmul_block_shift_right_64(in[n - 1]);
}

inline poly256_vec poly128_mul(poly128_vec x, poly128_vec y)
{
	poly128_vec karatsuba_out[3];
	karatsuba_mul_128_uncombined(x, y, &karatsuba_out[0]);

	poly256_vec out;
	combine_poly128s(&out.data[0], &karatsuba_out[0], 3);
	return out;
}

inline poly384_vec poly192_mul(poly192_vec x, poly192_vec y)
{
	// Something like Toom-Cook.

	// Evaluate at 0.
	poly128_vec xlow_ylow = clmul_block_clmul_ll(x.data[0], y.data[0]);

	// Evaluate at infinity.
	poly128_vec xhigh_yhigh = clmul_block_clmul_ll(x.data[1], y.data[1]);

	// Evaluate at 1.
	clmul_block x1_cat_y0_plus_y2 =
		clmul_block_mix_64(clmul_block_xor(y.data[0], y.data[1]), x.data[0]);
	clmul_block xsum =
		clmul_block_xor(clmul_block_xor(x.data[0], x.data[1]), x1_cat_y0_plus_y2); // Result in low.
	clmul_block ysum = clmul_block_xor(y.data[0], x1_cat_y0_plus_y2); // Result in high.
	poly128_vec xsum_ysum = clmul_block_clmul_lh(xsum, ysum);

	// Evaluate at the root of a^2 + a + 1.
	poly128_vec xa = poly128_add(x.data[0], clmul_block_broadcast_low64(x.data[1]));
	poly128_vec ya = poly128_add(y.data[0], clmul_block_broadcast_low64(y.data[1]));
	// Karatsuba multiplication of two degree 1 polynomials (with deg <64 polynomial coefficients).
	poly128_vec karatsuba_out[3];
	karatsuba_mul_128_uninterpolated_other_sum(xa, ya, x.data[0], y.data[0], &karatsuba_out[0]);
	poly128_vec xya0 = poly128_add(karatsuba_out[0], karatsuba_out[2]);
	poly128_vec xya1 = poly128_add(karatsuba_out[0], karatsuba_out[1]);

	// Interpolate through the 4 evaluation points.
	poly128_vec xya0_plus_xsum_ysum = poly128_add(xya0, xsum_ysum);
	poly128_vec interp[5];
	interp[0] = xlow_ylow;
	interp[1] = poly128_add(xya0_plus_xsum_ysum, xhigh_yhigh);
	interp[2] = poly128_add(xya0_plus_xsum_ysum, xya1);
	interp[3] = poly128_add(poly128_add(xlow_ylow, xsum_ysum), xya1);
	interp[4] = xhigh_yhigh;

	// Combine overlapping 128-bit chunks.
	poly384_vec out;
	combine_poly128s(&out.data[0], &interp[0], 5);
	return out;
}

inline poly512_vec poly256_mul(poly256_vec x, poly256_vec y)
{
	// Karatsuba multiplication.
	poly128_vec x0y0[3], x1y1[3], xsum_ysum[3];
	karatsuba_mul_128_uncombined(x.data[0], y.data[0], &x0y0[0]);
	karatsuba_mul_128_uncombined(x.data[1], y.data[1], &x1y1[0]);

	poly128_vec xsum = poly128_add(x.data[0], x.data[1]);
	poly128_vec ysum = poly128_add(y.data[0], y.data[1]);
	karatsuba_mul_128_uncombined(xsum, ysum, &xsum_ysum[0]);

	poly128_vec x0y0_2_plus_x1y1_0 = poly128_add(x0y0[2], x1y1[0]);

	poly128_vec combined[7];
	combined[0] = x0y0[0];
	combined[1] = x0y0[1];
	combined[2] = poly128_add(xsum_ysum[0], poly128_add(x0y0[0], x0y0_2_plus_x1y1_0));
	combined[3] = poly128_add(xsum_ysum[1], poly128_add(x0y0[1], x1y1[1]));
	combined[4] = poly128_add(xsum_ysum[2], poly128_add(x1y1[2], x0y0_2_plus_x1y1_0));
	combined[5] = x1y1[1];
	combined[6] = x1y1[2];

	poly512_vec out;
	combine_poly128s(&out.data[0], &combined[0], 7);
	return out;
}

// High 64 of output will be zero.
inline poly192_vec poly64x128_mul(poly64_vec x, poly128_vec y)
{
	clmul_block xy[2];
	xy[0] = clmul_block_clmul_ll(x, y);
	xy[1] = clmul_block_clmul_lh(x, y);

	poly192_vec out;
	combine_poly128s(&out.data[0], &xy[0], 2);
	return out;
}

inline poly256_vec poly64x192_mul(poly64_vec x, poly192_vec y)
{
	clmul_block xy[3];
	xy[0] = clmul_block_clmul_ll(x, y.data[0]);
	xy[1] = clmul_block_clmul_lh(x, y.data[0]);
	xy[2] = clmul_block_clmul_ll(x, y.data[1]);

	poly256_vec out;
	combine_poly128s(&out.data[0], &xy[0], 3);
	return out;
}

inline poly320_vec poly64x256_mul(poly64_vec x, poly256_vec y)
{
	clmul_block xy[4];
	xy[0] = clmul_block_clmul_ll(x, y.data[0]);
	xy[1] = clmul_block_clmul_lh(x, y.data[0]);
	xy[2] = clmul_block_clmul_ll(x, y.data[1]);
	xy[3] = clmul_block_clmul_lh(x, y.data[1]);

	poly320_vec out;
	combine_poly128s(&out.data[0], &xy[0], 4);
	return out;
}

inline clmul_block poly1_to_bit_mask(poly1_vec x)
{
#if POLY_VEC_LEN == 1
	return _mm_set1_epi8(x);
#elif POLY_VEC_LEN == 2
	block256 shuffle_mask = _mm256_setr_epi8(
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1);
	return _mm256_shuffle_epi8(_mm256_set1_epi16(x), shuffle_mask);
#endif
}

inline poly128_vec poly1x128_mul(poly1_vec x, poly128_vec y)
{
	return clmul_block_and(poly1_to_bit_mask(x), y);
}

inline poly192_vec poly1x192_mul(poly1_vec x, poly192_vec y)
{
	clmul_block mask = poly1_to_bit_mask(x);
	poly192_vec out;
	out.data[0] = clmul_block_and(mask, y.data[0]);
	out.data[1] = clmul_block_and(mask, y.data[1]);
	return out;
}

inline poly256_vec poly1x256_mul(poly1_vec x, poly256_vec y)
{
	clmul_block mask = poly1_to_bit_mask(x);
	poly256_vec out;
	out.data[0] = clmul_block_and(mask, y.data[0]);
	out.data[1] = clmul_block_and(mask, y.data[1]);
	return out;
}

inline void poly_shift_left_1(clmul_block* x, size_t chunks)
{
	clmul_block low[4], high[4], high_shifted[4];
	for (size_t i = 0; i < chunks; ++i)
	{
#if POLY_VEC_LEN == 1
		low[i] = _mm_slli_epi32(x[i], 1);
		high[i] = _mm_srli_epi32(x[i], 31);
		high_shifted[i] =
			i > 0 ? _mm_alignr_epi8(high[i], high[i - 1], 12) : _mm_slli_si128(high[i], 4);
#elif POLY_VEC_LEN == 2
		low[i] = _mm256_slli_epi32(x[i], 1);
		high[i] = _mm256_srli_epi32(x[i], 31);
		high_shifted[i] =
			i > 0 ? _mm256_alignr_epi8(high[i], high[i - 1], 12) : _mm256_bslli_epi128(high[i], 4);
#endif
	}

	for (size_t i = 0; i < chunks; ++i)
		x[i] = clmul_block_xor(low[i], high_shifted[i]);
}

inline void poly_shift_left_8(clmul_block* out, const clmul_block* in, size_t chunks)
{
	for (size_t i = 0; i < chunks; ++i)
#if POLY_VEC_LEN == 1
		out[i] = i > 0 ? _mm_alignr_epi8(in[i], in[i - 1], 15) : _mm_slli_si128(in[i], 1);
#elif POLY_VEC_LEN == 2
		out[i] = i > 0 ? _mm256_alignr_epi8(in[i], in[i - 1], 15) : _mm256_bslli_epi128(in[i], 1);
#endif
}

inline poly256_vec poly256_shift_left_1(poly256_vec x)
{
	poly_shift_left_1(&x.data[0], 2);
	return x;
}
inline poly384_vec poly384_shift_left_1(poly384_vec x)
{
	poly_shift_left_1(&x.data[0], 3);
	return x;
}
inline poly512_vec poly512_shift_left_1(poly512_vec x)
{
	poly_shift_left_1(&x.data[0], 4);
	return x;
}

inline poly256_vec poly256_shift_left_8(poly256_vec x)
{
	poly256_vec out;
	poly_shift_left_8(&out.data[0], &x.data[0], 2);
	return out;
}
inline poly384_vec poly384_shift_left_8(poly384_vec x)
{
	poly384_vec out;
	poly_shift_left_8(&out.data[0], &x.data[0], 3);
	return out;
}
inline poly512_vec poly512_shift_left_8(poly512_vec x)
{
	poly512_vec out;
	poly_shift_left_8(&out.data[0], &x.data[0], 4);
	return out;
}

inline poly64_vec poly64_set_zero()
{
	return clmul_block_set_zero();
}
inline poly128_vec poly128_set_zero()
{
	return clmul_block_set_zero();
}
inline poly192_vec poly192_set_zero()
{
	poly192_vec out;
	out.data[0] = poly128_set_zero();
	out.data[1] = poly128_set_zero();
	return out;
}
inline poly256_vec poly256_set_zero()
{
	poly256_vec out;
	out.data[0] = poly128_set_zero();
	out.data[1] = poly128_set_zero();
	return out;
}

inline poly64_vec poly64_set_low32(uint32_t x);
inline poly128_vec poly128_set_low32(uint32_t x);
inline poly192_vec poly192_set_low32(uint32_t x);
inline poly256_vec poly256_set_low32(uint32_t x);
inline poly320_vec poly320_set_low32(uint32_t x);
inline poly384_vec poly384_set_low32(uint32_t x);
inline poly512_vec poly512_set_low32(uint32_t x);

inline poly128_vec poly128_set_low32(uint32_t x)
{
#if POLY_VEC_LEN == 1
	return _mm_cvtsi32_si128(x);
#elif POLY_VEC_LEN == 2
	return _mm256_blend_epi32(_mm256_setzero_si256(), _mm256_set1_epi32(x), 0x11);
#endif
}

inline poly64_vec poly64_set_low32(uint32_t x)
{
	return poly128_set_low32(x);
}
inline poly192_vec poly192_set_low32(uint32_t x)
{
	poly192_vec out;
	out.data[0] = poly128_set_low32(x);
	memset(&out.data[1], 0, sizeof(out.data[1]));
	return out;
}
inline poly256_vec poly256_set_low32(uint32_t x)
{
	poly256_vec out;
	out.data[0] = poly128_set_low32(x);
	memset(&out.data[1], 0, sizeof(out.data[1]));
	return out;
}
inline poly320_vec poly320_set_low32(uint32_t x)
{
	poly320_vec out;
	out.data[0] = poly128_set_low32(x);
	memset(&out.data[1], 0, 2 * sizeof(out.data[1]));
	return out;
}
inline poly384_vec poly384_set_low32(uint32_t x)
{
	poly384_vec out;
	out.data[0] = poly128_set_low32(x);
	memset(&out.data[1], 0, 2 * sizeof(out.data[1]));
	return out;
}
inline poly512_vec poly512_set_low32(uint32_t x)
{
	poly512_vec out;
	out.data[0] = poly128_set_low32(x);
	memset(&out.data[1], 0, 3 * sizeof(out.data[1]));
	return out;
}

inline poly64_vec poly128_reduce64(poly128_vec x)
{
	poly64_vec modulus = poly64_set_low32(gf64_modulus);
	poly128_vec high = clmul_block_clmul_lh(modulus, x); // Degree < 64 + 4
	poly128_vec high_high = clmul_block_clmul_lh(modulus, high); // Degree < 4 + 4

	// Only low 64 bits are correct, but the high 64 will be ignored anyway.
	return poly128_add(poly128_add(x, high), high_high);
}

inline poly64_vec poly64_mul_a64_reduce64(poly64_vec x)
{
	poly64_vec modulus = poly64_set_low32(gf64_modulus);
	poly128_vec high = clmul_block_clmul_ll(modulus, x); // Degree < 64 + 4
	poly128_vec high_high = clmul_block_clmul_lh(modulus, high); // Degree < 4 + 4

	// Only low 64 bits are correct, but the high 64 will be ignored anyway.
	return poly128_add(high, high_high);
}

inline poly128_vec poly256_reduce128(poly256_vec x)
{
	poly64_vec modulus = poly64_set_low32(gf128_modulus);
	poly128_vec reduced_192 = clmul_block_clmul_lh(modulus, x.data[1]);
	x.data[0] = poly128_add(x.data[0], clmul_block_shift_left_64(reduced_192));
	x.data[1] = poly128_add(x.data[1], clmul_block_shift_right_64(reduced_192));
	x.data[0] = poly128_add(x.data[0], clmul_block_clmul_ll(modulus, x.data[1]));
	return x.data[0];
}

inline poly192_vec poly384_reduce192(poly384_vec x)
{
	poly64_vec modulus = poly64_set_low32(gf192_modulus);
	poly128_vec reduced_320 = clmul_block_clmul_lh(modulus, x.data[2]);
	poly128_vec reduced_256 = clmul_block_clmul_ll(modulus, x.data[2]);
	poly192_vec out;
	out.data[1] = poly128_add(x.data[1], reduced_320);
	out.data[0] = poly128_add(x.data[0], clmul_block_shift_left_64(reduced_256));
	out.data[0] = poly128_add(out.data[0], clmul_block_clmul_lh(modulus, out.data[1]));
	out.data[1] = poly128_add(out.data[1], clmul_block_shift_right_64(reduced_256));
	return out;
}

inline poly256_vec poly512_reduce256(poly512_vec x)
{
	poly64_vec modulus = poly64_set_low32(gf256_modulus);
	clmul_block xmod[4];
	xmod[0] = clmul_block_set_zero();
	xmod[1] = clmul_block_clmul_lh(modulus, x.data[2]);
	xmod[2] = clmul_block_clmul_ll(modulus, x.data[3]);
	xmod[3] = clmul_block_clmul_lh(modulus, x.data[3]);

	clmul_block xmod_combined[3];
	combine_poly128s(&xmod_combined[0], &xmod[0], 4);
	for (size_t i = 0; i < 3; ++i)
		xmod_combined[i] = poly128_add(xmod_combined[i], x.data[i]);
	xmod_combined[0] = poly128_add(xmod_combined[0], clmul_block_clmul_ll(modulus, xmod_combined[2]));

	poly256_vec out;
	for (size_t i = 0; i < 2; ++i)
		out.data[i] = xmod_combined[i];
	return out;
}

inline poly128_vec poly192_reduce128(poly192_vec x)
{
	poly64_vec modulus = poly64_set_low32(gf128_modulus);
	return poly128_add(x.data[0], clmul_block_clmul_ll(modulus, x.data[1]));
}

inline poly192_vec poly256_reduce192(poly256_vec x)
{
	poly64_vec modulus = poly64_set_low32(gf192_modulus);
	poly192_vec out;
	out.data[1] = x.data[1];
	out.data[0] = poly128_add(x.data[0], clmul_block_clmul_lh(modulus, out.data[1]));
	return out;
}

inline poly256_vec poly320_reduce256(poly320_vec x)
{
	poly64_vec modulus = poly64_set_low32(gf256_modulus);
	poly256_vec out;
	out.data[1] = x.data[1];
	out.data[0] = poly128_add(x.data[0], clmul_block_clmul_ll(modulus, x.data[2]));
	return out;
}

inline poly128_vec poly128_from_1(poly1_vec x)
{
#if POLY_VEC_LEN == 1
	return _mm_cvtsi32_si128(x & 1);
#elif POLY_VEC_LEN == 2
	block256 shuffle_mask = _mm256_setr_epi8(
		 0, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1,
		 1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1);
	return _mm256_shuffle_epi8(_mm256_set1_epi16(x & 0x0101), shuffle_mask);
#endif
}

inline poly192_vec poly192_from_1(poly1_vec x)
{
    poly192_vec out;
    out.data[0] = poly128_from_1(x);
    out.data[1] = clmul_block_set_zero();
    return out;
}

inline poly256_vec poly256_from_1(poly1_vec x)
{
    poly256_vec out;
    out.data[0] = poly128_from_1(x);
    out.data[1] = clmul_block_set_zero();
    return out;
}

extern const unsigned char gf8_in_gf128[7][16];
extern const unsigned char gf8_in_gf192[7][24];
extern const unsigned char gf8_in_gf256[7][32];

inline poly128_vec poly128_from_8_poly1(const poly1_vec* bits)
{
	poly128_vec out = poly128_from_1(bits[0]);
	for (size_t i = 1; i < 8; ++i)
		out = poly128_add(out, poly1x128_mul(bits[i], poly128_load_dup(gf8_in_gf128[i - 1])));
	return out;
}
inline poly192_vec poly192_from_8_poly1(const poly1_vec* bits)
{
	poly192_vec out = poly192_from_1(bits[0]);
	for (size_t i = 1; i < 8; ++i)
		out = poly192_add(out, poly1x192_mul(bits[i], poly192_load_dup(gf8_in_gf192[i - 1])));
	return out;
}
inline poly256_vec poly256_from_8_poly1(const poly1_vec* bits)
{
	poly256_vec out = poly256_from_1(bits[0]);
	for (size_t i = 1; i < 8; ++i)
		out = poly256_add(out, poly1x256_mul(bits[i], poly256_load_dup(gf8_in_gf256[i - 1])));
	return out;
}

inline poly128_vec poly128_from_8_poly128(const poly128_vec* polys)
{
	poly256_vec out = poly256_from_128(polys[0]);
	for (size_t i = 1; i < 8; ++i)
		out = poly256_add(out, poly128_mul(polys[i], poly128_load_dup(gf8_in_gf128[i - 1])));
	return poly256_reduce128(out);
}
inline poly192_vec poly192_from_8_poly192(const poly192_vec* polys)
{
	poly384_vec out = poly384_from_192(polys[0]);
	for (size_t i = 1; i < 8; ++i)
		out = poly384_add(out, poly192_mul(polys[i], poly192_load_dup(gf8_in_gf192[i - 1])));
	return poly384_reduce192(out);
}
inline poly256_vec poly256_from_8_poly256(const poly256_vec* polys)
{
	poly512_vec out = poly512_from_256(polys[0]);
	for (size_t i = 1; i < 8; ++i)
		out = poly512_add(out, poly256_mul(polys[i], poly256_load_dup(gf8_in_gf256[i - 1])));
	return poly512_reduce256(out);
}

inline poly128_vec poly128_from_byte(uint8_t byte)
{
	poly1_vec bits[8];
	for (size_t i = 0; i < 8; ++i)
		bits[i] = poly1_set_all(expand_bit_to_byte(byte, i));
	return poly128_from_8_poly1(bits);
}

inline poly192_vec poly192_from_byte(uint8_t byte)
{
	poly1_vec bits[8];
	for (size_t i = 0; i < 8; ++i)
		bits[i] = poly1_set_all(expand_bit_to_byte(byte, i));
	return poly192_from_8_poly1(bits);
}

inline poly256_vec poly256_from_byte(uint8_t byte)
{
	poly1_vec bits[8];
	for (size_t i = 0; i < 8; ++i)
		bits[i] = poly1_set_all(expand_bit_to_byte(byte, i));
	return poly256_from_8_poly1(bits);
}

inline bool poly64_eq(poly64_vec x, poly64_vec y)
{
#if POLY_VEC_LEN == 1
	return _mm_cvtsi128_si64(x) == _mm_cvtsi128_si64(y);
#elif POLY_VEC_LEN == 2
    __m128i tmp = _mm256_castsi256_si128(transpose2x2_64(poly64_add(x, y)));
    return _mm_test_all_zeros(tmp, tmp);
#endif
}

inline bool poly128_eq(poly128_vec x, poly128_vec y)
{
#if POLY_VEC_LEN == 1
    __m128i tmp = _mm_xor_si128(x, y);
    return _mm_test_all_zeros(tmp, tmp);
#elif POLY_VEC_LEN == 2
    __m256i tmp = _mm256_xor_si256(x, y);
    return _mm256_test_all_zeros(tmp, tmp);
#endif
}

inline bool poly192_eq(poly192_vec x, poly192_vec y)
{
#if POLY_VEC_LEN == 1
    __m128i tmp0 = _mm_xor_si128(x.data[0], y.data[0]);
	return _mm_test_all_zeros(tmp0, tmp0) && (_mm_cvtsi128_si64(x.data[1]) == _mm_cvtsi128_si64(y.data[1]));
#elif POLY_VEC_LEN == 2
    __m256i tmp0 = _mm256_xor_si256(x.data[0], y.data[0]);
    __m128i tmp1 = _mm256_castsi256_si128(transpose2x2_64(_mm256_xor_si256(x.data[1], y.data[1])));
    return _mm256_test_all_zeros(tmp0, tmp0) && _mm256_test_all_zeros(tmp1, tmp1);
#endif
}

inline bool poly256_eq(poly256_vec x, poly256_vec y)
{
#if POLY_VEC_LEN == 1
    __m128i tmp0 = _mm_xor_si128(x.data[0], y.data[0]);
    __m128i tmp1 = _mm_xor_si128(x.data[1], y.data[1]);
    return _mm_test_all_zeros(tmp0, tmp0) && _mm_test_all_zeros(tmp1, tmp1);
#elif POLY_VEC_LEN == 2
    __m256i tmp0 = _mm256_xor_si256(x.data[0], y.data[0]);
    __m256i tmp1 = _mm256_xor_si256(x.data[1], y.data[1]);
    return _mm256_test_all_zeros(tmp0, tmp0) && _mm256_test_all_zeros(tmp1, tmp1);
#endif
}

inline bool poly320_eq(poly320_vec x, poly320_vec y)
{
#if POLY_VEC_LEN == 1
    __m128i tmp0 = _mm_xor_si128(x.data[0], y.data[0]);
    __m128i tmp1 = _mm_xor_si128(x.data[1], y.data[1]);
    return _mm_test_all_zeros(tmp0, tmp0) && _mm_test_all_zeros(tmp1, tmp1) && \
	    (_mm_cvtsi128_si64(x.data[2]) == _mm_cvtsi128_si64(y.data[2]));
#elif POLY_VEC_LEN == 2
#error "not implemented"
#endif
}

inline bool poly384_eq(poly384_vec x, poly384_vec y)
{
#if POLY_VEC_LEN == 1
    __m128i tmp0 = _mm_xor_si128(x.data[0], y.data[0]);
    __m128i tmp1 = _mm_xor_si128(x.data[1], y.data[1]);
    __m128i tmp2 = _mm_xor_si128(x.data[2], y.data[2]);
    return _mm_test_all_zeros(tmp0, tmp0) && _mm_test_all_zeros(tmp1, tmp1) && \
           _mm_test_all_zeros(tmp2, tmp2);
#elif POLY_VEC_LEN == 2
    __m256i tmp0 = _mm256_xor_si256(x.data[0], y.data[0]);
    __m256i tmp1 = _mm256_xor_si256(x.data[1], y.data[1]);
    __m256i tmp2 = _mm256_xor_si256(x.data[2], y.data[2]);
    return _mm256_test_all_zeros(tmp0, tmp0) && _mm256_test_all_zeros(tmp1, tmp1) && \
           _mm256_test_all_zeros(tmp2, tmp2);
#endif
}

inline bool poly512_eq(poly512_vec x, poly512_vec y)
{
#if POLY_VEC_LEN == 1
    __m128i tmp0 = _mm_xor_si128(x.data[0], y.data[0]);
    __m128i tmp1 = _mm_xor_si128(x.data[1], y.data[1]);
    __m128i tmp2 = _mm_xor_si128(x.data[2], y.data[2]);
    __m128i tmp3 = _mm_xor_si128(x.data[3], y.data[3]);
    return _mm_test_all_zeros(tmp0, tmp0) && _mm_test_all_zeros(tmp1, tmp1) && \
           _mm_test_all_zeros(tmp2, tmp2) && _mm_test_all_zeros(tmp3, tmp3);
#elif POLY_VEC_LEN == 2
    __m256i tmp0 = _mm256_xor_si256(x.data[0], y.data[0]);
    __m256i tmp1 = _mm256_xor_si256(x.data[1], y.data[1]);
    __m256i tmp2 = _mm256_xor_si256(x.data[2], y.data[2]);
    __m256i tmp3 = _mm256_xor_si256(x.data[3], y.data[3]);
    return _mm256_test_all_zeros(tmp0, tmp0) && _mm256_test_all_zeros(tmp1, tmp1) && \
           _mm256_test_all_zeros(tmp2, tmp2) && _mm256_test_all_zeros(tmp3, tmp3);
#endif
}

inline poly64_vec poly64_extract(poly64_vec x, size_t index)
{
#if POLY_VEC_LEN == 1
    (void) index;
    return x;
#elif POLY_VEC_LEN == 2
#error "not implemented"
#endif
}

inline poly128_vec poly128_extract(poly128_vec x, size_t index)
{
#if POLY_VEC_LEN == 1
    (void) index;
    return x;
#elif POLY_VEC_LEN == 2
#error "not implemented"
#endif
}

inline poly192_vec poly192_extract(poly192_vec x, size_t index)
{
#if POLY_VEC_LEN == 1
    (void) index;
    return x;
#elif POLY_VEC_LEN == 2
#error "not implemented"
#endif
}

inline poly256_vec poly256_extract(poly256_vec x, size_t index)
{
#if POLY_VEC_LEN == 1
    (void) index;
    return x;
#elif POLY_VEC_LEN == 2
#error "not implemented"
#endif
}

inline poly384_vec poly384_extract(poly384_vec x, size_t index)
{
#if POLY_VEC_LEN == 1
    (void) index;
    return x;
#elif POLY_VEC_LEN == 2
#error "not implemented"
#endif
}

inline poly512_vec poly512_extract(poly512_vec x, size_t index)
{
#if POLY_VEC_LEN == 1
    (void) index;
    return x;
#elif POLY_VEC_LEN == 2
#error "not implemented"
#endif
}

#endif
