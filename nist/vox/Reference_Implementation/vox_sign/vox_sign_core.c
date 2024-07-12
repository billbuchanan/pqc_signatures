/******************************************************************************
 * Core implementation of signature and verification with cached keys
 ******************************************************************************
 * Copyright (C) 2023  The VOX team
 * License : MIT (see COPYING file for the full text)
 * Author  : Robin Larrieu
 *****************************************************************************/

#include "vox_sign_core.h"

#include "F4F5.h"
#include "vox_arith.h"
#include "vox_sample.h"
#include "vox_encode.h"
#include "fips202/fips202.h"
#include "rng/rng.h"
#include <malloc.h>
#include <string.h>

struct vox_cached_sk {
    Fq *Sec[VOX_O];
    Fqc T[VOX_OC*VOX_VC];
    Fq  S[VOX_T*(VOX_O-VOX_T)];
    unsigned char seed_SK[VOX_SEED_BYTES];
    unsigned char hpk[VOX_HPK_BYTES];
};


struct vox_cached_pk {
    Fqc *Pub[VOX_O];
    unsigned char hpk[VOX_HPK_BYTES];
};


vox_cached_sk_t* VOX_expand_sk(const unsigned char sk[VOX_SECRETKEYBYTES]) {
    int i,j;
    size_t nrows;

    /* Allocate output; free any allocated space and return NULL if it fails */
    vox_cached_sk_t *out = malloc(sizeof(vox_cached_sk_t));
    if (out == NULL)
        return NULL;
    for (i=0; i<VOX_O; i++) {
        nrows = i<VOX_T ? VOX_N : VOX_V;
        out->Sec[i] = calloc(nrows*VOX_N, sizeof(Fq));
        if (out->Sec[i] == NULL) {
            for (j=0; j<i; j++)
                free(out->Sec[j]);
            free(out);
            return NULL;
        }
    }

    /* Fill output */
    const unsigned char *seed_SK = sk + VOX_SEED_BYTES;
    const unsigned char *hpk     = sk + 2*VOX_SEED_BYTES;
    sk += 2*VOX_SEED_BYTES + VOX_HPK_BYTES;
    decode_SK_expand(out->Sec, sk);
    VOX_GenS(out->S, seed_SK);
    VOX_GenT(out->T, seed_SK);
    memcpy(out->seed_SK, seed_SK, VOX_SEED_BYTES);
    memcpy(out->hpk,     hpk,     VOX_HPK_BYTES);
    return out;
}

vox_cached_pk_t* VOX_expand_pk(const unsigned char pk[VOX_PUBLICKEYBYTES]) {
    int i,j;

    /* Check that the public key is zero-padded, otherwise it is invalid */
    if (VOX_PUB_BITS % 8 != 0) {
        if (pk[VOX_SEED_BYTES + (VOX_PUB_BITS / 8)] >> (VOX_PUB_BITS % 8))
            return NULL;
    }

    /* Allocate output; free any allocated space and return NULL if it fails */
    vox_cached_pk_t *out = malloc(sizeof(vox_cached_pk_t));
    if (out == NULL)
        return NULL;
    for (i=0; i<VOX_O; i++) {
        out->Pub[i] = calloc(VOX_NC*VOX_NC, sizeof(Fqc));
        if (out->Pub[i] == NULL) {
            for (j=0; j<i; j++)
                free(out->Pub[j]);
            free(out);
            return NULL;
        }
    }

    /* Fill output */
    VOX_GenPK_template(out->Pub, pk);
    for (i=VOX_T; i<VOX_O; i++)
        decode_PK_oil(out->Pub[i], pk, i);
    shake256(out->hpk, VOX_HPK_BYTES, pk, VOX_PUBLICKEYBYTES);
    return out;
}


void VOX_clear_cached_sk(vox_cached_sk_t *SK) {
    if (SK == NULL)
        return;
    for (int i=0; i<VOX_O; i++)
        free(SK->Sec[i]);
    free(SK);
}

void VOX_clear_cached_pk(vox_cached_pk_t *PK) {
    if (PK == NULL)
        return;
    for (int i=0; i<VOX_O; i++)
        free(PK->Pub[i]);
    free(PK);
}


/* The nonce for the vinegar variables is a 8-bit counter, so we have
 * at most 256 signature attemps
 * (the failure probability for that many attempts is negligible) */
#define MAX_SIG_ATTEMPTS 256

