/** 
 * \file rbc_37_qpoly.c
 * \brief Implementation of rbc_37_qpoly.h
 *
 * The rbc_37_qpolys are polynomials over a field of the form \f$ P(x) = \sum_{i=0}^{n} p_i \times x^{q^i} \f$ with \f$ q \f$ a rational prime.
 * See \cite ore:qpolynomials for a description of their main properties.
 *
 */

#include "rbc_37_vec.h"
#include "rbc_37_qpoly.h"

#define SUCCESS 0; /**< Return status indicating that computation ended normally */
#define INVALID_PARAMETERS 1; /**< Return status indicating that invalid inputs have been used */



/** 
 * \fn void rbc_37_qpoly_init(rbc_37_qpoly* p, int32_t max_degree)
 * \brief This function initializes a q_polynomial.
 *
 * \param[in] p q_polynomial to init
 * \param[in] max_degree The maximum degree of the q_polynomial
 */
void rbc_37_qpoly_init(rbc_37_qpoly* p, int32_t max_degree) {
  *p = malloc(sizeof(rbc_37_qpoly_struct));

  rbc_37_vec_init(&((*p)->values), max_degree + 1);
  (*p)->max_degree = max_degree;
  (*p)->degree = -1;
}





/** 
 * \fn void rbc_37_qpoly_clear(rbc_37_poly o)
 * \brief This function clears a q_polynomial.
 *
 * \param[in] p q_polynomial to clear
 */
void rbc_37_qpoly_clear(rbc_37_qpoly o) {
  rbc_37_vec_clear(o->values);
  free(o);
}




/** 
 * \fn void rbc_37_qpoly_update_degree(rbc_37_qpoly p, int32_t position)
 * \brief This function updates the degree of a rbc_37_qpoly starting from a given position.
 *
 * \param[out] p q_polynomial
 * \param[in] position Position to start from
 */
void rbc_37_qpoly_update_degree(rbc_37_qpoly p, int32_t position) {
  for(int32_t i = position ; i >= 0 ; --i) {
    if(rbc_37_elt_is_zero(p->values[i]) == 0) { 
      p->degree = i;
      return;
    }
  }

  p->degree = -1;
}




/** 
 * \fn int8_t rbc_37_qpoly_set_zero(rbc_37_qpoly o)
 * \brief This function sets a q_polynomial to zero.
 *
 * \param[out] o q_polynomial
 */
int8_t rbc_37_qpoly_set_zero(rbc_37_qpoly o) {
  o->degree = -1;
  rbc_37_vec_set_zero(o->values, o->max_degree + 1);

  return SUCCESS;
}




/** 
 * \fn int8_t rbc_37_qpoly_set_one(rbc_37_qpoly o)
 * \brief This function sets a q_polynomial to one.
 *
 * \param[out] o q_polynomial
 */
int8_t rbc_37_qpoly_set_one(rbc_37_qpoly o) {
  o->degree = 0;
  rbc_37_elt_set_one(o->values[0]);

  for(int32_t i = 1 ; i < o->max_degree + 1 ; ++i) {
    rbc_37_elt_set_zero(o->values[i]);
  }

  return SUCCESS;
}




/**
 * \fn int8_t rbc_37_qpoly_annulator(rbc_37_qpoly p, const rbc_37_vec v, uint32_t size)
 * \brief This function computes the annulator polynomial: \f$ p(v_i) = 0 \f$.
 *
 *
 * \param[out] p q_polynomial equal to the quotient of the division
 * \param[in] v n-dimensional vector
 * \param[in] size dimension
 * \return INVALID_PARAMETERS if <b>q.max_degree != size</b>, SUCCESS otherwise
 */
int8_t rbc_37_qpoly_annihilator(rbc_37_qpoly p, const rbc_37_vec v, int32_t size) {
  if(p->max_degree != size) {
    return INVALID_PARAMETERS;
  }

  rbc_37_elt p_at_vi;
  rbc_37_qpoly t;
  rbc_37_qpoly_init(&t, size);
  rbc_37_qpoly_set_zero(t);

  // We initialise p as x
  rbc_37_qpoly_set_one(p);

  int i;
  for(i = 0; i < size; ++i) {
    rbc_37_qpoly_evaluate(p_at_vi, p, v[i]);   // p_i(v_i)
    rbc_37_qpoly_qexp(t, p);                   // (p_i^q)(x)
    rbc_37_qpoly_scalar_mul(p, p, p_at_vi);    // (p_i(v_i) * p_i)(x)
    rbc_37_qpoly_add(p, p, t);                 // (p_i^q + p_i(v_i) * p_i)(x)
  }

  rbc_37_elt_set_zero(p_at_vi);
  rbc_37_qpoly_clear(t);



  return SUCCESS;
}




