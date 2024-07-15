#include "field.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// #define INV_CHECK

#ifdef PROFILE_OP_COUNTS
#include <stdio.h>
#endif

// extended GCD
// algorithm from https://doi.org/10.13154/tches.v2019.i3.340-398
uint64_t gf264_inv(uint64_t a)
{
    uint64_t f = 0x1b;
    uint64_t g = a << 1;
    uint64_t g0 = a >> 63;
    int64_t delta = 1;

    uint64_t r = 0;
    uint64_t v = 0;

    for (int i = 0; i < 127; i++)
    {
        uint64_t minus_delta = -delta;
        uint64_t swap = (minus_delta >> 63) & g0; // get sign bit
        // f0 is always 1

        // update delta
        delta = ((-swap) & (minus_delta << 1)) + delta + 1;

        // update f, g, v, r
        uint64_t vr = v ^ r;
        r ^= v & (-g0);
        v ^= vr & (-swap);
        // v0  = swap;

        uint64_t fg = f ^ g;
        g ^= f & (-g0);
        f ^= fg & (-swap);

        v = (v >> 1) | (swap << 63);
        g0 = g >> 63;
        g <<= 1;
    }
    return v;
}

static inline uint64_t gf264_twice(uint64_t a)
{
#ifdef PROFILE_OP_COUNTS
    gf264_twice_count++;
#endif
    return (a << 1) ^ ((a >> 63) & 1) * 0x1b;
}

#if defined(_USE_AVX2_)
#include "gf264_aesni.h"
uint64_t gf264_mul(uint64_t a, uint64_t b)
{
#ifdef PROFILE_OP_COUNTS
    gf264_mul_count++;
#endif
    return gf2ext64_mul_u64(a, b);
}
#elif defined(_USE_ARM_NEON_)
#include "gf264_neon.h"
uint64_t gf264_mul(uint64_t a, uint64_t b)
{
#ifdef PROFILE_OP_COUNTS
    gf264_mul_count++;
#endif
    return _gf264_mulx1_neon(a, b);
}
#else
// a * b over GF(2)/[x^64 + x^4 + x^3 + x + 1]
uint64_t gf264_mul(uint64_t a, uint64_t b)
{
    if (a == 0 || b == 0)
        return 0;
    else if (a == 1)
        return b;
    else if (b == 1)
        return a;
    else if (a == 2)
        return gf264_twice(b);
    else if (b == 2)
        return gf264_twice(a);

#ifdef PROFILE_OP_COUNTS
    gf264_mul_count++;
#endif
    uint64_t result = a & (-(b & 1));
    for (size_t i = 0; i < 64; i++)
    {
        int ar = ((a >> 63) & 1) * 0x1b;
        a = (a << 1) ^ ar;
        b >>= 1;
        result ^= a & (-(b & 1));
    }
    return result;
}
#endif

int is_zero(const uint64_t *a, const size_t field_words)
{
    uint64_t r = 0;
    for (size_t i = 0; i < field_words; i++)
    {
        r |= a[i];
    }
    return 0 == r;
}

int field_equal(const uint64_t *a, const uint64_t *b, const size_t field_words)
{
    uint64_t r = 0;
    for (size_t i = 0; i < field_words; i++)
    {
        r |= a[i] ^ b[i];
    }
    return 0 == r;
}

void field_pow(uint64_t *result, const uint64_t *a, const size_t exponent, const size_t field_words)
{
    uint64_t field_one[field_words];
    memset(field_one, 0, (field_words - 1) * sizeof(uint64_t));
    field_one[field_words - 1] = 1;

    memcpy(result, field_one, field_words * sizeof(uint64_t));
    int found_one = 0;

    for (long i = 63; i >= 0; --i)
    {
        if (found_one == 1)
        {
            field_mulEqual(result, result, field_words);
        }

        if (exponent & (1ull << i))
        {
            found_one = 1;
            field_mulEqual(result, a, field_words);
        }
    }
}

