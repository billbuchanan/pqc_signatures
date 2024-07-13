#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "api.h"
#include "impl.h"
#include "rng.h"
#include "littleendian.h"

static const size_t n = 128;

static void unpack_pk(pubkey_t *pk_out, const unsigned char *pk_in)
{

    size_t i;
    unsigned char *r = pk_in;

    for (i = 0; i < n >> 3; i++)
    {
        pk_out->h1[(i<<3)]=r[0]|(r[1]<<8)|(r[2]<<16)|((r[3]&63)<<24);
        pk_out->h1[(i<<3)+1]=(r[3]>>6)|(r[4]<<2)|(r[5]<<10)|(r[6]<<18)|((r[7]&15)<<26);
        pk_out->h1[(i<<3)+2]=(r[7]>>4)|(r[8]<<4)|(r[9]<<12)|(r[10]<<20)|((r[11]&3)<<28);
        pk_out->h1[(i<<3)+3]=(r[11]>>2)|(r[12]<<6)|(r[13]<<14)|(r[14]<<22);
        pk_out->h1[(i<<3)+4]=r[15]|(r[16]<<8)|(r[17]<<16)|((r[18]&63)<<24);
        pk_out->h1[(i<<3)+5]=(r[18]>>6)|(r[19]<<2)|(r[20]<<10)|(r[21]<<18)|((r[22]&15)<<26);
        pk_out->h1[(i<<3)+6]=(r[22]>>4)|(r[23]<<4)|(r[24]<<12)|(r[25]<<20)|((r[26]&3)<<28);
        pk_out->h1[(i<<3)+7]=(r[26]>>2)|(r[27]<<6)|(r[28]<<14)|(r[29]<<22);

        r += 30;
    }

    for (i = 0; i < n >> 3; i++)
    {
        pk_out->h2[(i<<3)]=r[0]|(r[1]<<8)|(r[2]<<16)|((r[3]&63)<<24);
        pk_out->h2[(i<<3)+1]=(r[3]>>6)|(r[4]<<2)|(r[5]<<10)|(r[6]<<18)|((r[7]&15)<<26);
        pk_out->h2[(i<<3)+2]=(r[7]>>4)|(r[8]<<4)|(r[9]<<12)|(r[10]<<20)|((r[11]&3)<<28);
        pk_out->h2[(i<<3)+3]=(r[11]>>2)|(r[12]<<6)|(r[13]<<14)|(r[14]<<22);
        pk_out->h2[(i<<3)+4]=r[15]|(r[16]<<8)|(r[17]<<16)|((r[18]&63)<<24);
        pk_out->h2[(i<<3)+5]=(r[18]>>6)|(r[19]<<2)|(r[20]<<10)|(r[21]<<18)|((r[22]&15)<<26);
        pk_out->h2[(i<<3)+6]=(r[22]>>4)|(r[23]<<4)|(r[24]<<12)|(r[25]<<20)|((r[26]&3)<<28);
        pk_out->h2[(i<<3)+7]=(r[26]>>2)|(r[27]<<6)|(r[28]<<14)|(r[29]<<22);

        r += 30;
    }
}

static void pack_sk(unsigned char *sk_out, const privkey_t *sk_in)
{
    size_t i;

    for (i = 0; i < n; i++)
    {
        *(sk_out++) = sk_in->x1[i];
    }
    for (i = 0; i < n; i++)
    {
        *(sk_out++) = sk_in->x2[i];
    }

    for (i = 0; i < n; i++)
    {
        *(sk_out++) = sk_in->F1[0][i];
    }
    for (i = 0; i < n; i++)
    {
        store_32(sk_out, sk_in->F1[1][i]);
        sk_out += 4;
    }
    for (i = 0; i < n; i++)
    {
        *(sk_out++) = sk_in->F2[0][i];
    }
    for (i = 0; i < n; i++)
    {
        store_32(sk_out, sk_in->F2[1][i]);
        sk_out += 4;
    }
    for (i = 0; i < n / 2; i++)
    {
        *(sk_out++) = sk_in->pkh[i];
    }
}

static void unpack_sk(privkey_t *sk_out, const unsigned char *sk_in)
{
    unsigned char *r = sk_in;
    size_t i;

    for (i = 0; i < n; i++)
    {
        sk_out->x1[i] = (int8_t)(*(r++));
    }
    for (i = 0; i < n; i++)
    {
        sk_out->x2[i] = (int8_t)(*(r++));
    }

    for (i = 0; i < n; i++)
    {
        sk_out->F1[0][i] = (int8_t)(*(r++));
    }
    for (i = 0; i < n; i++)
    {
        sk_out->F1[1][i] = (int32_t)load_32(r);
        r += 4;
    }
    for (i = 0; i < n; i++)
    {
        sk_out->F2[0][i] = (int8_t)(*(r++));
    }
    for (i = 0; i < n; i++)
    {
        sk_out->F2[1][i] = (int32_t)load_32(r);
        r += 4;
    }
    for (i = 0; i < n / 2; i++)
    {
        sk_out->pkh[i] = (uint8_t)(*(r++));
    }
}

