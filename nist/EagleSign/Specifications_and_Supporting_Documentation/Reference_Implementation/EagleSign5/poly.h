/*
 * Implementors: EagleSign Team
 * This implementation is highly inspired from Dilithium and
 * Falcon Signatures' implementations
 */

#ifndef POLY_H
#define POLY_H

#include <stdint.h>
#include "params.h"

typedef struct
{
  S_Q_SIZE coeffs[N];
} poly;

#define poly_add EAGLESIGN_NAMESPACE(poly_add)
void poly_add(poly *c, const poly *a, const poly *b);
#define poly_sub EAGLESIGN_NAMESPACE(poly_sub)
void poly_sub(poly *c, const poly *a, const poly *b);

#define poly_ntt EAGLESIGN_NAMESPACE(poly_ntt)
void poly_ntt(poly *a);
#define poly_invntt_tomont EAGLESIGN_NAMESPACE(poly_invntt_tomont)
void poly_invntt_tomont(poly *a);

#define poly_pointwise_montgomery EAGLESIGN_NAMESPACE(poly_pointwise_montgomery)
void poly_pointwise_montgomery(poly *c, const poly *a, const poly *b);

#define poly_uniform EAGLESIGN_NAMESPACE(poly_uniform)
void poly_uniform(poly *a,
                  const uint8_t seed[SEEDBYTES],
                  uint16_t nonce);
#define poly_uniform_eta_y2 EAGLESIGN_NAMESPACE(poly_uniform_eta_y2)
void poly_uniform_eta_y2(poly *a,
                         const uint8_t seed[CRHBYTES],
                         uint16_t nonce);

#define poly_uniform_eta_g EAGLESIGN_NAMESPACE(poly_uniform_eta_g)
void poly_uniform_eta_g(poly *a,
                        const uint8_t seed[CRHBYTES],
                        uint16_t nonce);

#define poly_uniform_eta_d EAGLESIGN_NAMESPACE(poly_uniform_eta_d)
void poly_uniform_eta_d(poly *a,
                        const uint8_t seed[CRHBYTES],
                        uint16_t nonce);

#define poly_challenge_y1_c EAGLESIGN_NAMESPACE(poly_challenge_y1_c)
void poly_challenge_y1_c(poly *c, const uint8_t seed[SEEDBYTES],
                         uint16_t nonce, int param);

#define polyQ_pack EAGLESIGN_NAMESPACE(polyQ_pack)
void polyQ_pack(uint8_t *r, const poly *a);

#define polyQ_unpack EAGLESIGN_NAMESPACE(polyQ_unpack)
void polyQ_unpack(poly *r, const uint8_t *a);

#define polyG_pack EAGLESIGN_NAMESPACE(polyG_pack)
void polyG_pack(uint8_t *r, const poly *a, unsigned int logeta);

#define polyG_unpack EAGLESIGN_NAMESPACE(polyG_unpack)
void polyG_unpack(poly *r, const uint8_t *a, unsigned int logeta);

#define polyZ_pack EAGLESIGN_NAMESPACE(polyZ_pack)
void polyZ_pack(uint8_t *r, const poly *a, unsigned int logeta);

#define polyZ_unpack EAGLESIGN_NAMESPACE(polyZ_unpack)
void polyZ_unpack(poly *r, const uint8_t *a, unsigned int logeta);

#endif
