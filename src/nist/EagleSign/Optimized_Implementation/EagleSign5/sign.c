/*
 * Implementors: EagleSign Team
 * This implementation is highly inspired from Dilithium and
 * Falcon Signatures' implementations
 */

#include <stdint.h>
#include <stdio.h>
#include "params.h"
#include "sign.h"
#include "packing.h"
#include "polymatrix.h"
#include "polyvec.h"
#include "poly.h"
#include "randombytes.h"
#include "symmetric.h"
#include "fips202.h"

/*************************************************
 * Name:        crypto_sign_keypair
 *
 * Description: Generates public and private key.
 *
 * Arguments:   - uint8_t *pk: pointer to output public key (allocated
 *                             array of CRYPTO_EAGLESIGN_PUBLICKEYBYTES bytes)
 *              - uint8_t *sk: pointer to output private key (allocated
 *                             array of CRYPTO_EAGLESIGN_SECRETKEYBYTES bytes)
 *
 * Returns 0 (success)
 **************************************************/

int crypto_sign_keypair(uint8_t *pk, uint8_t *sk)
{
  uint8_t seedbuf[SEEDBYTES + 2 * CRHBYTES];
  uint8_t tr[SEEDBYTES], *rhoprime2;
  const uint8_t *rho, *rhoprime1;
  polyvecl A[K], B[L], G[L], D[K], G_INV[L], tmp1[K], E[K];
  polyveck tmp2[L];
  keccak_state state;

  /* Get randomness for rho and rhoprime */
  randombytes(seedbuf, SEEDBYTES);
  shake256(seedbuf, SEEDBYTES + 2 * CRHBYTES, seedbuf, SEEDBYTES);
  rho = seedbuf;
  rhoprime1 = rho + SEEDBYTES;
  rhoprime2 = seedbuf + SEEDBYTES + CRHBYTES;

  /* Expand matrix A in NTT form*/
  polyvec_matrix_expand(A, rho);

  /* Small and Uniform polynomials based matrix D*/
  polymatrix_k_l_expand(D, rhoprime1);

retry:
  /* Small and Uniform polynomials based invertible matrix G and its inverse G_INV */
  shake256(rhoprime2, CRHBYTES, rhoprime2, CRHBYTES);
  if (polymatrix_l_expand_invertible(G_INV, G, rhoprime2))
    goto retry;

  /* Compute E = (A + D)*G_INV */
  polyvec_matrix_pointwise_add(tmp1, A, D);
  polyvec_matrix_pointwise_product(tmp2, tmp1, G_INV);
  polyvec_matrix_reformat(E, tmp2);

  /*Bring back elements from ntt*/
  polymatrix_invntt_tomont_k_l(E);
  polymatrix_invntt_tomont_l_l(G);
  polymatrix_invntt_tomont_k_l(D);

  /* Extracting public key pk = (rho, E) */
  pack_pk(pk, rho, E);

  /* Compute tr = H(pk)*/
  // tr = CRH1(pk)
  shake256(tr, SEEDBYTES, pk, CRYPTO_EAGLESIGN_PUBLICKEYBYTES);

  /* Extracting secret key sk = (rho, tr, G, D) */
  pack_sk(sk, rho, tr, G, D);
  return 0;
}

/*************************************************
 * Name:        crypto_sign_signature
 *
 * Description: Computes signature.
 *
 * Arguments:   - uint8_t *sig:   pointer to output signature (of length CRYPTO_EAGLESIGN_BYTES)
 *              - size_t *siglen: pointer to output length of signature
 *              - uint8_t *m:     pointer to message to be signed
 *              - size_t mlen:    length of message
 *              - uint8_t *sk:    pointer to bit-packed secret key
 *
 * Returns 0 (success)
 **************************************************/
