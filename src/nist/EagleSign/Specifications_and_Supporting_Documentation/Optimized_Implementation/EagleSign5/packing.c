/*
 * Implementors: EagleSign Team
 * This implementation is highly inspired from Dilithium and
 * Falcon Signatures' implementations
 */

#include "params.h"
#include "packing.h"
#include "polyvec.h"
#include "poly.h"

/*************************************************
 * Name:        pack_pk
 *
 * Description: Bit-pack public key pk = (rho, E).
 *
 * Arguments:   - uint8_t pk[]: output byte array
 *              - const uint8_t rho[]: byte array containing rho
 *              - const polyvecl E[K]: array containing the matrix E
 **************************************************/
void pack_pk(uint8_t pk[CRYPTO_EAGLESIGN_PUBLICKEYBYTES],
             const uint8_t rho[SEEDBYTES],
             const polyvecl E[K])
{
  unsigned int i, j;

  for (i = 0; i < SEEDBYTES; ++i)
    pk[i] = rho[i];
  pk += SEEDBYTES;

  for (i = 0; i < K; ++i)
    for (j = 0; j < L; ++j)
      polyQ_pack(pk + (i * L + j) * NBYTES * LOGQ, &E[i].vec[j]);
}

/*************************************************
 * Name:        unpack_pk
 *
 * Description: Unpack public key pk = (rho, E).
 *
 * Arguments:   - uint8_t rho[]: output byte array for rho
 *              - polyvecl E[K]: array containing the matrix E
 *              - const uint8_t pk[]: byte array containing bit-packed pk
 **************************************************/
void unpack_pk(
    uint8_t rho[SEEDBYTES],
    polyvecl E[K],
    const uint8_t pk[CRYPTO_EAGLESIGN_PUBLICKEYBYTES])
{
  unsigned int i, j;

  for (i = 0; i < SEEDBYTES; ++i)
    rho[i] = pk[i];
  pk += SEEDBYTES;

  for (i = 0; i < K; ++i)
    for (j = 0; j < L; ++j)
      polyQ_unpack(&E[i].vec[j], pk + (i * L + j) * NBYTES * LOGQ);
}

/*************************************************
 * Name:        pack_sk
 *
 * Description: Bit-pack secret key sk = (rho, tr, G, D).
 *
 * Arguments:   - uint8_t sk[]: output byte array
 *              - const uint8_t rho[]: byte array containing rho
 *              - const uint8_t tr[]: byte array containing tr
 *              - const polyvecl G[]: array containing the matrix G
 *              - const polyvecl D[]: array containing the matrix D
 **************************************************/
void pack_sk(uint8_t sk[CRYPTO_EAGLESIGN_SECRETKEYBYTES],
             const uint8_t rho[SEEDBYTES],
             const uint8_t tr[SEEDBYTES],
             const polyvecl G[L],
             const polyvecl D[K])
{
  unsigned int i, j;

  for (i = 0; i < SEEDBYTES; ++i)
    sk[i] = rho[i];
  sk += SEEDBYTES;

  for (i = 0; i < SEEDBYTES; ++i)
    sk[i] = tr[i];
  sk += SEEDBYTES;

  for (i = 0; i < L; ++i)
    for (j = 0; j < L; ++j)
      polyG_pack(sk + (i * L + j) * NBYTES * LOGETAG, &G[i].vec[j], LOGETAG);

  sk += L * L * NBYTES * LOGETAG;

  for (i = 0; i < K; ++i)
    for (j = 0; j < L; ++j)
      polyG_pack(sk + (i * L + j) * NBYTES * LOGETAD, &D[i].vec[j], LOGETAD);
}

/*************************************************
 * Name:        unpack_sk
 *
 * Description: Unpack secret key sk = (rho, tr, G, D).
 *
 * Arguments:   - uint8_t rho[]: output byte array for rho
 *              - uint8_t tr[]: byte array containing tr
 *              - polyvecl G[]: array containing the matrix G
 *              - polyvecl D[]: array containing the matrix D
 *              - const uint8_t sk[]: output byte array
 **************************************************/
void unpack_sk(
    uint8_t rho[SEEDBYTES],
    uint8_t tr[SEEDBYTES],
    polyvecl G[L],
    polyvecl D[K],
    const uint8_t sk[CRYPTO_EAGLESIGN_SECRETKEYBYTES])
{
  unsigned int i, j;

  for (i = 0; i < SEEDBYTES; ++i)
    rho[i] = sk[i];
  sk += SEEDBYTES;

  for (i = 0; i < SEEDBYTES; ++i)
    tr[i] = sk[i];
  sk += SEEDBYTES;

  for (i = 0; i < L; ++i)
    for (j = 0; j < L; ++j)
      polyG_unpack(&G[i].vec[j], sk + (i * L + j) * NBYTES * LOGETAG, LOGETAG);

  sk += L * L * NBYTES * LOGETAG;

  for (i = 0; i < K; ++i)
    for (j = 0; j < L; ++j)
      polyG_unpack(&D[i].vec[j], sk + (i * L + j) * NBYTES * LOGETAD, LOGETAD);
}

/*************************************************
 * Name:        pack_sig
 *
 * Description: Bit-pack signature sig = (C, Z, W).
 *
 * Arguments:   - uint8_t sig[]: output byte array
 *              - const uint8_t r: input byte array for r
 *              - const polyvecl *Z: pointer to vector Z
 *              - const polyveck *W: pointer to hint vector W
 **************************************************/
void pack_sig(uint8_t sig[CRYPTO_EAGLESIGN_BYTES],
              const uint8_t r[SEEDBYTES],
              const polyvecl *Z,
              const polyveck *W)
{
  unsigned int i;

  for (i = 0; i < SEEDBYTES; ++i)
    sig[i] = r[i];
  sig += SEEDBYTES;

  for (i = 0; i < L; ++i)
    polyZ_pack(sig + i * NBYTES * LOGDELTA, &Z->vec[i], LOGDELTA);

  sig += L * NBYTES * LOGDELTA;

  for (i = 0; i < K; ++i)
    polyZ_pack(sig + i * NBYTES * LOGDELTA_PRIME, &W->vec[i], LOGDELTA_PRIME);
}

/*************************************************
 * Name:        unpack_sig
 *
 * Description: Unpack signature sig = (C, Z, W).
 *
 * Arguments:   - uint8_t r: output byte array for r
 *              - polyvecl *Z: pointer to vector Z
 *              - polyveck *W: pointer to hint vector W
 *              - const uint8_t sig[]: byte array containing
 *                bit-packed signature
 *
 * Returns 1 in case of malformed signature; otherwise 0.
 **************************************************/
int unpack_sig(
    uint8_t r[SEEDBYTES],
    polyvecl *Z,
    polyveck *W,
    const uint8_t sig[CRYPTO_EAGLESIGN_BYTES])
{
  unsigned int i;

  for (i = 0; i < SEEDBYTES; ++i)
    r[i] = sig[i];
  sig += SEEDBYTES;

  for (i = 0; i < L; ++i)
    polyZ_unpack(&Z->vec[i], sig + i * NBYTES * LOGDELTA, LOGDELTA);

  sig += L * NBYTES * LOGDELTA;

  for (i = 0; i < K; ++i)
    polyZ_unpack(&W->vec[i], sig + i * NBYTES * LOGDELTA_PRIME, LOGDELTA_PRIME);

  return 0;
}
