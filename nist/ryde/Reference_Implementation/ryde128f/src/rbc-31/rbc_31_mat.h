/**
 * \file rbc_31_mat.h
 * \brief Interface for matrices over Fq^m
 */

#ifndef RBC_31_MAT_H
#define RBC_31_MAT_H

#include "rbc_31.h"
#include "rbc_31_elt.h"
#include "rbc_31_vec.h"

#include "seedexpander_shake.h"

void rbc_31_mat_init(rbc_31_mat* m, uint32_t rows, uint32_t columns);
void rbc_31_mat_clear(rbc_31_mat m);
void rbc_31_mat_set_zero(rbc_31_mat m, uint32_t rows, uint32_t columns);
void rbc_31_mat_set_random(seedexpander_shake_t* ctx, rbc_31_mat o, uint32_t rows, uint32_t columns);
void rbc_31_mat_vec_mul(rbc_31_vec o, const rbc_31_mat m, const rbc_31_vec v, uint32_t rows, uint32_t columns);
void rbc_31_mat_to_string(uint8_t* str, const rbc_31_mat m, uint32_t rows, uint32_t columns);
void rbc_31_mat_print(const rbc_31_mat m, uint32_t rows, uint32_t columns);
#endif

