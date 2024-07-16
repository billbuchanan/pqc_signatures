#ifndef SIGN_H
#define SIGN_H

#include <stdint.h>
#include <stddef.h>
#include "params.h"
// NOTE: implicit type cast from long long ptr to size_t: This will cause problems on RISCV at least, and likely on any 32 bit architecture, explicit cast needed

#define crypto_sign_keypair FULEECA_NAMESPACE(_keypair)
int crypto_sign_keypair(uint8_t *pk, uint8_t *sk);

#define crypto_sign_signature FULEECA_NAMESPACE(_signature)
int crypto_sign_signature(uint8_t *sig, size_t *siglen,
                          const uint8_t *m, size_t mlen,
                          const uint8_t *sk);

#define crypto_sign FULEECA_NAMESPACE()
int crypto_sign(uint8_t *sm, size_t *smlen,
                const uint8_t *m, size_t mlen,
                const uint8_t *sk);

#define crypto_sign_verify FULEECA_NAMESPACE(_verify)
int crypto_sign_verify(const uint8_t *sig, size_t siglen,
                       const uint8_t *m, size_t mlen,
                       const uint8_t *pk);

#define crypto_sign_open FULEECA_NAMESPACE(_open)
int crypto_sign_open(uint8_t *m, size_t *mlen,
                     const uint8_t *sm, size_t smlen,
                     const uint8_t *pk);

#endif
