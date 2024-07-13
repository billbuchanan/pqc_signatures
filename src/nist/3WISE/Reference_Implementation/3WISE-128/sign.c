#include <openssl/sha.h>
#include <openssl/evp.h>
#include <string.h>
#include "gen.h"
#include "serializer.h"
#include "api.h"
#include "rng.h"

fmpz_t p;
flint_rand_t state;

void seed_flint(){
    union uchar_to_ulong
    {
        unsigned char charNum[sizeof(unsigned long)];
        unsigned long longNum;
    } seed_long;

    unsigned long seed1 = 0, seed2 = 0;

    randombytes(seed_long.charNum, sizeof(unsigned long));
    seed1 = seed_long.longNum;

    randombytes(seed_long.charNum, sizeof(unsigned long));
    seed2 = seed_long.longNum;

    // Now seed flint with two unsigned longs platform agnostic seeds (64bit in x64 and 32 bit in x86)
    flint_randinit(state);
    flint_randseed(state, seed1, seed2 );
}

void sha256(const unsigned char* message, unsigned char *hash, const unsigned int mlen)
{
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();;
    unsigned int sha256_digest_len = EVP_MD_size(EVP_sha256());
    EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL);
    EVP_DigestUpdate(mdctx, message, mlen);
    EVP_DigestFinal_ex(mdctx, hash, &sha256_digest_len);
    EVP_MD_CTX_free(mdctx);
}

int crypto_sign(unsigned char *sm, unsigned long long *smlen,
            const unsigned char *m, unsigned long long mlen,
            const unsigned char *sk){
    
    fmpz_mod_mat_t inv_T, inv_S;
    fmpz_mod_mat_init(inv_T, N, N, p);
    fmpz_mod_mat_init(inv_S, N, N, p);

    fmpz_mod_ctx_t ord_ctx, p_ctx;
    fmpz_t order; fmpz_init(order); fmpz_set_ui(order, P-1);
    fmpz_t exp; fmpz_init(exp); fmpz_set_ui(exp, 3);
    fmpz_t invexp; fmpz_init(invexp);
    fmpz_mod_ctx_init(ord_ctx, order);
    fmpz_mod_ctx_init(p_ctx, p);
    fmpz_mod_inv(invexp, exp, ord_ctx);

    fmpz *message = _fmpz_vec_init(N);
    fmpz *signature = _fmpz_vec_init(N);
    fmpz *tmp = _fmpz_vec_init(N);

    // must truncate 256 to 128 bit
    #define SHA128_DIGEST_LENGTH 16
    unsigned char hash[SHA256_DIGEST_LENGTH]={0}, hash_hex[2*SHA128_DIGEST_LENGTH+1]={0};
    sha256(m, hash, mlen);

    for (int i = 0; i < SHA128_DIGEST_LENGTH; i++) //truncate to 128 bit (16 byte)
        sprintf((unsigned char*) &hash_hex[i*2],"%02x", (unsigned char) hash[i]);

    for (int i = 0; i < N; i++){
        unsigned char buff[2] = {0};
        memcpy(buff, hash_hex + i, sizeof(unsigned char));
        fmpz_set_str(&message[i], (const char*) &buff, 16);
    }

    DecodePrivateKey(inv_T, inv_S, sk);
    
    fmpz_mod_mat_mul_fmpz_vec(tmp, inv_T, message, N);
    
    for (int i = 0; i < N; i++)
        fmpz_mod_pow_fmpz(&tmp[i], &tmp[i], invexp, p_ctx);

    fmpz_mod_mat_mul_fmpz_vec(signature, inv_S, tmp, N);
    
    unsigned char sig[N]={0};
    for (int i = 0; i < N; i++)
        fmpz_get_str((char*) &sig[i], 17, &signature[i]);

    memcpy(sm, sig, CRYPTO_BYTES);
    memcpy(sm + CRYPTO_BYTES, m, mlen);

    *smlen = CRYPTO_BYTES + mlen; // (x, m)
    
    fmpz_mod_mat_clear(inv_T);
    fmpz_mod_mat_clear(inv_S);

    _fmpz_vec_clear(signature, N);
    _fmpz_vec_clear(message, N);
    _fmpz_vec_clear(tmp, N);

    fmpz_clear(order);
    fmpz_clear(exp);
    fmpz_clear(invexp);

    fmpz_mod_ctx_clear(ord_ctx);
    fmpz_mod_ctx_clear(p_ctx);

    return 0;
    
}