void field_inv(uint64_t *inv, const uint64_t *base)
{
#ifdef PROFILE_OP_COUNTS
    field_mul_count = 0;
    gf264_mul_count = 0;
    gf264_twice_count = 0;
#endif

#if defined(USE_PREON128A) || defined(USE_PREON128B) || defined(USE_PREON128C)
    GF192_inv(inv, base);
#elif defined(USE_PREON192A) || defined(USE_PREON192B) || defined(USE_PREON192C)
    GF256_inv(inv, base);
#elif defined(USE_PREON256A) || defined(USE_PREON256B) || defined(USE_PREON256C)
    GF320_inv(inv, base);
#endif

#ifdef PROFILE_OP_COUNTS
    printf("GF%d inverse directly calls %zu field_mul, %zu gf264_mul, and %zu gf264_twice\n", (int)field_words * 64, field_mul_count, gf264_mul_count, gf264_twice_count);
#endif
}

void field_batch_inverse_and_mul(uint64_t *result, const uint64_t *elements, const size_t elements_len, const uint64_t *mul, const size_t field_words)
{
    // elements should be all non-zero, any zero within will fail this function!!

    uint64_t c[field_words];
    memcpy(c, &elements[0], field_words * sizeof(uint64_t));
    memcpy(&result[0], c, field_words * sizeof(uint64_t));

    // Set result[i] to be the product of all elements[j], where 0 <= j <= i
    for (size_t i = 1; i < elements_len; ++i)
    {
        field_mulEqual(c, &elements[i * field_words], field_words);
        memcpy(&result[i * field_words], c, field_words * sizeof(uint64_t));
    }

    uint64_t c_inv[field_words];
    field_inv(c_inv, c);
    field_mulEqual(c_inv, mul, field_words);

    for (size_t i = elements_len - 1; i > 0; --i)
    {
        field_mul(&result[i * field_words], &result[(i - 1) * field_words], c_inv);
        field_mulEqual(c_inv, &elements[i * field_words], field_words);
    }

    memcpy(&result[0], c_inv, field_words * sizeof(uint64_t));
}

void field_add(uint64_t *sum, const uint64_t *a, const uint64_t *b, const size_t field_words)
{
#ifdef PROFILE_OP_COUNTS
    add_count++;
#endif
    for (size_t i = 0; i < field_words; i++)
        sum[i] = a[i] ^ b[i];
}

void field_addEqual(uint64_t *a, const uint64_t *b, const size_t field_words)
{
#ifdef PROFILE_OP_COUNTS
    add_count++;
#endif
    for (size_t i = 0; i < field_words; i++)
        a[i] ^= b[i];
}

void field_sub(uint64_t *difference, const uint64_t *a, const uint64_t *b, const size_t field_words)
{
    field_add(difference, a, b, field_words);
}

void field_subEqual(uint64_t *a, const uint64_t *b, const size_t field_words)
{
    field_addEqual(a, b, field_words);
}

void field_mul(uint64_t *product, const uint64_t *a, const uint64_t *b)
{
#ifdef PROFILE_OP_COUNTS
    field_mul_count++;
#endif

#if defined(USE_PREON128A) || defined(USE_PREON128B) || defined(USE_PREON128C)
    GF192_mul(product, a, b);
#elif defined(USE_PREON192A) || defined(USE_PREON192B) || defined(USE_PREON192C)
    GF256_mul(product, a, b);
#elif defined(USE_PREON256A) || defined(USE_PREON256B) || defined(USE_PREON256C)
    GF320_mul(product, a, b);
#endif
}

void GF192_mulEqual(uint64_t *a, const uint64_t *b)
{
    uint64_t temp[3];
    GF192_mul(temp, a, b);
    memcpy(a, temp, 3 * sizeof(uint64_t));
}

void GF256_mulEqual(uint64_t *a, const uint64_t *b)
{
    uint64_t temp[4];
    GF256_mul(temp, a, b);
    memcpy(a, temp, 4 * sizeof(uint64_t));
}

void GF320_mulEqual(uint64_t *a, const uint64_t *b)
{
    uint64_t temp[5];
    GF320_mul(temp, a, b);
    memcpy(a, temp, 5 * sizeof(uint64_t));
}

