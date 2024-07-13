#include <string.h>
#include "api.h"
#include "rng.h"

int
crypto_sign_keypair(unsigned char *pk, unsigned char *sk)
{
  QRUOV_SEED sk_seed ; randombytes(sk_seed, QRUOV_SEED_LEN) ;
  QRUOV_SEED pk_seed ; randombytes(pk_seed, QRUOV_SEED_LEN) ;
  static QRUOV_P3 P3 ; // to avoid huge array in stack,
                       // (but problematic in mult-thread environment)

  QRUOV_KeyGen (sk_seed, pk_seed, P3) ;

  size_t sk_pool_bits = 0 ;
  store_QRUOV_SEED(sk_seed, sk, &sk_pool_bits) ;
  store_QRUOV_SEED(pk_seed, sk, &sk_pool_bits) ;

  size_t pk_pool_bits = 0 ;
  store_QRUOV_SEED(pk_seed, pk, &pk_pool_bits) ;
  store_QRUOV_P3  (P3,      pk, &pk_pool_bits) ;

  return 0 ;
}

int
crypto_sign(unsigned char *sm, unsigned long long *smlen,
            const unsigned char *m, unsigned long long mlen,
            const unsigned char *sk)
{

  size_t sk_pool_bits = 0 ;
  QRUOV_SEED sk_seed      ; restore_QRUOV_SEED(sk, &sk_pool_bits, sk_seed) ;
  QRUOV_SEED pk_seed      ; restore_QRUOV_SEED(sk, &sk_pool_bits, pk_seed) ;

  QRUOV_SEED vineger_seed ; randombytes(vineger_seed, QRUOV_SEED_LEN) ;
  QRUOV_SEED r_seed       ; randombytes(r_seed      , QRUOV_SEED_LEN) ;

  QRUOV_SIGNATURE sig     ;
  QRUOV_Sign(sk_seed, pk_seed, vineger_seed, r_seed, m, mlen, sig) ;

  size_t sig_pool_bits = 0 ;
  store_QRUOV_SIGNATURE(sig, sm, &sig_pool_bits) ;

  memcpy(sm+CRYPTO_BYTES,m,mlen) ;
  *smlen = CRYPTO_BYTES+mlen ;

  return 0 ;
}

int
crypto_sign_open(unsigned char *m, unsigned long long *mlen,
                 const unsigned char *sm, unsigned long long smlen,
                 const unsigned char *pk){

  size_t pk_pool_bits = 0 ;
  QRUOV_SEED pk_seed      ; restore_QRUOV_SEED(pk, &pk_pool_bits, pk_seed) ;
  static QRUOV_P3 P3      ; // to avoid huge array in stack
                            // (but problematic in mult-thread environment)

  restore_QRUOV_P3(pk, &pk_pool_bits, P3) ;

  size_t sig_pool_bits = 0 ;
  QRUOV_SIGNATURE sig ; restore_QRUOV_SIGNATURE(sm, &sig_pool_bits, sig) ;

  if(QRUOV_Verify(pk_seed, P3, sm+CRYPTO_BYTES, smlen-CRYPTO_BYTES, sig)){
    *mlen = smlen - CRYPTO_BYTES ;
    memcpy(m, sm+CRYPTO_BYTES, *mlen) ;
  }else{
    return 1 ;
  }

  return 0 ;
}
