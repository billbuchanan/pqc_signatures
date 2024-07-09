/**
 * \file rbc_31_vec.h
 * \brief Interface for vectors of finite field elements
 */

#ifndef RBC_31_VEC_H
#define RBC_31_VEC_H

#include "rbc_31.h"
#include "rbc_31_elt.h"

#include "seedexpander_shake.h"

void rbc_31_vec_init(rbc_31_vec* v, uint32_t size);

void rbc_31_vec_clear(rbc_31_vec v);

void rbc_31_vec_set_zero(rbc_31_vec v, uint32_t size);

void rbc_31_vec_set(rbc_31_vec o, const rbc_31_vec v, uint32_t size);

void rbc_31_vec_set_random(seedexpander_shake_t* ctx, rbc_31_vec o, uint32_t size);

void rbc_31_vec_set_random_full_rank_with_one(seedexpander_shake_t* ctx, rbc_31_vec o, uint32_t size);

void rbc_31_vec_set_random_from_support(seedexpander_shake_t* ctx, rbc_31_vec o, uint32_t size, const rbc_31_vec support, uint32_t support_size, uint8_t copy_flag);
uint32_t rbc_31_vec_gauss(rbc_31_vec v, uint32_t size, uint8_t reduced_flag, rbc_31_vec *other_matrices, uint32_t nMatrices);

uint32_t rbc_31_vec_get_rank(const rbc_31_vec v, uint32_t size);

void rbc_31_vec_add(rbc_31_vec o, const rbc_31_vec v1, const rbc_31_vec v2, uint32_t size);

void rbc_31_vec_inner_product(rbc_31_elt o, const rbc_31_vec v1, const rbc_31_vec v2, uint32_t size);

void rbc_31_vec_scalar_mul(rbc_31_vec o, const rbc_31_vec v, const rbc_31_elt e, uint32_t size);

void rbc_31_vec_to_string(uint8_t* str, const rbc_31_vec v, uint32_t size);

void rbc_31_vec_from_string(rbc_31_vec v, uint32_t size, const uint8_t* str);

void rbc_31_vec_from_bytes(rbc_31_vec o, uint32_t size, uint8_t *random);

void rbc_31_vec_to_bytes( uint8_t *o, const rbc_31_vec v, uint32_t size);

void rbc_31_vec_print(const rbc_31_vec v, uint32_t size);

#endif

