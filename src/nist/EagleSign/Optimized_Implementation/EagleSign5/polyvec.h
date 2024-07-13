/*
 * Implementors: EagleSign Team
 * This implementation is highly inspired from Dilithium and
 * Falcon Signatures' implementations
 */

#ifndef POLYVEC_H
#define POLYVEC_H

#include <stdint.h>
#include "params.h"
#include "poly.h"

/* Vectors of polynomials of length L */
typedef struct
{
  poly vec[L];
} polyvecl;

#define polyvecl_uniform_eta_g EAGLESIGN_NAMESPACE(polyvecl_uniform_eta_g)
void polyvecl_uniform_eta_g(polyvecl *v, const uint8_t seed[CRHBYTES], uint16_t *nonce);

#define polyvecl_uniform_eta_d EAGLESIGN_NAMESPACE(polyvecl_uniform_eta_d)
void polyvecl_uniform_eta_d(polyvecl *v, const uint8_t seed[CRHBYTES], uint16_t *nonce);

#define polyvecl_challenge_y1_c EAGLESIGN_NAMESPACE(polyvecl_challenge_y1_c)
void polyvecl_challenge_y1_c(polyvecl *v, const uint8_t seed[SEEDBYTES], uint16_t *nonce, int param);

#define polyvecl_add EAGLESIGN_NAMESPACE(polyvecl_add)
void polyvecl_add(polyvecl *w, const polyvecl *u, const polyvecl *v);

#define polyvecl_ntt EAGLESIGN_NAMESPACE(polyvecl_ntt)
void polyvecl_ntt(polyvecl *v);
#define polyvecl_invntt_tomont EAGLESIGN_NAMESPACE(polyvecl_invntt_tomont)
void polyvecl_invntt_tomont(polyvecl *v);
#define polyvecl_pointwise_poly_montgomery EAGLESIGN_NAMESPACE(polyvecl_pointwise_poly_montgomery)
void polyvecl_pointwise_poly_montgomery(polyvecl *r, const poly *a, const polyvecl *v);
#define polyvecl_pointwise_acc_montgomery \
  EAGLESIGN_NAMESPACE(polyvecl_pointwise_acc_montgomery)
void polyvecl_pointwise_acc_montgomery(poly *w,
                                       const polyvecl *u,
                                       const polyvecl *v);

/* Vectors of polynomials of length K */
typedef struct
{
  poly vec[K];
} polyveck;

#define polyveck_uniform_eta_y2 EAGLESIGN_NAMESPACE(polyveck_uniform_eta_y2)
void polyveck_uniform_eta_y2(polyveck *v, const uint8_t seed[CRHBYTES], uint16_t *nonce);

#define polyveck_add EAGLESIGN_NAMESPACE(polyveck_add)
void polyveck_add(polyveck *w, const polyveck *u, const polyveck *v);
#define polyveck_sub EAGLESIGN_NAMESPACE(polyveck_sub)
void polyveck_sub(polyveck *w, const polyveck *u, const polyveck *v);

#define polyveck_ntt EAGLESIGN_NAMESPACE(polyveck_ntt)
void polyveck_ntt(polyveck *v);
#define polyveck_invntt_tomont EAGLESIGN_NAMESPACE(polyveck_invntt_tomont)
void polyveck_invntt_tomont(polyveck *v);
#define polyveck_pointwise_poly_montgomery EAGLESIGN_NAMESPACE(polyveck_pointwise_poly_montgomery)
void polyveck_pointwise_poly_montgomery(polyveck *r, const poly *a, const polyveck *v);

#define polyveck_pack_P EAGLESIGN_NAMESPACE(polyveck_pack_P)
void polyveck_pack_P(uint8_t r[K * NBYTES * LOGQ], polyveck *P);

#define polyveck_unpack_P EAGLESIGN_NAMESPACE(polyveck_unpack_P)
void polyveck_unpack_P(polyveck *P, uint8_t r[K * NBYTES * LOGQ]);

#define polyvec_chknorms EAGLESIGN_NAMESPACE(polyvec_chknorms)
int polyvec_chknorms(const polyvecl *Z, const polyveck *W);

#define polyvecl_chknorms EAGLESIGN_NAMESPACE(polyvecl_chknorms)
int polyvecl_chknorms(const polyvecl *Z, const unsigned int borm);

#endif
