/**
 * @file api.h
 * @brief NIST SIGN API
 */


#ifndef RYDE_256F_API_H
#define RYDE_256F_API_H

#include "parameters.h"

#define CRYPTO_ALGNAME "RYDE-256F"

#define CRYPTO_PUBLICKEYBYTES RYDE_256F_PUBLIC_KEY_BYTES
#define CRYPTO_SECRETKEYBYTES RYDE_256F_SECRET_KEY_BYTES
#define CRYPTO_BYTES          RYDE_256F_SIGNATURE_BYTES

int crypto_sign_keypair(unsigned char *pk, unsigned char *sk);
int crypto_sign(unsigned char *sm, unsigned long long *smlen, const unsigned char *m, unsigned long long mlen, const unsigned char *sk);
int crypto_sign_open(unsigned char *m, unsigned long long *mlen, const unsigned char *sm, unsigned long long smlen, const unsigned char *pk);


#endif //RYDE_256F_API_H
