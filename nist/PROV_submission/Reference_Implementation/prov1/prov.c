#include "prov.h"
#include "rng.h"

#include <string.h>

#ifdef PROV_BENCH
#include "bench.h"
#define PROV_MALLOC bench_malloc
#define PROV_FREE bench_free
#else
#define PROV_MALLOC malloc
#define PROV_FREE free
#endif

#define P1SIZE(N,M,DELTA) ((((N)-(M)-(DELTA))*((N)-(M)-(DELTA)+1))/2)
#define P2SIZE(N,M,DELTA) (((N)-(M)-(DELTA))*((M)+(DELTA)))
#define P3SIZE(N,M,DELTA) ((((M)+(DELTA))*((M)+(DELTA)+1))/2)
#define OSIZE(N,M,DELTA) (((M)+(DELTA))*((N)-(M)-(DELTA)))

void prov_alloc_skey(prov_param_t *param, prov_skey_t *skey) {
    skey->public_seed = (uint8_t*) PROV_MALLOC(param->len_public_seed);
    skey->secret_seed = (uint8_t*) PROV_MALLOC(param->len_secret_seed);
    skey->hashed_pkey = (uint8_t*) PROV_MALLOC(param->len_hashed_pkey);
}

void prov_free_skey(prov_skey_t *skey) {
    PROV_FREE(skey->public_seed);
    PROV_FREE(skey->secret_seed);
    PROV_FREE(skey->hashed_pkey);
}

void prov_alloc_expanded_skey(prov_param_t *param, prov_expanded_skey_t *expanded_skey) {
    expanded_skey->public_seed = (uint8_t*) PROV_MALLOC(param->len_public_seed);
    expanded_skey->secret_seed = (uint8_t*) PROV_MALLOC(param->len_secret_seed);
    expanded_skey->hashed_pkey = (uint8_t*) PROV_MALLOC(param->len_hashed_pkey);
    expanded_skey->aux = (uint8_t*) PROV_MALLOC(P2SIZE(param->n,param->m,param->delta)*param->m);
}

void prov_free_expanded_skey(prov_expanded_skey_t *expanded_skey) {
    PROV_FREE(expanded_skey->public_seed);
    PROV_FREE(expanded_skey->secret_seed);
    PROV_FREE(expanded_skey->hashed_pkey);
    PROV_FREE(expanded_skey->aux);
}

void prov_alloc_pkey(prov_param_t *param, prov_pkey_t *pkey) {
    pkey->public_seed = (uint8_t*) PROV_MALLOC(param->len_public_seed);
    pkey->p3 = (uint8_t*) PROV_MALLOC(P3SIZE(param->n,param->m,param->delta)*param->m);
}

void prov_free_pkey(prov_pkey_t *pkey) {
    PROV_FREE(pkey->public_seed);
    PROV_FREE(pkey->p3);
}

void prov_alloc_expanded_pkey(prov_param_t *param, prov_expanded_pkey_t *expanded_pkey) {
    expanded_pkey->public_seed = (uint8_t*) PROV_MALLOC(param->len_public_seed);
    expanded_pkey->p1 = (uint8_t*) PROV_MALLOC(P1SIZE(param->n,param->m,param->delta)*param->m);
    expanded_pkey->p2 = (uint8_t*) PROV_MALLOC(P2SIZE(param->n,param->m,param->delta)*param->m);
    expanded_pkey->p3 = (uint8_t*) PROV_MALLOC(P3SIZE(param->n,param->m,param->delta)*param->m);
}

void prov_free_expanded_pkey(prov_expanded_pkey_t *expanded_pkey) {
    PROV_FREE(expanded_pkey->public_seed);
    PROV_FREE(expanded_pkey->p1);
    PROV_FREE(expanded_pkey->p2);
    PROV_FREE(expanded_pkey->p3);
}

void prov_alloc_sig(prov_param_t *param, prov_sig_t *sig) {
    sig->sig = (uint8_t*) PROV_MALLOC(param->n);
    sig->salt = (uint8_t*) PROV_MALLOC(param->len_salt);
}

void prov_free_sig(prov_sig_t *sig) {
    PROV_FREE(sig->sig);
    PROV_FREE(sig->salt);
}

