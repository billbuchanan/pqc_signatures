#ifndef SERIALIZER_H
#define SERIALIZER_H

#include <m4ri/m4ri.h>
#include <flint/flint.h>
#include <flint/fq_poly.h>

void char2bits2(unsigned char c, unsigned char * bits);
void uchar_to_mzd(const unsigned char *vec, mzd_t *mzd, int len);
void mzd_to_uchar(const mzd_t *mzd, unsigned char *vec, int len);
void GetRow(mzd_t *row, const mzd_t *mat, int nrow, int len);
void SetRow(mzd_t *mat, const mzd_t *row, int nrow, int len);
void EncodePublicKey(const mzd_t *pub_key, unsigned char *pk);
void DecodePublicKey(mzd_t *pub_key, const unsigned char *pk);
void EncodePrivateKey(unsigned char *sk, const mzd_t *inv_T, const mzd_t *inv_S, const mzd_t *inv_L1, fq_poly_t private_polynomial, fq_ctx_t ctx);
void DecodePrivateKey(const unsigned char *sk, mzd_t *inv_T, mzd_t *inv_S, mzd_t *inv_L1, mzd_t *private_polynomial);

#endif