#include "consts.h"
#include "sampler.h"
#include "fixpoint.h"
#include "symmetric.h"
#include "fips202x4.h"
#include <stdint.h>

/*************************************************
 * Name:        rej_uniform
 *
 * Description: Sample uniformly random coefficients in [0, Q-1] by
 *              performing rejection sampling on array of random bytes.
 *
 * Arguments:   - int32_t *a: pointer to output array (allocated)
 *              - unsigned int len: number of coefficients to be sampled
 *              - const uint8_t *buf: array of random bytes
 *              - unsigned int buflen: length of array of random bytes
 *
 * Returns number of sampled coefficients. Can be smaller than len if not enough
 * random bytes were given.
 **************************************************/
unsigned int rej_uniform(int32_t *a, unsigned int len, const uint8_t *buf,
                         unsigned int buflen) {
    unsigned int ctr, pos;
    uint32_t t;

    ctr = pos = 0;
    while (ctr < len && pos + 2 <= buflen) {
        t = buf[pos++];
        t |= (uint32_t)buf[pos++] << 8;

        if (t < Q)
            a[ctr++] = t;
    }
    return ctr;
}

/*************************************************
 * Name:        rej_eta
 *
 * Description: Sample uniformly random coefficients in [-ETA, ETA] by
 *              performing rejection sampling on array of random bytes.
 *
 * Arguments:   - int32_t *a: pointer to output array (allocated)
 *              - unsigned int len: number of coefficients to be sampled
 *              - const uint8_t *buf: array of random bytes
 *              - unsigned int buflen: length of array of random bytes
 *
 * Returns number of sampled coefficients. Can be smaller than len if not enough
 * random bytes were given.
 **************************************************/
static int32_t mod3(uint8_t t) {
    int32_t r;
    r = (t >> 4) + (t & 0xf);
    r = (r >> 2) + (r & 3);
    r = (r >> 2) + (r & 3);
    r = (r >> 2) + (r & 3);
    return r - (3 * (r >> 1));
}
static int32_t mod3_leq26(uint8_t t) {
    int32_t r;
    r = (t >> 4) + (t & 0xf);
    r = (r >> 2) + (r & 3);
    r = (r >> 2) + (r & 3);
    return r - (3 * (r >> 1));
}
static int32_t mod3_leq8(uint8_t t) {
    int32_t r;
    r = (t >> 2) + (t & 3);
    r = (r >> 2) + (r & 3);
    return r - (3 * (r >> 1));
}
unsigned int rej_eta(int32_t *a, unsigned int len, const uint8_t *buf,
                     unsigned int buflen) {
    unsigned int ctr, pos;

    ctr = pos = 0;
    while (ctr < len && pos < buflen) {
#if ETA == 1
        uint32_t t = buf[pos++];
        if (t < 243) {
            // reduce mod 3
            a[ctr++] = mod3(t);

            if (ctr >= len)
                break;

            t *= 171; // 171*3 = 1 mod 256
            t >>= 9;
            a[ctr++] = mod3(t);

            if (ctr >= len)
                break;

            t *= 171;
            t >>= 9;
            a[ctr++] = mod3_leq26(t);

            if (ctr >= len)
                break;

            t *= 171;
            t >>= 9;
            a[ctr++] = mod3_leq8(t);

            if (ctr >= len)
                break;

            t *= 171;
            t >>= 9;
            a[ctr++] = (int32_t)t - (int32_t)3 * (t >> 1);
        }
#elif ETA == 2
        uint32_t t0, t1;
        t0 = buf[pos] & 0x0F;
        t1 = buf[pos++] >> 4;
        if (t0 < 15) {
            t0 = t0 - (205 * t0 >> 10) * 5;
            a[ctr++] = 2 - t0;
        }
        if (t1 < 15 && ctr < len) {
            t1 = t1 - (205 * t1 >> 10) * 5;
            a[ctr++] = 2 - t1;
        }
#endif
    }
    return ctr;
}

