/******************************************************************************
 * Main interface according to the NIST submission template
 ******************************************************************************
 * Copyright (C) 2023  The VOX team
 * License : MIT (see COPYING file for the full text)
 * Author  : Robin Larrieu
 *****************************************************************************/

#include "api.h"
#include "vox_arith.h"
#include "vox_encode.h"
#include "vox_sample.h"
#include "vox_sign_core.h"
#include "rng/rng.h"
#include "fips202/fips202.h"
#include <string.h>
#include <malloc.h>

#if  (VOX_SECRETKEYBYTES != CRYPTO_SECRETKEYBYTES)
#error "Inconsistent values for SECRETKEYBYTES"
#endif
#if  (VOX_PUBLICKEYBYTES != CRYPTO_PUBLICKEYBYTES)
#error "Inconsistent values for PUBLICKEYBYTES"
#endif
#if  (VOX_SIG_BYTES != CRYPTO_BYTES)
#error "Inconsistent values for SIG_BYTES"
#endif

static int alloc_eqn_Fqc(Fqc **dst, size_t n) {
    int i,j;
    for (i=0; i<VOX_O; i++) {
        dst[i] = calloc(n*n, sizeof(Fqc));
        if (dst[i] == NULL) {
            for (j=0; j<i; j++)
                free(dst[j]);
            return 1;
        }
    }
    return 0;
}

static void free_eqn_Fqc(Fqc **dst) {
    for (int i=0; i<VOX_O; i++)
        free(dst[i]);
}


#define OK            0
#define ALLOC_FAIL    1
#define INTERNAL_ERR  2
#define INVALID_SIG   -1

int crypto_sign_keypair(unsigned char *pk, unsigned char *sk)
{
    Fqc *Pub[VOX_O], *Tmp[VOX_O];
    Fqc T[VOX_OC*VOX_VC];
    Fq  S[VOX_T*(VOX_O-VOX_T)];

    if (alloc_eqn_Fqc(Pub, VOX_NC))
      return ALLOC_FAIL;
    if (alloc_eqn_Fqc(Tmp, VOX_OC)) {
      free_eqn_Fqc(Pub);
      return ALLOC_FAIL;
    }

    unsigned char *seed_PK = sk;
    unsigned char *seed_SK = sk + VOX_SEED_BYTES;
    randombytes(seed_PK, VOX_SEED_BYTES);
    randombytes(seed_SK, VOX_SEED_BYTES);

    VOX_GenPK_template(Pub, seed_PK);
    VOX_GenT(T, seed_SK);
    VOX_GenS(S, seed_SK);
    /* Compute public key; store the oil-oil block of the secret key in tmp[0:VOX_T] */
    VOX_CompletePetzoldPubkey(Pub, Tmp, S, T);
    encode_PK(pk, Pub, seed_PK);
    /* Store hash(PK) into sk */
    sk += 2*VOX_SEED_BYTES;
    shake256(sk, VOX_HPK_BYTES, pk, VOX_PUBLICKEYBYTES);
    sk += VOX_HPK_BYTES;
    /* Compute secret key by modifying Pub inplace */
    VOX_ComposeST(Pub, Tmp, S, T);
    encode_SK(sk, Pub);

    free_eqn_Fqc(Pub);
    free_eqn_Fqc(Tmp);
    return OK;
}


int crypto_sign(unsigned char *sm, unsigned long long *smlen,
                const unsigned char *m, unsigned long long mlen,
                const unsigned char *sk)
{
    Fq  Sig[VOX_N];
    int status;
    vox_cached_sk_t *SK = VOX_expand_sk(sk);
    if (SK == NULL)
        return ALLOC_FAIL;

    if (VOX_sign_core(Sig, m, mlen, SK) == OK) {
        /* copy message then signature in case m and sm overlap */
        memmove(sm + VOX_SIG_BYTES, m, (size_t) mlen);
        encode_sig(sm, Sig);
        *smlen = mlen + VOX_SIG_BYTES;
        status = OK;
    }
    else {
        status = INTERNAL_ERR;
    }
    VOX_clear_cached_sk(SK);
    return status;
}


