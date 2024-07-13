#pragma once
#include "qruov_misc.h"
#include "mgf.h"

/* =====================================================================
   F_q, F_q[X]/(f(X))
   ===================================================================== */

typedef uint32_t Fq ;

typedef struct Fql_t {
  Fq c[QRUOV_L] ;
} Fql ;

extern Fql Fql_zero ;

inline static Fq Fq_add(Fq X, Fq Y){
  int Z = (int) X + (int) Y ;
  Z %= QRUOV_q ;
  return (Fq)Z ;
}

inline static Fq Fq_sub(Fq X, Fq Y){
  int Z = (int) X - (int) Y + QRUOV_q ;
  Z %= QRUOV_q ;
  return (Fq)Z ;
}

inline static Fq Fq_mul(Fq X, Fq Y){
  int Z = (int) X * (int) Y ;
  Z %= QRUOV_q ;
  return (Fq)Z ;
}

inline static Fq Fq_inv(Fq X){
  extern Fq Fq_inv_table[QRUOV_q] ;
  return Fq_inv_table[X] ;
}

inline static void Fq_vector_add(const size_t n, Fq * X, Fq * Y, Fq * Z){
  for(size_t i=0; i<n; i++) Z[i] = Fq_add(X[i], Y[i]);
}

inline static void Fq_vector_sub(const size_t n, Fq * X, Fq * Y, Fq * Z){
  for(size_t i=0; i<n; i++) Z[i] = Fq_sub(X[i], Y[i]);
}

inline static Fq Fq_vector_inner_product(const size_t n, Fq * X, Fq * Y){
  uint64_t t = 0 ;
  for(size_t i=0;i<n;i++) t += (uint64_t) X[i] * (uint64_t) Y[i] ;
  return (Fq)(t % QRUOV_q) ;
}

inline static Fql Fql_add(Fql X, Fql Y){
  Fql Z ;
  Fq_vector_add(QRUOV_L, X.c, Y.c, Z.c);
  return Z ;
}

inline static Fql Fql_sub(Fql X, Fql Y){
  Fql Z ;
  Fq_vector_sub(QRUOV_L, X.c, Y.c, Z.c);
  return Z ;
}

inline static Fql Fql_mul(Fql X, Fql Y){
  uint64_t T[2*QRUOV_L-1] ;
  memset(T, 0, sizeof(uint64_t)*(2*QRUOV_L-1)) ;

  for(size_t i=0; i<QRUOV_L; i++){
    for(size_t j=0; j<QRUOV_L; j++){
      T[i+j] += (uint64_t) X.c[i] * (uint64_t) Y.c[j] ;
    }
  }

  for(size_t i = 2*QRUOV_L-2; i >= QRUOV_L; i--){
      T[i-QRUOV_L]          += QRUOV_fc0 * T[i] ;
      T[i-QRUOV_L+QRUOV_fe] += QRUOV_fc  * T[i] ;
  }

  Fql Z ;
  for(size_t i=0; i<QRUOV_L; i++){
    Z.c[i] = (Fq)(T[i] % QRUOV_q) ;
  }
  return Z ;
}

/* =====================================================================
   pseudo random number generator
   ===================================================================== */

TYPEDEF_STRUCT ( Fql_RANDOM_CTX,
  MGF_CTX   mgf_ctx ;
  unsigned  pool_bits ;
  uint64_t  pool ;
) ;

typedef uint8_t QRUOV_SEED  [QRUOV_SEED_LEN] ;
typedef uint8_t QRUOV_SALT  [QRUOV_SALT_LEN] ;

inline static void Fql_srandom_init(const uint8_t * seed, const size_t n0, Fql_RANDOM_CTX ctx){
  MGF_init(seed, n0, ctx->mgf_ctx) ;
  ctx->pool      = 0 ;
  ctx->pool_bits = 0 ;
  return ;
}

inline static void Fql_srandom(const QRUOV_SEED seed, Fql_RANDOM_CTX ctx){
  Fql_srandom_init(seed, QRUOV_SEED_LEN, ctx) ;
  return ;
}

inline static void Fql_srandom_update(const uint8_t * seed, const size_t n0, Fql_RANDOM_CTX ctx){
  MGF_update(seed, n0, ctx->mgf_ctx) ;
  return ;
}

/* random bits -> {0,...,q-1} */
extern Fq Fq_random (Fql_RANDOM_CTX ctx) ;

/* random bits -> (1) */
extern Fql   Fql_random (Fql_RANDOM_CTX ctx) ;
extern Fql * Fql_random_vector (Fql_RANDOM_CTX ctx, const size_t n0, Fql vec[]) ;

inline static void Fq_random_final (Fql_RANDOM_CTX ctx) {
  MGF_final(ctx->mgf_ctx) ;
  ctx->pool = 0 ;
}

inline static void Fql_random_final (Fql_RANDOM_CTX ctx) {
  Fq_random_final (ctx) ;
}

inline static void Fql_RANDOM_CTX_copy (Fql_RANDOM_CTX src, Fql_RANDOM_CTX dst) {
  memcpy(dst, src, sizeof(Fql_RANDOM_CTX)) ;
  MGF_CTX_copy(src->mgf_ctx, dst->mgf_ctx) ;
}