/** 
 * \fn void rbc_37_qpoly_evaluate(rbc_37_elt o, const rbc_37_qpoly p, const rbc_37_elt e)
 * \brief This function evaluates a q_polynomial on a finite field element.
 *
 * \param[out] o Finite field element equal to \f$ p(e) \f$
 * \param[in] p q_polynomial
 * \param[in] e Finite field element
 */
void rbc_37_qpoly_evaluate(rbc_37_elt o, const rbc_37_qpoly p, const rbc_37_elt e) {
  rbc_37_elt tmp1, tmp2;

  rbc_37_elt_set(tmp1, e);
  rbc_37_elt_mul(o, p->values[0], tmp1);

  for(int32_t i = 1 ; i <= p->degree ; ++i) {
    rbc_37_elt_sqr(tmp1, tmp1);
    rbc_37_elt_mul(tmp2, p->values[i], tmp1);
    rbc_37_elt_add(o, o, tmp2);
  }
}




/** 
 * \fn int8_t rbc_37_qpoly_scalar_mul(rbc_37_qpoly o, const rbc_37_qpoly p, const rbc_37_elt e)
 * \brief This function computes the scalar product between a q_polynomial and a finite field element.
 *
 * \param[out] o q_polynomial equal to \f$ e \times p \f$
 * \param[in] p q_polynomial
 * \param[in] e Finite field element
 * \return INVALID_PARAMETERS if <b>o.max_degree < p.degree</b>, SUCESS otherwise
 */
int8_t rbc_37_qpoly_scalar_mul(rbc_37_qpoly o, const rbc_37_qpoly p, const rbc_37_elt e) {
  if(o->max_degree < p->degree) {
    return INVALID_PARAMETERS;
  }

  rbc_37_elt tmp;
  for(int32_t i = 0 ; i <= p->degree ; ++i) {
    rbc_37_elt_mul(tmp, p->values[i], e);
    rbc_37_elt_set(o->values[i], tmp);
  }

  for(int32_t i = p->degree + 1 ; i <= o->max_degree ; ++i) {
    rbc_37_elt_set_zero(o->values[i]);
  }

  o->degree = p->degree;

  return SUCCESS;
}




/** 
 * \fn int8_t rbc_37_qpoly_qexp(rbc_37_qpoly o, const rbc_37_qpoly p)
 * \brief This function computes the qth power of a q_polynomial.
 *
 * \param[out] o q_polynomial equal to \f$ p^q \f$ where <b>q</b> is the characteristic of the field
 * \param[in] p q_polynomial
 * \return INVALID_PARAMETERS if <b>o.max_degree < p.degree + 1</b>, SUCCESS otherwise
 */
int8_t rbc_37_qpoly_qexp(rbc_37_qpoly o, const rbc_37_qpoly p) {
  if(o->max_degree < p->degree + 1) {
    return INVALID_PARAMETERS;
  }

  rbc_37_elt tmp;
  rbc_37_elt_set_zero(o->values[0]);
  for(int32_t i = 0 ; i <= p->degree ; ++i) {
    rbc_37_elt_sqr(tmp, p->values[i]);
    rbc_37_elt_set(o->values[(i + 1) % RBC_37_FIELD_M], tmp);
  }

  for(int32_t i = p->degree + 2 ; i <= o->max_degree ; ++i) {
    rbc_37_elt_set_zero(o->values[i]);
  }

  rbc_37_qpoly_update_degree(o, p->degree + 1);

  return SUCCESS;
}




/** 
 * \fn int8_t rbc_37_qpoly_add(rbc_37_qpoly o, const rbc_37_qpoly p1, const rbc_37_qpoly p2)
 * \brief This function adds two q_polynomials.
 *
 * \param[out] o Sum of q_polynomials <b>p1</b> and <b>p2</b>
 * \param[in] p1 q_polynomial
 * \param[in] p2 q_polynomial
 * \return INVALID_PARAMETERS if <b>o.max_degree < max(p1.degree, p2.degree)</b>, SUCCESS otherwise
 */
int8_t rbc_37_qpoly_add(rbc_37_qpoly o, const rbc_37_qpoly p1, const rbc_37_qpoly p2) {
  int32_t degree = p1->degree > p2->degree ? p1->degree : p2->degree; 
  if(o->max_degree < degree) {
    return INVALID_PARAMETERS;
  }

  o->degree = degree;

  for(int32_t i = 0 ; i <= o->degree ; ++i) {
    rbc_37_elt_add(o->values[i], p1->values[i], p2->values[i]);
  }

  for(int32_t i = o->degree + 1 ; i <= o->max_degree ; ++i) {
    rbc_37_elt_set_zero(o->values[i]);
  }

  rbc_37_qpoly_update_degree(o, o->degree);

  return SUCCESS;
}