/*
 * For the verification, we do not use the function from vox_sign_core
 * because we can parse each equation from the public key on-demand.
 * This uses a lot less memory, and it saves time in case of invalid signature
 * as we do not need to expand the remaining equations.
 */
int crypto_sign_open(unsigned char *m, unsigned long long *mlen,
                     const unsigned char *sm, unsigned long long smlen,
                     const unsigned char *pk)
{
    const unsigned char *msg = sm + VOX_SIG_BYTES;
    unsigned long long  msglen = smlen - VOX_SIG_BYTES;
    Fq  M[VOX_O], Sig[VOX_N];
    Fqc Pub_i[4][VOX_NC*VOX_NC];
    Fqc *Pub_p[4] = { Pub_i[0], Pub_i[1], Pub_i[2], Pub_i[3] };
    unsigned char hpk[VOX_HPK_BYTES];
    int i,j;

    if (smlen < VOX_SIG_BYTES)
        return INVALID_SIG;
    /* check zero padding to prevent trivial forgeries */
    if (VOX_SIG_BITS % 8 != 0) {
        if (sm[VOX_SIG_BITS / 8] >> (VOX_SIG_BITS % 8))
            return INVALID_SIG;
    }

    memset(Pub_i, 0, sizeof(Pub_i));
    shake256(hpk, VOX_HPK_BYTES, pk, VOX_PUBLICKEYBYTES);
    VOX_GenM(M, hpk, msg, (size_t) msglen);
    decode_sig(Sig, sm);
    for (i=0; i+3<VOX_T; i+=4) {
        VOX_GenPK_oil_x4(Pub_p, pk, i);
        VOX_GenPK_vin_x4(Pub_p, pk, i);
        for (j=0; j<4; j++) {
            if (Fqc_EvalMQSystem(Pub_p[j], Sig) != M[i+j])
                return INVALID_SIG;
        }
    }
#if (VOX_T % 4 != 0)
#if (VOX_T % 4 == 1)
    VOX_GenPK_oil(Pub_p[0], pk, i);
    for (j=1; j<4; j++)
        decode_PK_oil(Pub_p[j], pk, i+j);
#else /* 1 < VOX_T%4 < 4 */
    VOX_GenPK_oil_x4(Pub_p, pk, i);
    for (j=VOX_T%4; j<4; j++)
        decode_PK_oil(Pub_p[j], pk, i+j);
#endif /* (VOX_T % 4 == 1) ? */
    VOX_GenPK_vin_x4(Pub_p, pk, i);
    for (j=0; j<4; j++) {
        if (Fqc_EvalMQSystem(Pub_p[j], Sig) != M[i+j])
            return INVALID_SIG;
    }
    i+=4;
#endif /* (VOX_T % 4 == 0) ? */

    for (; i+3<VOX_O; i+=4) {
        VOX_GenPK_vin_x4(Pub_p, pk, i);
        for (j=0; j<4; j++) {
            decode_PK_oil(Pub_p[j], pk, i+j);
            if (Fqc_EvalMQSystem(Pub_p[j], Sig) != M[i+j])
                return INVALID_SIG;
        }
    }
#if (VOX_O % 4 != 0)
#if (VOX_O % 4 == 1)
    VOX_GenPK_vin(Pub_p, pk, i);
#else /* 1 < VOX_T%4 < 4 */
    VOX_GenPK_vin_x4(Pub_p, pk, i);
#endif /* (VOX_T % 4 == 1) ? */
    for (j=0; j<VOX_O%4; j++) {
        decode_PK_oil(Pub_p[j], pk, i+j);
        if (Fqc_EvalMQSystem(Pub_p[j], Sig) != M[i+j])
            return INVALID_SIG;
    }
#endif /* (VOX_T % 4 == 0) ? */

    memmove(m, msg, (size_t) msglen);
    *mlen = msglen;
    return OK;
}
