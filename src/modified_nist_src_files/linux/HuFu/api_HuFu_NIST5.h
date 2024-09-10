//modified-flag
#ifndef API_H
#define API_H

#define CRYPTO_ALGNAME "HuFu_NIST5"

#define CRYPTO_SECRETKEYBYTES (3657888 + 33760832)
#define CRYPTO_PUBLICKEYBYTES 3657888
#define CRYPTO_BYTES 4560

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