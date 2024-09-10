/******************************************************************************
 * Main interface according to the NIST submission template
 ******************************************************************************
 * Copyright (C) 2023  The VOX team
 * License : MIT (see COPYING file for the full text)
 * Author  : Robin Larrieu
 *****************************************************************************/

#include "api.h"
#include "vox.h"
#include "fips202.h"
#include "rng.h"
#include <string.h>

#if  (VOX_SECRETKEYBYTES != CRYPTO_SECRETKEYBYTES)
#error "Inconsistent values for SECRETKEYBYTES"
#endif
#if  (VOX_PUBLICKEYBYTES != CRYPTO_PUBLICKEYBYTES)
#error "Inconsistent values for PUBLICKEYBYTES"
#endif
#if  (VOX_SIG_BYTES != CRYPTO_BYTES)
#error "Inconsistent values for SIG_BYTES"
#endif

int crypto_sign_keypair(unsigned char *pk, unsigned char *sk)
{
    nmod_mat_t    S;
    fq_nmod_mat_t T, Pub[VOX_O], Sec[VOX_O];
    unsigned char seed_PK[VOX_SEED_BYTES];
    unsigned char seed_SK[VOX_SEED_BYTES];
    int i;

    InitContext();
    for (i = 0; i < VOX_O; i++) {
        fq_nmod_mat_init(Pub[i], VOX_NC, VOX_NC, Fqc_ctx);
        fq_nmod_mat_init(Sec[i], VOX_NC, VOX_NC, Fqc_ctx);
    }
    fq_nmod_mat_init(T, VOX_NC, VOX_NC, Fqc_ctx);
    nmod_mat_init(S,  VOX_O,  VOX_O, VOX_Q);

    randombytes(seed_PK, VOX_SEED_BYTES);
    randombytes(seed_SK, VOX_SEED_BYTES);

    ComputePetzoldQR(Pub, S, T, seed_PK, seed_SK);
    fq_nmod_mat_ComposeSTKey(Sec, Pub, S, T);

    /* Encode public key */
    encode_PK(pk, Pub, seed_PK);

    /* Encode secret key (store seed_PK so that we can recover the public key
     * from the private key) */
    memcpy(sk, seed_PK, VOX_SEED_BYTES); sk += VOX_SEED_BYTES;
    memcpy(sk, seed_SK, VOX_SEED_BYTES); sk += VOX_SEED_BYTES;
    shake256(sk, VOX_HPK_BYTES, pk, VOX_PUBLICKEYBYTES); sk += VOX_HPK_BYTES;
    encode_SK(sk, Sec);

    for (i = 0; i < VOX_O; i++) {
        fq_nmod_mat_clear(Pub[i], Fqc_ctx);
        fq_nmod_mat_clear(Sec[i], Fqc_ctx);
    }
    fq_nmod_mat_clear(T, Fqc_ctx);
    nmod_mat_clear(S);
    ClearContext();
    return 0;
}

int crypto_sign(unsigned char *sm, unsigned long long *smlen,
                const unsigned char *m, unsigned long long mlen,
                const unsigned char *sk)
{
    fq_nmod_mat_t T2;
    nmod_mat_t S, T, Sec[VOX_O];
    nmod_mat_t Message, Signature;
    unsigned char seed_V[VOX_SEED_BYTES];
    int i;

    InitContext();
    fq_nmod_mat_init(T2, VOX_NC, VOX_NC, Fqc_ctx);
    for (i = 0; i < VOX_O; i++) {
        nmod_mat_init(Sec[i], VOX_N, VOX_N, VOX_Q);
    }
    nmod_mat_init(T, VOX_N, VOX_N, VOX_Q);
    nmod_mat_init(S, VOX_O, VOX_O, VOX_Q);
    nmod_mat_init(Signature, 1, VOX_N, VOX_Q);
    nmod_mat_init(Message, 1, VOX_O, VOX_Q);

    /* Parse secret key */
    const unsigned char *seed_SK = sk + VOX_SEED_BYTES;
    const unsigned char *hpk = sk + 2*VOX_SEED_BYTES;
    sk += 2*VOX_SEED_BYTES + VOX_HPK_BYTES;
    VOX_GenS(S, seed_SK);
    VOX_GenT(T2, seed_SK);
    ExpandMat(T, T2, base1);
    decode_SK_expand(Sec, sk);

    /* Generate seed for the vinegar variables */
#ifdef DETERMINISTIC_SIGNING
    keccak_state hash_ctx;
    shake256_init(&hash_ctx);
    shake256_absorb(&hash_ctx, seed_SK, VOX_SEED_BYTES);
    shake256_absorb(&hash_ctx, m, mlen);
    shake256_finalize(&hash_ctx);
    shake256_squeeze(seed_V, VOX_SEED_BYTES, &hash_ctx);
#else
    randombytes(seed_V, VOX_SEED_BYTES);
#endif

    /* Compute signature */
    VOX_GenM(Message, hpk, m, (size_t) mlen);
    Sign(Signature, Message, Sec, S, T, seed_V);

    /* sm and m are allowed to overlap -> copy message then signature */
    memmove(sm+CRYPTO_BYTES, m, (size_t) mlen);
    encode_sig(sm, Signature);
    *smlen = mlen + CRYPTO_BYTES;

    fq_nmod_mat_clear(T2, Fqc_ctx);
    for (i = 0; i < VOX_O; i++) {
        nmod_mat_clear(Sec[i]);
    }
    nmod_mat_clear(T);
    nmod_mat_clear(S);
    nmod_mat_clear(Signature);
    nmod_mat_clear(Message);
    ClearContext();
    return 0;
}

int crypto_sign_open(unsigned char *m, unsigned long long *mlen,
                     const unsigned char *sm, unsigned long long smlen,
                     const unsigned char *pk)
{
    nmod_mat_t Pub[VOX_O];
    nmod_mat_t Message, Signature;
    unsigned char hpk[VOX_HPK_BYTES];
    int i, res;

    if (smlen < CRYPTO_BYTES)
        return -1;
    unsigned long long mlen1 = smlen - CRYPTO_BYTES;

    InitContext();
    for (i = 0; i < VOX_O; i++) {
        nmod_mat_init(Pub[i], VOX_N, VOX_N, VOX_Q);
    }
    nmod_mat_init(Signature, 1, VOX_N, VOX_Q);
    nmod_mat_init(Message, 1, VOX_O, VOX_Q);

    decode_PK_expand(Pub, pk);
    shake256(hpk, VOX_HPK_BYTES, pk, VOX_PUBLICKEYBYTES);
    VOX_GenM(Message, hpk, sm + CRYPTO_BYTES, (size_t) mlen1);
    decode_sig(Signature, sm);

    if (Verify(Signature, Message, Pub)) {
        memmove(m, sm+CRYPTO_BYTES, mlen1);
        *mlen = mlen1;
        res = 0;
    }
    else {
        res = -1;
    }

    for (i = 0; i < VOX_O; i++) {
        nmod_mat_clear(Pub[i]);
    }
    nmod_mat_clear(Signature);
    nmod_mat_clear(Message);
    ClearContext();
    return res;
}
