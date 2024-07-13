#ifndef FIELD_H
#define FIELD_H

#include <stdint.h>
#include <string.h>
#include "parameters.h"
#include "rnd.h"

#include "gf251.h"

// Unitary operations
#define add_points(a,b) gf251_add(a,b)
#define sub_points(a,b) gf251_sub(a,b)
#define mul_points(a,b) gf251_mul(a,b)
#define neg_point(a)    gf251_neg(a)

// Batched operations
static inline void set_zero_tab_points(void* vz, uint32_t size) { memset(vz, 0, size); }
static inline void set_tab_points(void* vz, const void* vx, uint32_t size) { memcpy(vz, vx, size); }
static inline void add_tab_points(void* vz, const void* vx, uint32_t size) { gf251_add_tab(vz, vx, size); }
static inline void sub_tab_points(void* vz, const void* vx, uint32_t size) { gf251_sub_tab(vz, vx, size); }
static inline void mul_tab_points(void* vz, const void* vx, uint8_t y, uint32_t size) { gf251_mul_tab(vz, vx, y, size); }
static inline void muladd_tab_points(void* vz, uint8_t y, const void* vx, uint32_t size) { gf251_muladd_tab(vz, y, vx, size); }
static inline void mul_and_add_tab_points(void* vz, uint8_t y, const void* vx, uint32_t size) { gf251_mul_and_add_tab(vz, y, vx, size); }
static inline void neg_tab_points(void* vz, uint32_t size) { gf251_neg_tab(vz, size); }
static inline void matcols_muladd(void* vz, const void* y, const void* vx, uint32_t nb_columns, uint32_t nb_rows) {
    gf251_matcols_muladd(vz, y, vx, nb_columns, nb_rows);
}
static inline void mat16cols_muladd(void* vz, const void* y, const void* vx, uint32_t nb_columns, uint32_t nb_rows) {
    gf251_mat16cols_muladd(vz, y, vx, nb_columns, nb_rows);
}

// Operations in field extension
#if PARAM_EXT_DEGREE == 8
#define mul_points_ext(r,a,b) gf251to8_mul(r,a,b)
#define add_points_ext(r,a,b) gf251to8_add(r,a,b)
#define sub_points_ext(r,a,b) gf251to8_sub(r,a,b)
#define mul_points_mixed(r,a,b) gf251to8_mul_gf251(r,a,b)
#define eq_points_ext(a,b) gf251to8_eq(a,b)
#elif PARAM_EXT_DEGREE == 4
#define mul_points_ext(r,a,b) gf251to4_mul(r,a,b)
#define add_points_ext(r,a,b) gf251to4_add(r,a,b)
#define sub_points_ext(r,a,b) gf251to4_sub(r,a,b)
#define mul_points_mixed(r,a,b) gf251to4_mul_gf251(r,a,b)
#define eq_points_ext(a,b) gf251to4_eq(a,b)
#else
#error "field lib - only support GF(251^4) or GF(251^8) as field extension"
#endif

// Randomness
static inline void random_points(void* points, uint32_t size, samplable_t* entropy) { gf251_random_elements(points, size, entropy); }

#endif /* FIELD_H */
