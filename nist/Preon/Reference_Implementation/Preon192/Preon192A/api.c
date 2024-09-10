#include <stdlib.h>
#include <stdio.h>

#include "api.h"
#include "preon.h"
#include "aurora_inner.h"

int crypto_sign_keypair(unsigned char *pk, unsigned char *sk)
{
    keygen(RAW_CRYPTO_SECRETKEYBYTES * 8, sk, RAW_CRYPTO_SECRETKEYBYTES, pk);
    memcpy(sk + RAW_CRYPTO_SECRETKEYBYTES, pk, CRYPTO_PUBLICKEYBYTES * sizeof(unsigned(char)));
    return 0;
}

int crypto_sign(unsigned char *sm, unsigned long long *smlen,
                const unsigned char *m, unsigned long long mlen,
                const unsigned char *sk)
{
    const unsigned char *pk = sk + RAW_CRYPTO_SECRETKEYBYTES;
    size_t max_sig_len = aurora_proof_max_size(PREON_SIG_TYPE);
    unsigned char *sig = (unsigned char *)malloc(max_sig_len * sizeof(unsigned char));
    size_t actual_sig_len = sign(sig, max_sig_len, PREON_SIG_TYPE, sk, RAW_CRYPTO_SECRETKEYBYTES, pk, CRYPTO_SECRETKEYBYTES, m, mlen);
    if (actual_sig_len == 0)
    {
        free(sig);
        return -1;
    }

    /*
        sm = {
            sizeof(size_t)
            actual_sig_len
            signature
            message
        }
    */

    *smlen = 0;
    sm[(*smlen)++] = sizeof(size_t);
    for (size_t i = 0; i < sizeof(size_t); ++i)
    {
        sm[(*smlen)++] = (actual_sig_len >> (i * 8)) & 0xFF;
    }
    memcpy(sm + *smlen, sig, actual_sig_len);
    *smlen += actual_sig_len;
    memcpy(sm + *smlen, m, mlen);
    *smlen += mlen;
    free(sig);
    return 0;
}

int crypto_sign_open(unsigned char *m, unsigned long long *mlen,
                     const unsigned char *sm, unsigned long long smlen,
                     const unsigned char *pk)
{
    size_t sizeof_size_t = sm[0];
    size_t sig_len = 0;
    for (size_t i = 0; i < sizeof_size_t; ++i)
    {
        sig_len |= ((size_t)sm[i + 1] << (i * 8));
    }
    const unsigned char *sig = sm + 1 + sizeof(size_t);
    *mlen = smlen - sizeof(size_t) - sig_len - 1;
    memcpy(m, sm + 1 + sizeof(size_t) + sig_len, *mlen);
    return !verify(sig, sig_len, PREON_SIG_TYPE, pk, CRYPTO_PUBLICKEYBYTES, m, *mlen);
}