void field_mulEqual(uint64_t *a, const uint64_t *b, const size_t field_words)
{
// Using copy-then-multiply for now
#if defined(USE_PREON128A) || defined(USE_PREON128B) || defined(USE_PREON128C)
    GF192_mulEqual(a, b);
#elif defined(USE_PREON192A) || defined(USE_PREON192B) || defined(USE_PREON192C)
    GF256_mulEqual(a, b);
#elif defined(USE_PREON256A) || defined(USE_PREON256B) || defined(USE_PREON256C)
    GF320_mulEqual(a, b);
#endif
}

void field_swap_with_tmp(uint64_t *a, uint64_t *b, uint64_t *tmp, const size_t field_bytesize)
{
    memcpy(tmp, a, field_bytesize);
    memcpy(a, b, field_bytesize);
    memcpy(b, tmp, field_bytesize);
}

void field_swap(uint64_t *a, uint64_t *b, const size_t field_bytesize)
{
    for (size_t i = 0; i < (field_bytesize / sizeof(uint64_t)); i++)
    {
        uint64_t tmp = a[i];
        a[i] = b[i];
        b[i] = tmp;
    }
}

// GF192 = GF64[y]/<y^3+y+1>
// GF64 = GF2[x]/<x^64+x^4+x^3+x+1>
void GF192_inv(uint64_t *inv, const uint64_t *base)
{
    // Computing base^{-1} with determinent method
    // a_ prefix variables means adding the following terms seperated by _
    // m_ prefix variables means multiplying the following terms seperated by _
    // All addtion and multiplication of terms computing over GF64
    /*
        {
            {base[0]+base[2], base[1]        , base[0]},
            {base[0]+base[1], base[0]+base[2], base[1]},
            {base[1]        , base[0]        , base[2]},
        }
    */

    uint64_t m00 = gf264_mul(base[0], base[0]);
    uint64_t m01 = gf264_mul(base[0], base[1]);
    uint64_t m02 = gf264_mul(base[0], base[2]);
    uint64_t m11 = gf264_mul(base[1], base[1]);
    uint64_t m12 = gf264_mul(base[1], base[2]);
    uint64_t m22 = gf264_mul(base[2], base[2]);
    uint64_t deltas[3] = {0};
    // deltas[0]
    /*
        {
            {base[1]        , base[0]},
            {base[0]+base[2], base[1]}
        }
    */
    deltas[0] = m00 ^ m02 ^ m11;
    // deltas[1]
    /*
        {
            {base[0]+base[2], base[0]},
            {base[0]+base[1], base[1]},
        }
    */
    deltas[1] = m00 ^ m12;
    // deltas[2]
    /*
        {
            {base[0]+base[2], base[1]        },
            {base[0]+base[1], base[0]+base[2]}
        }
    */
    deltas[2] = m00 ^ m01 ^ m11 ^ m22;

#ifdef INV_CHECK
    uint64_t checks[2][3] = {
        {base[0] ^ base[2], base[1], base[0]},
        {base[0] ^ base[1], base[0] ^ base[2], base[1]}};
    for (size_t i = 0; i < 2; i++)
    {
        uint64_t checking = 0;
        for (size_t j = 0; j < 3; j++)
            checking ^= gf264_mul(checks[i][j], deltas[j]);

        assert(checking == 0);
    }
#endif

    // delta
    uint64_t delta = gf264_mul(base[1], deltas[0]);
    delta ^= gf264_mul(base[0], deltas[1]);
    delta ^= gf264_mul(base[2], deltas[2]);

    uint64_t delta_inv = gf264_inv(delta);
    for (size_t i = 0; i < 3; i++)
        inv[i] = gf264_mul(deltas[i], delta_inv);
}

void GF192_mul(uint64_t *product, const uint64_t *a, const uint64_t *b)
{
    product[2] = gf264_mul(a[1], b[0]);
    product[1] = gf264_mul(a[0], b[1]);
    product[2] ^= product[1];
    product[1] = product[2];
    product[0] = gf264_mul(a[0], b[0]);
    product[1] ^= product[0];

    product[2] ^= gf264_mul(a[2], b[2]);
    product[1] ^= gf264_mul(a[2], b[1]);
    product[1] ^= gf264_mul(a[1], b[2]);
    product[0] ^= gf264_mul(a[2], b[0]);
    product[0] ^= gf264_mul(a[1], b[1]);
    product[0] ^= gf264_mul(a[0], b[2]);
}

