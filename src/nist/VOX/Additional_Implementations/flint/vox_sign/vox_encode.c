/******************************************************************************
 * Serialization of VOX objects
 ******************************************************************************
 * Copyright (C) 2023  The VOX team
 * License : MIT (see COPYING file for the full text)
 * Author  : Robin Larrieu
 *****************************************************************************/
#include "vox.h"
#include <string.h>

/*
 * Writes a coefficient 'x' in F_q into buffer '*out', assuming that the 'bits'
 * first bits in the current byte are already set from encoding the previous
 * coefficient.
 * Updates the out pointer and returns the number of bits used in the new current byte.
 */
static int encode_fq_coeff(unsigned char **out, int bits, mp_limb_t x) {
#if VOX_Q_BITS < 8
#error "This implementation assumes VOX_Q_BITS >=  8"
#elif VOX_Q_BITS == 8
    /* Assumes bits == 0 */
    **out = (unsigned char) x;
    (void) bits;
    *out += 1;
    return 0;
#else /* VOX_Q_BITS > 8 */
    int remaining_bits = VOX_Q_BITS;
    unsigned char *buf = *out;
    mp_limb_t xp = x;

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

static int decode_fq_coeff(mp_limb_t *x, int bits, const unsigned char **in) {
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
    unsigned char r;
    const unsigned char *buf = *in;
    mp_limb_t xp = 0;

    *in += (bits + VOX_Q_BITS) / 8;
    if (bits != 0) {
        xp = ((*buf++) >> bits);
        bits = 8-bits;
        remaining_bits -= bits;
    }
    while (remaining_bits >= 8) {
        xp |= ((mp_limb_t) (*buf++)) << bits;
        remaining_bits -= 8;
        bits += 8;
    }
    if (remaining_bits > 0) {
        r = (*buf) & ((1U << remaining_bits) - 1);
        xp |= ((mp_limb_t) r) << bits;
    }
    *x = xp;
    return remaining_bits;
#endif
}

static int encode_fqc_coeff(unsigned char **out, int bits, fq_nmod_t x) {
    for (int i=0; i<VOX_C; i++)
        bits = encode_fq_coeff(out, bits, nmod_poly_get_coeff_ui(x, i));
    return bits;
}

static int decode_fqc_coeff(fq_nmod_t x, int bits, const unsigned char **in) {
    nmod_poly_set_coeff_ui(x, VOX_C-1, 1); /* resize output if necessary */
    for (int i=0; i<VOX_C; i++)
        bits = decode_fq_coeff(x->coeffs + i, bits, in);
    nmod_poly_set_coeff_ui(x, VOX_C-1, x->coeffs[VOX_C-1]); /* recompute degree, length */
    return bits;
}


/*
 * Serialize the compressed public key
 * Public key contains VOX_O-VOX_T triangular blocks of size VOX_HC*VOX_HC,
 * plus a seed to generate the rest.
 */
void encode_PK(unsigned char *pk, fq_nmod_mat_t* Pub, const unsigned char *seed_PK)
{
    int i, j, k;
    int bits = 0;

    memcpy(pk, seed_PK, VOX_SEED_BYTES);
    pk += VOX_SEED_BYTES;
    for (i = VOX_T; i < VOX_O; i++) {
        for (j = 0; j < VOX_OC; j++) {
            for (k = 0; k <= j; k++) {
                bits = encode_fqc_coeff(&pk, bits, fq_nmod_mat_entry(Pub[i], j, k));
            }
        }
    }
}

/*
 * Deserialize and expand the public key.
 */
void decode_PK_expand(nmod_mat_t* Pub, const unsigned char *pk)
{
    int i, j, k;
    int bits = 0;
    fq_nmod_mat_t Pub2[VOX_O];

    for (i = 0; i < VOX_O; i++) {
        fq_nmod_mat_init(Pub2[i], VOX_NC, VOX_NC, Fqc_ctx);
    }

    /* Expand the fully random part from the seed */
    GenQRKey(Pub2, pk);
    pk += VOX_SEED_BYTES;
    /* Parse the triangular blocks */
    for (i = VOX_T; i < VOX_O; i++) {
        for (j = 0; j < VOX_OC; j++) {
            for (k = 0; k <= j; k++) {
                bits = decode_fqc_coeff(fq_nmod_mat_entry(Pub2[i], j, k), bits, &pk);
            }
        }
    }

    /* Expand the key into VOX_N*VOX_N matrices over F_q */
    for (i = 0; i < VOX_O; i++) {
        ExpandMat_LowTri(Pub[i], Pub2[i], base2);
        fq_nmod_mat_clear(Pub2[i], Fqc_ctx);
    }
}

/*
 * Serialize the secret system.
 * The secret key consists of VOX_O triangular matrices of size VOX_NC*VOX_NC,
 * where the first VOX_T are full and the remaining have no oil-oil block.
 */
void encode_SK(unsigned char *sk, fq_nmod_mat_t* Sec)
{
    int i, j, k;
    int bits = 0;

    /* The first VOX_T matrices are full triangular */
    for (i = 0; i < VOX_T; i++) {
        for (j = 0; j < VOX_NC; j++) {
            for (k = 0; k <= j; k++) {
                bits = encode_fqc_coeff(&sk, bits, fq_nmod_mat_entry(Sec[i], j, k));
            }
        }
    }
    /* The remaining matrices have their oil-oil block to 0 */
    for (i = VOX_T; i < VOX_O; i++) {
        for (j = VOX_OC; j < VOX_NC; j++) {
            for (k = 0; k <= j; k++) {
                bits = encode_fqc_coeff(&sk, bits, fq_nmod_mat_entry(Sec[i], j, k));
            }
        }
    }
}

/*
 * Deserialize and expand the secret key.
 */
void decode_SK_expand(nmod_mat_t* Sec, const unsigned char *sk)
{
    fq_nmod_mat_t tmp;
    int i, j, k;
    int bits = 0;

    /* Use a temporary VOX_NC*VOX_NC matrix over F_q to store each
     * compressed equation, then uncompress it in the result*/
    fq_nmod_mat_init(tmp, VOX_NC, VOX_NC, Fqc_ctx);

    /* The first VOX_T matrices are full triangular */
    for (i = 0; i < VOX_T; i++) {
        for (j = 0; j < VOX_NC; j++) {
            for (k = 0; k <= j; k++) {
                bits = decode_fqc_coeff(fq_nmod_mat_entry(tmp, j, k), bits, &sk);
            }
        }
        ExpandMat_LowTri(Sec[i], tmp, base2);
    }
    /* The remaining matrices have their oil-oil block to 0 */
    fq_nmod_mat_zero(tmp, Fqc_ctx);
    for (i = VOX_T; i < VOX_O; i++) {
        for (j = VOX_OC; j < VOX_NC; j++) {
            for (k = 0; k <= j; k++) {
                bits = decode_fqc_coeff(fq_nmod_mat_entry(tmp, j, k), bits, &sk);
            }
        }
        ExpandMat_LowTri(Sec[i], tmp, base2);
    }
    fq_nmod_mat_clear(tmp, Fqc_ctx);
}

/*
 * Serialize the signature, which is a vector of length VOX_N over F_q
 */
void encode_sig(unsigned char *sig, nmod_mat_t Signature)
{
    int i;
    int bits = 0;

    for (i = 0; i < VOX_N; i++) {
        bits = encode_fq_coeff(&sig, bits, nmod_mat_entry(Signature, 0, i));
    }
}

/*
 * Deserialize the signature
 */
void decode_sig(nmod_mat_t Signature, const unsigned char *sig)
{
    int i;
    int bits = 0;

    for (i = 0; i < VOX_N; i++) {
        bits = decode_fq_coeff(&nmod_mat_entry(Signature, 0, i), bits, &sig);
    }
}
