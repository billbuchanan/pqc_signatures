/*
 *  SPDX-License-Identifier: MIT
 */

#ifdef SUPERCOP
#include "crypto_sign.h"
#else
#include "api.h"
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "faest_em_128f.h"
#include "owf.h"
#include "randomness.h"
#include "parameters.h"

#include <string.h>

// memory layout of the public key: OWF input || OWF output
#define PK_INPUT(pk) (pk)
#define PK_OUTPUT(pk) (&pk[32 / 2])

// memory layout of the secret key: OWF input || OWF key
#define SK_INPUT(sk) (sk)
#define SK_KEY(sk) (&sk[32 / 2])

int crypto_sign_keypair(unsigned char* pk, unsigned char* sk) {
  int ret = faest_em_128f_keygen(pk, SK_KEY(sk));
  if (!ret) {
    memcpy(SK_INPUT(sk), PK_INPUT(pk), 32 / 2);
  }
  return ret;
}

int crypto_sign(unsigned char* sm, unsigned long long* smlen, const unsigned char* m,
                unsigned long long mlen, const unsigned char* sk) {
  uint8_t pk[32];
  memcpy(PK_INPUT(pk), SK_INPUT(sk), 32 / 2);
  if (!faest_em_128f_owf(SK_KEY(sk), PK_INPUT(pk), PK_OUTPUT(pk))) {
    // invalid key
    return -1;
  }

  *smlen = mlen + FAEST_EM_128F_SIGNATURE_SIZE;
  memmove(sm, m, mlen);

  size_t signature_len = FAEST_EM_128F_SIGNATURE_SIZE;
#if defined(CRYPTO_DETERMINISTIC) && CRYPTO_DETERMINISTIC == 1
  return faest_em_128f_sign(SK_KEY(sk), pk, sm, mlen, NULL, 0, sm + mlen, &signature_len);
#else
  uint8_t rho[FAEST_EM_128F_LAMBDA / 8];
  rand_bytes(rho, sizeof(rho));
  return faest_em_128f_sign(SK_KEY(sk), pk, sm, mlen, rho, sizeof(rho), sm + mlen, &signature_len);
#endif
}

int crypto_sign_open(unsigned char* m, unsigned long long* mlen, const unsigned char* sm,
                     unsigned long long smlen, const unsigned char* pk) {
  if (smlen < FAEST_EM_128F_SIGNATURE_SIZE) {
    // signature too short
    return -1;
  }
  unsigned long long m_length = smlen - FAEST_EM_128F_SIGNATURE_SIZE;
  if (faest_em_128f_verify(pk, sm, m_length, sm + m_length, FAEST_EM_128F_SIGNATURE_SIZE)) {
    return -1;
  }

  *mlen = m_length;
  memmove(m, sm, m_length);
  return 0;
}

// vim: ft=c
