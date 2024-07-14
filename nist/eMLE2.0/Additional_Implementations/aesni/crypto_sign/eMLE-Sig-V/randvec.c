#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "randvec.h"
#include "aes256ctr.h"
#include "mod.h"
#include "littleendian.h"

extern const int64_t x_max;
extern const int64_t c_max;

/* Sample [-x_max, x_max]^n */
void randVec_xmax(int64_t *a, const size_t n, aes256ctr_ctx *ctx)
{
    uint8_t r[AES256CTR_BLOCKBYTES];
    uint8_t *r_head;
    size_t i, rem = 0;
    uint8_t x;

    for (i = 0; i < n; i++)
    {
        do
        {
            if (rem < 1)
            {
                aes256ctr_squeezeblocks(r, 1, ctx);
                rem = AES256CTR_BLOCKBYTES;
                r_head = r;
            }
            x = *(r_head++);
            rem--;
        } while (1 ^ (ct_lt(x, 252) >> 63));
        a[i] = (int64_t)(mod_9(x)) - 4;
    }
}

#define COMP_ENTRY_SIZE 7
#define EXP_MANTISSA_PRECISION 52
#define EXP_MANTISSA_MASK ((1LL << EXP_MANTISSA_PRECISION) - 1)
#define R_MANTISSA_PRECISION (EXP_MANTISSA_PRECISION + 1)
#define R_MANTISSA_MASK ((1LL << R_MANTISSA_PRECISION) - 1)
#define R_EXPONENT_L (8 * COMP_ENTRY_SIZE - R_MANTISSA_PRECISION)
#define DOUBLE_ONE (1023LL << 52)
/* FACCT comparison */
static inline uint64_t comp(const unsigned char *r, const double x)
{
    uint64_t res = *((uint64_t *)(&x));
    uint64_t res_mantissa, res_exponent;
    uint64_t r1;
    uint64_t r_mantissa, r_exponent;

    res_mantissa = (res & EXP_MANTISSA_MASK) | (1ULL << EXP_MANTISSA_PRECISION);
    res_exponent = R_EXPONENT_L - 1023 + 1 + (res >> EXP_MANTISSA_PRECISION);

    r1 = load_56(r);

    r_mantissa = r1 & R_MANTISSA_MASK;
    r_exponent = r1 >> R_MANTISSA_PRECISION;

    return ((1ULL << 63) ^ ct_neq(res, DOUBLE_ONE)) | (ct_lt(r_mantissa, res_mantissa) & ct_lt(r_exponent, 1ULL << res_exponent));
}

/* Sample y */
void randVec_y(int64_t *a, const size_t n, const int64_t sumXn, const int64_t sumXp, aes256ctr_ctx *ctx)
{
    size_t i, k;
    uint32_t mod;
    double ccs;
    uint8_t r[AES256CTR_BLOCKBYTES];
    uint8_t *r_head;
    size_t rem = 0;
    uint16_t x;
    uint64_t accept;
    int64_t y_min, y_gap, y_min_final;

    int64_t sumX_small = ((-(ct_lt(sumXn, sumXp) >> 63)) & (sumXp ^ sumXn)) ^ sumXp;
    int64_t sumX_big = sumX_small ^ sumXn ^ sumXp;

    mod =  (sumX_small*c_max/8) - (sumX_small*c_max/10) + 1;
    k = 1;
    for (i = 1; i < 7; i++)
    {
        k += ct_lt(1 << i, mod) >> 63;
    }

    ccs = ((double)(1 << (k - 1))) / ((double)mod);

    do
    {
        if (rem < 1 + COMP_ENTRY_SIZE)
        {
            aes256ctr_squeezeblocks(r, 1, ctx);
            rem = AES256CTR_BLOCKBYTES;
            r_head = r;
        }
        x = (*r_head) & ((1 << k) - 1);
        accept = comp(r_head + 1, ccs);
        r_head += 1 + COMP_ENTRY_SIZE;
        rem -= 1 + COMP_ENTRY_SIZE;
    } while (1 ^ ((ct_lt(x, mod) & accept) >> 63));
    y_min = (sumX_small*c_max/10) + x;

    mod =  (sumX_big*c_max/5) - (sumX_big*c_max/7) + 1;
    k = 1;
    for (i = 1; i < 8; i++)
    {
        k += ct_lt(1 << i, mod) >> 63;
    }

    ccs = ((double)(1 << (k - 1))) / ((double)mod);

    do
    {
        if (rem < 1 + COMP_ENTRY_SIZE)
        {
            aes256ctr_squeezeblocks(r, 1, ctx);
            rem = AES256CTR_BLOCKBYTES;
            r_head = r;
        }
        x = (*r_head) & ((1 << k) - 1);
        accept = comp(r_head + 1, ccs);
        r_head += 1 + COMP_ENTRY_SIZE;
        rem -= 1 + COMP_ENTRY_SIZE;
    } while (1 ^ ((ct_lt(x, mod) & accept) >> 63));
    y_gap = (sumX_big*c_max/7) + x;
    y_min_final = ((-(ct_lt(sumXn, sumXp) >> 63)) & (y_gap ^ y_min)) ^ y_gap;

    mod = n*c_max*x_max/2-y_gap-y_min+1;
    k = 1;
    for (i = 1; i < 11; i++)
    {
        k += ct_lt(1 << i, mod) >> 63;
    }

    ccs = ((double)(1 << (k - 1))) / ((double)mod);

    for (i = 0; i < n; i++)
    {
        do
        {
            if (rem < 2 + COMP_ENTRY_SIZE)
            {
                aes256ctr_squeezeblocks(r, 1, ctx);
                rem = AES256CTR_BLOCKBYTES;
                r_head = r;
            }
            x = load_16(r_head) & ((1 << k) - 1);
            accept = comp(r_head + 2, ccs);
            r_head += 2 + COMP_ENTRY_SIZE;
            rem -= 2 + COMP_ENTRY_SIZE;
        } while (1 ^ ((ct_lt(x, mod) & accept) >> 63));
        a[i] = x + y_min_final;
    }
}