int crypto_sign_signature(uint8_t *sig,
                          size_t *siglen,
                          const uint8_t *m,
                          size_t mlen,
                          const uint8_t *sk)
{
  uint8_t seedbuf[3 * SEEDBYTES + 2 * CRHBYTES];
  uint8_t tmp[K * NBYTES * LOGQ],
      rho[SEEDBYTES], tr[SEEDBYTES],
      mu[CRHBYTES], rhoprime[CRHBYTES],
      r[SEEDBYTES], c[SEEDBYTES];
  polyvecl A[K], B[K], G[L], E[K], D[K], Y1, C, U, Z, V2;
  polyveck Y2, P, W, V, V1;
  uint16_t nonce = 0, nonce_c = 0;
  keccak_state state;

  unpack_sk(rho, tr, G, D, sk);

  polymatrix_ntt_k_l(D);
  polymatrix_ntt_l_l(G);

  /* Expand matrix A in NTT form*/
  polyvec_matrix_expand(A, rho);

  /* Generating ephemeral secret keys Y1 and Y2 */
  randombytes(rhoprime, CRHBYTES);
  polyvecl_challenge_y1_c(&Y1, rhoprime, &nonce, 0);
  polyveck_uniform_eta_y2(&Y2, rhoprime, &nonce);

  /* Computing P = AY1 + Y2 */
  polyvec_matrix_pointwise_montgomery(&P, A, &Y1);
  polyveck_add(&P, &P, &Y2);

  // /* Compute mu = CRH(tr, m) */
  shake256_init(&state);
  shake256_absorb(&state, tr, SEEDBYTES);
  shake256_absorb(&state, m, mlen);
  shake256_finalize(&state);
  shake256_squeeze(mu, CRHBYTES, &state);

  /* Compute r = G(P)*/
  polyveck_invntt_tomont(&P);
  polyveck_pack_P(tmp, &P);

  shake256_init(&state);
  shake256_absorb(&state, tmp, K * NBYTES * LOGQ);
  shake256_finalize(&state);
  shake256_squeeze(r, SEEDBYTES, &state);

  /* Call the random oracle and Compute C = H(mu,r)*/
  shake256_init(&state);
  shake256_absorb(&state, r, SEEDBYTES);
  shake256_absorb(&state, mu, CRHBYTES);
  shake256_finalize(&state);
  shake256_squeeze(c, SEEDBYTES, &state);
  polyvecl_challenge_y1_c(&C, c, &nonce_c, 1);

  /* Compute U = Y1 + C */
  polyvecl_add(&U, &Y1, &C);

  /* Compute Z = GU */
  polyvec_matrix_pointwise_montgomery_l_l(&Z, G, &U);

  /* Compute W = Y2 - DU */
  polyvec_matrix_pointwise_montgomery(&W, D, &U);
  polyveck_sub(&W, &Y2, &W);

  /* Packing Signature (C, Z, W)*/
  polyvecl_invntt_tomont(&Z);
  polyveck_invntt_tomont(&W);

  pack_sig(sig, r, &Z, &W);

  *siglen = CRYPTO_EAGLESIGN_BYTES;
  return 0;
}

/*************************************************
 * Name:        crypto_sign
 *
 * Description: Compute signed message.
 *
 * Arguments:   - uint8_t *sm: pointer to output signed message (allocated
 *                             array with CRYPTO_EAGLESIGN_BYTES + mlen bytes),
 *                             can be equal to m
 *              - size_t *smlen: pointer to output length of signed
 *                               message
 *              - const uint8_t *m: pointer to message to be signed
 *              - size_t mlen: length of message
 *              - const uint8_t *sk: pointer to bit-packed secret key
 *
 * Returns 0 (success)
 **************************************************/
int crypto_sign(uint8_t *sm,
                size_t *smlen,
                const uint8_t *m,
                size_t mlen,
                const uint8_t *sk)
{

  size_t i;

  for (i = 0; i < mlen; ++i)
    sm[CRYPTO_EAGLESIGN_BYTES + mlen - 1 - i] = m[mlen - 1 - i];
  crypto_sign_signature(sm, smlen, sm + CRYPTO_EAGLESIGN_BYTES, mlen, sk);

  *smlen += mlen;
  return 0;
}

/*************************************************
 * Name:        crypto_sign_verify
 *
 * Description: Verifies signature.
 *
 * Arguments:   - uint8_t *m: pointer to input signature
 *              - size_t siglen: length of signature
 *              - const uint8_t *m: pointer to message
 *              - size_t mlen: length of message
 *              - const uint8_t *pk: pointer to bit-packed public key
 *
 * Returns 0 if signature could be verified correctly and -1 otherwise
 **************************************************/
