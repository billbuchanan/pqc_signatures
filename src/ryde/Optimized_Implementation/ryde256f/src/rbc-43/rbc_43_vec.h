/**
 * \file rbc_43_vec.h
 * \brief Interface for vectors of finite field elements
 */

#ifndef RBC_43_VEC_H
#define RBC_43_VEC_H

#include "rbc_43.h"
#include "rbc_43_elt.h"

#include "seedexpander_shake.h"

void rbc_43_vec_init(rbc_43_vec* v, uint32_t size);

void rbc_43_vec_clear(rbc_43_vec v);

void rbc_43_vec_set_zero(rbc_43_vec v, uint32_t size);

void rbc_43_vec_set(rbc_43_vec o, const rbc_43_vec v, uint32_t size);

void rbc_43_vec_set_random(seedexpander_shake_t* ctx, rbc_43_vec o, uint32_t size);

void rbc_43_vec_set_random_full_rank_with_one(seedexpander_shake_t* ctx, rbc_43_vec o, uint32_t size);

void rbc_43_vec_set_random_from_support(seedexpander_shake_t* ctx, rbc_43_vec o, uint32_t size, const rbc_43_vec support, uint32_t support_size, uint8_t copy_flag);
uint32_t rbc_43_vec_gauss(rbc_43_vec v, uint32_t size, uint8_t reduced_flag, rbc_43_vec *other_matrices, uint32_t nMatrices);

uint32_t rbc_43_vec_get_rank(const rbc_43_vec v, uint32_t size);

void rbc_43_vec_add(rbc_43_vec o, const rbc_43_vec v1, const rbc_43_vec v2, uint32_t size);

void rbc_43_vec_inner_product(rbc_43_elt o, const rbc_43_vec v1, const rbc_43_vec v2, uint32_t size);

void rbc_43_vec_scalar_mul(rbc_43_vec o, const rbc_43_vec v, const rbc_43_elt e, uint32_t size);

void rbc_43_vec_to_string(uint8_t* str, const rbc_43_vec v, uint32_t size);

void rbc_43_vec_from_string(rbc_43_vec v, uint32_t size, const uint8_t* str);

void rbc_43_vec_from_bytes(rbc_43_vec o, uint32_t size, uint8_t *random);

void rbc_43_vec_to_bytes( uint8_t *o, const rbc_43_vec v, uint32_t size);

void rbc_43_vec_print(const rbc_43_vec v, uint32_t size);

#endif

