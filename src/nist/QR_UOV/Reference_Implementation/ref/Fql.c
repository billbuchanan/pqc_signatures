#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include "qruov.h"

Fql Fql_zero ;

#if QRUOV_q == 7
  Fq Fq_inv_table[QRUOV_q] = {0,1,4,5,2,3,6} ;
#elif QRUOV_q == 31
  Fq Fq_inv_table[QRUOV_q] = {
     0, 1,16,21, 8,25,26, 9,
     4, 7,28,17,13,12,20,29,
     2,11,19,18,14, 3,24,27,
    22, 5, 6,23,10,15,30
  } ;
#elif QRUOV_q == 127
  Fq Fq_inv_table[QRUOV_q] = {
      0,  1, 64, 85, 32, 51,106,109, 16,113, 89,104, 53, 88,118, 17,
      8, 15,120,107,108,121, 52,116, 90, 61, 44, 80, 59, 92, 72, 41,
      4, 77, 71, 98, 60,103,117,114, 54, 31,124, 65, 26, 48, 58,100,
     45, 70, 94,  5, 22, 12, 40, 97, 93, 78, 46, 28, 36, 25, 84,125,

      2, 43,102, 91, 99, 81, 49, 34, 30, 87,115,105,122, 33, 57, 82, 
     27, 69, 79,101, 62,  3, 96, 73, 13, 10, 24, 67, 29, 56, 50,123,
     86, 55, 35, 68, 47, 83, 66, 37, 11, 75,  6, 19, 20,  7,112,119,
    110,  9, 39, 74, 23, 38, 14,111, 18, 21, 76, 95, 42, 63,126
  } ;
#else
#  error "Unsupported: QRUOV_q = " # QRUOV_q
#endif

/* -----------------------------------------------------
  pseudo random number generator
   ----------------------------------------------------- */
//
// This interface may be altered for constant-time sampling
//
/* random bits -> {0,...,q-1} */
Fq Fq_random(Fql_RANDOM_CTX ctx){
  Fq r = QRUOV_q ;
  uint64_t pool      = ctx->pool      ;
  unsigned pool_bits = ctx->pool_bits ;
  while(r == QRUOV_q){
    if (pool_bits < QRUOV_ceil_log_2_q) {
      uint8_t  tmp0 [4] ;
      MGF_yield (ctx->mgf_ctx, tmp0, 4) ;
      uint64_t tmp1 = ((uint32_t)tmp0[0]    ) |
                      ((uint32_t)tmp0[1]<< 8) |
                      ((uint32_t)tmp0[2]<<16) |
                      ((uint32_t)tmp0[3]<<24) ;
      tmp1 <<= pool_bits ;
      pool |= tmp1 ;
      pool_bits += 32 ;
    }
    r = pool & QRUOV_q ; // q must be 2^n-1
    pool >>= QRUOV_ceil_log_2_q ;
    pool_bits -= QRUOV_ceil_log_2_q ;
  }
  ctx->pool      = pool      ;
  ctx->pool_bits = pool_bits ;
  return r ;
}

Fql Fql_random(Fql_RANDOM_CTX ctx) {
  Fql g ;
  for(size_t i=0; i<QRUOV_L; i++) g.c[i] = Fq_random(ctx) ;
  return g ;
}

Fql * Fql_random_vector(Fql_RANDOM_CTX ctx, const size_t n0, Fql vec[]) {
  for(size_t i=0;i<n0;i++) vec[i] = Fql_random(ctx) ;
  return vec ;
}

//
// for constant-time sampling
//
Fq Fq_random_with_error (Fql_RANDOM_CTX ctx){
  uint64_t pool      = ctx->pool      ;
  unsigned pool_bits = ctx->pool_bits ;

  if (pool_bits < QRUOV_ceil_log_2_q) {
      uint8_t  tmp0 [4] ;
      MGF_yield (ctx->mgf_ctx, tmp0, 4) ;
      uint64_t tmp1 = ((uint32_t)tmp0[0]    ) |
                      ((uint32_t)tmp0[1]<< 8) |
                      ((uint32_t)tmp0[2]<<16) |
                      ((uint32_t)tmp0[3]<<24) ;
      tmp1 <<= pool_bits ;
      pool |= tmp1 ;
      pool_bits += 32 ;
  }
  Fq r = pool & QRUOV_q ; // q must be 2^n-1
  pool >>= QRUOV_ceil_log_2_q ;
  pool_bits -= QRUOV_ceil_log_2_q ;

  ctx->pool      = pool      ;
  ctx->pool_bits = pool_bits ;
  return r ;
}

//
// t   : (input)  constant-time sampling threshold
// vec : (output) Fq array : Fq vec[t] ;
// return : k \in \mathbb{N},
//    vec[i] \in {0,...,q-1} for i \in {0,...,k-1}
//
// When n (<=t) Fq elements must be drawn,
//   if k<n, sampling is failed, please abort.
//
// if t is large enough, the cumulative binomial probability
//
//    Pr[k<n] = \sum_{i=n}^t binom(t,n)((q-1)/q)^n(1/q)^n
//
// can be negligible.
//

size_t Fq_random_vector_for_constant_time_sampling(
  Fql_RANDOM_CTX ctx,      // random context
  const size_t   t,        // constant-time sampling threshold
  Fq             vec[]     // (output)
) {
  size_t j = 0 ;
  size_t k = t ;

  for(size_t i=0;i<t;i++){
    Fq r = Fq_random_with_error (ctx) ;
    if(r < QRUOV_q){ // need to implement constant-time
      vec[j++] = r ;
    }else{
      vec[--k] = r ;
    }
  }
  return k ;
}
