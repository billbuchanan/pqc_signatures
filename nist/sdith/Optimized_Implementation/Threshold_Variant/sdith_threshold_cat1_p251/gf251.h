#ifndef SDITH_GF251_H
#define SDITH_GF251_H

#include <stdint.h>

/*************************************/
/***        Basic Operations       ***/
/*************************************/

// Return "a+b" over GF(251)
uint8_t gf251_add(uint8_t a, uint8_t b);

// Return "a-b" over GF(251)
uint8_t gf251_sub(uint8_t a, uint8_t b);

// Return "a*b" over GF(251)
uint8_t gf251_neg(uint8_t a);

// Return "-a" over GF(251)
uint8_t gf251_mul(uint8_t a, uint8_t b);

/*************************************/
/***       Batched Operations      ***/
/*************************************/

// vz[] += vx[] over GF(251)
void gf251_add_tab(uint8_t* vz, const uint8_t* vx, uint32_t size);

// vz[] -= vx[] over GF(251)
void gf251_sub_tab(uint8_t* vz, const uint8_t* vx, uint32_t size);

// vz[] = vx[] * y over GF(251)
void gf251_mul_tab(uint8_t* vz, const uint8_t* vx, uint8_t y, uint32_t size);

// vz[] += vx[] * y over GF(251)
void gf251_muladd_tab(uint8_t* vz, uint8_t y, const uint8_t* vx, uint32_t size);

// vz[] = vz[] * y + vx[] over GF(251)
void gf251_mul_and_add_tab(uint8_t* vz, uint8_t y, const uint8_t* vx, uint32_t size);

// vz[] = -vz[] over GF(251)
void gf251_neg_tab(uint8_t* vz, uint32_t size);

/*************************************/
/***        Matrix Operations      ***/
/*************************************/

// vz[] += vx[][1] * y[1] + ... + vx[][nb] * y[nb]
void gf251_matcols_muladd(uint8_t* vz, const uint8_t* y, const uint8_t* vx, uint32_t nb, uint32_t size);

// vz[] += vx[][1] * y[1] + ... + vx[][nb] * y[nb], scaled
void gf251_matcols_muladd_scaled(uint8_t* vz, const uint8_t* y, const uint8_t* vx, uint32_t nb, uint32_t size, uint32_t scale);

// vz[] += vx[][1] * y[1] + ... + vx[][nb] * y[nb], size must be a multiple of 16
void gf251_mat16cols_muladd(uint8_t* vz, const uint8_t* y, const uint8_t* vx, uint32_t nb, uint32_t size);

/*************************************/
/***        Field Extensions       ***/
/*************************************/

// Check if a == b over GF(251^4)
int gf251to4_eq(const uint8_t a[4], const uint8_t b[4]);

// Set res as a+b over GF(251^4)
void gf251to4_add(uint8_t res[4], const uint8_t a[4], const uint8_t b[4]);

// Set res as a-b over GF(251^4)
void gf251to4_sub(uint8_t res[4], const uint8_t a[4], const uint8_t b[4]);

// Set res as a*b over GF(251^4)
void gf251to4_mul(uint8_t res[4], const uint8_t a[4], const uint8_t b[4]);

// Set res as a*b over GF(251^4), a belongs to GF(251)
void gf251to4_mul_gf251(uint8_t res[4], uint8_t a, const uint8_t b[4]);

/*************************************/
/***           Randomness          ***/
/*************************************/

#ifndef NO_RND
#include "sample.h"
// Fill the array "points" with random GF(251) elements
void gf251_random_elements(uint8_t points[], uint32_t nb_points, samplable_t* entropy);
#endif

#endif /* SDITH_GF251_H */
