/**
 * \file rbc_37_mat.h
 * \brief Interface for matrices over Fq^m
 */

#ifndef RBC_37_MAT_H
#define RBC_37_MAT_H

#include "rbc_37.h"
#include "rbc_37_elt.h"
#include "rbc_37_vec.h"

#include "seedexpander_shake.h"

void rbc_37_mat_init(rbc_37_mat* m, uint32_t rows, uint32_t columns);
void rbc_37_mat_clear(rbc_37_mat m);
void rbc_37_mat_set_zero(rbc_37_mat m, uint32_t rows, uint32_t columns);
void rbc_37_mat_set_random(seedexpander_shake_t* ctx, rbc_37_mat o, uint32_t rows, uint32_t columns);
void rbc_37_mat_vec_mul(rbc_37_vec o, const rbc_37_mat m, const rbc_37_vec v, uint32_t rows, uint32_t columns);
void rbc_37_mat_to_string(uint8_t* str, const rbc_37_mat m, uint32_t rows, uint32_t columns);
void rbc_37_mat_print(const rbc_37_mat m, uint32_t rows, uint32_t columns);
#endif

