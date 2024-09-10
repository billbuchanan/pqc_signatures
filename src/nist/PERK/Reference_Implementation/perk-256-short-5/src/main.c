
/**
 * @file main.c
 * @brief NIST api test
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/random.h>
#include "api.h"
#include "parameters.h"
#include "rng.h"

static void init_randomness(void) {
    unsigned char seed[48] = {0};

#ifndef VERBOSE
    if (0 != getentropy(seed, sizeof(seed))) {
        printf("failed to get entropy for randombytes()\n");
        exit(1);
    }
#endif

    randombytes_init(seed, NULL, 256);
}

int main(void) {
    init_randomness();

    printf("\n");
    printf("*****************************\n");
    printf("**** %s-%d ****\n", CRYPTO_ALGNAME, SECURITY_BYTES * 8);
    printf("*****************************\n");

    printf("\n");
    printf("n: %d   ", PARAM_N1);
    printf("m: %d   ", PARAM_M);
    printf("Q: %d   ", PARAM_Q);
    printf("N: %d   ", PARAM_N);
    printf("tau: %d   ", PARAM_TAU);
    printf("Sec: %d bits   ", SECURITY_BYTES * 8);
    printf("Public key size: %d   ", CRYPTO_PUBLICKEYBYTES);
    printf("Private key size: %d   ", CRYPTO_SECRETKEYBYTES);
    printf("Signature size: %d   ", CRYPTO_BYTES);
    printf("\n");

    unsigned char pk[CRYPTO_PUBLICKEYBYTES];
    unsigned char sk[CRYPTO_SECRETKEYBYTES];

    unsigned char m[32] = {0};
    unsigned long long mlen = sizeof(m);

    unsigned long long smlen;
    unsigned char sm[sizeof(m) + CRYPTO_BYTES];

    int ret;

    crypto_sign_keypair(pk, sk);
    crypto_sign(sm, &smlen, m, sizeof(m), sk);
    ret = crypto_sign_open(m, &mlen, sm, smlen, pk);
    if (ret == 0) {
        printf("\nDone");
    } else {
        printf("\nFailed");
    }

    printf("\n\n");

    return ret;
}
