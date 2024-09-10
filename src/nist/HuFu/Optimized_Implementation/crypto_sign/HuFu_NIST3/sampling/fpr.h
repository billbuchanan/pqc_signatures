/*
 * Floating-point operations.
 *
 * ==========================(LICENSE BEGIN)============================
 *
 * Copyright (c) 2017-2019  Falcon Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * ===========================(LICENSE END)=============================
 *
 * @author   Thomas Pornin <thomas.pornin@nccgroup.com>
 */

#include <math.h>
#include <stdint.h>

/*
 * We wrap the native 'double' type into a structure so that the C compiler
 * complains if we inadvertently use raw arithmetic operators on the 'fpr'
 * type instead of using the inline functions below. This should have no
 * extra runtime cost, since all the functions below are 'inline'.
 */
typedef struct { double v; } fpr;

static inline fpr
FPR(double v)
{
	fpr x;

	x.v = v;
	return x;
}

static const fpr fpr_log2 = { 0.69314718055994530941723212146 };
static const fpr fpr_inv_log2 = { 1.4426950408889634073599246810 };
static const fpr fpr_ptwo63 = { 9223372036854775808.0 };

static inline uint64_t
fpr_expm_p63(fpr x)
{
	/*
	 * Polynomial approximation of exp(-x) is taken from FACCT:
	 *   https://eprint.iacr.org/2018/1234
	 * Specifically, values are extracted from the implementation
	 * referenced from the FACCT article, and available at:
	 *   https://github.com/raykzhao/gaussian
	 * Tests over more than 24 billions of random inputs in the
	 * 0..log(2) range have never shown a deviation larger than
	 * 2^(-50) from the true mathematical value.
	 */

#if FALCON_AVX2  // yyyAVX2+1

	/*
	 * AVX2 implementation uses more operations than Horner's method,
	 * but with a lower expression tree depth. This helps because
	 * additions and multiplications have a latency of 4 cycles on
	 * a Skylake, but the CPU can issue two of them per cycle.
	 */

	static const union {
		double d[12];
		__m256d v[3];
	} c = {
		{
			0.999999999999994892974086724280,
			0.500000000000019206858326015208,
			0.166666666666984014666397229121,
			0.041666666666110491190622155955,
			0.008333333327800835146903501993,
			0.001388888894063186997887560103,
			0.000198412739277311890541063977,
			0.000024801566833585381209939524,
			0.000002755586350219122514855659,
			0.000000275607356160477811864927,
			0.000000025299506379442070029551,
			0.000000002073772366009083061987
		}
	};

	double d1, d2, d4, d8, y;
	__m256d d14, d58, d9c;

	d1 = -x.v;
	d2 = d1 * d1;
	d4 = d2 * d2;
	d8 = d4 * d4;
	d14 = _mm256_set_pd(d4, d2 * d1, d2, d1);
	d58 = _mm256_mul_pd(d14, _mm256_set1_pd(d4));
	d9c = _mm256_mul_pd(d14, _mm256_set1_pd(d8));
	d14 = _mm256_mul_pd(d14, _mm256_loadu_pd(&c.d[0]));
	d58 = FMADD(d58, _mm256_loadu_pd(&c.d[4]), d14);
	d9c = FMADD(d9c, _mm256_loadu_pd(&c.d[8]), d58);
	d9c = _mm256_hadd_pd(d9c, d9c);
	y = 1.0 + _mm_cvtsd_f64(_mm256_castpd256_pd128(d9c)) // _mm256_cvtsd_f64(d9c)
		+ _mm_cvtsd_f64(_mm256_extractf128_pd(d9c, 1));

	/*
	 * Final conversion goes through int64_t first, because that's what
	 * the underlying opcode (vcvttsd2si) will do, and we know that the
	 * result will fit, since x >= 0 and ccs < 1. If we did the
	 * conversion directly to uint64_t, then the compiler would add some
	 * extra code to cover the case of a source value of 2^63 or more,
	 * and though the alternate path would never be exercised, the
	 * extra comparison would cost us some cycles.
	 */
	return (uint64_t)(int64_t)(y * fpr_ptwo63.v);

#else  // yyyAVX2+0

	/*
	 * Normal implementation uses Horner's method, which minimizes
	 * the number of operations.
	 */

	double d, y;

	d = x.v;
	y = 0.000000002073772366009083061987;
	y = 0.000000025299506379442070029551 - y * d;
	y = 0.000000275607356160477811864927 - y * d;
	y = 0.000002755586350219122514855659 - y * d;
	y = 0.000024801566833585381209939524 - y * d;
	y = 0.000198412739277311890541063977 - y * d;
	y = 0.001388888894063186997887560103 - y * d;
	y = 0.008333333327800835146903501993 - y * d;
	y = 0.041666666666110491190622155955 - y * d;
	y = 0.166666666666984014666397229121 - y * d;
	y = 0.500000000000019206858326015208 - y * d;
	y = 0.999999999999994892974086724280 - y * d;
	y = 1.000000000000000000000000000000 - y * d;
	return (uint64_t)(y * fpr_ptwo63.v);

#endif  // yyyAVX2-
}

static inline int64_t
fpr_trunc(fpr x)
{
	return (int64_t)x.v;
}

static inline fpr
fpr_sub(fpr x, fpr y)
{
	return FPR(x.v - y.v);
}

static inline fpr
fpr_mul(fpr x, fpr y)
{
	return FPR(x.v * y.v);
}
static inline fpr
fpr_of(int64_t i)
{
	return FPR((double)i);
}

static uint64_t
ExpM63(fpr x)
{
	int s;
	fpr r;
	uint32_t sw;

	/*
	 * Reduce x modulo log(2): x = s*log(2) + r, with s an integer,
	 * and 0 <= r < log(2). Since x >= 0, we can use fpr_trunc().
	 */
	s = (int)fpr_trunc(fpr_mul(x, fpr_inv_log2));
	r = fpr_sub(x, fpr_mul(fpr_of(s), fpr_log2));

	/*
	 * It may happen (quite rarely) that s >= 64; if sigma = 1.2
	 * (the minimum value for sigma), r = 0 and b = 1, then we get
	 * s >= 64 if the half-Gaussian produced a z >= 13, which happens
	 * with probability about 0.000000000230383991, which is
	 * approximatively equal to 2^(-32). In any case, if s >= 64,
	 * then BerExp will be non-zero with probability less than
	 * 2^(-64), so we can simply saturate s at 63.
	 */
	sw = (uint32_t)s;
	sw ^= (sw ^ 63) & -((63 - sw) >> 31);
	s = (int)sw;

	/*
	 * Compute exp(-r); we know that 0 <= r < log(2) at this point, so
	 * we can use fpr_expm_p63(), which yields a result scaled to 2^63.
	 * We scale it up to 2^64, then right-shift it by s bits because
	 * we really want exp(-x) = 2^(-s)*exp(-r).
	 *
	 * The "-1" operation makes sure that the value fits on 64 bits
	 * (i.e. if r = 0, we may get 2^64, and we prefer 2^64-1 in that
	 * case). The bias is negligible since fpr_expm_p63() only computes
	 * with 51 bits of precision or so.
	 */
    return ((fpr_expm_p63(r) << 1) - 1) >> s;
}