void prov_expand_p1_p2(prov_param_t *param, prov_expanded_pkey_t *expanded_pkey) {
    HashInstance ctx;
    HashInit(&ctx, HASH_PREFIX_P1_P2);
    HashUpdate(&ctx, expanded_pkey->public_seed, param->len_public_seed);
    HashFinal(&ctx);
    HashSqueeze(&ctx, expanded_pkey->p1, P1SIZE(param->n,param->m,param->delta)*param->m);
    HashSqueeze(&ctx, expanded_pkey->p2, P2SIZE(param->n,param->m,param->delta)*param->m);
}

void prov_expand_oil(prov_param_t *param, uint8_t *mat_o, uint8_t *secret_seed) {
    HashInstance ctx;
    HashInit(&ctx, HASH_PREFIX_OIL);
    HashUpdate(&ctx, secret_seed, param->len_secret_seed);
    HashFinal(&ctx);
    HashSqueeze(&ctx, mat_o, OSIZE(param->n,param->m,param->delta));
}

void prov_compute_p3(uint16_t oil_size, uint16_t vinegar_size, uint8_t p3[oil_size][oil_size], uint8_t p1[vinegar_size][vinegar_size], uint8_t p2[oil_size][vinegar_size], uint8_t mat_o[oil_size][vinegar_size], uint8_t mat_temp[oil_size][vinegar_size]) {
    //note: all matrices are represented by their transpose compared to spec
    matrix_mul_transpose(oil_size,vinegar_size,vinegar_size,mat_temp,mat_o,p1,MATRIX_ASSIGN);
    matrix_mul_transpose(oil_size,oil_size,vinegar_size,p3,mat_temp,mat_o,MATRIX_ASSIGN);
    matrix_mul_transpose(oil_size,oil_size,vinegar_size,p3,mat_o,p2,MATRIX_ADD);
    matrix_sym(oil_size,p3);
}

int prov_keygen(prov_param_t *param, prov_pkey_t *pkey, prov_skey_t *skey) {
    unsigned int i;
    
    HashInstance ctx;
    HashInit(&ctx, HASH_PREFIX_SECRET);
    HashUpdate(&ctx, skey->secret_seed, param->len_secret_seed);
    HashFinal(&ctx);
    HashSqueeze(&ctx, pkey->public_seed, param->len_public_seed);
    
    for (i=0;i<param->len_public_seed;i++)
        skey->public_seed[i] = pkey->public_seed[i];
    
    uint16_t oil_size = param->m + param->delta;
    uint16_t vinegar_size = param->n - oil_size;
    uint8_t (*mat_o)[oil_size][vinegar_size] = PROV_MALLOC(sizeof(*mat_o));
    
    prov_expand_oil(param, (uint8_t*) mat_o, skey->secret_seed);
    
    prov_expanded_pkey_t expanded_pkey;
    prov_alloc_expanded_pkey(param,&expanded_pkey);
    for (i=0;i<param->len_public_seed;i++)
        expanded_pkey.public_seed[i] = pkey->public_seed[i];
    prov_expand_p1_p2(param,&expanded_pkey);
    
    uint8_t (*mat_p1)[vinegar_size][vinegar_size] = PROV_MALLOC(sizeof(*mat_p1));
    uint8_t (*mat_p3)[oil_size][oil_size] = PROV_MALLOC(sizeof(*mat_p3));
    uint8_t (*mat_temp)[oil_size][vinegar_size] = PROV_MALLOC(sizeof(*mat_temp));
    uint8_t *pos_p1 = expanded_pkey.p1;
    uint8_t *pos_p2 = expanded_pkey.p2;
    uint8_t *pos_p3 = pkey->p3;
    
    for (i=0;i<param->m;i++) {
        matrix_lower_to_full(vinegar_size,*mat_p1,pos_p1);
        
        prov_compute_p3(oil_size, vinegar_size, *mat_p3, *mat_p1, (uint8_t (*)[vinegar_size]) pos_p2, *mat_o, *mat_temp);
        
        matrix_full_to_lower(oil_size,pos_p3,*mat_p3);
        
        pos_p1 += P1SIZE(param->n,param->m,param->delta);
        pos_p2 += P2SIZE(param->n,param->m,param->delta);
        pos_p3 += P3SIZE(param->n,param->m,param->delta);
    }
    
    HashInit(&ctx, HASH_PREFIX_PUBLIC);
    HashUpdate(&ctx, pkey->public_seed, param->len_public_seed);
    HashUpdate(&ctx, pkey->p3, P3SIZE(param->n,param->m,param->delta)*param->m);
    HashFinal(&ctx);
    HashSqueeze(&ctx, skey->hashed_pkey, param->len_hashed_pkey);
    
    PROV_FREE(mat_o);
    PROV_FREE(mat_p1);
    PROV_FREE(mat_p3);
    PROV_FREE(mat_temp);
    prov_free_expanded_pkey(&expanded_pkey);
    
    return 0;
}

