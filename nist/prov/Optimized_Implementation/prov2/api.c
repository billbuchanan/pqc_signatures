#include <string.h>
#include <stdio.h>

#include "api.h"

#include "prov.h"
#include "rng.h"

#ifdef SUPERCOP
    #include "crypto_sign.h"
#endif

static prov_param_t param = {200, 70, 8, 24, 24, 32, 48};

int crypto_sign_keypair(unsigned char *pk, unsigned char *sk) {
    prov_pkey_t pkey;
    prov_skey_t skey;
    prov_expanded_skey_t expanded_skey;
    
    prov_alloc_pkey(&param, &pkey);
    prov_alloc_skey(&param, &skey);
    prov_alloc_expanded_skey(&param, &expanded_skey);
    
    randombytes(skey.secret_seed,param.len_secret_seed);
    
    prov_keygen(&param, &pkey, &skey);
    prov_expand_secret(&param,&expanded_skey,&skey);
    
    prov_write_pkey(&param,pk,&pkey);
    
    prov_write_expanded_skey(&param,sk,&expanded_skey);
    
    prov_free_pkey(&pkey);
    prov_free_skey(&skey);
    prov_free_expanded_skey(&expanded_skey);
    
    return 0;
}

int crypto_sign(unsigned char *sm, unsigned long long *smlen,
            const unsigned char *m, unsigned long long mlen,
                const unsigned char *sk) {
    
    *smlen = mlen + CRYPTO_BYTES;
    memcpy(sm, m, mlen);
    
    prov_expanded_skey_t expanded_skey;
    prov_sig_t sig;
    
    prov_alloc_expanded_skey(&param, &expanded_skey);
    prov_alloc_sig(&param, &sig);
    
    prov_read_expanded_skey(&param,&expanded_skey,(unsigned char*)sk);
    
    int ret = prov_expanded_sign(&param, &sig, &expanded_skey, m, (size_t) mlen);
    
    prov_write_sig(&param,sm+mlen,&sig);

    prov_free_expanded_skey(&expanded_skey);
    prov_free_sig(&sig);
    
    return ret;
}

int crypto_sign_open(unsigned char *m, unsigned long long *mlen,
                 const unsigned char *sm, unsigned long long smlen,
                     const unsigned char *pk) {
    
    *mlen = smlen - CRYPTO_BYTES;
    memcpy(m, sm, *mlen);//SDFQSDFQSDF
    
    prov_pkey_t pkey;
    prov_sig_t sig;
    
    prov_alloc_pkey(&param, &pkey);
    prov_alloc_sig(&param, &sig);

    prov_read_pkey(&param,&pkey,(unsigned char*)pk);
    
    prov_read_sig(&param,&sig,(unsigned char*) (sm + (*mlen)));
    
    int ret = prov_verify(&param, &sig, &pkey, m, (size_t) *mlen);
    
    prov_free_pkey(&pkey);
    prov_free_sig(&sig);
    
    return ret;
}
