#ifndef API_H
#define API_H

#ifndef SQUIRRELS_LEVEL
#define SQUIRRELS_LEVEL 1
#endif

#if SQUIRRELS_LEVEL == 1
#define CRYPTO_SECRETKEYBYTES 12829872
#define CRYPTO_PUBLICKEYBYTES 681780
#define CRYPTO_BYTES 1019
#define CRYPTO_ALGNAME "Squirrels-I"
#endif
#if SQUIRRELS_LEVEL == 2
#define CRYPTO_SECRETKEYBYTES 16258752
#define CRYPTO_PUBLICKEYBYTES 874576
#define CRYPTO_BYTES 1147
#define CRYPTO_ALGNAME "Squirrels-II"
#endif
#if SQUIRRELS_LEVEL == 3
#define CRYPTO_SECRETKEYBYTES 29053632
#define CRYPTO_PUBLICKEYBYTES 1629640
#define CRYPTO_BYTES 1554
#define CRYPTO_ALGNAME "Squirrels-III"
#endif
#if SQUIRRELS_LEVEL == 4
#define CRYPTO_SECRETKEYBYTES 35418288
#define CRYPTO_PUBLICKEYBYTES 1888700
#define CRYPTO_BYTES 1676
#define CRYPTO_ALGNAME "Squirrels-IV"
#endif
#if SQUIRRELS_LEVEL == 5
#define CRYPTO_SECRETKEYBYTES 50725632
#define CRYPTO_PUBLICKEYBYTES 2786580
#define CRYPTO_BYTES 2025
#define CRYPTO_ALGNAME "Squirrels-V"
#endif

int crypto_sign_keypair(unsigned char *pk, unsigned char *sk);

int crypto_sign(unsigned char *sm, unsigned long long *smlen,
                const unsigned char *m, unsigned long long mlen,
                const unsigned char *sk);

int crypto_sign_open(unsigned char *m, unsigned long long *mlen,
                     const unsigned char *sm, unsigned long long smlen,
                     const unsigned char *pk);

#endif
