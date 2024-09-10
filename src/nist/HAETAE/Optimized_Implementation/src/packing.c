#include "packing.h"
#include "encoding.h"
#include "params.h"
#include "poly.h"
#include "polymat.h"
#include "polyvec.h"
#include <string.h>

/*************************************************
 * Name:        pack_pk
 *
 * Description: Bit-pack public key pk = (seed, b).
 *
 * Arguments:   - uint8_t pk[]: output byte array
 *              - const polyveck *b: polynomial vector of length K containg b
 *              - const uint8_t seed[]: seed for A'
 **************************************************/
void pack_pk(uint8_t pk[CRYPTO_PUBLICKEYBYTES], polyveck *b,
             const uint8_t seed[SEEDBYTES]) {
    unsigned int i;

    memcpy(pk, seed, SEEDBYTES);

    pk += SEEDBYTES;
    for (i = 0; i < K; ++i) {
#if D == 0
        poly_nttunpack(&b->vec[i]);
#endif
        polyq_pack(pk + i * POLYQ_PACKEDBYTES, &b->vec[i]);
    }
}

/*************************************************
 * Name:        unpack_pk
 *
 * Description: Unpack public key pk = (seed, b).
 *
 * Arguments:   - uint8_t seed[]: seed for A'
 *              - polyveck *b: polynomial vector of length K containg b
 *              - const uint8_t pk[]: output byte array
 **************************************************/
void unpack_pk(polyveck *b, uint8_t seed[SEEDBYTES],
               const uint8_t pk[CRYPTO_PUBLICKEYBYTES]) {
    unsigned int i;

    memcpy(seed, pk, SEEDBYTES);

    pk += SEEDBYTES;
    for (i = 0; i < K; ++i) {
        polyq_unpack(&b->vec[i], pk + i * POLYQ_PACKEDBYTES);
#if D == 0
        poly_nttunpack(&b->vec[i]);
#endif
    }
}

/*************************************************
 * Name:        pack_sk
 *
 * Description: Bit-pack secret key sk = (pk, s).
 *
 * Arguments:   - uint8_t sk[]: output byte array
 *              - const uint8_t pk[PUBLICKEYBYTES]: packed pk
 *              - const polyvecl *s0: polyvecl pointer containing s0 (encoding
 *starting at offset 1)
 *              - const polyveck *s1: polyveck pointer containing s1
 **************************************************/
void pack_sk(uint8_t sk[CRYPTO_SECRETKEYBYTES],
             const uint8_t pk[CRYPTO_PUBLICKEYBYTES], const polyvecm *s0,
             const polyveck *s1, const uint8_t key[SEEDBYTES]) {
    unsigned int i;
    memcpy(sk, pk, CRYPTO_PUBLICKEYBYTES);

    sk += CRYPTO_PUBLICKEYBYTES;
    for (i = 0; i < M; ++i)
        polyeta_pack(sk + i * POLYETA_PACKEDBYTES, &s0->vec[i]);

    sk += (L - 1) * POLYETA_PACKEDBYTES;
#if D == 1
    for (i = 0; i < K; ++i)
        poly2eta_pack(sk + i * POLY2ETA_PACKEDBYTES, &s1->vec[i]);
    sk += K * POLY2ETA_PACKEDBYTES;
#elif D == 0
    for (i = 0; i < K; ++i)
        polyeta_pack(sk + i * POLYETA_PACKEDBYTES, &s1->vec[i]);
    sk += K * POLYETA_PACKEDBYTES;
#else
#error "Not yet implemented."
#endif
    memcpy(sk, key, SEEDBYTES);
}

/*************************************************
 * Name:        unpack_sk
 *
 * Description: Unpack secret key sk = (A, s).
 *
 * Arguments:   - polyvecl A[K]: output polyvecl array for A
 *              - polyvecl s0: output polyvecl pointer for s0
 *              - polyveck s1: output polyveck pointer for s1
 *              - const uint8_t sk[]: byte array containing bit-packed sk
 **************************************************/
void unpack_sk(polyvecl A[K], polyvecm *s0, polyveck *s1, uint8_t *key,
               const uint8_t sk[CRYPTO_SECRETKEYBYTES]) {
    unsigned int i;
    uint8_t rhoprime[SEEDBYTES];
    polyveck b1;
#if D > 0
    polyveck a;
#endif

    unpack_pk(&b1, rhoprime, sk);

    sk += CRYPTO_PUBLICKEYBYTES;
    for (i = 0; i < M; ++i)
        polyeta_unpack(&s0->vec[i], sk + i * POLYETA_PACKEDBYTES);

    sk += M * POLYETA_PACKEDBYTES;
#if D == 1
    for (i = 0; i < K; ++i)
        poly2eta_unpack(&s1->vec[i], sk + i * POLY2ETA_PACKEDBYTES);

    sk += K * POLY2ETA_PACKEDBYTES;
#elif D == 0
    for (i = 0; i < K; ++i)
        polyeta_unpack(&s1->vec[i], sk + i * POLYETA_PACKEDBYTES);

    sk += K * POLYETA_PACKEDBYTES;
#else
#error "Not yet implemented."
#endif
    memcpy(key, sk, SEEDBYTES);

    // A' = PRG(rhoprime)
    polymatkl_expand(A, rhoprime);
    polymatkl_double(A);
#if D > 0
    polyveck_expand(&a, rhoprime);
#endif

#if D == 1
    // first column of A = 2(a-b1*2^d)
    polyveck_double(&b1);
    polyveck_sub(&b1, &a, &b1);
    polyveck_double(&b1);
    polyveck_ntt(&b1);
#elif D == 0
#else
#error "Not yet implemented."
#endif
    // append b into A
    for (i = 0; i < K; ++i) {
        A[i].vec[0] = b1.vec[i];
    }
}

