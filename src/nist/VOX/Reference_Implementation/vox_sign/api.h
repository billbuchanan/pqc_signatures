#ifdef PARAM_SET_VOX128
#define CRYPTO_PUBLICKEYBYTES   9104
#define CRYPTO_SECRETKEYBYTES   35120
#define CRYPTO_BYTES            102
#define CRYPTO_ALGNAME          "VOX-128"
#endif

#ifdef PARAM_SET_VOX192
#define CRYPTO_PUBLICKEYBYTES   30351
#define CRYPTO_SECRETKEYBYTES   111297
#define CRYPTO_BYTES            184
#define CRYPTO_ALGNAME          "VOX-192"
#endif

#ifdef PARAM_SET_VOX256
#define CRYPTO_PUBLICKEYBYTES   82400
#define CRYPTO_SECRETKEYBYTES   292160
#define CRYPTO_BYTES            300
#define CRYPTO_ALGNAME          "VOX-256"
#endif

int crypto_sign_keypair(unsigned char *pk, unsigned char *sk);

int crypto_sign(unsigned char *sm, unsigned long long *smlen,
                const unsigned char *m, unsigned long long mlen,
                const unsigned char *sk);

int crypto_sign_open(unsigned char *m, unsigned long long *mlen,
                     const unsigned char *sm, unsigned long long smlen,
                     const unsigned char *pk);
