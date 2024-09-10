#include "gf256.h"
#include <stdlib.h>
#include <string.h>
#include "gf256-avx2-polytable-ct.h"

static int memeq_ct(const uint8_t* a, const uint8_t* b, uint16_t n) {
    int res = 1;
    for(uint16_t i=0; i<n; i++)
        res &= (a[i] == b[i]);
    return res;
}

/*************************************************/
/***********      FIELD OPERATIONS     ***********/
/*************************************************/

#define MODULUS 0x1B
#define MASK_LSB_PER_BIT ((uint64_t)0x0101010101010101)
#define MASK_MSB_PER_BIT (MASK_LSB_PER_BIT*0x80)
#define MASK_XLSB_PER_BIT (MASK_LSB_PER_BIT*0xFE)

uint8_t gf256_add(uint8_t a, uint8_t b) {
    return a^b;
}

uint8_t gf256_mul(uint8_t a, uint8_t b) {
    uint8_t r;
    r = (-(b>>7    ) & a);
    r = (-(b>>6 & 1) & a) ^ (-(r>>7) & MODULUS) ^ (r+r);
    r = (-(b>>5 & 1) & a) ^ (-(r>>7) & MODULUS) ^ (r+r);
    r = (-(b>>4 & 1) & a) ^ (-(r>>7) & MODULUS) ^ (r+r);
    r = (-(b>>3 & 1) & a) ^ (-(r>>7) & MODULUS) ^ (r+r);
    r = (-(b>>2 & 1) & a) ^ (-(r>>7) & MODULUS) ^ (r+r);
    r = (-(b>>1 & 1) & a) ^ (-(r>>7) & MODULUS) ^ (r+r);
 return (-(b    & 1) & a) ^ (-(r>>7) & MODULUS) ^ (r+r);
}

void gf256_innerproduct_batched(uint8_t* out_batched, uint8_t* a_batched, uint8_t* b, uint32_t bytes, uint32_t n_batched) {
    for(uint32_t j=0; j<n_batched; j++)
        out_batched[j] = 0;
    uint32_t ind = 0;
    for(uint32_t i=0; i<bytes; i++) {
        for(uint32_t j=0; j<n_batched; j++) {
            out_batched[j] ^= gf256_mul(a_batched[ind],b[i]);
            ind++;
        }
    }
}

// vz[] += vx[]
void gf256_add_tab(uint8_t* vz, const uint8_t* vx, uint32_t bytes) {
    for(uint32_t i=0; i<bytes; i++)
        vz[i] ^= vx[i];
}

// vz[] = vx[] * y
void gf256_mul_tab(uint8_t* vz, const uint8_t* vx, uint8_t y, uint32_t bytes) {
    for(uint32_t i=0; i<bytes; i++)
        vz[i] = gf256_mul(vx[i], y);
}

// vz[] += vx[] * y
void gf256_muladd_tab(uint8_t* vz, uint8_t y, const uint8_t* vx, uint32_t bytes) {
    for(uint32_t i=0; i<bytes; i++)
        vz[i] = vz[i] ^ gf256_mul(vx[i], y);
}

void gf256_vec_mat128cols_muladd_polytable_avx2_ct(void* vz, void const* vx, void const* my, uint64_t m, uint64_t scaling);

// vz[] += vx[1][] * y[1] + ... + vx[nb][] * y[nb]
void gf256_matcols_muladd(uint8_t* vz, const uint8_t* y, const uint8_t* vx, uint32_t nb, uint32_t bytes) {
    uint32_t* unreduced_vz = (uint32_t*) aligned_alloc(32, ((sizeof(uint32_t)*bytes+31)>>5)<<5);
    memset(unreduced_vz, 0, sizeof(uint32_t)*bytes);
    size_t ind=0;
    for(uint32_t j=0; j<nb; j++) {
        for(uint32_t i=0; i<bytes; i++) {
            unreduced_vz[i] ^= gf256_mul(vx[ind++], y[j]);
        }
    }
    for(uint32_t i=0; i<bytes; i++)
        vz[i] ^= unreduced_vz[i];
    free(unreduced_vz);
}

