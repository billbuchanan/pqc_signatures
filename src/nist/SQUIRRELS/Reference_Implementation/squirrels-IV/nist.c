/*
 * ==========================(LICENSE BEGIN)============================
 *
 * Copyright (c) 2023  Squirrels Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * ===========================(LICENSE END)=============================
 *
 * @author   Guilhem Niot <guilhem.niot@gniot.fr>
 */

/*
 * Wrapper for implementing the NIST API for the PQC standardization
 * process.
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "api.h"
#include "inner.h"

#define NONCELEN 40

// #define NIST_VERBOSE
#ifdef NIST_VERBOSE
#include <stdio.h>
#endif

/*
 * If stack usage is an issue, define TEMPALLOC to static in order to
 * allocate temporaries in the data section instead of the stack. This
 * would make the crypto_sign_keypair(), crypto_sign(), and
 * crypto_sign_open() functions not reentrant and not thread-safe, so
 * this should be done only for testing purposes.
 */
#define TEMPALLOC

void randombytes_init(unsigned char *entropy_input,
                      unsigned char *personalization_string,
                      int security_strength);
int randombytes(unsigned char *x, unsigned long long xlen);

int crypto_sign_keypair(unsigned char *pk, unsigned char *sk) {
  secret_key *tmpsk = malloc(sizeof(secret_key));
  public_key *tmppk = malloc(sizeof(public_key));

  unsigned char seed[48];
  inner_shake256_context rng;
  size_t u, v;
  unsigned savcw;

  savcw = set_fpu_cw(2);

  /*
   * Generate key pair.
   */
  randombytes(seed, sizeof seed);
  inner_shake256_init(&rng);
  inner_shake256_inject(&rng, seed, sizeof seed);
  inner_shake256_flip(&rng);
  Zf(keygen)(&rng, tmpsk, tmppk);

  set_fpu_cw(savcw);

  u = 0;
  // Encode basis
  for (int i = 0; i < SQUIRRELS_D; i++) {
    for (int j = 0; j < SQUIRRELS_D; j++) {
      v = Zf(i32_encode)(sk + u, &tmpsk->b[i].coeffs[j], 1);
      u += v;
    }
  }

  // Encode Gram Schmidt basis
  size_t s_gm = 8 * SQUIRRELS_D * SQUIRRELS_D;
  memcpy(sk + u, &tmpsk->b_gm[0].coeffs[0], s_gm);

  u += s_gm;

#ifdef NIST_VERBOSE
  /* Check key consistency */
  for (int i = 0; i < SQUIRRELS_D; i++) {
    for (int j = 0; j < SQUIRRELS_D; j++) {
    	if(coeff_sk_b(sk,i,j) != tmpsk->b[i].coeffs[j]) {
	  printf("Inconsistency in B matrix; (i,j)=(%d,%d)\n",i,j);
	  printf("coeff_sk_b(sk,i,j)    = %d\n", coeff_sk_b(sk,i,j));
	  printf("tmpsk->b[i].coeffs[j] = %d\n", tmpsk->b[i].coeffs[j]);
	  return -1;
	}  
    	if(coeff_sk_b_gm(sk,i,j).v != tmpsk->b_gm[i].coeffs[j].v) {
	  printf("Inconsistency in GSO matrix; (i,j)=(%d,%d)\n",i,j);
	  printf("coeff_sk_b_gm(sk,i,j)    = %f\n", coeff_sk_b_gm(sk,i,j).v);
	  printf("tmpsk->b_gm[i].coeffs[j] = %f\n", tmpsk->b_gm[i].coeffs[j].v);
	  return -1;
	}  
    }
  }
#endif

  if (u != CRYPTO_SECRETKEYBYTES) {
#ifdef NIST_VERBOSE
    printf("Incorrect key size\n");
#endif
    return -1;
  }

  /*
   * Encode public key.
   */
  Zf(ui32_encode)(pk, tmppk->equation,
                  sizeof(tmppk->equation) / sizeof(tmppk->equation[0]));

  free(tmpsk);
  free(tmppk);

  return 0;
}

