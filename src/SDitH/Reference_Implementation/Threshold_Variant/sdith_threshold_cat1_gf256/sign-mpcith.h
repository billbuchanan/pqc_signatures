#ifndef SDITH_SIGN_THRESHOLD_NFPR_H
#define SDITH_SIGN_THRESHOLD_NFPR_H

#include <stdint.h>
#include <stddef.h>

int sdith_threshold_nfpr_sign(uint8_t* sig, size_t* siglen,
                const uint8_t* m, size_t mlen,
                const uint8_t* sk,
                const uint8_t* salt, const uint8_t* seed
                );
int sdith_threshold_nfpr_sign_verify(const uint8_t* sig, size_t siglen,
                const uint8_t* m, size_t mlen,
                const uint8_t* pk
                );
#define sdith_sign(sig, siglen, m, mlen, sk, salt, seed) sdith_threshold_nfpr_sign(sig, siglen, m, mlen, sk, salt, seed)
#define sdith_sign_verify(sig, siglen, m, mlen, pk) sdith_threshold_nfpr_sign_verify(sig, siglen, m, mlen, pk)

#endif /* SDITH_SIGN_THRESHOLD_NFPR_H */