// vz[] += vx[1][] * y[1] + ... + vx[nb][] * y[nb]
void gf256_matcols_muladd_scaled(uint8_t* vz, const uint8_t* y, const uint8_t* vx, uint32_t nb, uint32_t bytes, uint32_t scale) {
    uint32_t* unreduced_vz = (uint32_t*) aligned_alloc(32, ((sizeof(uint32_t)*bytes+31)>>5)<<5);
    memset(unreduced_vz, 0, sizeof(uint32_t)*bytes);
    size_t ind=0;
    for(uint32_t j=0; j<nb; j++) {
        for(uint32_t i=0; i<bytes; i++) {
            unreduced_vz[i] ^= gf256_mul(vx[ind++], y[scale*j]);
        }
    }
    for(uint32_t i=0; i<bytes; i++)
        vz[scale*i] ^= unreduced_vz[i];
    free(unreduced_vz);
}

// vz[] += vx[1][] * y[1] + ... + vx[nb][] * y[nb]
// bytes must be multiple of 16
void gf256_mat16cols_muladd(uint8_t* vz, const uint8_t* y, const uint8_t* vx, uint32_t nb, uint32_t bytes) {
    uint32_t nb_packs = bytes>>4;
    uint32_t nb_even = nb-nb%2;

    for(uint32_t i=0; i<nb_packs; i++)
        gf256_vec_mat16cols_muladd_polytable_avx2_ct(vz+16*i, y, vx+16*i, nb_even, nb_packs);
    if(nb > nb_even) {
        uint8_t s = y[nb_even];
        const uint8_t* cur = vx+bytes*nb_even;
        for(uint32_t i=0; i<bytes; i++)
            vz[i] ^= gf256_mul(s, *(cur++));
    }
}

// vz[] = vz[] * y + vx[]
void gf256_mul_and_add_tab(uint8_t* vz, uint8_t y, const uint8_t* vx, uint32_t bytes) {
    for(uint32_t i=0; i<bytes; i++)
        vz[i] = vx[i] ^ gf256_mul(vz[i], y);
}

/*************************************************/
/***********      FIELD EXTENSION      ***********/
/*************************************************/

// X2 + X + 32 = 0
static void gf2to16_mul(uint8_t res[2], const uint8_t a[2], const uint8_t b[2]) {
    uint8_t leading = gf256_mul(a[1], b[1]);
    uint8_t cnst = gf256_mul(a[0], b[0]);
    uint8_t sum_a = gf256_add(a[0], a[1]);
    uint8_t sum_b = gf256_add(b[0], b[1]);
    res[0] = gf256_add(
        cnst,
        gf256_mul(leading, 0x20)
    );
    res[1] = gf256_add(
        gf256_mul(sum_a,sum_b),
        cnst
    );
}

static void gf2to16_add(uint8_t res[2], const uint8_t a[2], const uint8_t b[2]) {
    res[0] = a[0]^b[0];
    res[1] = a[1]^b[1];
}

static void gf2to16_mul_0x2000(uint8_t res[2], const uint8_t a[2]) {
    res[0] = gf256_mul(gf256_mul(a[1], 32), 0x20);
    res[1] = gf256_mul(gf256_add(a[0], a[1]), 0x20);
}

// X2 + X + (256*32) = 0
void gf2to32_mul(uint8_t res[4], const uint8_t a[4], const uint8_t b[4]) {
    uint8_t leading[2], cnst[2], sum_a[2], sum_b[2];
    gf2to16_mul(leading, &a[2], &b[2]);
    gf2to16_mul(cnst, &a[0], &b[0]);
    gf2to16_add(sum_a, &a[0], &a[2]);
    gf2to16_add(sum_b, &b[0], &b[2]);
    // Compute &res[0]
    gf2to16_mul_0x2000(&res[0], leading);
    gf2to16_add(&res[0], &res[0], cnst);
    // Compute &res[2]
    gf2to16_mul(&res[2], sum_a, sum_b);
    gf2to16_add(&res[2], &res[2], cnst);
}

void gf2to32_add(uint8_t res[4], const uint8_t a[4], const uint8_t b[4]) {
    gf2to16_add(&res[0], &a[0], &b[0]);
    gf2to16_add(&res[2], &a[2], &b[2]);
}

int gf2to32_eq(const uint8_t a[4], const uint8_t b[4]) {
    return memeq_ct(a, b, 4);
}

void gf2to32_mul_gf256(uint8_t res[4], uint8_t a, const uint8_t b[4]) {
    res[0] = gf256_mul(a, b[0]);
    res[1] = gf256_mul(a, b[1]);
    res[2] = gf256_mul(a, b[2]);
    res[3] = gf256_mul(a, b[3]);
}

#ifndef NO_RND
/*************************************************/
/************        RANDOMNESS       ************/
/*************************************************/

#include "sample.h"

void gf256_random_elements(uint8_t points[], uint32_t nb_points, samplable_t* entropy) {
    byte_sample(entropy, points, nb_points);
}
#endif