/* Sample 2 non-duplicate locations a[0], a[1], and n/2 locations not equal to a[0], a[1] */
void randVec_loc(size_t *a, const size_t n, aes256ctr_ctx *ctx)
{
    uint8_t r[AES256CTR_BLOCKBYTES];
    uint8_t *r_head;
    size_t i, rem = 0;
    uint8_t x;

    if ((n == 64) || (n == 128))
    {
        aes256ctr_squeezeblocks(r, 1, ctx);
        a[0] = r[0] & (n - 1);
        rem = AES256CTR_BLOCKBYTES - 1;
        r_head = r + 1;

        do
        {
            if (rem < 1)
            {
                aes256ctr_squeezeblocks(r, 1, ctx);
                rem = AES256CTR_BLOCKBYTES;
                r_head = r;
            }
            x = (*(r_head++)) & (n - 1);
            rem--;
        } while (1 ^ (ct_neq(x, a[0]) >> 63));
        a[1] = x;

        for (i = 2; i < (n >> 1) + 2; i++)
        {
            do
            {
                if (rem < 1)
                {
                    aes256ctr_squeezeblocks(r, 1, ctx);
                    rem = AES256CTR_BLOCKBYTES;
                    r_head = r;
                }
                x = (*(r_head++)) & (n - 1);
                rem--;
            } while (1 ^ ((ct_neq(x, a[0]) & ct_neq(x, a[1])) >> 63));
            a[i] = x;
        }
    }
    else if (n == 96)
    {
        do
        {
            if (rem < 1)
            {
                aes256ctr_squeezeblocks(r, 1, ctx);
                rem = AES256CTR_BLOCKBYTES;
                r_head = r;
            }
            x = (*(r_head++)) & 127;
            rem--;
        } while (1 ^ (ct_lt(x, 96) >> 63));
        a[0] = x;

        do
        {
            if (rem < 1)
            {
                aes256ctr_squeezeblocks(r, 1, ctx);
                rem = AES256CTR_BLOCKBYTES;
                r_head = r;
            }
            x = (*(r_head++)) & 127;
            rem--;
        } while (1 ^ ((ct_lt(x, 96) & ct_neq(x, a[0])) >> 63));
        a[1] = x;

        for (i = 2; i < (n >> 1) + 2; i++)
        {
            do
            {
                if (rem < 1)
                {
                    aes256ctr_squeezeblocks(r, 1, ctx);
                    rem = AES256CTR_BLOCKBYTES;
                    r_head = r;
                }
                x = (*(r_head++)) & 127;
                rem--;
            } while (1 ^ ((ct_lt(x, 96) & ct_neq(x, a[0]) & ct_neq(x, a[1])) >> 63));
            a[i] = x;
        }
    }
}

