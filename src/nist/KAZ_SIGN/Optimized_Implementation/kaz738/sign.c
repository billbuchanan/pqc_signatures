#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "gmp.h"
#include "api.h"
#include "kaz_api.h"

int
crypto_sign_keypair(unsigned char *pk, unsigned char *sk)
{
    KAZ_DS_KeyGen(pk, sk);
    //printf("crypto_sign_keypair OK\n");
    if(strlen(pk)!=0 || strlen(sk)!=0) return 0;
    else return -4;
}

int
crypto_sign(unsigned char *sm, unsigned long long *smlen,
            const unsigned char *m, unsigned long long mlen,
            const unsigned char *sk)
{
    //printf("crypto_sign IN (%d, %d, %d)\n", mlen, *smlen, 0);
    int status=KAZ_DS_SIGNATURE(sm, smlen, m, mlen, sk);
    //printf("crypto_sign OK (%d, %d, %d)\n", mlen, *smlen, status);
    if(*smlen>mlen && status==0) return 0;
    else return status;
}

int
crypto_sign_open(unsigned char *m, unsigned long long *mlen,
                 const unsigned char *sm, unsigned long long smlen,
                 const unsigned char *pk)
{
    //printf("crypto_sign_open IN (%d, %d, %d)\n", smlen, *mlen, 0);
    int status=KAZ_DS_VERIFICATION(m, mlen, sm, smlen, pk);
    //printf("crypto_sign_open OK (%d, %d, %d)\n", smlen, *mlen, status);
    if(status==0) return 0;
    else return status;
}
