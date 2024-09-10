/** 
 * \file rbc_37_qpoly.h
 * \brief Functions to manipulate q-polynomials. 
 *
 * The rbc_37_qpolys are polynomials over a field of the form \f$ P(x) = \sum_{i=0}^{n} p_i \times x^{q^i} \f$ with \f$ q \f$ a rational prime.
 * See \cite ore:qpolynomials for a description of their main properties.
 *
 */

#ifndef RBC_37_QPOLY_H
#define RBC_37_QPOLY_H

#include "rbc_37_elt.h"


/**
  * \typedef rbc_37_qpoly
  * \brief Structure of a q-polynomial
  */
typedef struct {
  rbc_37_vec values; /**< Coefficients of the q-polynomial */
  int32_t max_degree; /**< Maximum q-degree that the q-polynomial may have. This value never changes */
  int32_t degree; /**< Q-degree of the q-polynomial. This value is updated whenever necessary */
} rbc_37_qpoly_struct;
typedef rbc_37_qpoly_struct* rbc_37_qpoly;

void rbc_37_qpoly_init(rbc_37_qpoly* p, int32_t max_degree);

void rbc_37_qpoly_clear(rbc_37_qpoly o);

void rbc_37_qpoly_update_degree(rbc_37_qpoly p, int32_t position);

int8_t rbc_37_qpoly_set_zero(rbc_37_qpoly o);

int8_t rbc_37_qpoly_set_one(rbc_37_qpoly o);

int8_t rbc_37_qpoly_annihilator(rbc_37_qpoly p, const rbc_37_vec v, int32_t size);

void rbc_37_qpoly_evaluate(rbc_37_elt o, const rbc_37_qpoly p, const rbc_37_elt e);

int8_t rbc_37_qpoly_scalar_mul(rbc_37_qpoly o, const rbc_37_qpoly p, const rbc_37_elt e);

int8_t rbc_37_qpoly_qexp(rbc_37_qpoly o, const rbc_37_qpoly p);

int8_t rbc_37_qpoly_add(rbc_37_qpoly o, const rbc_37_qpoly p1, const rbc_37_qpoly p2);

#endif

