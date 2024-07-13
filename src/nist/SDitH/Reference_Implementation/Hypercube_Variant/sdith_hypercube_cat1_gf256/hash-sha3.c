#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>

#include "param.h"
#include "rng.h"
#include "sha3/KeccakHash.h"
#include "sha3/KeccakHashtimes4.h"

HASH_CTX *sdith_hash_create_hash_ctx(uint8_t prefix) {
  Keccak_HashInstance *inst =
      (Keccak_HashInstance *)malloc(sizeof(Keccak_HashInstance));
#if defined(CAT_1)
  Keccak_HashInitialize_SHA3_256(inst);
#elif defined(CAT_3)
  Keccak_HashInitialize_SHA3_384(inst);
#else
  Keccak_HashInitialize_SHA3_512(inst);
#endif
  Keccak_HashUpdate(inst, &prefix, sizeof(uint8_t) << 3);
  return (HASH_CTX *)inst;
}

void sdith_hash_free_hash_ctx(HASH_CTX *ctx) { free(ctx); }

void sdith_hash_digest_update(HASH_CTX *ctx, void const *in, int inBytes) {
  Keccak_HashUpdate((Keccak_HashInstance *)ctx, (uint8_t const *)in,
                    inBytes << 3);
}

void sdith_hash_final(HASH_CTX *ctx, void *dest) {
  Keccak_HashFinal((Keccak_HashInstance *)ctx, dest);
}

void sdith_hash(uint8_t prefix, void *dest, void const *data, int dataBytes) {
  HASH_CTX *ctx = sdith_hash_create_hash_ctx(prefix);
  sdith_hash_digest_update(ctx, data, dataBytes);
  sdith_hash_final(ctx, dest);
  sdith_hash_free_hash_ctx(ctx);
}

static Keccak_HashInstancetimes4 hash4_ctx;

HASH4_CTX *sdith_hash_create_hash4_ctx(uint8_t prefix) {
#if defined(CAT_1)
  Keccak_HashInitializetimes4_SHA3_256(&hash4_ctx);
#elif defined(CAT_3)
  Keccak_HashInitializetimes4_SHA3_384(&hash4_ctx);
#else
  Keccak_HashInitializetimes4_SHA3_512(&hash4_ctx);
#endif
  const uint8_t *prefix_ptr[4] = {&prefix, &prefix, &prefix, &prefix};
  Keccak_HashUpdatetimes4(&hash4_ctx, prefix_ptr, sizeof(prefix) << 3);
  return (HASH4_CTX *)&hash4_ctx;
}

void sdith_hash_free_hash4_ctx(HASH4_CTX *ctx) { (void)ctx; }

void sdith_hash4_digest_update(HASH4_CTX *ctx, void **in, int inBytes) {
  Keccak_HashUpdatetimes4((Keccak_HashInstancetimes4 *)ctx,
                          (const uint8_t **)in, inBytes << 3);
}

void sdith_hash4_final(HASH4_CTX *ctx, void **dest) {
  Keccak_HashFinaltimes4((Keccak_HashInstancetimes4 *)ctx, (uint8_t **)dest);
}

void sdith_hash4(uint8_t prefix, void **dest, void **data, int dataBytes) {
  HASH4_CTX *ctx = sdith_hash_create_hash4_ctx(prefix);
  sdith_hash4_digest_update(ctx, data, dataBytes);
  sdith_hash4_final(ctx, dest);
  sdith_hash_free_hash4_ctx(ctx);
}