void prov_compute_aux(uint16_t oil_size, uint16_t vinegar_size, uint8_t aux[oil_size][vinegar_size], uint8_t p1[vinegar_size][vinegar_size], uint8_t p2[oil_size][vinegar_size], uint8_t o[oil_size][vinegar_size]) {
    matrix_add_own_transpose(vinegar_size,p1);//this destroys p1
    matrix_mul_transpose(oil_size,vinegar_size,vinegar_size,aux,o,p1,MATRIX_ASSIGN);
    matrix_add(oil_size,vinegar_size,aux,aux,p2);
}

void prov_expand_secret(prov_param_t *param, prov_expanded_skey_t *expanded_skey, prov_skey_t *skey) {
    unsigned int i;
    
    for (i=0;i<param->len_secret_seed;i++)
        expanded_skey->secret_seed[i] = skey->secret_seed[i];
    for (i=0;i<param->len_public_seed;i++)
        expanded_skey->public_seed[i] = skey->public_seed[i];
    for (i=0;i<param->len_hashed_pkey;i++)
        expanded_skey->hashed_pkey[i] = skey->hashed_pkey[i];

    prov_expanded_pkey_t expanded_pkey;
    prov_alloc_expanded_pkey(param,&expanded_pkey);
    
    for (i=0;i<param->len_public_seed;i++)
        expanded_pkey.public_seed[i] = expanded_skey->public_seed[i];

    uint16_t oil_size = param->m + param->delta;
    uint16_t vinegar_size = param->n - oil_size;
    uint8_t (*mat_o)[oil_size][vinegar_size] = PROV_MALLOC(sizeof(*mat_o));
    
    prov_expand_oil(param, (uint8_t*) mat_o, expanded_skey->secret_seed);

    prov_expand_p1_p2(param,&expanded_pkey);
    
    uint8_t (*mat_p1)[vinegar_size][vinegar_size] = PROV_MALLOC(sizeof(*mat_p1));
    uint8_t *pos_p1 = expanded_pkey.p1;
    uint8_t *pos_p2 = expanded_pkey.p2;
    uint8_t *pos_aux = expanded_skey->aux;
    
    for (i=0;i<param->m;i++) {
        matrix_lower_to_full(vinegar_size,*mat_p1,pos_p1);
        
        prov_compute_aux(oil_size, vinegar_size, (uint8_t (*)[vinegar_size]) pos_aux, *mat_p1, (uint8_t (*)[vinegar_size]) pos_p2, *mat_o);

        pos_p1 += P1SIZE(param->n,param->m,param->delta);
        pos_p2 += P2SIZE(param->n,param->m,param->delta);
        pos_aux += P2SIZE(param->n,param->m,param->delta);
    }
    
    PROV_FREE(mat_o);
    PROV_FREE(mat_p1);
    
    prov_free_expanded_pkey(&expanded_pkey);
}

int prov_sign(prov_param_t *param, prov_sig_t *sig, prov_skey_t *skey, const uint8_t *msg, size_t len_msg) {
    int ret;
    
    prov_expanded_skey_t expanded_skey;
    prov_alloc_expanded_skey(param, &expanded_skey);
    prov_expand_secret(param, &expanded_skey, skey);

    ret = prov_expanded_sign(param,sig,&expanded_skey,msg,len_msg);
    
    prov_free_expanded_skey(&expanded_skey);
    
    return ret;
}

