#ifndef API_H
#define API_H

#define CRYPTO_SECRETKEYBYTES 1828
#define CRYPTO_PUBLICKEYBYTES 9923
#define CRYPTO_BYTES 9896

#define CRYPTO_ALGNAME "MEDS9923"

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

