/******************************************************************************
 * Serialization of VOX objects
 ******************************************************************************
 * Copyright (C) 2023  The VOX team
 * License : MIT (see COPYING file for the full text)
 * Author  : Robin Larrieu
 *****************************************************************************/
#include "vox_encode.h"
#include "vox_arith.h"
#include <string.h>

/*
 * Writes a coefficient 'x' in F_q into buffer '*out', assuming that the 'bits'
 * first bits in the current byte are already set from encoding the previous
 * coefficient.
 * Updates the out pointer and returns the number of bits used in the new current byte.
 */
static int encode_fq(unsigned char **out, int bits, Fq x) {
#if VOX_Q_BITS < 8
#error "This implementation assumes 8 <= VOX_Q_BITS"
#elif VOX_Q_BITS == 8
    /* Assumes bits == 0 */
    **out = (unsigned char) x;
    (void) bits;
    *out += 1;
    return 0;
#else /* VOX_Q_BITS > 8 */
    int remaining_bits = VOX_Q_BITS;
    unsigned char *buf = *out;
    Fq xp = x;

    *out += (bits + VOX_Q_BITS) / 8;
    if (bits != 0) {
        *buf++ |= (unsigned char) (x << bits);
        xp = x >> (8-bits);
        remaining_bits -= (8-bits);
    }
    while (remaining_bits >= 8) {
        *buf++ = (unsigned char) xp;
        xp >>= 8;
        remaining_bits -= 8;
    }
    if (remaining_bits > 0)
        *buf = (unsigned char) xp;
    return remaining_bits;
#endif
}

/*
 * Reads a coefficient in Fq, using the convention as above
 */
static int decode_fq(Fq *x, int bits, const unsigned char **in) {
#if VOX_Q_BITS < 8
#error "This implementation assumes VOX_Q_BITS >=  8"
#elif VOX_Q_BITS == 8
    /* Assumes bits == 0 */
    *x = **in;
    (void) bits;
    *in += 1;
    return 0;
#else /* VOX_Q_BITS > 8 */
    int remaining_bits = VOX_Q_BITS;
    const unsigned char *buf = *in;
    unsigned int r, xp = 0;

    *in += (bits + VOX_Q_BITS) / 8;
    if (bits != 0) {
        xp = ((*buf++) >> bits);
        bits = 8-bits;
        remaining_bits -= bits;
    }
    while (remaining_bits >= 8) {
        xp |= ((Fq) (*buf++)) << bits;
        remaining_bits -= 8;
        bits += 8;
    }
    if (remaining_bits > 0) {
        r = (*buf) & ((1U << remaining_bits) - 1);
        xp |= r << bits;
    }
    *x = (Fq) xp;
    return remaining_bits;
#endif
}

/*
 * Writes a coefficient in F_q^c, using the convention as above
 */
static int encode_fqc(unsigned char **out, int bits, const Fqc *x) {
    for (int i=0; i<VOX_C; i++)
        bits = encode_fq(out, bits, x->coeffs[i]);
    return bits;
}


/*
 * Writes a coefficient in F_q^c, using the convention as above
 */
static int decode_fqc(Fqc *x, int bits, const unsigned char **in) {
    for (int i=0; i<VOX_C; i++)
        bits = decode_fq(x->coeffs + i, bits, in);
    return bits;
}


/*****************************************************************************/


/*
 * Serialize the compressed public key
 */
void encode_PK(unsigned char *pk, Fqc **Pub, const unsigned char *seed_PK)
{
    int i, j, k;
    int bits = 0;

    memcpy(pk, seed_PK, VOX_SEED_BYTES);
    pk += VOX_SEED_BYTES;
    for (i = VOX_T; i < VOX_O; i++) {
        for (j = 0; j < VOX_OC; j++) {
            for (k = 0; k <= j; k++) {
                bits = encode_fqc(&pk, bits, &Pub[i][j*VOX_NC + k]);
            }
        }
    }
}


/*
 * Deserialize the oil-oil block of Pub[i]
 */
void decode_PK_oil(Fqc Pub_i[VOX_NC*VOX_NC], const unsigned char *pk, int i)
{
    int j,k,bits;
    if (i < VOX_T)
      return;
    bits = (i-VOX_T) * (VOX_Q_BITS * VOX_C) * (VOX_OC * (VOX_OC+1)) / 2;
    pk   += VOX_SEED_BYTES + bits / 8;
    bits = bits % 8;
    for (j = 0; j < VOX_OC; j++) {
        for (k = 0; k <= j; k++) {
            bits = decode_fqc(&Pub_i[j*VOX_NC + k], bits, &pk);
        }
    }
}


/*
 * Serialize the secret key.
 */
void encode_SK(unsigned char *sk, Fqc **Sec)
{
    int i, j, k;
    int bits = 0;

    /* The first _T matrices are full triangular */
    for (i = 0; i < VOX_T; i++) {
        for (j = 0; j < VOX_NC; j++) {
            for (k = 0; k <= j; k++) {
                bits = encode_fqc(&sk, bits, &Sec[i][j*VOX_NC + k]);
            }
        }
    }
    /* The remaining matrices have their oil-oil block to 0 */
    for (i = VOX_T; i < VOX_O; i++) {
        for (j = VOX_OC; j < VOX_NC; j++) {
            for (k = 0; k <= j; k++) {
                bits = encode_fqc(&sk, bits, &Sec[i][j*VOX_NC + k]);
            }
        }
    }
}


/*
 * Deserialize and expand the secret key.
 */
void decode_SK_expand(Fq **Sec, const unsigned char *sk)
{
    Fqc tmp;
    int i, j, k;
    int bits = 0;

    for (i = 0; i < VOX_T; i++) {
        for (j = 0; j < VOX_NC; j++) {
            for (k = 0; k < j; k++) {
                bits = decode_fqc(&tmp, bits, &sk);
                Fqc_to_block(&Sec[i][j*VOX_N*VOX_C + k*VOX_C], &tmp, VOX_N);
            }
            /* Fold the diagonal block as lower triangular */
            bits = decode_fqc(&tmp, bits, &sk);
            Fqc_to_LowTriBlock(&Sec[i][j*VOX_N*VOX_C + j*VOX_C], &tmp, VOX_N);
        }
    }

    for (i = VOX_T; i < VOX_O; i++) {
        for (j = VOX_OC; j < VOX_NC; j++) {
            for (k = 0; k < j; k++) {
                bits = decode_fqc(&tmp, bits, &sk);
                Fqc_to_block(&Sec[i][(j-VOX_OC)*VOX_N*VOX_C + k*VOX_C], &tmp, VOX_N);
            }
            /* Fold the diagonal block as lower triangular */
            bits = decode_fqc(&tmp, bits, &sk);
            Fqc_to_LowTriBlock(&Sec[i][(j-VOX_OC)*VOX_N*VOX_C + j*VOX_C], &tmp, VOX_N);
        }
    }
}


/*
 * Serialize the signature
 */
void encode_sig(unsigned char *sig, Fq Signature[VOX_N])
{
    int i;
    int bits = 0;

    for (i = 0; i < VOX_N; i++) {
        bits = encode_fq(&sig, bits, Signature[i]);
    }
}


/*
 * Deserialize the signature
 */
void decode_sig(Fq Signature[VOX_N], const unsigned char *sig)
{
    int i;
    int bits = 0;

    for (i = 0; i < VOX_N; i++) {
        bits = decode_fq(&Signature[i], bits, &sig);
    }
}