int prov_expanded_sign(prov_param_t *param, prov_sig_t *sig, prov_expanded_skey_t *expanded_skey, const uint8_t *msg, size_t len_msg) {
    unsigned int i;
    unsigned int oil_size = param->m + param->delta;
    unsigned int vinegar_size = param->n - oil_size;
    HashInstance ctx_v, ctx_msg;
    
    uint8_t *offset = PROV_MALLOC(param->m);
    uint8_t *system = PROV_MALLOC(param->m * oil_size);
    uint8_t *target = PROV_MALLOC(param->m);
    
    //compute vinegar-dependent values
    prov_expanded_pkey_t expanded_pkey;
    prov_alloc_expanded_pkey(param,&expanded_pkey);
    for (i=0;i<param->len_public_seed;i++)
        expanded_pkey.public_seed[i] = expanded_skey->public_seed[i];
    prov_expand_p1_p2(param,&expanded_pkey);

    HashInit(&ctx_v, HASH_PREFIX_V);
    HashUpdate(&ctx_v, msg, len_msg);
    HashFinal(&ctx_v);
    HashSqueeze(&ctx_v, sig->sig, param->n);
    
    uint8_t *pos_p1 = expanded_pkey.p1;
    uint8_t *pos_system = system;
    uint8_t *pos_aux = expanded_skey->aux;
    
    for (i=0;i<param->m;i++) {
        offset[i] = matrix_lower_bilin(vinegar_size,pos_p1,sig->sig);
        pos_p1 += P1SIZE(param->n,param->m,param->delta);

        matrix_mul_transpose(1, oil_size, vinegar_size, (uint8_t (*)[oil_size]) pos_system, (uint8_t (*)[vinegar_size]) sig->sig, (uint8_t (*)[vinegar_size]) pos_aux, MATRIX_ASSIGN);
        
        pos_system += oil_size;
        pos_aux += oil_size*vinegar_size;
    }
        
    while(1) {
        
        //new salt
        HashSqueeze(&ctx_v, sig->salt, param->len_salt);
                
        //new hash
        HashInit(&ctx_msg, HASH_PREFIX_MESSAGE);
        HashUpdate(&ctx_msg, expanded_skey->hashed_pkey, param->len_hashed_pkey);
        HashUpdate(&ctx_msg, msg, len_msg);
        HashUpdate(&ctx_msg, sig->salt, param->len_salt);
        HashFinal(&ctx_msg);
        HashSqueeze(&ctx_msg, target, param->m);
                
        for (i=0;i<param->m;i++)
            target[i] = field_add(target[i], offset[i]);
        
        if (matrix_solve(param->m,oil_size,sig->sig+vinegar_size,(uint8_t (*)[oil_size])system,target) == 0)
            break;
    }
    
    uint8_t (*mat_o)[oil_size][vinegar_size] = PROV_MALLOC(sizeof(*mat_o));
    prov_expand_oil(param, (uint8_t*) mat_o, expanded_skey->secret_seed);
    matrix_mul(1,vinegar_size,oil_size,(uint8_t (*)[vinegar_size])sig->sig,(uint8_t (*)[oil_size])(sig->sig+vinegar_size),*mat_o,MATRIX_ADD);
        
    PROV_FREE(offset);
    PROV_FREE(system);
    PROV_FREE(target);
    PROV_FREE(mat_o);
    prov_free_expanded_pkey(&expanded_pkey);
    
    return 0;
}

void prov_expand_public(prov_param_t *param, prov_expanded_pkey_t *expanded_pkey, prov_pkey_t *pkey) {
    unsigned int i;
    
    for (i=0;i<param->len_public_seed;i++)
        expanded_pkey->public_seed[i] = pkey->public_seed[i];
    
    for (i=0;i<P3SIZE(param->n,param->m,param->delta)*param->m;i++)
        expanded_pkey->p3[i] = pkey->p3[i];
    
    prov_expand_p1_p2(param,expanded_pkey);
}

int prov_verify(prov_param_t *param, prov_sig_t *sig, prov_pkey_t *pkey, const uint8_t *msg, size_t len_msg) {
    int ret;
    
    prov_expanded_pkey_t expanded_pkey;
    prov_alloc_expanded_pkey(param, &expanded_pkey);
    prov_expand_public(param, &expanded_pkey, pkey);

    ret = prov_expanded_verify(param,sig,&expanded_pkey,msg,len_msg);
    
    prov_free_expanded_pkey(&expanded_pkey);
    
    return ret;
}

