#include <openssl/sha.h>
#include <openssl/evp.h>

#include "gen.h"
#include "serializer.h"
#include "api.h"
#include "rng.h"
#include "ntl_utils.h"

fq_ctx_t ctx;
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

    mzd_t *message = mzd_init(N, 1), 
    *out = mzd_init(N, 1),
    *intermediate = mzd_init(N, 1),
    *root_poly = mzd_init(N, 1),
    *inv_T = mzd_init(N, N),
    *inv_S = mzd_init(N, N),
    *private_poly = mzd_init((d+1)*N, 1),
    *inv_L1 = mzd_init(N, N);

    // vinegar is a N vector but with VIN_VARS random entries, rest of N - VIN_VARS entries are set to 0
    mzd_t *vinegar = mzd_init(N,1); 

    unsigned char hash[SHA256_DIGEST_LENGTH];
    sha256(m, hash, mlen);
    DecodePrivateKey(sk, inv_T, inv_S, inv_L1, private_poly);
    uchar_to_mzd(hash, message, N / 8);
    
    mzd_t *tmp = mzd_init(N,1), *term = mzd_init(N,1);

    bool has_root = false;
    while (!has_root){

        //mzd_randomize(vinegar);
        unsigned char vin_uchar[VIN_VARS/8] = {0};
        randombytes(vin_uchar, sizeof(vin_uchar));
        uchar_to_mzd(vin_uchar, vinegar, sizeof(vin_uchar));

        for (int i = VIN_VARS; i < N; i++)
            mzd_write_bit(vinegar, i, 0, 0); // set N - VIN_VARS entries to 0

        mzd_add(tmp, vinegar, message); //P(X) = H(m) - v = y

        #ifdef MULT_STRASSEN
            mzd_mul(term, inv_T, tmp, N); // inv_T . y = (L1 X L2L1) . (S X S) . (x X x)
        #else
            mzd_mul_naive(term, inv_T, tmp); // inv_T . y = (L1 X L2L1) . (S X S) . (x X x)
        #endif

        has_root = RootPrivatePoly(private_poly, root_poly, term, N, d, ctx);

    }

    #ifdef MULT_STRASSEN
        mzd_mul(intermediate, inv_L1, root_poly, N);
        mzd_mul(out, inv_S, intermediate, N);
    #else
        mzd_mul_naive(intermediate, inv_L1, root_poly);
        mzd_mul_naive(out, inv_S, intermediate);
    #endif

    mzd_to_uchar(out, sm, N / 8);
    mzd_to_uchar(vinegar, sm + N / 8, VIN_VARS / 8);
    memcpy(sm + CRYPTO_BYTES, m, mlen);

    *smlen = CRYPTO_BYTES + mlen; // (x, v, m) so Verify(x, v, m) = P(x) + v = H(m)

    mzd_free(tmp);
    mzd_free(term);
    mzd_free(message);
    mzd_free(vinegar);
    mzd_free(out);
    mzd_free(intermediate);
    mzd_free(root_poly);
    mzd_free(inv_T);
    mzd_free(inv_S);
    mzd_free(inv_L1);

    return 0;
    
}

