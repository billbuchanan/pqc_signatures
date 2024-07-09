/**
 * \file rbc_43_mat.h
 * \brief Interface for matrices over Fq^m
 */

#ifndef RBC_43_MAT_H
#define RBC_43_MAT_H

#include "rbc_43.h"
#include "rbc_43_elt.h"
#include "rbc_43_vec.h"

#include "seedexpander_shake.h"

void rbc_43_mat_init(rbc_43_mat* m, uint32_t rows, uint32_t columns);
void rbc_43_mat_clear(rbc_43_mat m);
void rbc_43_mat_set_zero(rbc_43_mat m, uint32_t rows, uint32_t columns);
void rbc_43_mat_set_random(seedexpander_shake_t* ctx, rbc_43_mat o, uint32_t rows, uint32_t columns);
void rbc_43_mat_vec_mul(rbc_43_vec o, const rbc_43_mat m, const rbc_43_vec v, uint32_t rows, uint32_t columns);
void rbc_43_mat_to_string(uint8_t* str, const rbc_43_mat m, uint32_t rows, uint32_t columns);
void rbc_43_mat_print(const rbc_43_mat m, uint32_t rows, uint32_t columns);
#endif

