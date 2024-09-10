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

    if(strlen(pk)!=0 || strlen(sk)!=0) return 0;
    else return -4;
}

int
crypto_sign(unsigned char *sm, unsigned long long *smlen,
            const unsigned char *m, unsigned long long mlen,
            const unsigned char *sk)
{
    int status=KAZ_DS_SIGNATURE(sm, smlen, m, mlen, sk);

    if(*smlen>mlen && status==0) return 0;
    else return status;
}

int
crypto_sign_open(unsigned char *m, unsigned long long *mlen,
                 const unsigned char *sm, unsigned long long smlen,
                 const unsigned char *pk)
{

    int status=KAZ_DS_VERIFICATION(m, mlen, sm, smlen, pk);

    if(status==0) return 0;
    else return status;
}
