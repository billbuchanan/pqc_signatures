/******************************************************************************
 * Serialization of VOX objects.
 * Elements of F_q are encoded as a little-endian bitstring of length log2(q).
 * Elements of F_q^c are encoded as the concatenation of each coefficient,
 * assuming the representation a0 + a1*X + ... + aC*X^c mod p(X).
 * Matrices are encoded as the concatenation of each coefficient
 * in row-major order, excluding the parts that are known to be zero.
 ******************************************************************************
 * Copyright (C) 2023  The VOX team
 * License : MIT (see COPYING file for the full text)
 * Author  : Robin Larrieu
 *****************************************************************************/
#ifndef VOX_ENCODE_H
#define VOX_ENCODE_H

#include "vox_params.h"

/*
 * Serialize the compressed public key (seed, oil-oil blocks of Pub[VOX_T:VOX_O]).
 * Each Pub[i] is a VOX_NC x VOX_NC lower triangular matrix over F_q^c.
 */
void encode_PK(unsigned char *pk, Fqc **Pub, const unsigned char *seed_PK);

/*
 * Deserialize the oil-oil block of Pub[i] (assumes VOX_T <= i < VOX_O)
 */
void decode_PK_oil(Fqc Pub_i[VOX_NC*VOX_NC], const unsigned char *pk, int i);

/*
 * Serialize the secret key.
 * Each Sec[i] is a VOX_NC x VOX_NC lower triangular matrix over F_q^c,
 * where the first VOX_OC rows are zero if i >= VOX_T.
 */
void encode_SK(unsigned char *sk, Fqc **Sec);

/*
 * Deserialize the secret key.
 * Each Sec[i] is expanded into a VOX_N x VOX_N lower triangular matrix over F_q,
 * such that X * Sec[i] * tX = Tr(x * sec[i] * x).
 */
void decode_SK_expand(Fq **Sec, const unsigned char *sk);

/*
 * Serialize the signature
 */
void encode_sig(unsigned char *sig, Fq Signature[VOX_N]);

/*
 * Deserialize the signature
 */
void decode_sig(Fq Signature[VOX_N], const unsigned char *sig);

#endif /* VOX_ENCODE_H */
