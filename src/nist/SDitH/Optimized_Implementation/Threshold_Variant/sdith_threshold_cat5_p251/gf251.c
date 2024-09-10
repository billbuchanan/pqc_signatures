#include "gf251.h"
#include <stdlib.h>
#include <string.h>

#include "gf251-internal.h"

/*************************************************/
/***********      FIELD OPERATIONS     ***********/
/*************************************************/

uint8_t gf251_add(uint8_t a, uint8_t b) {
    uint16_t res = a+b;
    res -= PRIME*(PRIME <= res);
    return (uint8_t) res;
}

uint8_t gf251_sub(uint8_t a, uint8_t b) {
    uint16_t res = a+PRIME-b;
    res -= PRIME*(PRIME <= res);
    return (uint8_t) res;
}

uint8_t gf251_mul(uint8_t a, uint8_t b) {
    uint16_t res = a*b;
    return (uint8_t) _gf251_reduce16(res);
}

uint8_t gf251_neg(uint8_t a) {
    return (a != 0)*(PRIME - a);
}

/*************************************************/
/***********     BATCHED OPERATIONS    ***********/
/*************************************************/

// vz[] += vx[]
void gf251_add_tab(uint8_t* vz, const uint8_t* vx, uint32_t size) {
    uint16_t res;
    for(uint32_t i=0; i<size; i++) {
        res = vz[i]+vx[i];
        res -= PRIME*(PRIME <= res);
        vz[i] = (uint8_t) res;
    }
}

// vz[] -= vx[]
void gf251_sub_tab(uint8_t* vz, const uint8_t* vx, uint32_t size) {
    uint16_t res;
    for(uint32_t i=0; i<size; i++) {
        res = vz[i]+PRIME-vx[i];
        res -= PRIME*(PRIME <= res);
        vz[i] = (uint8_t) res;
    }
}

// vz[] = vx[] * y
void gf251_mul_tab(uint8_t* vz, const uint8_t* vx, uint8_t y, uint32_t size) {
    uint16_t res;
    for(uint32_t i=0; i<size; i++) {
        res = vx[i]*y;
        vz[i] = (uint8_t) _gf251_reduce16(res);
    }
}

// vz[] += vx[] * y
void gf251_muladd_tab(uint8_t* vz, uint8_t y, const uint8_t* vx, uint32_t size) {
    uint16_t res;
    for(uint32_t i=0; i<size; i++) {
        res = vz[i] + vx[i]*y;
        vz[i] = (uint8_t) _gf251_reduce16(res);
    }
}

// vz[] = vz[] * y + vx[]
void gf251_mul_and_add_tab(uint8_t* vz, uint8_t y, const uint8_t* vx, uint32_t size) {
    uint16_t res;
    for(uint32_t i=0; i<size; i++) {
        res = vz[i]*y + vx[i];
        vz[i] = (uint8_t) _gf251_reduce16(res);
    }
}

// vz[] = -vz[]
void gf251_neg_tab(uint8_t* vz, uint32_t size) {
    for(uint32_t i=0; i<size; i++)
        vz[i] = (vz[i] != 0)*(PRIME - vz[i]);
}

#ifndef NO_RND
/*************************************************/
/************        RANDOMNESS       ************/
/*************************************************/

#include "sample.h"

void gf251_random_elements(uint8_t points[], uint32_t nb_points, samplable_t* entropy) {
    uint32_t pos = 0;
    do {
        byte_sample(entropy, points+pos, nb_points-pos);
        for(uint32_t i=pos; i<nb_points; i++) {
            if(points[i] < PRIME) {
                points[pos] = points[i];
                pos++;
            }
        }
    } while(pos < nb_points);
}
#endif
