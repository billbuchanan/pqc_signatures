#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "prov.h"

#define P1SIZE(N,M,DELTA) ((((N)-(M)-(DELTA))*((N)-(M)-(DELTA)+1))/2)
#define P2SIZE(N,M,DELTA) (((N)-(M)-(DELTA))*((M)+(DELTA)))
#define P3SIZE(N,M,DELTA) ((((M)+(DELTA))*((M)+(DELTA)+1))/2)

int main(void) {

    prov_param_t param = {136, 46, 8, 16, 16, 24, 32};
    
    prov_pkey_t pkey;
    prov_skey_t skey;
    prov_sig_t sig;
    
    prov_alloc_pkey(&param, &pkey);
    prov_alloc_skey(&param, &skey);
    prov_alloc_sig(&param, &sig);
    
    unsigned long len_secret = param.len_public_seed+param.len_secret_seed+param.len_hashed_pkey+(P2SIZE(param.n,param.m,param.delta)*param.m);
    printf("Length of (extended) secret key: %lu\n",len_secret);
    unsigned long len_public = param.len_public_seed+(P3SIZE(param.n,param.m,param.delta)*param.m);
    printf("Length of public key: %lu\n",len_public);
    unsigned long len_sig = param.len_salt+param.n;
    printf("Length of sig: %lu\n",len_sig);
    
    srandom(123);
    unsigned int i;
    for (i=0;i<param.len_secret_seed;i++)
        skey.secret_seed[i] = (uint8_t) random();
    
    puts("KEYGEN");
    prov_keygen(&param, &pkey, &skey);
    puts("KEYGEN DONE");
    
    unsigned int len_msg = 5;
    uint8_t msg[] = {72, 101, 108, 108, 111};
    
    puts("SIGN");
    prov_sign(&param, &sig, &skey, msg, len_msg);
    puts("SIGN DONE");
    
    puts("VERIFY:");
    if (prov_verify(&param, &sig, &pkey, msg, len_msg) == 0)
        puts("SUCCESS");
    else
        puts("FAIL");
    
    return 0;
}