/*************************************************
 * Name:        pack_sig
 *
 * Description: Bit-pack signature sig = (c, LB(z1), len(x), x= Enc(HB(z1)),
 *Enc(h)).
 *
 * Arguments:   - uint8_t sig[]: output byte array
 *              - const uint8_t c_seed[]: pointer to seed of challenge
 *              - const polyvecl *lowbits_z1: pointer to vector LowBits(z1) of
 *length L
 *              - const polyvecl *highbits_z1: pointer to vector HighBits(z1) of
 *length L
 *              - const polyveck *h: pointer t vector h of length K
 * Returns 1 in case the signature size is above the threshold; otherwise 0.
 **************************************************/
int pack_sig(uint8_t sig[CRYPTO_BYTES], const uint8_t c_seed[SEEDBYTES],
             const polyvecl *lowbits_z1, const polyvecl *highbits_z1,
             const polyveck *h) {

    uint8_t encoded_h[MAX_ENC_H_BYTES];
    uint8_t encoded_hb_z1[MAX_ENC_HB_Z1_BYTES];
    uint8_t *begin_enc_h,
        *begin_enc_hb_z1; // encoding populates "encoded_h" from right to left
    uint16_t size_enc_h, size_enc_hb_z1;
    unsigned int i;

    memcpy(sig, c_seed, SEEDBYTES);
    sig += SEEDBYTES;

    for (i = 0; i < L; ++i)
        poly_decomposed_pack(sig + N * i, &lowbits_z1->vec[i]);
    sig += L * N;

    begin_enc_hb_z1 =
        encode_hb_z1(encoded_hb_z1, &highbits_z1->vec[0].coeffs[0]);
    size_enc_hb_z1 =
        (uint16_t)(encoded_hb_z1 + MAX_ENC_HB_Z1_BYTES - begin_enc_hb_z1);

    begin_enc_h = encode_h(encoded_h, &h->vec[0].coeffs[0]);
    size_enc_h = (uint16_t)(encoded_h + MAX_ENC_H_BYTES - begin_enc_h);

    if (SEEDBYTES + L * N + 2 + size_enc_hb_z1 + size_enc_h >
        CRYPTO_BYTES) // signature too big
        return 1;

    sig[1] = (uint8_t)(size_enc_hb_z1 >> 8);
    sig[0] = (uint8_t)(size_enc_hb_z1 & 0xFF);
    sig += 2;

    memcpy(sig, begin_enc_hb_z1, size_enc_hb_z1);
    sig += size_enc_hb_z1;

    memcpy(sig, begin_enc_h, size_enc_h);
    sig += size_enc_h;

    return 0;
}

/*************************************************
 * Name:        unpack_sig
 *
 * Description: Unpack signature sig = (c, LB(z1), len(x), x= Enc(HB(z1)),
 *Enc(h)).
 *
 * Arguments:   - uint8_t c_seed[]: pointer to output seed of challenge
 *              - polyvecl *lowbits_z1: pointer to output vector LowBits(z1)
 *              - polyvecl *highbits_z1: pointer to output vector HighBits(z1)
 *              - polyveck *h: pointer to output vector h
 *              - const uint8_t sig[]: byte array containing
 *                bit-packed signature
 *
 * Returns 1 in case of malformed signature; otherwise 0.
 **************************************************/
int unpack_sig(uint8_t c_seed[SEEDBYTES], polyvecl *lowbits_z1,
               polyvecl *highbits_z1, polyveck *h,
               const uint8_t sig[CRYPTO_BYTES]) {

    uint8_t encoded_h[MAX_ENC_H_BYTES]; // MAX_ENC_H_BYTES is the upper bound
                                        // for the size of an encoded h.
    uint8_t encoded_hb_z1[MAX_ENC_HB_Z1_BYTES]; // MAX_ENC_HB_Z1_BYTES is the
                                                // upper bound for the size of
                                                // an encoded HB(z1).
    unsigned int i;
    uint16_t size_enc_hb_z1, size_enc_h;

    memcpy(c_seed, sig, SEEDBYTES);
    sig += SEEDBYTES;

    for (i = 0; i < L; ++i)
        poly_decomposed_unpack(&lowbits_z1->vec[i], sig + N * i);
    sig += L * N;

    size_enc_hb_z1 = ((uint16_t)sig[1]) << 8 | sig[0];
    sig += 2;
    size_enc_h = CRYPTO_BYTES - SEEDBYTES - L * N - 2 - size_enc_hb_z1;

    memcpy(encoded_hb_z1, sig, size_enc_hb_z1);
    decode_hb_z1(&highbits_z1->vec[0].coeffs[0], encoded_hb_z1);
    sig += size_enc_hb_z1;

    memcpy(encoded_h, sig, size_enc_h);
    decode_h(&h->vec[0].coeffs[0], encoded_h);

    return 0;
}
