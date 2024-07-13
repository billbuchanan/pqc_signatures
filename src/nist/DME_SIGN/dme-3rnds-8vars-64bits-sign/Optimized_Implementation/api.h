#ifndef API_H
#define API_H

#include "dme.h"

#define CRYPTO_SECRETKEYBYTES  SKEY_BYTES
#define CRYPTO_PUBLICKEYBYTES  PKEY_BYTES
#define CRYPTO_BYTES           64

#define CRYPTO_ALGNAME "dme-3rnds-8vars-64bits-sign"

int crypto_sign_keypair(unsigned char *pk, unsigned char *sk);

int crypto_sign(unsigned char *sm, unsigned long long *smlen,
    const unsigned char *m, unsigned long long mlen,
    const unsigned char *sk);

int crypto_sign_open(unsigned char *m, unsigned long long *mlen,
    const unsigned char *sm, unsigned long long smlen,
    const unsigned char *pk);

#endif
