#include <string.h>
#include "rng.h"
#include "dme.h"
#include "api.h"
#include "sha3.h"

int crypto_sign_keypair(unsigned char *pk, unsigned char *sk)
{
    struct skey_t skey;
    struct pkey_t pkey;
    generate_skey(&skey);
    generate_pkey(&pkey, &skey);
    skey_serialize(sk, &skey);
    pkey_serialize(pk, &pkey);
    return 0;
}

int crypto_sign(unsigned char *sm, unsigned long long *smlen,
    const unsigned char *m, unsigned long long mlen,
    const unsigned char *sk)
{
    unsigned int i;
    fq_elem mhash[8], signature[8];
    unsigned char r[12], w[24], g[24];
    struct skey_t skey;
    struct sha3_ctx ctx;
    if (!skey_parse(&skey, (unsigned char *) sk))
        return -1;
    randombytes(r, 12);
    sha3_init_ctx(&ctx, 24);
    sha3_process_bytes((char *)m, mlen, &ctx);
    sha3_process_bytes((char *)r, 12,   &ctx);
    sha3_finish_ctx(&ctx, (char *)w);
//  if (*smlen < mlen + 64)
//      return -1;
    *smlen = mlen + 48;
    memcpy(sm, m, mlen);
    sha3_init_ctx(&ctx, 24);
    sha3_process_bytes((char *)w, 24, &ctx);
    sha3_finish_ctx(&ctx, (char *)g);
    for (i=0; i<12; i++)
        g[i] ^= r[i];
    fq_parse(&mhash[0], &w[0]);
    fq_parse(&mhash[1], &w[6]);
    fq_parse(&mhash[2], &w[12]);
    fq_parse(&mhash[3], &w[18]);
    fq_parse(&mhash[4], &g[0]);
    fq_parse(&mhash[5], &g[6]);
    fq_parse(&mhash[6], &g[12]);
    fq_parse(&mhash[7], &g[18]);
    decrypt_with_skey(signature, &skey, mhash);
    for (i=0; i<8; i++)
        fq_serialize(&sm[mlen+6*i], signature[i]);
    return 0;
}

int crypto_sign_open(unsigned char *m, unsigned long long *mlen,
    const unsigned char *sm, unsigned long long smlen,
    const unsigned char *pk)
{
    unsigned int i;
    struct pkey_t pkey;
    struct sha3_ctx ctx;
    fq_elem signature[8], hash2[8];
    unsigned char g[24], w[24], r[12], g2[24], w2[24];
    if (!pkey_parse(&pkey, (unsigned char *) pk))
        return -1;
//  if (*mlen + 64 < smlen)
//      return -1;
    *mlen = smlen - 48;
    memcpy(m, sm, *mlen);
    for (i=0; i<8; i++)
        if (!fq_parse(&signature[i], &sm[*mlen + 6*i]))
            return -1;
    encrypt_with_pkey(hash2, &pkey, signature);
    fq_serialize(&w[0],  hash2[0]);
    fq_serialize(&w[6],  hash2[1]);
    fq_serialize(&w[12], hash2[2]);
    fq_serialize(&w[18], hash2[3]);
    fq_serialize(&g[0],  hash2[4]);
    fq_serialize(&g[6],  hash2[5]);
    fq_serialize(&g[12], hash2[6]);
    fq_serialize(&g[18], hash2[7]);
    sha3_init_ctx(&ctx, 24);
    sha3_process_bytes((char *)w, 24, &ctx);
    sha3_finish_ctx(&ctx, (char *)g2);
    for (i=0; i<12; i++)
        r[i] = g2[i] ^ g[i];
    if (memcmp(&g[12], &g2[12], 12))
        return -1;
    sha3_init_ctx(&ctx, 24);
    sha3_process_bytes((char *)m, *mlen, &ctx);
    sha3_process_bytes((char *)r, 12, &ctx);
    sha3_finish_ctx(&ctx, (char *)w2);
    if (memcmp(w, w2, 24))
        return -1;
    return 0;
}
