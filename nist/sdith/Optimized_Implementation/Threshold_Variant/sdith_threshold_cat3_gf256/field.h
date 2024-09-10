#ifndef FIELD_H
#define FIELD_H

#include <stdint.h>
#include <string.h>
#include "parameters.h"
#include "rnd.h"

#include "gf256.h"

// Unitary operations
#define add_points(a,b) gf256_add(a,b)
#define sub_points(a,b) gf256_add(a,b)
#define mul_points(a,b) gf256_mul(a,b)
#define neg_point(a)    (a)

// Batched operations
static inline void set_zero_tab_points(void* vz, uint32_t size) { memset(vz, 0, size); }
static inline void set_tab_points(void* vz, const void* vx, uint32_t size) { memcpy(vz, vx, size); }
static inline void add_tab_points(void* vz, const void* vx, uint32_t size) { gf256_add_tab(vz, vx, size); }
static inline void sub_tab_points(void* vz, const void* vx, uint32_t size) { gf256_add_tab(vz, vx, size); }
static inline void mul_tab_points(void* vz, const void* vx, uint8_t y, uint32_t size) { gf256_mul_tab(vz, vx, y, size); }
static inline void muladd_tab_points(void* vz, uint8_t y, const void* vx, uint32_t size) { gf256_muladd_tab(vz, y, vx, size); }
static inline void mul_and_add_tab_points(void* vz, uint8_t y, const void* vx, uint32_t size) { gf256_mul_and_add_tab(vz, y, vx, size); }
#define neg_tab_points(vz, bytes) {(void) vz; (void) bytes;}
static inline void matcols_muladd(void* vz, const void* y, const void* vx, uint32_t nb_columns, uint32_t nb_rows) {
    gf256_matcols_muladd(vz, y, vx, nb_columns, nb_rows);
}
static inline void mat16cols_muladd(void* vz, const void* y, const void* vx, uint32_t nb_columns, uint32_t nb_rows) {
    gf256_mat16cols_muladd(vz, y, vx, nb_columns, nb_rows);
}

// Operations in field extension
#if PARAM_EXT_DEGREE == 8
#define mul_points_ext(r,a,b) gf2to64_mul(r,a,b)
#define add_points_ext(r,a,b) gf2to64_add(r,a,b)
#define sub_points_ext(r,a,b) gf2to64_add(r,a,b)
#define mul_points_mixed(r,a,b) gf2to64_mul_gf256(r,a,b)
#define eq_points_ext(a,b) gf2to64_eq(a,b)
#elif PARAM_EXT_DEGREE == 4
#define mul_points_ext(r,a,b) gf2to32_mul(r,a,b)
#define add_points_ext(r,a,b) gf2to32_add(r,a,b)
#define sub_points_ext(r,a,b) gf2to32_add(r,a,b)
#define mul_points_mixed(r,a,b) gf2to32_mul_gf256(r,a,b)
#define eq_points_ext(a,b) gf2to32_eq(a,b)
#else
#error "field lib - only support GF(2^32) or GF(2^64) as field extension"
#endif

// Randomness
static inline void random_points(void* points, uint32_t size, samplable_t* entropy) { gf256_random_elements(points, size, entropy); }

#endif /* FIELD_H */