int crypto_sign_verify(const uint8_t *sig,
                       size_t siglen,
                       const uint8_t *m,
                       size_t mlen,
                       const uint8_t *pk)
{
  unsigned int i, j;
  uint8_t rho[SEEDBYTES], tmp[K * NBYTES * LOGQ], mu[CRHBYTES], c[SEEDBYTES], r[SEEDBYTES], r_prime[SEEDBYTES];
  polyvecl A[K], E[K], C, C_prime, Z;
  polyveck W, V, V1;
  keccak_state state;
  uint16_t nonce_c = 0;

  unpack_pk(rho, E, pk);

  unpack_sig(r, &Z, &W, sig);

  if (polyvec_chknorms(&Z, &W))
    return -1;

  /* Applying NTT Transformation*/
  polymatrix_ntt_k_l(E);
  polyvecl_ntt(&Z);
  polyveck_ntt(&W);

  /* Compute mu = CRH(H(pk), msg) */
  shake256(mu, SEEDBYTES, pk, CRYPTO_EAGLESIGN_PUBLICKEYBYTES);
  shake256_init(&state);
  shake256_absorb(&state, mu, SEEDBYTES);
  shake256_absorb(&state, m, mlen);
  shake256_finalize(&state);
  shake256_squeeze(mu, CRHBYTES, &state);

  /* Call the random oracle and Compute C = H(mu,r) dans B_tau^l*/
  shake256_init(&state);
  shake256_absorb(&state, r, SEEDBYTES);
  shake256_absorb(&state, mu, CRHBYTES);
  shake256_finalize(&state);
  shake256_squeeze(c, SEEDBYTES, &state);
  polyvecl_challenge_y1_c(&C, c, &nonce_c, 1);

  /* Expand matrix A in NTT form*/
  polyvec_matrix_expand(A, rho);

  /* Compute V = EZ − AC + W */
  polyvec_matrix_pointwise_montgomery(&V, E, &Z);
  polyvec_matrix_pointwise_montgomery(&V1, A, &C);
  polyveck_sub(&V, &V, &V1);
  polyveck_add(&V, &V, &W);

  /* Compute r_prime = G(V) */
  polyveck_invntt_tomont(&V);
  polyveck_pack_P(tmp, &V);

  shake256_init(&state);
  shake256_absorb(&state, tmp, K * NBYTES * LOGQ);
  shake256_finalize(&state);
  shake256_squeeze(r_prime, SEEDBYTES, &state);

  /* Compute C_prime = H(mu,r_prime) */
  shake256_init(&state);
  shake256_absorb(&state, r_prime, SEEDBYTES);
  shake256_absorb(&state, mu, CRHBYTES);
  shake256_finalize(&state);
  shake256_squeeze(c, SEEDBYTES, &state);
  nonce_c = 0;
  polyvecl_challenge_y1_c(&C_prime, c, &nonce_c, 1);

  /* Comparing C and C_prime */
  for (i = 0; i < L; ++i)
    for (j = 0; j < N; ++j)
      if (C.vec[i].coeffs[j] != C_prime.vec[i].coeffs[j])
        return -1;

  return 0;
}

/*************************************************
 * Name:        crypto_sign_open
 *
 * Description: Verify signed message.
 *
 * Arguments:   - uint8_t *m: pointer to output message (allocated
 *                            array with smlen bytes), can be equal to sm
 *              - size_t *mlen: pointer to output length of message
 *              - const uint8_t *sm: pointer to signed message
 *              - size_t smlen: length of signed message
 *              - const uint8_t *pk: pointer to bit-packed public key
 *
 * Returns 0 if signed message could be verified correctly and -1 otherwise
 **************************************************/
int crypto_sign_open(uint8_t *m,
                     size_t *mlen,
                     const uint8_t *sm,
                     size_t smlen,
                     const uint8_t *pk)
{

  size_t i;

  if (smlen < CRYPTO_EAGLESIGN_BYTES)
    goto badsig;

  *mlen = smlen - CRYPTO_EAGLESIGN_BYTES;

  if (crypto_sign_verify(sm, CRYPTO_EAGLESIGN_BYTES, sm + CRYPTO_EAGLESIGN_BYTES, *mlen, pk))
    goto badsig;
  else
  {
    /* All good, copy msg, return 0 */
    for (i = 0; i < *mlen; ++i)
      m[i] = sm[CRYPTO_EAGLESIGN_BYTES + i];
    return 0;
  }

badsig:
  /* Signature verification failed */
  *mlen = -1;
  for (i = 0; i < smlen; ++i)
    m[i] = 0;

  return -1;
}