int prov_expanded_verify(prov_param_t *param, prov_sig_t *sig, prov_expanded_pkey_t *expanded_pkey, const uint8_t *msg, size_t len_msg) {
    unsigned int i;
    unsigned int oil_size = param->m + param->delta;
    unsigned int vinegar_size = param->n - oil_size;
    
    uint8_t *target = PROV_MALLOC(param->m);
    
    uint8_t *hashed_pkey = PROV_MALLOC(param->len_hashed_pkey);
    
    HashInstance ctx;
    HashInit(&ctx, HASH_PREFIX_PUBLIC);
    HashUpdate(&ctx, expanded_pkey->public_seed, param->len_public_seed);
    HashUpdate(&ctx, expanded_pkey->p3, P3SIZE(param->n,param->m,param->delta)*param->m);
    HashFinal(&ctx);
    HashSqueeze(&ctx, hashed_pkey, param->len_hashed_pkey);
    
    HashInit(&ctx, HASH_PREFIX_MESSAGE);
    HashUpdate(&ctx, hashed_pkey, param->len_hashed_pkey);
    HashUpdate(&ctx, msg, len_msg);
    HashUpdate(&ctx, sig->salt, param->len_salt);
    HashFinal(&ctx);
    HashSqueeze(&ctx, target, param->m);
    
    uint8_t *pos_p1 = expanded_pkey->p1;
    uint8_t *pos_p2 = expanded_pkey->p2;
    uint8_t *pos_p3 = expanded_pkey->p3;
    
    uint8_t res;
    
    for (i=0;i<param->m;i++) {
        res = 0;
        res ^= matrix_lower_bilin(vinegar_size,pos_p1,sig->sig);
        res ^= matrix_full_bilin(oil_size,vinegar_size,(uint8_t (*)[vinegar_size])pos_p2,sig->sig+vinegar_size,sig->sig);
        res ^= matrix_lower_bilin(oil_size,pos_p3,sig->sig+vinegar_size);
        
        if (res != target[i])
            return -1;
        
        pos_p1 += P1SIZE(param->n,param->m,param->delta);
        pos_p2 += P2SIZE(param->n,param->m,param->delta);
        pos_p3 += P3SIZE(param->n,param->m,param->delta);
    }
    
    PROV_FREE(target);
    PROV_FREE(hashed_pkey);
    
    return 0;
}

void prov_write_pkey(prov_param_t *param,uint8_t *pos,prov_pkey_t *pkey) {
    memcpy(pos,pkey->public_seed,param->len_public_seed);
    pos += param->len_public_seed;
    memcpy(pos,pkey->p3,P3SIZE(param->n,param->m,param->delta)*param->m);
}

void prov_read_pkey(prov_param_t *param,prov_pkey_t *pkey,uint8_t *pos) {
    memcpy(pkey->public_seed,pos,param->len_public_seed);
    pos += param->len_public_seed;
    memcpy(pkey->p3,pos,P3SIZE(param->n,param->m,param->delta)*param->m);
}

void prov_write_expanded_skey(prov_param_t *param,uint8_t *pos,prov_expanded_skey_t *expanded_skey) {
    memcpy(pos,expanded_skey->public_seed,param->len_public_seed);
    pos += param->len_public_seed;
    memcpy(pos,expanded_skey->secret_seed,param->len_secret_seed);
    pos += param->len_secret_seed;
    memcpy(pos,expanded_skey->hashed_pkey,param->len_hashed_pkey);
    pos += param->len_hashed_pkey;
    memcpy(pos,expanded_skey->aux,P2SIZE(param->n,param->m,param->delta)*param->m);
}

void prov_read_expanded_skey(prov_param_t *param,prov_expanded_skey_t *expanded_skey,uint8_t *pos) {
    memcpy(expanded_skey->public_seed,pos,param->len_public_seed);
    pos += param->len_public_seed;
    memcpy(expanded_skey->secret_seed,pos,param->len_secret_seed);
    pos += param->len_secret_seed;
    memcpy(expanded_skey->hashed_pkey,pos,param->len_hashed_pkey);
    pos += param->len_hashed_pkey;
    memcpy(expanded_skey->aux,pos,P2SIZE(param->n,param->m,param->delta)*param->m);
}

void prov_write_sig(prov_param_t *param,uint8_t *pos,prov_sig_t *sig) {
    memcpy(pos,sig->sig,param->n);
    pos += param->n;
    memcpy(pos,sig->salt,param->len_salt);
}

void prov_read_sig(prov_param_t *param,prov_sig_t *sig,uint8_t *pos) {
    memcpy(sig->sig,pos,param->n);
    pos += param->n;
    memcpy(sig->salt,pos,param->len_salt);
}