int crypto_sign(unsigned char *sm, unsigned long long *smlen,
                const unsigned char *m, unsigned long long mlen,
                const unsigned char *sk) {
  discrete_vector hm;
  unsigned char seed[48], nonce[NONCELEN];
  unsigned char esig[CRYPTO_BYTES - 2 - sizeof nonce];
  inner_shake256_context sc;
  size_t sig_len;
  unsigned savcw;

  for (;;) {
    /*
     * Create a random nonce (40 bytes).
     */
    randombytes(nonce, sizeof nonce);

    /*
     * Hash message nonce + message into a vector.
     */
    inner_shake256_init(&sc);
    inner_shake256_inject(&sc, nonce, sizeof nonce);
    inner_shake256_inject(&sc, m, mlen);
    inner_shake256_flip(&sc);
    Zf(hash_to_point)(&sc, &hm);

    /*
     * Initialize a RNG.
     */
    randombytes(seed, sizeof seed);
    inner_shake256_init(&sc);
    inner_shake256_inject(&sc, seed, sizeof seed);
    inner_shake256_flip(&sc);

    savcw = set_fpu_cw(2);

    /*
     * Compute the signature.
     */
    if (!Zf(sign)(&sc, sk, &hm)) {
      continue; // Signature failure
    }

    set_fpu_cw(savcw);

    if (!(sig_len = Zf(comp_encode)(esig + 1, sizeof(esig) - 1, &hm,
                                    SQUIRRELS_SIG_RATE))) {
      continue; // Encoding is too long
    }

    esig[0] = 0x20 + SQUIRRELS_LEVEL; // Signature header
    sig_len++;

    /*
     * Encode the signature and bundle it with the message. Format is:
     *   signature length     2 bytes, big-endian
     *   nonce                40 bytes
     *   message              mlen bytes
     *   signature            slen bytes
     */
    memmove(sm + 2 + sizeof nonce, m, mlen);
    sm[0] = (unsigned char)(sig_len >> 8);
    sm[1] = (unsigned char)sig_len;
    memcpy(sm + 2, nonce, sizeof nonce);
    memcpy(sm + 2 + (sizeof nonce) + mlen, esig, sig_len);
    *smlen = 2 + (sizeof nonce) + mlen + sig_len;

    return 0;
  }
}

int crypto_sign_open(unsigned char *m, unsigned long long *mlen,
                     const unsigned char *sm, unsigned long long smlen,
                     const unsigned char *pk) {
  const unsigned char *esig;
  TEMPALLOC inner_shake256_context sc;
  size_t sig_len, msg_len;
  discrete_vector sig, hm;

  /*
   * Find nonce, signature, message length.
   */
  if (smlen < 2 + NONCELEN) {
    return -1;
  }
  sig_len = ((size_t)sm[0] << 8) | (size_t)sm[1];
  if (sig_len > (smlen - 2 - NONCELEN)) {
    return -1;
  }
  msg_len = smlen - 2 - NONCELEN - sig_len;

  /*
   * Decode signature.
   */
  esig = sm + 2 + NONCELEN + msg_len;
  if (sig_len < 1 || esig[0] != 0x20 + SQUIRRELS_LEVEL) {
    return -1;
  }
  if (Zf(comp_decode)(&sig, esig + 1, sig_len - 1, SQUIRRELS_SIG_RATE) !=
      sig_len - 1) {
    return -1;
  }

  /*
   * Hash nonce + message into a vector.
   */
  inner_shake256_init(&sc);
  inner_shake256_inject(&sc, sm + 2, NONCELEN + msg_len);
  inner_shake256_flip(&sc);
  Zf(hash_to_point)(&sc, &hm);

  /*
   * Verify signature.
   */
  if (!Zf(verify_raw)(pk, &sig, &hm)) {
    return -1;
  }

  /*
   * Return plaintext.
   */
  memmove(m, sm + 2 + NONCELEN, msg_len);
  *mlen = msg_len;
  return 0;
}