int VOX_sign_core(Fq Sig[VOX_N],
                  const unsigned char m[], size_t mlen,
                  const vox_cached_sk_t *SK)
{
    Fq X[VOX_T], M[VOX_O], hint;
    Fq Ker[VOX_O][VOX_T+1], eqns[VOX_T*MQ_T_TERMS];
    uint32_t lin[VOX_O-VOX_T][VOX_O+1];
    int pivots[VOX_O+1];
    unsigned char seed_V[VOX_SEED_BYTES];
    int ctr, i, j, rank, n_non_pivots, col;

    VOX_GenM(M, SK->hpk, m, (size_t) mlen);

#ifdef DETERMINISTIC_SIGNING
    keccak_state hash_ctx;
    shake256_init(&hash_ctx);
    shake256_absorb(&hash_ctx, SK->seed_SK, VOX_SEED_BYTES);
    shake256_absorb(&hash_ctx, m, mlen);
    shake256_finalize(&hash_ctx);
    shake256_squeeze(seed_V, VOX_SEED_BYTES, &hash_ctx);
#else
    randombytes(seed_V, _SEED_SIZE);
#endif

    Fq_InjectS(M, SK->S);
    Fq *V = Sig+VOX_O;
    for (ctr=0; ctr<MAX_SIG_ATTEMPTS; ctr++) {
        VOX_GenVinegar(V, &hint, seed_V, (uint8_t) ctr);
        /*
         * 1 - Handle the linear equations
         */
        for (i=VOX_T; i<VOX_O; i++)
            Fq_GetLinEqn(lin[i-VOX_T], SK->Sec[i], V, M[i]);
        rank = rref_with_pivots(lin[0], pivots);
        /* System must be full rank */
        if (rank != VOX_O-VOX_T)
            continue;
        /* The last column corresponds to the constant part of the equations,
         * so it must not be a pivot */
        if (pivots[VOX_O-VOX_T-1] == VOX_O)
            continue;
        /* Build a basis of the kernel */
        n_non_pivots = VOX_T+1; /* VOX_O + 1 - rank */
        for (i=0; i<rank; i++) {
            for (j=0; j<n_non_pivots; j++) {
                col = pivots[VOX_O+1-j-1]; /* j-th non-pivot */
                Ker[pivots[i]][j] = (Fq) (VOX_Q - lin[i][col]);
            }
        }
        for (i=0; i<VOX_T; i++) {
            j = pivots[VOX_O+1-i-1]; /* i-th non-pivot */
            for (col=0; col<n_non_pivots; col++)
                Ker[j][col] = (col==i) ? 1 : 0;
        }
        /*
         * 2 - Construct the multivariate system (VOX_T equations in VOX_T variables)
         * and solve it
         */
        for (i=0; i<VOX_T; i++)
            Fq_GetMQEqn(eqns+i*MQ_T_TERMS, SK->Sec[i], V, Ker[0], M[i]);
        if (Solve_MQ(X, eqns, hint))
            break;
    }
    if (ctr < MAX_SIG_ATTEMPTS) {
        /* Compute oil variables in Sig (vinegar variables are already set) */
        Fq_GetOilVars(Sig, Ker[0], X);
        /* Mix variables using T */
        Fq_InjectT(Sig, SK->T);
        return 0;
    }
    else {
        return 1;
    }
}


int VOX_sign_cachedSK(unsigned char sig[VOX_SIG_BYTES],
                      const unsigned char m[], size_t mlen,
                      const vox_cached_sk_t *SK)
{
    Fq  Sig[VOX_N];

    if (SK == NULL)
        return 1;
    if (VOX_sign_core(Sig, m, mlen, SK))
        return 1;
    encode_sig(sig, Sig);
    return 0;
}

int VOX_verify_cachedPK(const unsigned char sig[VOX_SIG_BYTES],
                        const unsigned char m[], size_t mlen,
                        const vox_cached_pk_t *PK)
{
    Fq  M[VOX_O], Sig[VOX_N];
    if (PK == NULL)
        return 1;
    /* check zero padding to prevent trivial forgeries */
    if (VOX_SIG_BITS % 8 != 0) {
        if (sig[VOX_SIG_BITS / 8] >> (VOX_SIG_BITS % 8))
            return -1;
    }

    VOX_GenM(M, PK->hpk, m, (size_t) mlen);
    decode_sig(Sig, sig);
    for (int i=0; i<VOX_O; i++) {
        if (Fqc_EvalMQSystem(PK->Pub[i], Sig) != M[i])
            return -1;
    }
    return 0;
}
