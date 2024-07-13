#ifndef FFI_H
#define FFI_H

#include <stdint.h>
#include "seedexpander_shake.h"

//GF(16)
typedef uint8_t gf16;

gf16 gf16_add(gf16 a, gf16 b);
gf16 gf16_sub(gf16 a, gf16 b);
gf16 gf16_mul(gf16 a, gf16 b);

void gf16_print(gf16 e);

//GF(16) matrices
typedef gf16 *gf16_mat;

void gf16_mat_init(gf16_mat *o, int lines, int columns);
void gf16_mat_set(gf16_mat o, gf16_mat a, int lines, int columns);
void gf16_mat_set_zero(gf16_mat o, int lines, int columns);
void gf16_mat_set_random(seedexpander_shake_t* ctx, gf16_mat o, int lines, int columns);
void gf16_mat_add(gf16_mat o, gf16_mat a, gf16_mat b, int lines, int columns);
void gf16_mat_scalar_mul(gf16_mat o, gf16_mat a, gf16 e, int lines, int columns);
void gf16_mat_mul(gf16_mat o, gf16_mat a, gf16_mat b, int lines, int length, int columns);
void gf16_mat_clear(gf16_mat o);

void gf16_mat_print(gf16_mat o, int lines, int columns);
void gf16_mat_to_string(uint8_t* o, const gf16_mat a, int lines, int columns);
void gf16_mat_from_string(gf16_mat o, int lines, int columns, const uint8_t *src);

//GF(16)^19
//P_ext = x^19 + x^5 + x^2 + x + 1
typedef gf16 gfqm[19];
typedef gfqm *gfqm_vec;

void gfqm_set(gfqm o, const gfqm a);
void gfqm_set_zero(gfqm o);
void gfqm_set_one(gfqm o);
void gfqm_add(gfqm o, const gfqm a, const gfqm b);
void gfqm_scalar_mul(gfqm o, const gfqm a, const gf16 e);
void gfqm_pow15(gfqm o, gfqm a);
void gfqm_pow16(gfqm o, gfqm a);
void gfqm_mul(gfqm o, const gfqm a, const gfqm b);

void gfqm_to_string(uint8_t *o, const gfqm e);
void gfqm_from_string(gfqm o, const uint8_t *src);

void gfqm_print(gfqm e);

void gfqm_vec_init(gfqm_vec* o, int size);
void gfqm_vec_set(gfqm_vec o, gfqm_vec a, int size);
void gfqm_vec_set_zero(gfqm_vec o, int size);

void gfqm_vec_add(gfqm_vec o, gfqm_vec a, gfqm_vec b, int size);
void gfqm_vec_scalar_mul(gfqm_vec o, gfqm_vec a, const gfqm e, int size);
void gfqm_vec_inner_product(gfqm o, gfqm_vec a, gfqm_vec b, int size);

void gfqm_vec_to_string(uint8_t *o, const gfqm_vec e, int size);
void gfqm_vec_from_string(gfqm_vec o, int size, const uint8_t *src);

void gfqm_vec_clear(gfqm_vec o);

//Linearized polynomials
void gfqm_qpoly_eval(gfqm o, gfqm *poly, gfqm x, int terms);
void gfqm_qpoly_qexp(gfqm *o, gfqm *a, int terms);
void gfqm_qpoly_q_minus_one_exp(gfqm *o, gfqm *a, int terms);
void gfqm_qpoly_add(gfqm *o, gfqm *a, gfqm *b, int terms);
void gfqm_qpoly_scalar_mul(gfqm *o, gfqm *a, gfqm e, int terms);

void gfqm_qpoly_annihilator(gfqm *o, gfqm *support, int size);

void gfqm_qpoly_print(gfqm *o, int terms);

#endif
