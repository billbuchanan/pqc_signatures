/** 
 * \file rbc_43_qpoly.h
 * \brief Functions to manipulate q-polynomials. 
 *
 * The rbc_43_qpolys are polynomials over a field of the form \f$ P(x) = \sum_{i=0}^{n} p_i \times x^{q^i} \f$ with \f$ q \f$ a rational prime.
 * See \cite ore:qpolynomials for a description of their main properties.
 *
 */

#ifndef RBC_43_QPOLY_H
#define RBC_43_QPOLY_H

#include "rbc_43_elt.h"


/**
  * \typedef rbc_43_qpoly
  * \brief Structure of a q-polynomial
  */
typedef struct {
  rbc_43_vec values; /**< Coefficients of the q-polynomial */
  int32_t max_degree; /**< Maximum q-degree that the q-polynomial may have. This value never changes */
  int32_t degree; /**< Q-degree of the q-polynomial. This value is updated whenever necessary */
} rbc_43_qpoly_struct;
typedef rbc_43_qpoly_struct* rbc_43_qpoly;

void rbc_43_qpoly_init(rbc_43_qpoly* p, int32_t max_degree);

void rbc_43_qpoly_clear(rbc_43_qpoly o);

void rbc_43_qpoly_update_degree(rbc_43_qpoly p, int32_t position);

int8_t rbc_43_qpoly_set_zero(rbc_43_qpoly o);

int8_t rbc_43_qpoly_set_one(rbc_43_qpoly o);

int8_t rbc_43_qpoly_annihilator(rbc_43_qpoly p, const rbc_43_vec v, int32_t size);

void rbc_43_qpoly_evaluate(rbc_43_elt o, const rbc_43_qpoly p, const rbc_43_elt e);

int8_t rbc_43_qpoly_scalar_mul(rbc_43_qpoly o, const rbc_43_qpoly p, const rbc_43_elt e);

int8_t rbc_43_qpoly_qexp(rbc_43_qpoly o, const rbc_43_qpoly p);

int8_t rbc_43_qpoly_add(rbc_43_qpoly o, const rbc_43_qpoly p1, const rbc_43_qpoly p2);

#endif

