#ifndef _API_H_
#define _API_H_

#include "params.h"
//  Set these three values apropriately for your algorithm
#define CRYPTO_SECRETKEYBYTES PRIVKEY_BYTES
#define CRYPTO_PUBLICKEYBYTES PUBKEY_BYTES
#define CRYPTO_BYTES SIGNATURE_BYTES

// Change the algorithm name
#define CRYPTO_ALGNAME "biscuit192s"

int
crypto_sign_keypair (unsigned char *pk, unsigned char *sk);

int
crypto_sign (unsigned char *sm, unsigned long long *smlen,
             const unsigned char *m, unsigned long long mlen,
             const unsigned char *sk);

int
crypto_sign_open (unsigned char *m, unsigned long long *mlen,
                  const unsigned char *sm, unsigned long long smlen,
                  const unsigned char *pk);

#endif
