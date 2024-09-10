#include "api.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "keygen.h"
#include "params.h"
#include "sign.h"
#include "types.h"
#include "util/bitstream.h"
#include "util/compress.h"
#include "util/tritstream.h"
#include "verify.h"

int crypto_sign_keypair(unsigned char *pk, unsigned char *sk) {
  wave_sk_t wsk;
  wave_pk_t wpk;

  keygen(&wsk, &wpk);

  // export pk to byte array
  {
    memset(pk, 0, CRYPTO_PUBLICKEYBYTES);

    tritstream_t ts;
    ts_init(&ts, pk, CRYPTO_PUBLICKEYBYTES);

    for (int row = 0; row < wpk.R->n_row; row++)
      for (int col = 0; col < wpk.R->n_col; col++)
        ts_write(&ts, vf3_get_element(col, &wpk.R->rows[row]));
  }

  // export sk to byte array
  {
    memset(sk, 0, CRYPTO_SECRETKEYBYTES);

    bitstream_t stream;
    bs_init(&stream, sk, CRYPTO_SECRETKEYBYTES);

    for (int col = 0; col < MK_SIZE; col++) bs_write(&stream, wsk.mk[col], 8);

    for (int i = 0; i < N; i++) bs_write(&stream, wsk.perm[i], 16);

    uint32_t pos = stream.byte_pos + (stream.bit_pos > 0 ? 1 : 0);

    tritstream_t ts;
    ts_init(&ts, &sk[pos], CRYPTO_SECRETKEYBYTES - pos);

    for (int i = 0; i < N2; i++) ts_write(&ts, vf3_get_element(i, wsk.b));

    for (int i = 0; i < N2; i++) ts_write(&ts, vf3_get_element(i, wsk.c));
  }

  wave_sk_free(&wsk);
  wave_pk_free(&wpk);

  return 0;
}

int crypto_sign(unsigned char *sm, unsigned long long *smlen,
                const unsigned char *m, unsigned long long mlen,
                const unsigned char *sk) {
  while (0 == 0) {
    wave_sk_t wsk;

    wave_sk_alloc(&wsk);

    // import sk from byte array
    {
      bitstream_t stream;
      bs_init(&stream, (uint8_t *)sk, CRYPTO_SECRETKEYBYTES);

      for (int col = 0; col < MK_SIZE; col++) wsk.mk[col] = bs_read(&stream, 8);

      for (int i = 0; i < N; i++) wsk.perm[i] = bs_read(&stream, 16);

      uint32_t pos = stream.byte_pos + (stream.bit_pos > 0 ? 1 : 0);

      tritstream_t ts;
      ts_init(&ts, (uint8_t *)&sk[pos], CRYPTO_SECRETKEYBYTES - pos);

      for (int i = 0; i < N2; i++) vf3_set_coeff(i, wsk.b, ts_read(&ts));

      for (int i = 0; i < N2; i++) vf3_set_coeff(i, wsk.c, ts_read(&ts));
    }

    vf3_e *sig = vf3_alloc(K);

    uint8_t salt[SALT_SIZE] = {0};

    sign(sig, salt, m, mlen, &wsk);

    memset(sm, 0, CRYPTO_BYTES);

    bitstream_t bs;
    bs_init(&bs, sm, CRYPTO_BYTES);

    if (compress(&bs, sig) != 0) {
      // Signature size exceeds maximum; redo.

      vf3_free(sig);
      wave_sk_free(&wsk);

      continue;
    }

    *smlen = bs.byte_pos + (bs.bit_pos > 0 ? 1 : 0);

    memcpy(&sm[*smlen], salt, SALT_SIZE);
    *smlen += SALT_SIZE;

    memcpy(&sm[*smlen], m, mlen);
    *smlen += mlen;

    vf3_free(sig);
    wave_sk_free(&wsk);

    return 0;
  }
}

int crypto_sign_open(unsigned char *m, unsigned long long *mlen,
                     const unsigned char *sm, unsigned long long smlen,
                     const unsigned char *pk) {
  wave_pk_t wpk;

  wave_pk_alloc(&wpk);

  // import pk to byte array
  {
    tritstream_t ts;
    ts_init(&ts, (uint8_t *)pk, CRYPTO_PUBLICKEYBYTES);

    for (int row = 0; row < wpk.R->n_row; row++)
      for (int col = 0; col < wpk.R->n_col; col++)
        vf3_set_coeff(col, &wpk.R->rows[row], ts_read(&ts));
  }

  vf3_e *signature = vf3_alloc(K);

  bitstream_t bs;
  bs_init(&bs, (uint8_t *)sm, smlen);

  decompress(&bs, signature);

  uint32_t sig_len = bs.byte_pos + (bs.bit_pos > 0 ? 1 : 0);

  uint8_t salt[SALT_SIZE];

  memcpy(salt, &sm[sig_len], SALT_SIZE);

  size_t smlen_tmp = sig_len;
  size_t mlen_tmp = smlen - sig_len - SALT_SIZE;

  if (verify(signature, &smlen_tmp, salt, &sm[sig_len + SALT_SIZE], mlen_tmp,
             &wpk) == 1) {
    *mlen = smlen - sig_len - SALT_SIZE;

    memcpy(m, &sm[sig_len + SALT_SIZE], *mlen);

    vf3_free(signature);
    wave_pk_free(&wpk);

    return 0;
  }

  vf3_free(signature);
  wave_pk_free(&wpk);

  return -1;
}
