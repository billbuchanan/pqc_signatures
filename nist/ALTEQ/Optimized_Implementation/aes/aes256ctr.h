#ifndef AES256CTR_H
#define AES256CTR_H

/* This code has been directly extracted from the dilithium source code, which main submission was recently standardized by the NIST */
/* Note, that -maes should be enabled for machines with aesni present */

#include <stddef.h>
#include <stdint.h>
#include <immintrin.h>

typedef struct {
  __m128i rkeys[16];
  __m128i n;
} aes256ctr_ctx;

/* use this to initialize some seedexpander */
void aes256ctr_init(aes256ctr_ctx *state,
                    const uint8_t key[32],
                    uint64_t nonce);

/* 512-bits blocks seedexpander (it gives 64 characters) */
void aes256ctr_squeezeblocks(uint8_t *out,
                             size_t nblocks,
                             aes256ctr_ctx *state);

#endif
