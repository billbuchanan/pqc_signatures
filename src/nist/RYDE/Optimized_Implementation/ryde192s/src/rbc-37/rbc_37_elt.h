/**
 * \file rbc_37_vec.h
 * \brief Interface for finite field elements
 */

#ifndef RBC_37_ELT_H
#define RBC_37_ELT_H

#include "rbc_37.h"


void rbc_37_field_init(void);
void rbc_37_elt_set_zero(rbc_37_elt o);

void rbc_37_elt_set_one(rbc_37_elt o);

void rbc_37_elt_set(rbc_37_elt o, const rbc_37_elt e);

void rbc_37_elt_set_mask1(rbc_37_elt o, const rbc_37_elt e1, const rbc_37_elt e2, uint32_t mask);

uint8_t rbc_37_elt_is_zero(const rbc_37_elt e);

uint8_t rbc_37_elt_is_equal_to(const rbc_37_elt e1, const rbc_37_elt e2);

int32_t rbc_37_elt_get_degree(const rbc_37_elt e);

uint8_t rbc_37_elt_get_coefficient(const rbc_37_elt e, uint32_t index);

void rbc_37_elt_set_coefficient_vartime(rbc_37_elt o, uint32_t index, uint8_t bit);

void rbc_37_elt_add(rbc_37_elt o, const rbc_37_elt e1, const rbc_37_elt e2);

void rbc_37_elt_mul(rbc_37_elt o, const rbc_37_elt e1, const rbc_37_elt e2);

void rbc_37_elt_sqr(rbc_37_elt o, const rbc_37_elt e);

void rbc_37_elt_reduce(rbc_37_elt o, const rbc_37_elt_ur e);

void rbc_37_elt_print(const rbc_37_elt e);

void rbc_37_elt_ur_set(rbc_37_elt_ur o, const rbc_37_elt_ur e);

void rbc_37_elt_ur_set_zero(rbc_37_elt_ur o);

void rbc_37_elt_ur_mul(rbc_37_elt_ur o, const rbc_37_elt e1, const rbc_37_elt e2);

void rbc_37_elt_ur_sqr(rbc_37_elt_ur o, const rbc_37_elt e);

void rbc_37_elt_to_string(uint8_t* str, const rbc_37_elt e);

#endif

