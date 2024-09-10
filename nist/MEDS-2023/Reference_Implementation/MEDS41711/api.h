#ifndef API_H
#define API_H

#define CRYPTO_SECRETKEYBYTES 4420
#define CRYPTO_PUBLICKEYBYTES 41711
#define CRYPTO_BYTES 41080

#define CRYPTO_ALGNAME "MEDS41711"

int crypto_sign_keypair(
    unsigned char *pk,
    unsigned char *sk
  );

int crypto_sign(
    unsigned char *sm, unsigned long long *smlen,
    const unsigned char *m, unsigned long long mlen,
    const unsigned char *sk
  );

int crypto_sign_open(
    unsigned char *m, unsigned long long *mlen,
    const unsigned char *sm, unsigned long long smlen,
    const unsigned char *pk
  );

#endif

