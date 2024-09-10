/** 
 * \file rbc_31_qpoly.h
 * \brief Functions to manipulate q-polynomials. 
 *
 * The rbc_31_qpolys are polynomials over a field of the form \f$ P(x) = \sum_{i=0}^{n} p_i \times x^{q^i} \f$ with \f$ q \f$ a rational prime.
 * See \cite ore:qpolynomials for a description of their main properties.
 *
 */

#ifndef RBC_31_QPOLY_H
#define RBC_31_QPOLY_H

#include "rbc_31_elt.h"


/**
  * \typedef rbc_31_qpoly
  * \brief Structure of a q-polynomial
  */
typedef struct {
  rbc_31_vec values; /**< Coefficients of the q-polynomial */
  int32_t max_degree; /**< Maximum q-degree that the q-polynomial may have. This value never changes */
  int32_t degree; /**< Q-degree of the q-polynomial. This value is updated whenever necessary */
} rbc_31_qpoly_struct;
typedef rbc_31_qpoly_struct* rbc_31_qpoly;

void rbc_31_qpoly_init(rbc_31_qpoly* p, int32_t max_degree);

void rbc_31_qpoly_clear(rbc_31_qpoly o);

void rbc_31_qpoly_update_degree(rbc_31_qpoly p, int32_t position);

int8_t rbc_31_qpoly_set_zero(rbc_31_qpoly o);

int8_t rbc_31_qpoly_set_one(rbc_31_qpoly o);

int8_t rbc_31_qpoly_annihilator(rbc_31_qpoly p, const rbc_31_vec v, int32_t size);

void rbc_31_qpoly_evaluate(rbc_31_elt o, const rbc_31_qpoly p, const rbc_31_elt e);

int8_t rbc_31_qpoly_scalar_mul(rbc_31_qpoly o, const rbc_31_qpoly p, const rbc_31_elt e);

int8_t rbc_31_qpoly_qexp(rbc_31_qpoly o, const rbc_31_qpoly p);

int8_t rbc_31_qpoly_add(rbc_31_qpoly o, const rbc_31_qpoly p1, const rbc_31_qpoly p2);

#endif