static void pack_sig(unsigned char *sig_out, const signature_t *sig_in)
{
    size_t i;

    sig_out += pack_u(sig_out, sig_in->u, n);

    for (i = 0; i < n >> 3; i++)
    {
        sig_out[0]=sig_in->s[(i<<3)];
        sig_out[1]=(sig_in->s[(i<<3)]>>8)|(sig_in->s[(i<<3)+1]<<2);
        sig_out[2]=(sig_in->s[(i<<3)+1]>>6)|(sig_in->s[(i<<3)+2]<<4);
        sig_out[3]=(sig_in->s[(i<<3)+2]>>4)|(sig_in->s[(i<<3)+3]<<6);
        sig_out[4]=sig_in->s[(i<<3)+3]>>2;
        sig_out[5]=sig_in->s[(i<<3)+4];
        sig_out[6]=(sig_in->s[(i<<3)+4]>>8)|(sig_in->s[(i<<3)+5]<<2);
        sig_out[7]=(sig_in->s[(i<<3)+5]>>6)|(sig_in->s[(i<<3)+6]<<4);
        sig_out[8]=(sig_in->s[(i<<3)+6]>>4)|(sig_in->s[(i<<3)+7]<<6);
        sig_out[9]=sig_in->s[(i<<3)+7]>>2;

        sig_out += 10;
    }
}

static void unpack_sig(signature_t *sig_out, const unsigned char *sig_in)
{

    size_t i;
    unsigned char *r = sig_in;

    for (i = 0; i < n >> 3; i++)
    {
        sig_out->u[(i<<3)]=r[0]|(r[1]<<8)|(r[2]<<16)|((r[3]&63)<<24);
        sig_out->u[(i<<3)+1]=(r[3]>>6)|(r[4]<<2)|(r[5]<<10)|(r[6]<<18)|((r[7]&15)<<26);
        sig_out->u[(i<<3)+2]=(r[7]>>4)|(r[8]<<4)|(r[9]<<12)|(r[10]<<20)|((r[11]&3)<<28);
        sig_out->u[(i<<3)+3]=(r[11]>>2)|(r[12]<<6)|(r[13]<<14)|(r[14]<<22);
        sig_out->u[(i<<3)+4]=r[15]|(r[16]<<8)|(r[17]<<16)|((r[18]&63)<<24);
        sig_out->u[(i<<3)+5]=(r[18]>>6)|(r[19]<<2)|(r[20]<<10)|(r[21]<<18)|((r[22]&15)<<26);
        sig_out->u[(i<<3)+6]=(r[22]>>4)|(r[23]<<4)|(r[24]<<12)|(r[25]<<20)|((r[26]&3)<<28);
        sig_out->u[(i<<3)+7]=(r[26]>>2)|(r[27]<<6)|(r[28]<<14)|(r[29]<<22);

        r += 30;
    }

    for (i = 0; i < n >> 3; i++)
    {
        sig_out->s[(i<<3)]=r[0]|((r[1]&3)<<8);
        sig_out->s[(i<<3)+1]=(r[1]>>2)|((r[2]&15)<<6);
        sig_out->s[(i<<3)+2]=(r[2]>>4)|((r[3]&63)<<4);
        sig_out->s[(i<<3)+3]=(r[3]>>6)|(r[4]<<2);
        sig_out->s[(i<<3)+4]=r[5]|((r[6]&3)<<8);
        sig_out->s[(i<<3)+5]=(r[6]>>2)|((r[7]&15)<<6);
        sig_out->s[(i<<3)+6]=(r[7]>>4)|((r[8]&63)<<4);
        sig_out->s[(i<<3)+7]=(r[8]>>6)|(r[9]<<2);

        r += 10;
    }
}

int
crypto_sign_keypair(unsigned char *pk, unsigned char *sk)
{
    uint8_t seed[32];
    pubkey_t pk1;
    privkey_t sk1;

    randombytes(seed, 32);
    keygen(&pk1, &sk1, n, seed);
    pack_pk(pk, &pk1, n);
    pack_sk(sk, &sk1);

    return 0;
}

int
crypto_sign(unsigned char *sm, unsigned long long *smlen,
            const unsigned char *m, unsigned long long mlen,
            const unsigned char *sk)
{
    privkey_t sk1;
    uint8_t seed[32];
    signature_t sig;
    size_t i;

    for(i = 0; i < mlen; ++i)
        sm[CRYPTO_BYTES + mlen - 1 - i] = m[mlen - 1 - i];

    unpack_sk(&sk1, sk);
    randombytes(seed, 32);
    sign(&sig, &sk1, sm + CRYPTO_BYTES, mlen, n, seed);
    pack_sig(sm, &sig);
    *smlen = CRYPTO_BYTES + mlen;

    return 0;
}

int
crypto_sign_open(unsigned char *m, unsigned long long *mlen,
                 const unsigned char *sm, unsigned long long smlen,
                 const unsigned char *pk)
{
    pubkey_t pk1;
    signature_t sig;
    size_t i;

    if(smlen < CRYPTO_BYTES)
        goto badsig;

    *mlen = smlen - CRYPTO_BYTES;
    unpack_pk(&pk1, pk);
    unpack_sig(&sig, sm);
    if(!verify(&pk1, sm + CRYPTO_BYTES, *mlen, &sig, n))
        goto badsig;
    else {
        /* All good, copy msg, return 0 */
        for(i = 0; i < *mlen; ++i)
            m[i] = sm[CRYPTO_BYTES + i];
        return 0;
    }

badsig:
    /* Signature verification failed */
    *mlen = -1;
    for(i = 0; i < smlen; ++i)
        m[i] = 0;

    return -1;
}