// GF256 = GF64[y]/<y^4+y^3+y^2+GF64(2)y+GF64(2)>
// GF64 = GF2[x]/<x^64+x^4+x^3+x+1>
void GF256_inv(uint64_t *inv, const uint64_t *base)
{
    // Computing base^{-1} with determinent method
    // a_ prefix variables means adding the following terms seperated by _
    // m_ prefix variables means multiplying the following terms seperated by _
    // All addtion and multiplication of terms computing over GF64
    /*
        {
            {GF64(3)*base[0]+base[2]+base[3], base[1]+base[2]                , base[0]+base[1]        , base[0]},
            {GF64(3)*base[1]+base[2]        , GF64(3)*base[0]+base[1]+base[3], base[0]+base[2]        , base[1]},
            {GF64(2)*(base[0]+base[2])      , GF64(2)*base[1]                , GF64(2)*base[0]+base[3], base[2]},
            {GF64(2)*(base[1]+base[2])      , GF64(2)*(base[0]+base[1])      , GF64(2)*base[0]        , base[3]}
        }
    */
    uint64_t m00 = gf264_mul(base[0], base[0]);
    uint64_t m01 = gf264_mul(base[0], base[1]);
    uint64_t m02 = gf264_mul(base[0], base[2]);
    uint64_t m03 = gf264_mul(base[0], base[3]);
    uint64_t m11 = gf264_mul(base[1], base[1]);
    uint64_t m12 = gf264_mul(base[1], base[2]);
    uint64_t m13 = gf264_mul(base[1], base[3]);
    uint64_t m22 = gf264_mul(base[2], base[2]);
    uint64_t m23 = gf264_mul(base[2], base[3]);
    uint64_t m33 = gf264_mul(base[3], base[3]);
    uint64_t a01 = base[0] ^ base[1];
    uint64_t a02 = base[0] ^ base[2];
    uint64_t a12 = base[1] ^ base[2];

    uint64_t twice_0 = gf264_twice(base[0]);
    uint64_t twice_1 = gf264_twice(base[1]);
    uint64_t twice_m00 = gf264_twice(m00);
    uint64_t twice_m01 = gf264_twice(m01);
    uint64_t twice_a01 = gf264_twice(a01);

    uint64_t a_m00_m02_m11 = m00 ^ m02 ^ m11;
    uint64_t twice_a_m00_m02_m11 = gf264_twice(a_m00_m02_m11);

    // dc = delta components
    uint64_t dc_00 = twice_m01 ^ m02 ^ m13 ^ m22;
    uint64_t dc_01 = twice_m00 ^ m02 ^ m03 ^ m12;
    uint64_t dc_02 = a_m00_m02_m11 ^ m01;
    uint64_t dc_10 = gf264_twice(twice_a_m00_m02_m11) ^ gf264_twice(m01 ^ m03 ^ m23) ^ twice_a_m00_m02_m11;
    uint64_t dc_11 = gf264_twice(twice_m01) ^ gf264_twice(m02 ^ m13 ^ m22);
    uint64_t dc_12 = gf264_twice(twice_m00) ^ gf264_twice(m01 ^ m02 ^ m11 ^ m12) ^ dc_02 ^ m12 ^ m13 ^ m22 ^ m23 ^ m33;

    uint64_t deltas[4] = {0};

    // deltas[0]
    /*
        {
            {base[1]+base[2]                , base[0]+base[1]        , base[0]},
            {GF64(3)*base[0]+base[1]+base[3], base[0]+base[2]        , base[1]},
            {GF64(2)*base[1]                , GF64(2)*base[0]+base[3], base[2]},
        }
    */
    deltas[0] = gf264_mul(a12, dc_00);
    deltas[0] ^= gf264_mul(twice_0 ^ a01 ^ base[3], dc_01);
    deltas[0] ^= gf264_mul(twice_1, dc_02);

    // deltas[1]
    /*
        {
            {GF64(3)*base[0]+base[2]+base[3], base[0]+base[1]        , base[0]},
            {GF64(3)*base[1]+base[2]        , base[0]+base[2]        , base[1]},
            {GF64(2)*(base[0]+base[2])      , GF64(2)*base[0]+base[3], base[2]},
        }
    */
    deltas[1] = gf264_mul(twice_0 ^ a02 ^ base[3], dc_00);
    deltas[1] ^= gf264_mul(twice_1 ^ a12, dc_01);
    deltas[1] ^= gf264_mul(gf264_twice(a02), dc_02);

    // deltas[2]
    /*
        {
            {GF64(3)*base[0]+base[2]+base[3], base[1]+base[2]                , base[0]},
            {GF64(3)*base[1]+base[2]        , GF64(3)*base[0]+base[1]+base[3], base[1]},
            {GF64(2)*(base[0]+base[2])      , GF64(2)*base[1]                , base[2]},
        }
    */
    deltas[2] = gf264_mul(base[0], dc_10);
    deltas[2] ^= gf264_mul(base[1], dc_11);
    deltas[2] ^= gf264_mul(base[2], dc_12);

    // deltas[3]
    /*
        {
            {GF64(3)*base[0]+base[2]+base[3], base[1]+base[2]                , base[0]+base[1]        },
            {GF64(3)*base[1]+base[2]        , GF64(3)*base[0]+base[1]+base[3], base[0]+base[2]        },
            {GF64(2)*(base[0]+base[2])      , GF64(2)*base[1]                , GF64(2)*base[0]+base[3]},
        }
    */
    deltas[3] = gf264_mul(a01, dc_10);
    deltas[3] ^= gf264_mul(a02, dc_11);
    deltas[3] ^= gf264_mul(twice_0 ^ base[3], dc_12);

#ifdef INV_CHECK
    uint64_t checks[3][4] = {
        {twice_0 ^ a02 ^ base[3], a12, a01, base[0]},
        {twice_1 ^ a12, twice_0 ^ a01 ^ base[3], a02, base[1]},
        {gf264_twice(a02), twice_1, twice_0 ^ base[3], base[2]}};
    for (size_t i = 0; i < 3; i++)
    {
        uint64_t checking = 0;
        for (size_t j = 0; j < 4; j++)
            checking ^= gf264_mul(checks[i][j], deltas[j]);

        assert(checking == 0);
    }
#endif
    // delta
    uint64_t delta = gf264_mul(gf264_twice(a12), deltas[0]);
    delta ^= gf264_mul(twice_a01, deltas[1]);
    delta ^= gf264_mul(twice_0, deltas[2]);
    delta ^= gf264_mul(base[3], deltas[3]);

    uint64_t delta_inv = gf264_inv(delta);
    for (size_t i = 0; i < 4; i++)
        inv[i] = gf264_mul(deltas[i], delta_inv);
}
void GF256_mul(uint64_t *product, const uint64_t *a, const uint64_t *b)
{
    product[0] = gf264_mul(a[0], b[0]);
    product[1] = gf264_mul(a[0], b[1]);
    product[1] ^= gf264_mul(a[1], b[0]);

    uint64_t tmp = gf264_mul(a[0], b[2]);
    tmp ^= gf264_mul(a[1], b[1]);
    tmp ^= gf264_mul(a[2], b[0]);

    product[2] = gf264_twice(product[0]);
    product[3] = gf264_twice(product[1]);

    product[0] ^= product[2];
    product[1] ^= product[3];

    product[0] ^= tmp;
    product[1] ^= tmp;

    tmp = gf264_twice(tmp);
    product[2] ^= tmp;
    product[3] ^= tmp;

    product[3] ^= gf264_mul(a[3], b[3]);
    product[2] ^= gf264_mul(a[3], b[2]);
    product[2] ^= gf264_mul(a[2], b[3]);
    product[1] ^= gf264_mul(a[3], b[1]);
    product[1] ^= gf264_mul(a[2], b[2]);
    product[1] ^= gf264_mul(a[1], b[3]);
    product[0] ^= gf264_mul(a[3], b[0]);
    product[0] ^= gf264_mul(a[2], b[1]);
    product[0] ^= gf264_mul(a[1], b[2]);
    product[0] ^= gf264_mul(a[0], b[3]);
}

