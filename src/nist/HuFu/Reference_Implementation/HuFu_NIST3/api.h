#ifndef API_H
#define API_H

#define CRYPTO_ALGNAME "HuFu"

#define CRYPTO_SECRETKEYBYTES (2228256 + 20944704)
#define CRYPTO_PUBLICKEYBYTES 2228256
#define CRYPTO_BYTES 3580

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