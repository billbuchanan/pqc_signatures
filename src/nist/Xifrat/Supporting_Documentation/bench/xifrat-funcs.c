/* DannyNiu/NJF, 2022-04-03. Public Domain. */

#include "xifrat-funcs.h"
// #include "../src-crypto/endian.h"

#define P 16
#define P2 256

static uint16_t f_table[P2] = {
    10, 11,  0,  3, 12,  4,  1,  5, 15,  6,  8, 14,  2,  9,  7, 13,
    15,  8,  9,  7,  2, 13,  5,  1, 10, 14, 11,  6, 12,  0,  3,  4,
    +2,  3,  6, 11, 15,  5, 13,  4, 12,  0,  7,  9, 10, 14,  8,  1,
    +0,  5, 10,  4, 14,  3,  8, 11,  9,  2,  1, 12,  6, 15, 13,  7,
    +8, 15,  1, 12,  3, 14,  0,  9, 11, 13, 10,  4,  7,  5,  2,  6,
    +6,  4,  2,  5,  9, 11,  7,  3, 14, 10, 13, 15,  0, 12,  1,  8,
    13, 14,  7,  9,  5, 15,  2, 12,  4,  8,  6, 11,  1,  3,  0, 10,
    12,  7, 14,  8, 10,  1,  4, 13,  2,  9,  3,  0, 15,  6, 11,  5,
    +5,  0, 11,  6, 13,  2, 15, 10,  1,  3,  9,  7,  4,  8, 14, 12,
    14, 13, 12,  1,  0,  8,  3,  7,  6, 15,  4, 10,  9,  2,  5, 11,
    +1,  9,  8, 14,  4, 12, 10, 15,  5,  7,  0,  3, 13, 11,  6,  2,
    +7, 12, 13, 15, 11,  9,  6, 14,  3,  1,  2,  5,  8,  4, 10,  0,
    +9,  1, 15, 13,  6,  7, 11,  8,  0, 12,  5,  2, 14, 10,  4,  3,
    +4,  6,  3,  0,  1, 10, 12,  2, 13, 11, 14,  8,  5,  7,  9, 15,
    11, 10,  5,  2,  7,  6,  9,  0,  8,  4, 15, 13,  3,  1, 12, 14,
    +3,  2,  4, 10,  8,  0, 14,  6,  7,  5, 12,  1, 11, 13, 15,  9,
};

static uint64_t f_wide(uint64_t a, uint64_t b)
{
    const static uint64_t mbase = 0x1111111111111111;
    uint64_t m, n, u, v, ret;
    int16_t i;

    a &= UINT64_MAX;
    b &= UINT64_MAX;
    ret = 0;

    for(i=0; i<P2; i++)
    {
        m = (i / P) * mbase;
        n = (i % P) * mbase;

        u = ~(a ^ m);
        u &= (u >> 1) & (u >> 2) & (u >> 3) & mbase;
        u *= 15;
        
        v = ~(b ^ n);
        v &= (v >> 1) & (v >> 2) & (v >> 3) & mbase;
        v *= 15;

        ret |= (f_table[i] * mbase) & u & v;
    }

    return ret;
}

uint64_t xifrat_Blk(uint64_t a, uint64_t b)
{
    uint64_t u, v;
    int i;
    
    u = a &= UINT64_MAX;
    v = b &= UINT64_MAX;
    
    for(i=1; i<21; i++)
    {
        a = (a >> 4 | a << 60) & UINT64_MAX;
        b = (b >> 4 | b << 60) & UINT64_MAX;
        u = f_wide(u, a);
        v = f_wide(v, b);
    }

    return f_wide(f_wide(f_wide(u, v), u), v);
}

void xifrat_Vec(uint64vec_t out, const uint64vec_t a, const uint64vec_t b)
{
    uint64vec_t u, v;
    int i, j;

    for(j=0; j<VLEN; j++)
    {
        u[j] = a[j] & UINT64_MAX;
        v[j] = b[j] & UINT64_MAX;
    }
    
    for(i=1; i<VLEN; i++)
    {
        for(j=0; j<VLEN; j++)
        {
            u[j] = xifrat_Blk(u[j], a[(i+j)%VLEN]);
            v[j] = xifrat_Blk(v[j], b[(i+j)%VLEN]);
        }
    }

    for(j=0; j<VLEN; j++)
        out[j] = xifrat_Blk(xifrat_Blk(xifrat_Blk(u[j], v[j]), u[j]), v[j]);
}

void xifrat_Dup(uint64dup_t out, const uint64dup_t a, const uint64dup_t b)
{
    uint64dup_t u, v;
    int i, j;

    for(j=0; j<DLEN*VLEN; j++)
    {
        u[j] = a[j] & UINT64_MAX;
        v[j] = b[j] & UINT64_MAX;
    }
    
    for(i=1; i<DLEN; i++)
    {
        for(j=0; j<DLEN; j++)
        {
            xifrat_Vec(u+j*VLEN, u+j*VLEN, a+((i+j)%DLEN)*VLEN);
            xifrat_Vec(v+j*VLEN, v+j*VLEN, b+((i+j)%DLEN)*VLEN);
        }
    }

    for(j=0; j<DLEN; j++)
    {
        xifrat_Vec(out+j*VLEN, u+j*VLEN, v+j*VLEN);
        xifrat_Vec(u+j*VLEN, out+j*VLEN, u+j*VLEN);
        xifrat_Vec(out+j*VLEN, u+j*VLEN, v+j*VLEN);
    }
}