static uint64_t approx_exp(const uint64_t x) {
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
    {3189,  6372,  9535,  12667, 15758, 18795, 21767, 24665, 27479, 30201,
     32824, 35342, 37748, 40041, 42215, 44270, 46204, 48016, 49710, 51285,
     52745, 54093, 55333, 56468, 57505, 58445, 59296, 60065, 60756, 61372,
     61923, 62411, 62842, 63222, 63556, 63847, 64101, 64321, 64510, 64674,
     64815, 64934, 65035, 65121, 65193, 65254, 65305, 65348, 65383, 65412,
     65435, 65455, 65471, 65484, 65496, 65504, 65512, 65517, 65521, 65525,
     65527, 65529, 65531, 65533, 65535}; // 16 bit precision

static uint64_t sample_gauss16(const uint64_t rand16) {
    unsigned int i;
    uint64_t r = 0;
    for (i = 0; i < CDTLEN; i++) {
        r += (((uint64_t)CDT[i] - rand16) >> 63) & 1;
    }
    return r;
}

#define GAUSS_RAND (72 + 16 + 48)
#define GAUSS_RAND_BYTES ((GAUSS_RAND + 7) / 8)
static int sample_gauss_sigma76(uint64_t *r, fp96_76 *sqr,
                                const uint8_t rand[GAUSS_RAND_BYTES]) {
    const uint64_t *rand_gauss16_ptr = (uint64_t *)rand,
                   *rand_rej_ptr = (uint64_t *)(&rand[2]);
    const uint64_t rand_gauss16 = (*rand_gauss16_ptr) & ((1ULL << 16) - 1);
    const uint64_t rand_rej = (*rand_rej_ptr) & ((1ULL << 48) - 1);
    uint64_t x, exp_in;
    fp96_76 y, xy, sqy_plus_2kxy;

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
    fixpoint_square(sqr, &y);

    // sample x
    x = sample_gauss16(rand_gauss16);

    // add x to r
    *r += x << 56;

    // multiply 2kx to y
    fixpoint_mul_high(&xy, &y, x << (73 - 48));

    // add xy<<77 to y^2 (stored in sqr)
    fixpoint_add(&sqy_plus_2kxy, sqr, &xy);

    // sqy_plus_2kxy is now exactly what we want to put into approx_exp, but we
    // only want the first 48 digits after the fix point.
    exp_in = sqy_plus_2kxy.limb48[1] << 19;
    exp_in += (sqy_plus_2kxy.limb48[0] + (1UL << 28)) >> 29; // rounding

    y.limb48[1] ^= x << 24;
    fixpoint_square(sqr, &y);

    return ((((int64_t)(rand_rej ^
                        (rand_rej & 1)) // set lowest bit to zero in order to
                                        // use it for rejection if sample==0
              - (int64_t)approx_exp(exp_in)) >>
             63) // reject with prob 1-approx_exp(exp_in)
            & (((*r | -*r) >> 63) | rand_rej)) &
           1; // if the sample is zero, clear the return value with prob 1/2
}

static int sample_gauss(uint64_t *r, fp96_76 *sqsum, const uint8_t *buf, const size_t buflen, const size_t len, const int dont_write_last)
{
    const uint8_t *pos = buf;
    fp96_76 sqr;
    size_t bytecnt = buflen, coefcnt = 0, cnt = 0;
    int accepted;
    uint64_t dummy;
    
    while (coefcnt < len) {
        if (bytecnt < GAUSS_RAND_BYTES) {
          renormalize(sqsum);
          return coefcnt;
        }

        if (dont_write_last && coefcnt == len-1)
        {
          accepted = sample_gauss_sigma76(&dummy, &sqr, pos);
        } else {
          accepted = sample_gauss_sigma76(&r[coefcnt], &sqr, pos);
        }
        cnt += 1;
        coefcnt += accepted;
        pos += GAUSS_RAND_BYTES;
        bytecnt -= GAUSS_RAND_BYTES;

        sqsum->limb48[0] += sqr.limb48[0] & -(int64_t)accepted;
        sqsum->limb48[1] += sqr.limb48[1] & -(int64_t)accepted;
    }

    renormalize(sqsum);
    return len;
}

#define POLY_HYPERBALL_BUFLEN (GAUSS_RAND_BYTES * N)
#define POLY_HYPERBALL_NBLOCKS ((POLY_HYPERBALL_BUFLEN + STREAM256_BLOCKBYTES - 1) / STREAM256_BLOCKBYTES)
void sample_gauss_N(uint64_t *r, uint8_t *signs, fp96_76 *sqsum,
                    const uint8_t seed[CRHBYTES], const uint16_t nonce,
                    const size_t len) {
    uint8_t buf[POLY_HYPERBALL_NBLOCKS * STREAM256_BLOCKBYTES];
    size_t bytecnt, coefcnt;
    stream256_state state;
    stream256_init(&state, seed, nonce);

    stream256_squeezeblocks(buf, POLY_HYPERBALL_NBLOCKS, &state);
    bytecnt = POLY_HYPERBALL_NBLOCKS * STREAM256_BLOCKBYTES - len / 8;
    for (size_t i = 0; i < len / 8; i++) {
        signs[i] = buf[i];
    }
    coefcnt = sample_gauss(r, sqsum, buf + len / 8, bytecnt, len, len%N);
    while (coefcnt < len) {
        size_t off = bytecnt % GAUSS_RAND_BYTES;
        for (size_t i = 0; i < off; i++) {
            buf[i] = buf[bytecnt - off + i];
        }
        stream256_squeezeblocks(buf + off, 1, &state);
        bytecnt = STREAM256_BLOCKBYTES + off;

        coefcnt += sample_gauss(r + coefcnt, sqsum, buf, bytecnt, len - coefcnt, len%N);
    }
}

#ifndef HAETAE_USE_AES
void sample_gauss_N_4x(uint64_t *r0, uint64_t *r1, uint64_t *r2, uint64_t *r3, 
    uint8_t *signs0, uint8_t *signs1, uint8_t *signs2, uint8_t *signs3,
    fp96_76 *sqsum,  const uint8_t seed[CRHBYTES], 
    const uint16_t nonce0, const uint16_t nonce1, const uint16_t nonce2, const uint16_t nonce3,
    const size_t len0, const size_t len1, const size_t len2, const size_t len3) {
    unsigned int ctr0, ctr1, ctr2, ctr3;
    size_t bytecnt;
    ALIGNED_UINT8(POLY_HYPERBALL_BUFLEN+8) buf[4];
    keccakx4_state state;
    __m256i f;

    f = _mm256_loadu_si256((__m256i *)seed);
    _mm256_store_si256(buf[0].vec,f);
    _mm256_store_si256(buf[1].vec,f);
    _mm256_store_si256(buf[2].vec,f);
    _mm256_store_si256(buf[3].vec,f);
    f = _mm256_loadu_si256((__m256i *)&seed[32]);
    _mm256_store_si256(&buf[0].vec[1],f);
    _mm256_store_si256(&buf[1].vec[1],f);
    _mm256_store_si256(&buf[2].vec[1],f);
    _mm256_store_si256(&buf[3].vec[1],f);

    buf[0].coeffs[CRHBYTES+0] = nonce0;
    buf[0].coeffs[CRHBYTES+1] = nonce0 >> 8;
    buf[1].coeffs[CRHBYTES+0] = nonce1;
    buf[1].coeffs[CRHBYTES+1] = nonce1 >> 8;
    buf[2].coeffs[CRHBYTES+0] = nonce2;
    buf[2].coeffs[CRHBYTES+1] = nonce2 >> 8;
    buf[3].coeffs[CRHBYTES+0] = nonce3;
    buf[3].coeffs[CRHBYTES+1] = nonce3 >> 8;

    shake256x4_absorb_once(&state, buf[0].coeffs, buf[1].coeffs, buf[2].coeffs, buf[3].coeffs, CRHBYTES + 2);
    shake256x4_squeezeblocks(buf[0].coeffs, buf[1].coeffs, buf[2].coeffs, buf[3].coeffs, POLY_HYPERBALL_NBLOCKS, &state);

    // copy signs
    bytecnt = POLY_HYPERBALL_BUFLEN - len0 / 8;
    for (size_t i = 0; i < len0 / 8; i++) {
        signs0[i] = buf[0].coeffs[i];
        signs1[i] = buf[1].coeffs[i];
        signs2[i] = buf[2].coeffs[i];
        signs3[i] = buf[3].coeffs[i];
    }

    ctr0 = sample_gauss(r0, sqsum, buf[0].coeffs + len0/8, bytecnt, len0, len0%N);
    ctr1 = sample_gauss(r1, sqsum, buf[1].coeffs + len1/8, bytecnt, len1, len1%N);
    ctr2 = sample_gauss(r2, sqsum, buf[2].coeffs + len2/8, bytecnt, len2, len2%N);
    ctr3 = sample_gauss(r3, sqsum, buf[3].coeffs + len3/8, bytecnt, len3, len3%N);

    while (ctr0 < len0 || ctr1 < len1 || ctr2 < len2 || ctr3 < len3)
    {
        size_t off = bytecnt % GAUSS_RAND_BYTES;
        for (size_t i = 0; i < off; i++)
        {
          for (size_t j = 0; j < 4; j++)
          {
            buf[j].coeffs[i] = buf[j].coeffs[bytecnt - off + i];
          }
        }
        shake256x4_squeezeblocks(&buf[0].coeffs[off], &buf[1].coeffs[off], &buf[2].coeffs[off], &buf[3].coeffs[off], 1, &state);
        bytecnt = STREAM256_BLOCKBYTES + off;

        ctr0 += sample_gauss(r0 + ctr0, sqsum, buf[0].coeffs, bytecnt, len0 - ctr0, len0%N);
        ctr1 += sample_gauss(r1 + ctr1, sqsum, buf[1].coeffs, bytecnt, len1 - ctr1, len1%N);
        ctr2 += sample_gauss(r2 + ctr2, sqsum, buf[2].coeffs, bytecnt, len2 - ctr2, len2%N);
        ctr3 += sample_gauss(r3 + ctr3, sqsum, buf[3].coeffs, bytecnt, len3 - ctr3, len3%N);
    }
}
#endif
