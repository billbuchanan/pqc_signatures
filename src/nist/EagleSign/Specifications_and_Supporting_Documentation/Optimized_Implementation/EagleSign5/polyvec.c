/*
 * Implementors: EagleSign Team
 * This implementation is highly inspired from Dilithium and
 * Falcon Signatures' implementations
 */

#include <stdint.h>
#include <stdio.h>
#include "params.h"
#include "polyvec.h"
#include "poly.h"
#include "fips202.h"

/**************************************************************/
/************ Vectors of polynomials of length L **************/
/**************************************************************/

void polyvecl_uniform_eta_g(polyvecl *v, const uint8_t seed[CRHBYTES], uint16_t *nonce)
{
  unsigned int i;

  for (i = 0; i < L; ++i)
    poly_uniform_eta_g(&v->vec[i], seed, (*nonce)++);
}

void polyvecl_uniform_eta_d(polyvecl *v, const uint8_t seed[CRHBYTES], uint16_t *nonce)
{
  unsigned int i;

  for (i = 0; i < L; ++i)
    poly_uniform_eta_d(&v->vec[i], seed, (*nonce)++);
}

void polyvecl_challenge_y1_c(polyvecl *v, const uint8_t seed[SEEDBYTES], uint16_t *nonce, int param)
{
  unsigned int i;

  for (i = 0; i < L; ++i)
    poly_challenge_y1_c(&v->vec[i], seed, (*nonce)++, param);
}

void polyvecl_add(polyvecl *w, const polyvecl *u, const polyvecl *v)
{
  unsigned int i;

  for (i = 0; i < L; ++i)
    poly_add(&w->vec[i], &u->vec[i], &v->vec[i]);
}

void polyvecl_ntt(polyvecl *v)
{
  unsigned int i;

  for (i = 0; i < L; ++i)
    poly_ntt(&v->vec[i]);
}

void polyvecl_invntt_tomont(polyvecl *v)
{
  unsigned int i;

  for (i = 0; i < L; ++i)
    poly_invntt_tomont(&v->vec[i]);
}

void polyvecl_pointwise_poly_montgomery(polyvecl *r, const poly *a, const polyvecl *v)
{
  unsigned int i;

  for (i = 0; i < L; ++i)
    poly_pointwise_montgomery(&r->vec[i], a, &v->vec[i]);
}

void polyvecl_pointwise_acc_montgomery(poly *w,
                                       const polyvecl *u,
                                       const polyvecl *v)
{
  unsigned int i;
  poly t;

  poly_pointwise_montgomery(w, &u->vec[0], &v->vec[0]);
  for (i = 1; i < L; ++i)
  {
    poly_pointwise_montgomery(&t, &u->vec[i], &v->vec[i]);
    poly_add(w, w, &t);
  }
}

/**************************************************************/
/************ Vectors of polynomials of length K **************/
/**************************************************************/

void polyveck_uniform_eta_y2(polyveck *v, const uint8_t seed[CRHBYTES], uint16_t *nonce)
{
  unsigned int i;

  for (i = 0; i < K; ++i)
    poly_uniform_eta_y2(&v->vec[i], seed, (*nonce)++);
}

void polyveck_add(polyveck *w, const polyveck *u, const polyveck *v)
{
  unsigned int i;

  for (i = 0; i < K; ++i)
    poly_add(&w->vec[i], &u->vec[i], &v->vec[i]);
}

void polyveck_sub(polyveck *w, const polyveck *u, const polyveck *v)
{
  unsigned int i;

  for (i = 0; i < K; ++i)
    poly_sub(&w->vec[i], &u->vec[i], &v->vec[i]);
}

void polyveck_ntt(polyveck *v)
{
  unsigned int i;

  for (i = 0; i < K; ++i)
    poly_ntt(&v->vec[i]);
}

void polyveck_invntt_tomont(polyveck *v)
{
  unsigned int i;

  for (i = 0; i < K; ++i)
    poly_invntt_tomont(&v->vec[i]);
}

void polyveck_pointwise_poly_montgomery(polyveck *r, const poly *a, const polyveck *v)
{
  unsigned int i;

  for (i = 0; i < K; ++i)
    poly_pointwise_montgomery(&r->vec[i], a, &v->vec[i]);
}

void polyveck_pack_P(uint8_t r[K * NBYTES * LOGQ], polyveck *P)
{
  unsigned int i;

  for (i = 0; i < K; ++i)
    polyQ_pack(r + i * NBYTES * LOGQ, &P->vec[i]);
}

void polyveck_unpack_P(polyveck *P, uint8_t r[K * NBYTES * LOGQ])
{
  unsigned int i;

  for (i = 0; i < K; ++i)
    polyQ_unpack(&P->vec[i], r + i * NBYTES * LOGQ);
}

int polyvec_chknorms(const polyvecl *Z, const polyveck *W)
{
  unsigned int i, j, min = L, max = K;
  Q_SIZE tmp;

  if (K < L)
  {
    min = K;
    max = L;
  }

  for (i = 0; i < min; ++i)
  {
    for (j = 0; j < N; ++j)
    {
      tmp = (Q_SIZE)Z->vec[i].coeffs[j];
      tmp -= (2 * tmp) & -(tmp >> 15); // |x| = x if x >= 0 and -x if x < 0
      if ((S_Q_SIZE)tmp > DELTA)
      {
        return -1;
      }

      tmp = (Q_SIZE)W->vec[i].coeffs[j];
      tmp -= (2 * tmp) & -(tmp >> 15);
      if ((S_Q_SIZE)tmp > DELTA_PRIME)
        return -1;
    }
  }

  if (max == L)
  {
    for (i = min; i < max; ++i)
    {
      for (j = 0; j < N; ++j)
      {
        tmp = (Q_SIZE)Z->vec[i].coeffs[j];
        tmp -= (2 * tmp) & -(tmp >> 15);
        if ((S_Q_SIZE)tmp > DELTA)
          return -1;
      }
    }
  }
  else
  {
    for (i = min; i < max; ++i)
    {
      for (j = 0; j < N; ++j)
      {
        tmp = (Q_SIZE)W->vec[i].coeffs[j];
        tmp -= (2 * tmp) & -(tmp >> 15);
        if ((S_Q_SIZE)tmp > DELTA_PRIME)
          return -1;
      }
    }
  }

  return 0;
}

int polyvecl_chknorms(const polyvecl *Z, const unsigned int borm)
{
  unsigned int i, j;
  Q_SIZE tmp;

  for (i = 0; i < L; ++i)
  {
    for (j = 0; j < N; ++j)
    {
      tmp = (Q_SIZE)Z->vec[i].coeffs[j];
      tmp -= (2 * tmp) & -(tmp >> 15); // |x| = x if x >= 0 and -x if x < 0
      if ((S_Q_SIZE)tmp > borm)
      {
        return -1;
      }
    }
  }
  return 0;
}