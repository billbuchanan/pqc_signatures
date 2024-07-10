// SPDX-License-Identifier: Apache-2.0

#include "api.h"
#include <mayo.h>

int
crypto_sign_keypair(unsigned char *pk, unsigned char *sk) {
    return mayo_keypair(&MAYO_1, pk, sk);
}

int
crypto_sign(unsigned char *sm, unsigned long long *smlen,
            const unsigned char *m, unsigned long long mlen,
            const unsigned char *sk) {
    return mayo_sign(&MAYO_1, sm, smlen, m, mlen, sk);
}

int
crypto_sign_open(unsigned char *m, unsigned long long *mlen,
                 const unsigned char *sm, unsigned long long smlen,
                 const unsigned char *pk) {
    return mayo_open(&MAYO_1, m, mlen, sm, smlen, pk);
}
