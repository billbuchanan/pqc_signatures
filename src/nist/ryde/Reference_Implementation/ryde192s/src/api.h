/**
 * @file api.h
 * @brief NIST SIGN API
 */


#ifndef RYDE_192S_API_H
#define RYDE_192S_API_H

#include "parameters.h"

#define CRYPTO_ALGNAME "RYDE-192S"

#define CRYPTO_PUBLICKEYBYTES RYDE_192S_PUBLIC_KEY_BYTES
#define CRYPTO_SECRETKEYBYTES RYDE_192S_SECRET_KEY_BYTES
#define CRYPTO_BYTES          RYDE_192S_SIGNATURE_BYTES

int crypto_sign_keypair(unsigned char *pk, unsigned char *sk);
int crypto_sign(unsigned char *sm, unsigned long long *smlen, const unsigned char *m, unsigned long long mlen, const unsigned char *sk);
int crypto_sign_open(unsigned char *m, unsigned long long *mlen, const unsigned char *sm, unsigned long long smlen, const unsigned char *pk);


#endif //RYDE_192S_API_H