int crypto_sign_open(unsigned char *m, unsigned long long *mlen,
                 const unsigned char *sm, unsigned long long smlen,
                 const unsigned char *pk){

    int ret = 0; // return 0 when msg_vec matches verify_vec otherwise ERROR as -1
    
    fmpz *in_vec = _fmpz_vec_init(PK_ROW_SIZE);
    fmpz *signature = _fmpz_vec_init(N);
    fmpz *message = _fmpz_vec_init(N);
    fmpz *verify_vec = _fmpz_vec_init(N);

    fmpz_mod_mat_t pub_key;
    fmpz_mod_mat_init(pub_key, N, PK_ROW_SIZE, p);
    DecodePublicKey(pub_key, pk);

    // encode to fmpz*
    for (int i = 0; i < N; i++){
        unsigned char buff[2] = {0};
        memcpy(buff, sm + i, sizeof(unsigned char));
        fmpz_set_str(&signature[i], (const char*) &buff, 17);
    }
    
    KroneckerProductVec(signature, in_vec);
    fmpz_mod_mat_mul_fmpz_vec(verify_vec, pub_key, in_vec, PK_ROW_SIZE); // Eval P(X)

    *mlen = smlen - CRYPTO_BYTES;
    memcpy(m, sm + CRYPTO_BYTES , *mlen);
    
    #define SHA128_DIGEST_LENGTH 16
    unsigned char hash[SHA256_DIGEST_LENGTH]={0}, hash_hex[2*SHA128_DIGEST_LENGTH+1]={0};
    sha256(m, hash, *mlen);

    for (int i = 0; i < SHA128_DIGEST_LENGTH; i++)
        sprintf((unsigned char*) &hash_hex[i*2],"%02x", (unsigned char) hash[i]);

    for (int i = 0; i < N; i++){
        unsigned char buff[2] = {0};
        memcpy(buff, hash_hex + i, sizeof(unsigned char));
        fmpz_set_str(&message[i], (const char*) &buff, 16);
    }
    
    // we should have same values in in verify_vec ( P(X) ) and message ( Hex(H(M)) mod p )
    if (!_fmpz_vec_equal(message, verify_vec, N))
        ret -=1;

    fmpz_mod_mat_clear(pub_key);
    _fmpz_vec_clear(signature, N);
    _fmpz_vec_clear(message, N);
    _fmpz_vec_clear(verify_vec, N);
    _fmpz_vec_clear(in_vec, PK_ROW_SIZE);

    return ret;
}

int crypto_sign_keypair(unsigned char *pk, unsigned char *sk){
    if (state != NULL){
        flint_randclear(state);
    }
    if (p != NULL){
        fmpz_clear(p);
    }
    
    seed_flint();
    fmpz_init(p); 
    fmpz_set_ui(p, P);

    fmpz_mod_mat_t inv_T, inv_S, pub_key;
    fmpz_mod_mat_init(inv_T, N, N, p);
    fmpz_mod_mat_init(inv_S, N, N, p);
    fmpz_mod_mat_init(pub_key, N, PK_ROW_SIZE, p);

    GeneratePublicKey(pub_key, inv_T, inv_S);

    EncodePublicKey(pub_key, pk);
    EncodePrivateKey(inv_T, inv_S, sk);

    fmpz_mod_mat_clear(inv_T);
    fmpz_mod_mat_clear(inv_S);
    fmpz_mod_mat_clear(pub_key);

    return 0;
}