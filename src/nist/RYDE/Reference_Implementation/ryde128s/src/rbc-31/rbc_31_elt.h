/**
 * \file rbc_31_vec.h
 * \brief Interface for finite field elements
 */

#ifndef RBC_31_ELT_H
#define RBC_31_ELT_H

#include "rbc_31.h"


void rbc_31_field_init(void);
void rbc_31_elt_set_zero(rbc_31_elt o);

void rbc_31_elt_set_one(rbc_31_elt o);

void rbc_31_elt_set(rbc_31_elt o, const rbc_31_elt e);

void rbc_31_elt_set_mask1(rbc_31_elt o, const rbc_31_elt e1, const rbc_31_elt e2, uint32_t mask);

uint8_t rbc_31_elt_is_zero(const rbc_31_elt e);

uint8_t rbc_31_elt_is_equal_to(const rbc_31_elt e1, const rbc_31_elt e2);

int32_t rbc_31_elt_get_degree(const rbc_31_elt e);

uint8_t rbc_31_elt_get_coefficient(const rbc_31_elt e, uint32_t index);

void rbc_31_elt_set_coefficient_vartime(rbc_31_elt o, uint32_t index, uint8_t bit);

void rbc_31_elt_add(rbc_31_elt o, const rbc_31_elt e1, const rbc_31_elt e2);

void rbc_31_elt_mul(rbc_31_elt o, const rbc_31_elt e1, const rbc_31_elt e2);

void rbc_31_elt_sqr(rbc_31_elt o, const rbc_31_elt e);

void rbc_31_elt_reduce(rbc_31_elt o, const rbc_31_elt_ur e);

void rbc_31_elt_print(const rbc_31_elt e);

void rbc_31_elt_ur_set(rbc_31_elt_ur o, const rbc_31_elt_ur e);

void rbc_31_elt_ur_set_zero(rbc_31_elt_ur o);

void rbc_31_elt_ur_mul(rbc_31_elt_ur o, const rbc_31_elt e1, const rbc_31_elt e2);

void rbc_31_elt_ur_sqr(rbc_31_elt_ur o, const rbc_31_elt e);

void rbc_31_elt_to_string(uint8_t* str, const rbc_31_elt e);

#endif

