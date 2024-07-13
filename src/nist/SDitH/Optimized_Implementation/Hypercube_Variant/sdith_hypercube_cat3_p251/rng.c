#include "rng.h"

#include "param.h"
#include "sha3/KeccakHash.h"
#include "sha3/KeccakHashtimes4.h"
#include "types.h"
#include <stdlib.h>
#include <string.h>

EXPORT XOF_CTX *sdith_rng_create_xof_ctx(void *in, int inBytes) {
  Keccak_HashInstance *inst =
      (Keccak_HashInstance *)malloc(sizeof(Keccak_HashInstance));
#if defined(CAT_1)
  Keccak_HashInitialize_SHAKE128(inst);
#else
  Keccak_HashInitialize_SHAKE256(inst);
#endif
  Keccak_HashUpdate(inst, in, inBytes << 3);
  Keccak_HashFinal(inst, NULL);
  return (XOF_CTX *)inst;
}

EXPORT void sdith_rng_free_xof_ctx(XOF_CTX *ctx) { free(ctx); }

EXPORT void sdith_xof_next_bytes(XOF_CTX *ctx, void *out, int outLen) {
  Keccak_HashSqueeze((Keccak_HashInstance *)ctx, out, outLen << 3);
}

EXPORT void sdith_xof_next_bytes_mod251(XOF_CTX *ctx, void *out, int outLen) {
  // Roughly sample 1.03x of original length, which is greater than 256/251.
  int len = outLen + (outLen >> 5);
  uint8_t *buf = (uint8_t *)malloc(len);
  sdith_xof_next_bytes(ctx, buf, len);
  int bytes_remaining = len;
  uint8_t *buf_ptr = buf;
  uint8_t *out_buf = (uint8_t *)out;
  int counter = 0;
  while (counter < outLen) {
    if (*buf_ptr < 251) {
      out_buf[counter++] = *buf_ptr;
    }
    buf_ptr++;
    bytes_remaining--;
    if (bytes_remaining == 0) {
      sdith_xof_next_bytes(ctx, buf, len);
      bytes_remaining = len;
      buf_ptr = buf;
    }
  }
  free(buf);
}

static Keccak_HashInstancetimes4 xof4_ctx;

EXPORT XOF4_CTX *sdith_rng_create_xof4_ctx(void **in, int inBytes) {
#if defined(CAT_1)
  Keccak_HashInitializetimes4_SHAKE128(&xof4_ctx);
#else
  Keccak_HashInitializetimes4_SHAKE256(&xof4_ctx);
#endif
  Keccak_HashUpdatetimes4(&xof4_ctx, (const uint8_t **)in, inBytes << 3);
  Keccak_HashFinaltimes4(&xof4_ctx, NULL);
  return (XOF4_CTX *)&xof4_ctx;
}

EXPORT void sdith_rng_free_xof4_ctx(XOF4_CTX *ctx) { (void)ctx; }

EXPORT void sdith_xof4_next_bytes(XOF4_CTX *ctx, void **out, int outLen) {
  Keccak_HashSqueezetimes4((Keccak_HashInstancetimes4 *)ctx, (uint8_t **)out,
                           outLen << 3);
}

EXPORT void sdith_xof4_next_bytes_mod251(XOF4_CTX *ctx, void **out,
                                         int outLen) {
  // Roughly sample 1.03x of original length, which is greater than 256/251.
  int len = outLen + (outLen >> 5);
  uint8_t *buf = (uint8_t *)malloc(len * 4);
  uint8_t *bufs[4] = {&buf[0], &buf[len], &buf[len * 2], &buf[len * 3]};
  int bytes_counter[4] = {0};
  while (bytes_counter[0] + bytes_counter[1] + bytes_counter[2] +
             bytes_counter[3] <
         outLen * 4) {
    sdith_xof4_next_bytes(ctx, (void **)bufs, len);
    for (uint64_t i = 0; i < 4; ++i) {
      uint8_t *buf_ptr = bufs[i];
      uint8_t *out_buf = (uint8_t *)out[i];
      int bytes_remaining = len;
      while (bytes_counter[i] < outLen) {
        if (*buf_ptr < 251) {
          out_buf[bytes_counter[i]++] = *buf_ptr;
        }
        buf_ptr++;
        bytes_remaining--;
        if (bytes_remaining == 0) {
          break;
        }
      }
    }
  }
  free(buf);
}