/* Sample the distribution of num (i in Algorithm 2 of the specification) */
void randVec_dist(int64_t *a, const size_t loc, size_t num, aes256ctr_ctx *ctx)
{
    size_t i, j, k;
    uint32_t mod;
    double ccs;
    uint8_t r[AES256CTR_BLOCKBYTES];
    uint8_t *r_head;
    size_t rem = 0;
    int64_t x;
    uint64_t accept;

    mod = num / 3;
    k = 1;
    for (i = 1; i < 18; i++)
    {
        k += ct_lt(1 << i, mod) >> 63;
    }

    ccs = ((double)(1 << (k - 1))) / ((double)mod);

    if ((loc == 32) || (loc == 48))
    {
        do
        {
            if (rem < 2 + COMP_ENTRY_SIZE)
            {
                aes256ctr_squeezeblocks(r, 1, ctx);
                rem = AES256CTR_BLOCKBYTES;
                r_head = r;
            }
            x = load_16(r_head) & ((1 << k) - 1);
            accept = comp(r_head + 2, ccs);
            r_head += 2 + COMP_ENTRY_SIZE;
            rem -= 2 + COMP_ENTRY_SIZE;
        } while (1 ^ ((ct_lt(x, mod) & accept) >> 63));
    }
    else if (loc == 64)
    {
        do
        {
            if (rem < 3 + COMP_ENTRY_SIZE)
            {
                aes256ctr_squeezeblocks(r, 1, ctx);
                rem = AES256CTR_BLOCKBYTES;
                r_head = r;
            }
            x = load_24(r_head) & ((1 << k) - 1);
            accept = comp(r_head + 3, ccs);
            r_head += 3 + COMP_ENTRY_SIZE;
            rem -= 3 + COMP_ENTRY_SIZE;
        } while (1 ^ ((ct_lt(x, mod) & accept) >> 63));
    }
    a[0] = -x;
    a[1] = x - mod;

    for (j = 0; j < loc - 1; j++)
    {
        mod = num / (loc - j);
        mod += 1 ^ (ct_neq(mod, 0) >> 63);

        k = 1;
        for (i = 1; i < 18; i++)
        {
            k += ct_lt(1 << i, mod) >> 63;
        }

        ccs = ((double)(1 << (k - 1))) / ((double)mod);

        if (loc == 32)
        {
            do
            {
                if (rem < 2 + COMP_ENTRY_SIZE)
                {
                    aes256ctr_squeezeblocks(r, 1, ctx);
                    rem = AES256CTR_BLOCKBYTES;
                    r_head = r;
                }
                x = load_16(r_head) & ((1 << k) - 1);
                accept = comp(r_head + 2, ccs);
                r_head += 2 + COMP_ENTRY_SIZE;
                rem -= 2 + COMP_ENTRY_SIZE;
            } while (1 ^ ((ct_lt(x, mod) & accept) >> 63));
        }
        else if ((loc == 48) || (loc == 64))
        {
            do
            {
                if (rem < 3 + COMP_ENTRY_SIZE)
                {
                    aes256ctr_squeezeblocks(r, 1, ctx);
                    rem = AES256CTR_BLOCKBYTES;
                    r_head = r;
                }
                x = load_24(r_head) & ((1 << k) - 1);
                accept = comp(r_head + 3, ccs);
                r_head += 3 + COMP_ENTRY_SIZE;
                rem -= 3 + COMP_ENTRY_SIZE;
            } while (1 ^ ((ct_lt(x, mod) & accept) >> 63));
        }
        a[j + 2] = x;
        num -= x;
    }
    a[loc + 1] = num;
}

void randVec_noise(int64_t *a, const size_t n, const int flag, aes256ctr_ctx *ctx)
{
    size_t i, j, k;
    uint32_t mod, sub;
    double ccs;
    uint8_t r[AES256CTR_BLOCKBYTES];
    uint8_t *r_head;
    size_t rem = 0;
    int64_t x;
    uint64_t accept;

    k = 1;
    if (flag)
    {
        mod = 64 * n + 1;
        sub = 32 * n;

        for (i = 1; i < 14; i++)
        {
            k += ct_lt(1 << i, mod) >> 63;
        }
    }
    else
    {
        mod = 32 * n + 1;
        sub = 16 * n;

        for (i = 1; i < 13; i++)
        {
            k += ct_lt(1 << i, mod) >> 63;
        }
    }

    ccs = ((double)(1 << (k - 1))) / ((double)mod);

    for (j = 0; j < n; j++)
    {
        do
        {
            if (rem < 2 + COMP_ENTRY_SIZE)
            {
                aes256ctr_squeezeblocks(r, 1, ctx);
                rem = AES256CTR_BLOCKBYTES;
                r_head = r;
            }
            x = load_16(r_head) & ((1 << k) - 1);
            accept = comp(r_head + 2, ccs);
            r_head += 2 + COMP_ENTRY_SIZE;
            rem -= 2 + COMP_ENTRY_SIZE;
        } while (1 ^ ((ct_lt(x, mod) & accept) >> 63));
        a[j] = x - sub;
    }
}
