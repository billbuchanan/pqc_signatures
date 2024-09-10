#include "gf251.h"
#include <stdlib.h>
#include <string.h>

#include "gf251-internal.h"

/*************************************************/
/***********      FIELD EXTENSION      ***********/
/*************************************************/

#define SET_RES_REDUCE8(i,exp) {v=exp; res[i]=(uint8_t)(v-PRIME*(PRIME<=v));}
#define SET_RES_REDUCE16(i,exp) {v=exp; res[i]=(uint8_t)_gf251_reduce16(v);}

// X^2 - 2 = 0
static void gf251to2_mul(uint8_t res[2], const uint8_t a[2], const uint8_t b[2]) {
    uint16_t leading = a[1]*b[1];
    uint16_t cnst = a[0]*b[0];
    uint32_t v;
    v = (uint32_t) (a[0]+a[1])*(b[0]+b[1])+2*PRIME*PRIME-leading-cnst;
    res[1] = (uint8_t)_gf251_reduce32(v);
    v = (uint32_t) 2*leading+cnst;
    res[0] = (uint8_t)_gf251_reduce32(v);
}

static void gf251to2_add(uint8_t res[2], const uint8_t a[2], const uint8_t b[2]) {
    uint16_t v;
    SET_RES_REDUCE8(0, a[0]+b[0]);
    SET_RES_REDUCE8(1, a[1]+b[1]);
}

static void gf251to2_sub(uint8_t res[2], const uint8_t a[2], const uint8_t b[2]) {
    uint16_t v;
    SET_RES_REDUCE8(0, a[0]+PRIME-b[0]);
    SET_RES_REDUCE8(1, a[1]+PRIME-b[1]);
}

static void gf251to2_mul_0x0101(uint8_t res[2], const uint8_t a[2]) {
    uint16_t v;
    SET_RES_REDUCE16(0, 2*a[1]+a[0]);
    SET_RES_REDUCE16(1, a[0]+a[1]);
}

// Y^2 - (X+1) = 0
void gf251to4_mul(uint8_t res[4], const uint8_t a[4], const uint8_t b[4]) {
    uint8_t leading[2], cnst[2], sum_a[2], sum_b[2];
    gf251to2_mul(leading, &a[2], &b[2]);
    gf251to2_mul(cnst, &a[0], &b[0]);
    gf251to2_add(sum_a, &a[0], &a[2]);
    gf251to2_add(sum_b, &b[0], &b[2]);
    // Compute &res[0]
    gf251to2_mul_0x0101(&res[0],leading);
    gf251to2_add(&res[0], &res[0], cnst);
    // Compute &res[2]
    gf251to2_mul(&res[2], sum_a, sum_b);
    gf251to2_sub(&res[2], &res[2], cnst);
    gf251to2_sub(&res[2], &res[2], leading);
}

void gf251to4_add(uint8_t res[4], const uint8_t a[4], const uint8_t b[4]) {
    uint16_t v;
    SET_RES_REDUCE8(0, a[0]+b[0]);
    SET_RES_REDUCE8(1, a[1]+b[1]);
    SET_RES_REDUCE8(2, a[2]+b[2]);
    SET_RES_REDUCE8(3, a[3]+b[3]);
}

void gf251to4_sub(uint8_t res[4], const uint8_t a[4], const uint8_t b[4]) {
    uint16_t v;
    SET_RES_REDUCE8(0, a[0]+PRIME-b[0]);
    SET_RES_REDUCE8(1, a[1]+PRIME-b[1]);
    SET_RES_REDUCE8(2, a[2]+PRIME-b[2]);
    SET_RES_REDUCE8(3, a[3]+PRIME-b[3]);
}

void gf251to4_mul_gf251(uint8_t res[4], uint8_t a, const uint8_t b[4]) {
    uint16_t v;
    SET_RES_REDUCE16(0, a * b[0]);
    SET_RES_REDUCE16(1, a * b[1]);
    SET_RES_REDUCE16(2, a * b[2]);
    SET_RES_REDUCE16(3, a * b[3]);
}

int gf251to4_eq(const uint8_t a[4], const uint8_t b[4]) {
    int res = 1;
    res &= (a[0] == b[0]);
    res &= (a[1] == b[1]);
    res &= (a[2] == b[2]);
    res &= (a[3] == b[3]);
    return res;
}
