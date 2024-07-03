/*
 *  SPDX-License-Identifier: MIT
 */

#ifndef CRYPTO_SIGN_128F_H
#define CRYPTO_SIGN_128F_H

#define CRYPTO_SECRETKEYBYTES (16 + 32 / 2)
#define CRYPTO_PUBLICKEYBYTES 32
#define CRYPTO_BYTES 6336
#define CRYPTO_ALGNAME "faest_128f"

int crypto_sign_keypair(unsigned char* pk, unsigned char* sk);
int crypto_sign(unsigned char* sm, unsigned long long* smlen, const unsigned char* m,
                unsigned long long mlen, const unsigned char* sk);
int crypto_sign_open(unsigned char* m, unsigned long long* mlen, const unsigned char* sm,
                     unsigned long long smlen, const unsigned char* pk);

#endif

// vim: ft=c