int crypto_sign_open(unsigned char *m, unsigned long long *mlen,
                 const unsigned char *sm, unsigned long long smlen,
                 const unsigned char *pk){

    int ret = -1; // return any ERROR as -1

    mzd_t *vinegar = mzd_init(N,1), // N vector but with VIN_VARS set and rest N - VIN_VARS to 0
    *in_vec = mzd_init(N,1),
    *out = mzd_init(N,1),
    *pub_key = mzd_init(N, PK_ROW_SIZE);

    DecodePublicKey(pub_key, pk);

    // signature sm = (x, v, m) take (x,v)
    uchar_to_mzd(sm, in_vec, N / 8); // x
    uchar_to_mzd(sm + N / 8, vinegar, VIN_VARS / 8); // v

    for (int i = VIN_VARS; i < N; i++)
        mzd_write_bit(vinegar, i, 0, 0); // set N - VIN_VARS entries to 0

    mzd_t *tens_v1_v1 = mzd_init(PK_ROW_SIZE, 1);

    KroneckerProductVec(in_vec, in_vec, tens_v1_v1);

    #ifdef MULT_STRASSEN
        mzd_mul(in_vec, pub_key, tens_v1_v1, N); // Eval P(X)
    #else
        mzd_mul_naive(in_vec, pub_key, tens_v1_v1); // Eval P(X)
    #endif

    mzd_add(out, in_vec, vinegar); // P(x) + v

    *mlen = smlen - CRYPTO_BYTES;
    memcpy(m, sm + CRYPTO_BYTES , *mlen); // map m from signature (x, v, m);

    unsigned char hash[SHA256_DIGEST_LENGTH];
    sha256(m, hash, *mlen);
    
    unsigned char recovered_digest[N / 8] = {0};
    mzd_to_uchar(out, recovered_digest, N / 8);

    // compare H(m) to the output P(x)+v
    // if equal ret = 0 and no ERROR
    // if unequal ret = -1 and returns ERROR
    if (!memcmp(hash, recovered_digest, N / 8)) 
        ret = 0;

    mzd_free(in_vec);
    mzd_free(pub_key);
    mzd_free(tens_v1_v1);
    mzd_free(out);
    mzd_free(vinegar);

    return ret;
}

int crypto_sign_keypair(unsigned char *pk, unsigned char *sk){
    fmpz_init_set_ui(p, 2);
    fq_ctx_init(ctx, p, N, "t");

    union uchar_to_ulong
    {
        unsigned char charNum[sizeof(unsigned long)];
        unsigned long longNum;
    } seed_long;
    
    randombytes(seed_long.charNum, sizeof(unsigned long));
    unsigned long seed = seed_long.longNum;
    InitNTL(N, ctx, seed);

    seed_flint();

    mzd_t *T = mzd_init(N, N),
    *S = mzd_init(N, N),
    *L1 = mzd_init(N, N),
    *L2 = mzd_init(N, N),
    *M = mzd_init(N, N*N),
    *pub_key = mzd_init(N, N * N),
    *short_pk = mzd_init(N, PK_ROW_SIZE),
    *inv_T = mzd_init(N, N),
    *inv_S = mzd_init(N, N),
    *inv_L1 = mzd_init(N, N),
    *id = mzd_init(N, N);

    fq_poly_t private_polynomial;
    fq_mat_t vander;
    fq_poly_init(private_polynomial, ctx);
    fq_mat_init(vander, N, d+1, ctx);

    GenRndMat(T);
    GenRndMat(S);
    GenRndMat(L1);
    
    Ident(id);
    mzd_invert_naive(inv_S, S, id);
    mzd_invert_naive(inv_T, T, id);
    mzd_invert_naive(inv_L1, L1, id);
    
    GenVandermonde(vander);
    GenRndLinPermPolyMat(L2, private_polynomial, vander);

    GenerateBasis(M);
    GenPublicKey(pub_key, M, T, S, L1, L2);
    AdjustPublicKey(pub_key, short_pk);

    EncodePublicKey(short_pk, pk);
    EncodePrivateKey(sk, inv_T, inv_S, inv_L1, private_polynomial, ctx);

    fq_mat_clear(vander, ctx);
    fq_poly_clear(private_polynomial, ctx);
    mzd_free(pub_key);
    mzd_free(short_pk);
    mzd_free(T);
    mzd_free(S);
    mzd_free(L1);
    mzd_free(L2);
    mzd_free(M);
    mzd_free(id);
    mzd_free(inv_S);
    mzd_free(inv_T);
    mzd_free(inv_L1);
    
    fq_ctx_clear(ctx);
    fmpz_clear(p);
    flint_randclear(state);

    return 0;
}