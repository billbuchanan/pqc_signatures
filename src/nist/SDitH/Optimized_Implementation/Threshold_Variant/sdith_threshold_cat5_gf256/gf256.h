#ifndef SDITH_GF256_H
#define SDITH_GF256_H

#include <stdint.h>

uint8_t gf256_add(uint8_t a, uint8_t b);
uint8_t gf256_mul(uint8_t a, uint8_t b);

void gf256_innerproduct_batched(uint8_t* out_batched, uint8_t* a_batched, uint8_t* b, uint32_t bytes, uint32_t n_batched);
// vz[] += vx[1][] * y[1] + ... + vx[nb][] * y[nb]
void gf256_matcols_muladd(uint8_t* vz, const uint8_t* y, const uint8_t* vx, uint32_t nb, uint32_t bytes);
void gf256_matcols_muladd_scaled(uint8_t* vz, const uint8_t* y, const uint8_t* vx, uint32_t nb, uint32_t bytes, uint32_t scale);
void gf256_mat16cols_muladd(uint8_t* vz, const uint8_t* y, const uint8_t* vx, uint32_t nb, uint32_t bytes);

// vz[] += vx[]
void gf256_add_tab(uint8_t* vz, const uint8_t* vx, uint32_t bytes);
// vz[] = vx[] * y
void gf256_mul_tab(uint8_t* vz, const uint8_t* vx, uint8_t y, uint32_t bytes);
// vz[] += vx[] * y
void gf256_muladd_tab(uint8_t* vz, uint8_t y, const uint8_t* vx, uint32_t bytes);
// vz[] = vz[] * y + vx[]
void gf256_mul_and_add_tab(uint8_t* vz, uint8_t y, const uint8_t* vx, uint32_t bytes);

int gf2to32_eq(const uint8_t a[4], const uint8_t b[4]);
void gf2to32_mul(uint8_t res[4], const uint8_t a[4], const uint8_t b[4]);
void gf2to32_add(uint8_t res[4], const uint8_t a[4], const uint8_t b[4]);
void gf2to32_mul_gf256(uint8_t res[4], uint8_t a, const uint8_t b[4]);

#ifndef NO_RND
#include "sample.h"
void gf256_random_elements(uint8_t points[], uint32_t nb_points, samplable_t* entropy);
#endif

#endif /* SDITH_GF256_H */
