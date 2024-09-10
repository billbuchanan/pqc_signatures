/******************************************************************************
 * Core implementation of signature and verification with cached keys
 ******************************************************************************
 * Copyright (C) 2023  The VOX team
 * License : MIT (see COPYING file for the full text)
 * Author  : Robin Larrieu
 *****************************************************************************/
#ifndef VOX_SIGN_CORE_H
#define VOX_SIGN_CORE_H

#include "vox_params.h"
#include <stddef.h>

/*
 * Types for the cached keys to allow faster multiple
 * signatures/verifications with the same key
 */
typedef struct vox_cached_sk vox_cached_sk_t;
typedef struct vox_cached_pk vox_cached_pk_t;

/* Expand a key and cache the result */
vox_cached_sk_t* VOX_expand_sk(const unsigned char sk[VOX_SECRETKEYBYTES]);
vox_cached_pk_t* VOX_expand_pk(const unsigned char pk[VOX_PUBLICKEYBYTES]);

/* Releases memory used by a cached key */
void VOX_clear_cached_sk(vox_cached_sk_t *SK);
void VOX_clear_cached_pk(vox_cached_pk_t *PK);

/*
 * Core function for the signature.
 * Output is not yet serialized because the NIST API requires a minor
 * post-processing
 */
int VOX_sign_core(Fq Sig[VOX_N],
                  const unsigned char m[], size_t mlen,
                  const vox_cached_sk_t *SK);

/*
 * Sign using a pre-expanded secret key.
 */
int VOX_sign_cachedSK(unsigned char sig[VOX_SIG_BYTES],
                      const unsigned char m[], size_t mlen,
                      const vox_cached_sk_t *SK);

/*
 * Verify a signature using a pre-expanded public key
 */
int VOX_verify_cachedPK(const unsigned char sig[VOX_SIG_BYTES],
                        const unsigned char m[], size_t mlen,
                        const vox_cached_pk_t *PK);

#endif /* VOX_SIGN_CORE_H */
