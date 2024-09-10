#include "api.h"
#include "sign.h"

int kat_crypto_sign_keypair(uint8_t *pk, uint8_t *sk) {
    return crypto_sign_keypair(pk, sk);
}

int kat_crypto_sign(uint8_t *sm, size_t *smlen, const uint8_t *m, size_t mlen,
                    const uint8_t *sk) {
    return crypto_sign_sign(sm, smlen, m, mlen, sk);
}

int kat_crypto_sign_open(uint8_t *m, size_t *mlen, const uint8_t *sm,
                         size_t smlen, const uint8_t *pk) {
    return crypto_sign_open(m, mlen, sm, smlen, pk);
}