/* 2021-03-31, DannyNiu/NJF. Public Domain */

#include "xifrat-sign.h"
#include "../Reference_Implementation/shake.h"

void *xifrat_sign_keygen(
    xifrat_sign_privkey_context_t *restrict x,
    GenFunc_t prng_gen, void *restrict prng)
{
    uint64dup_t cryptogram;

    prng_gen(prng, &cryptogram, sizeof(cryptogram));
    xifrat_cryptogram_decode(x->C, cryptogram);
    
    prng_gen(prng, &cryptogram, sizeof(cryptogram));
    xifrat_cryptogram_decode(x->K, cryptogram);
    
    prng_gen(prng, &cryptogram, sizeof(cryptogram));
    xifrat_cryptogram_decode(x->Q, cryptogram);

    xifrat_Dup(x->P1, x->C, x->K);
    xifrat_Dup(x->P2, x->K, x->Q);

    return x;
}

void *xifrat_sign_sign(
    xifrat_sign_privkey_context_t *restrict x,
    void const *restrict msg, size_t msglen)
{
    uint64dup_t cryptogram, ct;
    shake256_t hash;
    int i;

    for(i=0; i<DLEN*VLEN; i++) cryptogram[i] = 0;
    SHAKE256_Init(&hash);
    SHAKE_Write(&hash, msg, msglen);
    SHAKE_Final(&hash);
    SHAKE_Read(&hash, &cryptogram, sizeof(cryptogram));
    xifrat_cryptogram_decode(ct, cryptogram);
    
    xifrat_Dup(x->signature, ct, x->Q);

    return x;
}

void const *xifrat_sign_verify(
    xifrat_sign_pubkey_context_t *restrict x,
    void const *restrict msg, size_t msglen)
{
    uint64dup_t t1, t2, ch; // signature verification transcripts.
    uint64dup_t cryptogram;
    shake256_t hash;
    uint64_t v;
    int i;

    for(i=0; i<DLEN*VLEN; i++) cryptogram[i] = 0;
    SHAKE256_Init(&hash);
    SHAKE_Write(&hash, msg, msglen);
    SHAKE_Final(&hash);
    SHAKE_Read(&hash, &cryptogram, sizeof(cryptogram));
    xifrat_cryptogram_decode(t2, cryptogram);
    
    for(i=0; i<DLEN*VLEN; i++) t1[i] = x->C[i];
    xifrat_Dup(ch, t1, t2);
    xifrat_Dup(t1, ch, x->P2); // (CH)(KQ)

    for(i=0; i<DLEN*VLEN; i++) ch[i] = x->P1[i];
    xifrat_Dup(t2, ch, x->signature);

    v = 0;
    for(i=0; i<DLEN*VLEN; i++) v |= t1[i] ^ t2[i];

    v |= v >> 32;
    v |= v >> 16;
    v |= v >> 8;
    v |= v >> 4;
    v |= v >> 2;
    v |= v >> 1;
    v &= 1;
    
    if( !v ) return msg;
    else return NULL;
}

void *xifrat_sign_export_pubkey(
    xifrat_sign_privkey_context_t *restrict x,
    xifrat_sign_pubkey_t *restrict out, size_t outlen)
{
    if( outlen < sizeof(xifrat_sign_pubkey_t) ) return NULL;

    xifrat_cryptogram_encode(out->C, x->C);
    xifrat_cryptogram_encode(out->P1, x->P1);
    xifrat_cryptogram_encode(out->P2, x->P2);

    return out;
}

void *xifrat_sign_decode_pubkey(
    xifrat_sign_pubkey_context_t *restrict x,
    xifrat_sign_pubkey_t const *restrict in, size_t inlen)
{
    if( inlen < sizeof(xifrat_sign_pubkey_t) ) return NULL;

    xifrat_cryptogram_decode(x->C, in->C);
    xifrat_cryptogram_decode(x->P1, in->P1);
    xifrat_cryptogram_decode(x->P2, in->P2);

    return x;
}

void *xifrat_sign_encode_privkey(
    xifrat_sign_privkey_context_t *restrict x,
    xifrat_sign_privkey_t *restrict out, size_t outlen)
{
    if( outlen < sizeof(xifrat_sign_privkey_t) ) return NULL;

    xifrat_cryptogram_encode(out->C, x->C);
    xifrat_cryptogram_encode(out->K, x->K);
    xifrat_cryptogram_encode(out->Q, x->Q);
    xifrat_cryptogram_encode(out->P1, x->P1);
    xifrat_cryptogram_encode(out->P2, x->P2);

    return out;
}

void *xifrat_sign_decode_privkey(
    xifrat_sign_privkey_context_t *restrict x,
    xifrat_sign_privkey_t const *restrict in, size_t inlen)
{
    if( inlen < sizeof(xifrat_sign_privkey_t) ) return NULL;

    xifrat_cryptogram_decode(x->C, in->C);
    xifrat_cryptogram_decode(x->K, in->K);
    xifrat_cryptogram_decode(x->Q, in->Q);
    xifrat_cryptogram_decode(x->P1, in->P1);
    xifrat_cryptogram_decode(x->P2, in->P2);

    return x;
}

void *xifrat_sign_encode_signature(
    xifrat_sign_privkey_context_t *restrict x,
    xifrat_sign_signature_t *restrict out, size_t outlen)
{
    if( outlen < sizeof(xifrat_sign_signature_t) ) return NULL;

    xifrat_cryptogram_encode(out->signature, x->signature);

    return out;
}

void *xifrat_sign_decode_signature(
    xifrat_sign_pubkey_context_t *restrict x,
    xifrat_sign_signature_t const *restrict in, size_t inlen)
{
    if( inlen < sizeof(xifrat_sign_signature_t) ) return NULL;

    xifrat_cryptogram_decode(x->signature, in->signature);

    return x;
}
