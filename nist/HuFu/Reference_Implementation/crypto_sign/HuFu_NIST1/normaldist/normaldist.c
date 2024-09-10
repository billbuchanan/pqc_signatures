/*
Copyright (c) 2023 Team HAETAE
Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#include "../random/random.h"
#include "fixpoint.h"
#include "normaldist.h"
#include "../sha3/shake.h"
/* Store a double-precision centered normal vector in vec.
 * Computation using Box-Muller. PARAM_N is assumed to be even.
 * Standard deviation is PARAM_N/2, so that the output vector has
 * the same distribution as the FFT of a standard normal vector.
 */
static uint64_t approx_exp(const uint64_t x)
{
	int64_t result;
	result = -0x0000B6C6340925AELL;
	result = ((smulh48(result, x) + (1LL << 2)) >> 3) + 0x0000B4BD4DF85227LL;
	result = ((smulh48(result, x) + (1LL << 2)) >> 3) - 0x0000887F727491E2LL;
	result = ((smulh48(result, x) + (1LL << 1)) >> 2) + 0x0000AAAA643C7E8DLL;
	result = ((smulh48(result, x) + (1LL << 1)) >> 2) - 0x0000AAAAA98179E6LL;
	result = ((smulh48(result, x) + 1LL) >> 1) + 0x0000FFFFFFFB2E7ALL;
	result = ((smulh48(result, x) + 1LL) >> 1) - 0x0000FFFFFFFFF85FLL;
	result = ((smulh48(result, x))) + 0x0000FFFFFFFFFFFCLL;
	return result;
}

#define CDTLEN 65
static const uint32_t CDT[CDTLEN] =
	{3189, 6372, 9535, 12667, 15758, 18795, 21767, 24665, 27479, 30201,
	 32824, 35342, 37748, 40041, 42215, 44270, 46204, 48016, 49710, 51285,
	 52745, 54093, 55333, 56468, 57505, 58445, 59296, 60065, 60756, 61372,
	 61923, 62411, 62842, 63222, 63556, 63847, 64101, 64321, 64510, 64674,
	 64815, 64934, 65035, 65121, 65193, 65254, 65305, 65348, 65383, 65412,
	 65435, 65455, 65471, 65484, 65496, 65504, 65512, 65517, 65521, 65525,
	 65527, 65529, 65531, 65533, 65535}; // 16 bit precision

static uint64_t sample_gauss16(const uint64_t rand16)
{
	unsigned int i;
	uint64_t r = 0;
	for (i = 0; i < CDTLEN; i++)
	{
		r += (((uint64_t)CDT[i] - rand16) >> 63) & 1;
	}
	return r;
}

#define GAUSS_RAND (72 + 16 + 48)
#define GAUSS_RAND_BYTES ((GAUSS_RAND + 7) / 8)
static int sample_gauss_sigma76(uint64_t *r, const uint8_t rand[GAUSS_RAND_BYTES])
{
	const uint64_t *rand_gauss16_ptr = (uint64_t *)rand,
				   *rand_rej_ptr = (uint64_t *)(&rand[2]);
	const uint64_t rand_gauss16 = (*rand_gauss16_ptr) & ((1ULL << 16) - 1);
	const uint64_t rand_rej = (*rand_rej_ptr) & ((1ULL << 48) - 1);

	uint64_t x, exp_in;
	fp96_76 y, xy, sqr, sqy_plus_2kxy;

	// leave 16 bit for carries
	y.limb48[0] = rand[8] ^ ((uint64_t)rand[9] << 8) ^
				  ((uint64_t)rand[10] << 16) ^ ((uint64_t)rand[11] << 24) ^
				  ((uint64_t)rand[12] << 32) ^ ((uint64_t)rand[13] << 40);
	y.limb48[1] =
		rand[14] ^ ((uint64_t)rand[15] << 8) ^ ((uint64_t)rand[16] << 16);

	// round y to r
	*r = (y.limb48[0] >> 15) ^ (y.limb48[1] << 33);
	*r += 1; // rounding
	*r >>= 1;

	// square y
	fixpoint_square(&sqr, &y);

	// sample x
	x = sample_gauss16(rand_gauss16);

	// add x to r
	*r += x << 56;

	// multiply 2kx to y
	fixpoint_mul_high(&xy, &y, x << (73 - 48));

	// add xy<<77 to y^2 (stored in sqr)
	fixpoint_add(&sqy_plus_2kxy, &sqr, &xy);

	// sqy_plus_2kxy is now exactly what we want to put into approx_exp, but we
	// only want the first 48 digits after the fix point.
	exp_in = sqy_plus_2kxy.limb48[1] << 19;
	exp_in += (sqy_plus_2kxy.limb48[0] + (1UL << 28)) >> 29; // rounding

	y.limb48[1] ^= x << 24;

	return ((((int64_t)(rand_rej ^
						(rand_rej & 1)) // set lowest bit to zero in order to
										// use it for rejection if sample==0
			  - (int64_t)approx_exp(exp_in)) >>
			 63) // reject with prob 1-approx_exp(exp_in)
			& (((*r | -*r) >> 63) | rand_rej)) &
		   1; // if the sample is zero, clear the return value with prob 1/2
}

void normaldist(double *vec, size_t n, uint8_t seed[32])
{
	prng rng;
	get_seed(32, &rng, seed);
	uint64_t r[n];
	const size_t sign_size_bytes = (n + 7) / 8;
	uint8_t signs[sign_size_bytes];
	// randombytes(signs, sign_size_bytes);
	prng_get_bytes(&rng, signs, sign_size_bytes);
	const size_t buf_size = GAUSS_RAND_BYTES * 2;
	uint8_t buf[buf_size];
	// randombytes(buf, buf_size);
	prng_get_bytes(&rng, buf, buf_size);
	size_t buf_cur = 0;

	size_t nmb_sampled = 0;
	int accepted;

	while (nmb_sampled < n)
	{
		if (buf_cur == buf_size)
		{
			// randombytes(buf, buf_size);
			prng_get_bytes(&rng, buf, buf_size);

			buf_cur = 0;
		}

		accepted = sample_gauss_sigma76(&r[nmb_sampled], buf + buf_cur);
		nmb_sampled += accepted;
		buf_cur += GAUSS_RAND_BYTES;
	}

	for (size_t i = 0; i < n; i++)
	{
		const double sign = (double)((signs[(i >> 3)] >> (i & 7)) & 1) * 2.0 - 1.0;
		vec[i] = sign * r[i] / (1LL << 60);
	}
}
