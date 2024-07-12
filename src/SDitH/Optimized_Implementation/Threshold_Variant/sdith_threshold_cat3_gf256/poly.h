#ifndef SDITH_POLY_H
#define SDITH_POLY_H

#include <stdint.h>
#include <stddef.h>

#include "parameters.h"
#include "field.h"

// Polynomials
void set_polynomial_as_zero(uint8_t* poly, uint16_t degree);
void add_polynomials(uint8_t* q_poly, uint8_t* p_poly, uint16_t common_degree); // Q += P

uint8_t evaluate_polynomial(const uint8_t* poly, uint16_t degree, uint8_t point);
uint8_t evaluate_monic_polynomial(const uint8_t* poly, uint16_t degree, uint8_t point);
void evaluate_polynomial_in_extfield(uint8_t* out, const uint8_t* poly, uint16_t degree, uint8_t* point, uint8_t ext_degree);
void evaluate_monic_polynomial_in_extfield(uint8_t* out, const uint8_t* poly, uint16_t degree, uint8_t* point, uint8_t ext_degree);

void multiply_polynomial_by_scalar(uint8_t* out, const uint8_t* in, uint16_t degree_of_in, uint8_t scalar);
void remove_one_degree_factor(uint8_t* out, const uint8_t* in, uint16_t in_degree, uint8_t alpha);
void remove_one_degree_factor_from_monic(uint8_t* out, const uint8_t* in, uint16_t in_degree, uint8_t alpha);

#endif /* SDITH_POLY_H */
