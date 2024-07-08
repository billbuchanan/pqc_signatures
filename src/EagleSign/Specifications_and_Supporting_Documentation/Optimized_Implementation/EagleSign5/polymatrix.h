/*
 * Implementors: EagleSign Team
 * This implementation is highly inspired from Dilithium and
 * Falcon Signatures' implementations
 */

#ifndef POLYMATRIX_H
#define POLYMATRIX_H

#include <stdint.h>
#include "params.h"
#include "polyvec.h"
#include "poly.h"

#define polyvec_matrix_expand EAGLESIGN_NAMESPACE(polyvec_matrix_expand)
void polyvec_matrix_expand(polyvecl mat[K], const uint8_t rho[SEEDBYTES]);

#define polyvec_matrix_pointwise_montgomery EAGLESIGN_NAMESPACE(polyvec_matrix_pointwise_montgomery)
void polyvec_matrix_pointwise_montgomery(polyveck *t, const polyvecl mat[K], const polyvecl *v);

#define polyvec_matrix_pointwise_montgomery_l_l EAGLESIGN_NAMESPACE(polyvec_matrix_pointwise_montgomery_l_l)
void polyvec_matrix_pointwise_montgomery_l_l(polyvecl *t, const polyvecl mat[L], const polyvecl *v);

#define polyvec_matrix_pointwise_product EAGLESIGN_NAMESPACE(polyvec_matrix_pointwise_product)
void polyvec_matrix_pointwise_product(polyveck c[L], const polyvecl a[K], const polyvecl b[L]);

#define polyvec_matrix_pointwise_product_l_l EAGLESIGN_NAMESPACE(polyvec_matrix_pointwise_product_l_l)
void polyvec_matrix_pointwise_product_l_l(polyvecl c[L], const polyvecl a[L], const polyvecl b[L]);

#define polyvec_matrix_reformat EAGLESIGN_NAMESPACE(polyvec_matrix_reformat)
void polyvec_matrix_reformat(polyvecl b[K], const polyveck a[L]);

#define polyvec_matrix_reformat_l_l EAGLESIGN_NAMESPACE(polyvec_matrix_reformat_l_l)
void polyvec_matrix_reformat_l_l(polyvecl b[L], const polyvecl a[L]);

#define polymatrix_l_expand EAGLESIGN_NAMESPACE(polymatrix_l_expand)
void polymatrix_l_expand(polyvecl v[L], const uint8_t seed[CRHBYTES]);

#define polymatrix_k_l_expand EAGLESIGN_NAMESPACE(polymatrix_k_l_expand)
void polymatrix_k_l_expand(polyvecl v[K], const uint8_t seed[CRHBYTES]);

#define polymatrix_ntt_k_l EAGLESIGN_NAMESPACE(polymatrix_ntt_k_l)
void polymatrix_ntt_k_l(polyvecl a[K]);

#define polymatrix_ntt_l_l EAGLESIGN_NAMESPACE(polymatrix_ntt_l_l)
void polymatrix_ntt_l_l(polyvecl a[L]);

#define polymatrix_invntt_tomont_k_l EAGLESIGN_NAMESPACE(polymatrix_invntt_tomont_k_l)
void polymatrix_invntt_tomont_k_l(polyvecl a[K]);

#define polymatrix_invntt_tomont_l_l EAGLESIGN_NAMESPACE(polymatrix_invntt_tomont_l_l)
void polymatrix_invntt_tomont_l_l(polyvecl a[L]);

#define polymatrix_l_is_invertible EAGLESIGN_NAMESPACE(polymatrix_l_is_invertible)
int polymatrix_l_is_invertible(poly *d, const polyvecl v[L]);

#define polymatrix_l_expand_invertible EAGLESIGN_NAMESPACE(polymatrix_l_expand_invertible)
int polymatrix_l_expand_invertible(polyvecl v[L], polyvecl f[L], const uint8_t seed[CRHBYTES]);

#define polymatrix_l_inverse EAGLESIGN_NAMESPACE(polymatrix_l_inverse)
int polymatrix_l_inverse(polyvecl v[L], const polyvecl u[L]);

#define polyvec_matrix_pointwise_add EAGLESIGN_NAMESPACE(polyvec_matrix_pointwise_add)
void polyvec_matrix_pointwise_add(polyvecl c[K], const polyvecl a[K], const polyvecl b[K]);

#endif