// GF320 = GF64[y]/<y^5+y^2+1>
// GF64 = GF2[x]/<x^64+x^4+x^3+x+1>
void GF320_inv(uint64_t *inv, const uint64_t *base)
{
    // Computing base^{-1} with determinent method
    // a_ prefix variables means adding the following terms seperated by _
    // m_ prefix variables means multiplying the following terms seperated by _
    // All addtion and multiplication of terms computing over GF64
    /*
        {
            {base[1]+base[4]        , base[0]+base[3], base[2]        , base[1]        , base[0]},
            {base[0]+base[2]        , base[1]+base[4], base[0]+base[3], base[2]        , base[1]},
            {base[0]+base[1]+base[3], base[0]+base[2], base[1]+base[4], base[0]+base[3], base[2]},
            {base[2]                , base[1]        , base[0]        , base[4]        , base[3]},
            {base[0]+base[3]        , base[2]        , base[1]        , base[0]        , base[4]}
        }
    */
    // Prepare all elements
    uint64_t m00 = gf264_mul(base[0], base[0]);
    uint64_t m01 = gf264_mul(base[0], base[1]);
    uint64_t m02 = gf264_mul(base[0], base[2]);
    uint64_t m03 = gf264_mul(base[0], base[3]);
    uint64_t m04 = gf264_mul(base[0], base[4]);
    uint64_t m11 = gf264_mul(base[1], base[1]);
    uint64_t m12 = gf264_mul(base[1], base[2]);
    uint64_t m13 = gf264_mul(base[1], base[3]);
    uint64_t m14 = gf264_mul(base[1], base[4]);
    uint64_t m22 = gf264_mul(base[2], base[2]);
    uint64_t m23 = gf264_mul(base[2], base[3]);
    uint64_t m24 = gf264_mul(base[2], base[4]);
    uint64_t m33 = gf264_mul(base[3], base[3]);
    uint64_t m34 = gf264_mul(base[3], base[4]);
    uint64_t m44 = gf264_mul(base[4], base[4]);
    uint64_t a_m01_m24 = m01 ^ m24;
    uint64_t a_m02_m11 = m02 ^ m11;
    uint64_t a_m03_m12 = m03 ^ m12;
    uint64_t a_m04_m13 = m04 ^ m13;
    uint64_t a_m13_m22 = m13 ^ m22;
    uint64_t a_m00_m03_m12 = m00 ^ m03 ^ m12;
    uint64_t a_m01_m13_m22 = m01 ^ a_m13_m22;
    uint64_t a_m02_m13_m34 = m02 ^ m13 ^ m34;
    uint64_t a_m03_m12_m23 = a_m03_m12 ^ m23;
    uint64_t a_m03_m24_m33 = m03 ^ m24 ^ m33;
    uint64_t a_m00_m02_m11_m14 = m00 ^ a_m02_m11 ^ m14;
    uint64_t a_m00_m03_m14_m44 = m00 ^ m03 ^ m14 ^ m44;
    uint64_t a_m00_m12_m24_m33 = m00 ^ m12 ^ m24 ^ m33;
    uint64_t a_m01_m04_m13_m24 = m01 ^ a_m04_m13 ^ m24;
    uint64_t a_m02_m11_m14_m23 = a_m02_m11 ^ m14 ^ m23;
    uint64_t a_m03_m13_m22_m33 = m03 ^ a_m13_m22 ^ m33;
    uint64_t a_m00_m01_m03_m12_m24 = m00 ^ m01 ^ a_m03_m12 ^ m24;
    uint64_t a_m01_m02_m11_m13_m22 = m01 ^ a_m02_m11 ^ a_m13_m22;
    uint64_t a_m02_m04_m14_m23_m34 = m02 ^ m04 ^ m14 ^ m23 ^ m34;
    uint64_t a_m00_m02_m03_m11_m23_m44 = m00 ^ m02 ^ m03 ^ m11 ^ m23 ^ m44;
    uint64_t a_m01_m02_m04_m13_m22_m34 = m01 ^ m02 ^ m04 ^ a_m13_m22 ^ m34;

    // Compute deltas[0] ~ deltas[4] with reusing as much computation as possible
    uint64_t deltas[5] = {0};
    uint64_t temp = 0;

    // deltas[0]
    /*
        {
            {base[0]+base[3], base[2]        , base[1]        , base[0]},
            {base[1]+base[4], base[0]+base[3], base[2]        , base[1]},
            {base[0]+base[2], base[1]+base[4], base[0]+base[3], base[2]},
            {base[1]        , base[0]        , base[4]        , base[3]}
        }
    */
    deltas[0] = gf264_mul(a_m00_m12_m24_m33, a_m03_m24_m33);
    deltas[0] ^= gf264_mul(a_m02_m11_m14_m23, a_m02_m13_m34);
    deltas[0] ^= gf264_mul(a_m04_m13, a_m00_m03_m14_m44);
    deltas[0] ^= gf264_mul(a_m00_m02_m11_m14, a_m02_m11);
    deltas[0] ^= gf264_mul(a_m01_m04_m13_m24, a_m00_m03_m12);
    deltas[0] ^= gf264_mul(a_m03_m12_m23, a_m01_m13_m22);

    // deltas[1]
    /*
    {
        {base[1]+base[4]        , base[2]        , base[1]        , base[0]},
        {base[0]+base[2]        , base[0]+base[3], base[2]        , base[1]},
        {base[0]+base[1]+base[3], base[1]+base[4], base[0]+base[3], base[2]},
        {base[2]                , base[0]        , base[4]        , base[3]}
    }
    */
    deltas[1] = gf264_mul(a_m01_m02_m04_m13_m22_m34, a_m03_m24_m33);
    deltas[1] ^= gf264_mul(a_m01_m24, a_m02_m13_m34);
    deltas[1] ^= gf264_mul(a_m00_m02_m11_m14, a_m00_m03_m14_m44);
    deltas[1] ^= gf264_mul(a_m00_m01_m03_m12_m24, a_m02_m11);
    deltas[1] ^= gf264_mul(a_m02_m04_m14_m23_m34, a_m00_m03_m12);
    deltas[1] ^= gf264_mul(a_m03_m13_m22_m33, a_m01_m13_m22);

    // deltas[2]
    /*
        {
            {base[1]+base[4]        , base[0]+base[3], base[1]        , base[0]},
            {base[0]+base[2]        , base[1]+base[4], base[2]        , base[1]},
            {base[0]+base[1]+base[3], base[0]+base[2], base[0]+base[3], base[2]},
            {base[2]                , base[1]        , base[4]        , base[3]}
        }
    */
    deltas[2] = gf264_mul(a_m00_m02_m03_m11_m23_m44, a_m03_m24_m33);
    deltas[2] ^= gf264_mul(a_m01_m24, a_m03_m12_m23);
    temp = m00 ^ m02 ^ m11 ^ m14;
    deltas[2] ^= gf264_mul(temp, a_m01_m04_m13_m24);
    deltas[2] ^= gf264_mul(a_m01_m02_m11_m13_m22, a_m02_m11);
    deltas[2] ^= gf264_mul(a_m02_m04_m14_m23_m34, a_m04_m13);
    temp = m02 ^ m11 ^ m14 ^ m23;
    deltas[2] ^= gf264_mul(temp, a_m03_m13_m22_m33);

    // deltas[3]
    /*
        {
            {base[1]+base[4]        , base[0]+base[3], base[2]        , base[0]},
            {base[0]+base[2]        , base[1]+base[4], base[0]+base[3], base[1]},
            {base[0]+base[1]+base[3], base[0]+base[2], base[1]+base[4], base[2]},
            {base[2]                , base[1]        , base[0]        , base[3]}
        }
    */
    deltas[3] = gf264_mul(a_m00_m02_m11_m14, a_m00_m02_m11_m14);
    deltas[3] ^= gf264_mul(a_m04_m13, a_m00_m01_m03_m12_m24);
    deltas[3] ^= gf264_mul(a_m00_m03_m12, a_m01_m02_m11_m13_m22);
    deltas[3] ^= gf264_mul(a_m03_m13_m22_m33, a_m00_m12_m24_m33);
    deltas[3] ^= gf264_mul(a_m03_m12_m23, a_m01_m02_m04_m13_m22_m34);
    deltas[3] ^= gf264_mul(a_m02_m13_m34, a_m00_m02_m03_m11_m23_m44);

    // deltas[4]
    /*
        {
            {base[1]+base[4]        , base[0]+base[3], base[2]        , base[1]        },
            {base[0]+base[2]        , base[1]+base[4], base[0]+base[3], base[2]        },
            {base[0]+base[1]+base[3], base[0]+base[2], base[1]+base[4], base[0]+base[3]},
            {base[2]                , base[1]        , base[0]        , base[4]        }
        }
    */
    deltas[4] = gf264_mul(a_m01_m24, a_m00_m02_m11_m14);
    deltas[4] ^= gf264_mul(a_m02_m11_m14_m23, a_m00_m01_m03_m12_m24);
    deltas[4] ^= gf264_mul(a_m01_m13_m22, a_m01_m02_m11_m13_m22);
    deltas[4] ^= gf264_mul(a_m02_m04_m14_m23_m34, a_m00_m12_m24_m33);
    deltas[4] ^= gf264_mul(a_m01_m04_m13_m24, a_m01_m02_m04_m13_m22_m34);
    deltas[4] ^= gf264_mul(a_m00_m03_m14_m44, a_m00_m02_m03_m11_m23_m44);

    uint64_t a03 = base[0] ^ base[3];
#ifdef INV_CHECK
    uint64_t a01 = base[0] ^ base[1];
    uint64_t a02 = base[0] ^ base[2];
    uint64_t a14 = base[1] ^ base[4];
    uint64_t checks[4][5] = {
        {a14, a03, base[2], base[1], base[0]},
        {a02, a14, a03, base[2], base[1]},
        {a01 ^ base[3], a02, a14, a03, base[2]},
        {base[2], base[1], base[0], base[4], base[3]}};
    for (size_t i = 0; i < 4; i++)
    {
        uint64_t checking = 0;
        for (size_t j = 0; j < 5; j++)
            checking ^= gf264_mul(checks[i][j], deltas[j]);

        assert(checking == 0);
    }
#endif

    // delta
    uint64_t delta = gf264_mul(a03, deltas[0]);
    delta ^= gf264_mul(base[2], deltas[1]);
    delta ^= gf264_mul(base[1], deltas[2]);
    delta ^= gf264_mul(base[0], deltas[3]);
    delta ^= gf264_mul(base[4], deltas[4]);

    uint64_t delta_inv = gf264_inv(delta);
    for (size_t i = 0; i < 5; i++)
        inv[i] = gf264_mul(deltas[i], delta_inv);
}
void GF320_mul(uint64_t *product, const uint64_t *a, const uint64_t *b)
{
    product[4] = gf264_mul(a[0], b[3]);
    product[4] ^= gf264_mul(a[1], b[2]);
    product[4] ^= gf264_mul(a[2], b[1]);
    product[4] ^= gf264_mul(a[3], b[0]);
    product[3] = gf264_mul(a[0], b[2]);
    product[3] ^= gf264_mul(a[1], b[1]);
    product[3] ^= gf264_mul(a[2], b[0]);
    product[2] = gf264_mul(a[0], b[1]);
    product[2] ^= gf264_mul(a[1], b[0]);
    product[1] = gf264_mul(a[0], b[0]);
    product[0] = product[2];
    product[2] ^= product[4];
    product[4] ^= product[1];
    product[2] ^= product[1];
    product[1] ^= product[3];

    product[4] ^= gf264_mul(a[4], b[4]);
    product[3] ^= gf264_mul(a[3], b[4]);
    product[3] ^= gf264_mul(a[4], b[3]);
    product[2] ^= gf264_mul(a[2], b[4]);
    product[2] ^= gf264_mul(a[3], b[3]);
    product[2] ^= gf264_mul(a[4], b[2]);
    product[1] ^= gf264_mul(a[1], b[4]);
    product[1] ^= gf264_mul(a[2], b[3]);
    product[1] ^= gf264_mul(a[3], b[2]);
    product[1] ^= gf264_mul(a[4], b[1]);
    product[0] ^= gf264_mul(a[0], b[4]);
    product[0] ^= gf264_mul(a[1], b[3]);
    product[0] ^= gf264_mul(a[2], b[2]);
    product[0] ^= gf264_mul(a[3], b[1]);
    product[0] ^= gf264_mul(a[4], b[0]);
}
