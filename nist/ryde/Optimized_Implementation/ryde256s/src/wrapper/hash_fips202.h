
#ifndef HASH_H
#define HASH_H

#include "KeccakHash.h"
#include "SimpleFIPS202.h"

typedef Keccak_HashInstance hash_sha3_ctx;

static inline void hash_sha3_init(hash_sha3_ctx *ctx) {
    Keccak_HashInitialize_SHA3_512(ctx);
}

static inline void hash_sha3_absorb(hash_sha3_ctx *ctx, const uint8_t *input, size_t size) {
    Keccak_HashUpdate(ctx, input, size << 3);
}

static inline void hash_sha3_finalize(uint8_t *output, hash_sha3_ctx *ctx) {
    Keccak_HashFinal(ctx, output);
}

static inline void hash_shake(uint8_t *output, size_t output_size, const uint8_t * input, size_t input_size) {
    SHAKE256(output, output_size, input, input_size);
}



#if defined(SHAKE_TIMES4)

#include "KeccakHashtimes4.h"

typedef Keccak_HashInstancetimes4 hash_sha3_x4_ctx;

static inline void hash_sha3_x4_init(hash_sha3_x4_ctx *ctx) {
  Keccak_HashInitializetimes4_SHA3_512(ctx);
}

static inline void hash_sha3_x4_absorb(hash_sha3_x4_ctx *ctx, const uint8_t **input, size_t size) {
  Keccak_HashUpdatetimes4(ctx, input, size << 3);
}

static inline void hash_sha3_x4_finalize(uint8_t **output, hash_sha3_x4_ctx *ctx) {
  Keccak_HashFinaltimes4(ctx, output);
}

#endif

#endif
