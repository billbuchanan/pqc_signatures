#include "decompose.h"
#include "fips202.h"
#include "packing.h"
#include "polyvec.h"
#include "randombytes.h"
#include "reduce.h"
#include "sign.h"
#include <stdio.h>
#include <string.h>

#define BUF_BYTES (1)
// #define Iterations (1000000)
#define Iterations (100)

uint8_t print_flag = 1;

int sign_verify_test(void);

int main(void) {
    printf("HAETAE mode = %d\n", HAETAE_MODE);
    size_t count = 1;
    for (int i = 0; i < Iterations; ++i) {
        if (sign_verify_test()) {
            printf("Invalid on %d-th test\n", i);
            break;
        }

        if (!(i % (Iterations / 10))) {
            printf("...%lu%%", count * 10);
            fflush(stdout);
            ++count;
        }
    }
    printf("\n");

    return 0;
}

int sign_verify_test(void) {
    if (print_flag) {
        printf(">> sign and verify test\n");
        print_flag = 0;
    }

    uint8_t pk[CRYPTO_PUBLICKEYBYTES] = {0};
    uint8_t sk[CRYPTO_SECRETKEYBYTES] = {0};

    crypto_sign_keypair(pk, sk);

    size_t siglen = 0;
    uint8_t sig[CRYPTO_BYTES] = {0};
    uint8_t msg[SEEDBYTES] = {0};
    randombytes(msg, SEEDBYTES);
    crypto_sign_signature(sig, &siglen, msg, SEEDBYTES, sk);

    if (crypto_sign_verify(sig, siglen, msg, SEEDBYTES, pk)) {
        return 1;
    }

    return 0;
}
