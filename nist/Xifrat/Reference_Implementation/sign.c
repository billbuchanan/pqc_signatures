#include "api.h"
#include "rng.h"
#include <string.h>

void prng_src(void *restrict x, void *restrict data, size_t len)
{
    (void)x;
    randombytes(data, len);
}

int
crypto_sign_keypair(unsigned char *pk, unsigned char *sk)
{
    xifrat_sign_privkey_context_t x;
    xifrat_sign_keygen(&x, prng_src, NULL);
    
    xifrat_sign_export_pubkey(
        &x, (void *)pk,
        sizeof(xifrat_sign_pubkey_t));
    
    xifrat_sign_encode_privkey(
        &x, (void *)sk,
        sizeof(xifrat_sign_privkey_t));

    return 0;
}

int
crypto_sign(unsigned char *sm, unsigned long long *smlen,
            const unsigned char *m, unsigned long long mlen,
            const unsigned char *sk)
{
    xifrat_sign_privkey_context_t x;
    
    xifrat_sign_decode_privkey(
        &x, (const void *)sk,
        sizeof(xifrat_sign_privkey_t));
    
    xifrat_sign_sign(&x, m, mlen);
    
    xifrat_sign_encode_signature(
        &x, (void *)sm,
        sizeof(xifrat_sign_signature_t));
    
    memcpy(sm + sizeof(xifrat_sign_signature_t), m, mlen);
    *smlen = mlen + sizeof(xifrat_sign_signature_t);
    return 0;
}

int
crypto_sign_open(unsigned char *m, unsigned long long *mlen,
                 const unsigned char *sm, unsigned long long smlen,
                 const unsigned char *pk)
{
    xifrat_sign_pubkey_context_t x;
    const void *subret = NULL;
    
    xifrat_sign_decode_pubkey(
        &x, (const void *)pk,
        sizeof(xifrat_sign_pubkey_t));
    
    xifrat_sign_decode_signature(
        &x, (const void *)sm,
        sizeof(xifrat_sign_pubkey_t));
    
    subret = xifrat_sign_verify(
        &x, sm + sizeof(xifrat_sign_signature_t),
        smlen - sizeof(xifrat_sign_signature_t));

    *mlen = smlen - sizeof(xifrat_sign_signature_t);
    memcpy(m, sm + sizeof(xifrat_sign_signature_t), *mlen);
    
    if( !subret ) return -1;
    else return 0;
}
