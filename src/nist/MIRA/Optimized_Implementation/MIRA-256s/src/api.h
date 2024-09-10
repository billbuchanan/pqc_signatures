/**
 * @file api.h
 * @brief NIST SIGN API
 */


#ifndef SIGN_MIRA_256S_API_H
#define SIGN_MIRA_256S_API_H

#include "parameters.h"

#define CRYPTO_ALGNAME "MIRA-256S"

#define CRYPTO_PUBLICKEYBYTES SIGN_MIRA_256_PUBLIC_KEY_BYTES
#define CRYPTO_SECRETKEYBYTES SIGN_MIRA_256_SECRET_KEY_BYTES
#define CRYPTO_BYTES          SIGN_MIRA_256_SIGNATURE_BYTES

int crypto_sign_keypair(unsigned char *pk, unsigned char *sk);
int crypto_sign(unsigned char *sm, unsigned long long *smlen, const unsigned char *m, unsigned long long mlen, const unsigned char *sk);
int crypto_sign_open(unsigned char *m, unsigned long long *mlen, const unsigned char *sm, unsigned long long smlen, const unsigned char *pk);


#endif //SIGN_MIRA_256S_API_H
