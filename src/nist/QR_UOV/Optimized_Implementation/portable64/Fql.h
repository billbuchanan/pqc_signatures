#pragma once
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <inttypes.h>

#include "qruov_misc.h"
#include "mgf.h"

/* =====================================================================
   F_q, F_q[X]/(f(X))
   ===================================================================== */

typedef uint8_t Fq ;

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

#if ((QRUOV_q == 7)||(QRUOV_q == 31)) && QRUOV_L == 10 

//  g(x) = \sum_{i=0}^10 a[i] * x^i |-> \sum_{i=0}^10 a[i] * (2^16)^i 

typedef union Fql_t {
  uint64_t c64[ 3] ;
  uint32_t c32[ 6] ;
  uint16_t c16[12] ;
  uint8_t  c8 [24] ;
} Fql ;

typedef union Fql_accumulator_t {
  Fql      c       ;
  uint64_t c64[ 5] ;
  uint32_t c32[10] ;
  uint16_t c16[20] ;
  uint8_t  c8 [40] ;
} Fql_accumulator ;

extern Fql Fql_zero ;
extern Fql_accumulator Fql_accumulator_zero ;

inline int Fql_eq(Fql A, Fql B){
  return (A.c64[0] == B.c64[0])&&(A.c64[1] == B.c64[1])&&(A.c64[2] == B.c64[2]) ;
}

inline void Fql_print(char * header, Fql A){
  printf("%s%016lx,%016lx,%016lx\n",header,A.c64[2], A.c64[1], A.c64[0]);
}

inline void Fql_accumulator_print(char * header, Fql_accumulator A){
  printf("%s%016lx,%016lx,%016lx,%016lx,%016lx\n",header, A.c64[4], A.c64[3], A.c64[2], A.c64[1], A.c64[0]);
}

#define WORD_ORDER_LITTLE(i,j) ((i*4)+j)
#define WORD_ORDER_BIG(i,j)    ((i*4)+(3-j))

#if   __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#  define WORD_ORDER(i,j) WORD_ORDER_LITTLE(i,j)
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#  define WORD_ORDER(i,j) WORD_ORDER_BIG(i,j)
#else 
#  error "unsupported WORD_ORDER()"
#endif

inline static Fql Fq2Fql(uint16_t c[QRUOV_L]){
  Fql Z ; // = Fql_zero ;
  int l ;
  for(l=0;l<QRUOV_L;l++) {
    int j = l &  3 ;
    int i = l >> 2 ;
    int k = WORD_ORDER(i,j) ;
    Z.c16[k] = c[l] ;
  }
  for(   ;l<12;l++) {
    int j = l &  3 ;
    int i = l >> 2 ;
    int k = WORD_ORDER(i,j) ;
    Z.c16[k] = 0 ;
  }
  return Z ;
}

inline static uint16_t Fql2Fq(Fql Z, int l){
  int j = l &  3 ;
  int i = l >> 2 ;
  int k = WORD_ORDER(i,j) ;
  return Z.c16[k] ;
}

#  define Fql_mask0    ( (uint64_t)QRUOV_q     )
#  define Fql_mask1    (((uint64_t)QRUOV_q)<<16)
#  define Fql_mask2    (((uint64_t)QRUOV_q)<<32)
#  define Fql_mask3    (((uint64_t)QRUOV_q)<<48)
#  define Fql_mask     (Fql_mask0|Fql_mask1|Fql_mask2|Fql_mask3)

inline static Fql Fql_reduction(Fql Z){
  int l ;
  for(l=0;l<QRUOV_L;l++){
    int j = l &  3 ;
    int i = l >> 2 ;
    int k = WORD_ORDER(i,j) ;
    Z.c16[k] %= QRUOV_q ;
  }
  for(   ;l<12;l++) {
    int j = l &  3 ;
    int i = l >> 2 ;
    int k = WORD_ORDER(i,j) ;
    Z.c16[k] = 0 ;
  }
  return Z ;
}

/*
inline static Fql Fql_reduction_0(Fql Z_){
  for(int k=0;k<3;k++){
    uint64_t * Z = Z_.c64 + k ;
    *Z = (*Z & Fql_mask) + ((*Z & ~Fql_mask) >> QRUOV_ceil_log_2_q) ;
    if((*Z&Fql_mask0)==Fql_mask0) *Z^=Fql_mask0 ;
    if((*Z&Fql_mask1)==Fql_mask1) *Z^=Fql_mask1 ;
    if((*Z&Fql_mask2)==Fql_mask2) *Z^=Fql_mask2 ;
    if((*Z&Fql_mask3)==Fql_mask3) *Z^=Fql_mask3 ;
  }
  return Z_ ;
}
*/

inline static Fql Fql_add(Fql X, Fql Y){
  Fql Z ;
  for(int k=0; k<3; k++) Z.c64[k]  = X.c64[k] + Y.c64[k] ;
  return Fql_reduction(Z) ;
}

inline static Fql Fql_sub(Fql X, Fql Y){
  Fql Z ;
  for(int k=0; k<3; k++) Z.c64[k] = Fql_mask + X.c64[k] - Y.c64[k] ;
  return Fql_reduction(Z) ;
}

inline static Fql_accumulator Fql_accumulator_add(Fql_accumulator X, Fql_accumulator Y){
  Fql_accumulator Z = X ;
  for(int i=0; i<5; i++) Z.c64[i] += Y.c64[i] ;
  return Z ;
}

inline static Fql_accumulator Fql_accumulator_mul(Fql X, Fql Y){
  Fql_accumulator Z = Fql_accumulator_zero ;
  for(int i=0; i<3; i++){
    for(int j=0; j<3; j++) {
      UINT128_T ZZ  = (UINT128_T)X.c64[i] * (UINT128_T)Y.c64[j] ;
      Z.c64[i+j  ] += (uint64_t)ZZ ;
      Z.c64[i+j+1] += (uint64_t)(ZZ>>64) ;
    }
  }
  return Z ;
}

inline static Fql_accumulator Fql_accumulator_reduce_0(Fql_accumulator Z){
  int l ;
  for(l=0;l<19;l++){
    int j = l &  3 ;
    int i = l >> 2 ;
    int k = WORD_ORDER(i,j) ;
    Z.c16[k] %= QRUOV_q ;
  }
  for(  ;l<20;l++){
    int j = l &  3 ;
    int i = l >> 2 ;
    int k = WORD_ORDER(i,j) ;
    Z.c16[k] = 0 ;
  }
  return Z ;
}

#  if QRUOV_q == 7

inline static Fql Fql_accumulator_reduce_1(Fql_accumulator Z){
  uint64_t T2 = Z.c64[2] ;
  uint64_t T3 = Z.c64[3] ;
  uint64_t T4 = Z.c64[4] ;

  uint64_t U3 = (T2>>32) | (T3<<32) ;
  uint64_t U4 = (T3>>32) | (T4<<32) ;
  uint64_t U5 = (T4>>32) ;

  uint64_t V3 =            (U3<<17) ;
  uint64_t V4 = (U3>>47) | (U4<<17) ;
  uint64_t V5 = (T4>>15) ;

  Z.c64[2] &= 0x00000000FFFFFFFFULL ;

  Z.c64[0] += U3 ;
  Z.c64[1] += U4 ;
  Z.c64[2] += U5 ;

  Z.c64[0] += V3 ;
  Z.c64[1] += V4 ;
  Z.c64[2] += V5 ;

  return Fql_reduction(Z.c) ;
}

inline static Fql Fql_mul_1(Fql X, Fql Y){
  uint16_t Z[19] ;
  for(int i=0; i<19; i++) Z[i] = 0 ;

  for(int i=0; i<10; i++){
    for(int j=0; j<10; j++){
      uint32_t ZZ = (uint32_t)Fql2Fq(X,i) * (uint32_t)Fql2Fq(Y,j) ;
      Z[i+j  ] += (uint16_t)ZZ ;
      Z[i+j+1] += (uint16_t)(ZZ>>16) ;
    }
  }
  for(int i=0; i< 9; i++) Z[i] += Z[i+10] ;
  for(int i=1; i<10; i++) Z[i] += Z[i+ 9] * 2 ;
  for(int i=0; i<10; i++) Z[i] %= QRUOV_q ;
  return Fq2Fql(Z) ;
}

inline static Fql Fql_accumulator_reduce(Fql_accumulator Z){
  return Fql_accumulator_reduce_1(Fql_accumulator_reduce_0(Z)) ;
}

inline static Fql Fql_mul(Fql X, Fql Y){
  Fql_accumulator Z = Fql_accumulator_mul(X, Y) ;
  return Fql_accumulator_reduce_1(Z);
}

#  elif QRUOV_q == 31

inline static Fql Fql_accumulator_reduce_1(Fql_accumulator Z){
  uint64_t T2 = Z.c64[2] ;
  uint64_t T3 = Z.c64[3] ;
  uint64_t T4 = Z.c64[4] ;

  uint64_t U3 = (T2>>32) | (T3<<32) ;
  uint64_t U4 = (T3>>32) | (T4<<32) ;
  uint64_t U5 = (T4>>32) ;

  uint64_t V3 =            (U3<<48) ;
  uint64_t V4 = (U3>>16) | (U4<<48) ;
  uint64_t V5 = (U4>>16) | (U5<<48) ;

  uint64_t W3 = V3 * 5 ;
  uint64_t W4 = V4 * 5 ;
  uint64_t W5 = V5 * 5 ;

  uint64_t S3 = (W5>>32) ;
  uint64_t S4 = S3 * 5 ;
           S3 |= (S4<<48) ;
           S4 >>= 16 ;

  Z.c64[0] += U3 ;
  Z.c64[1] += U4 ;
  Z.c64[2] += U5 ;

  Z.c64[0] += W3 ;
  Z.c64[1] += W4 ;
  Z.c64[2] += W5 ;

  Z.c64[0] += S3 ;
  Z.c64[1] += S4 ;
  Z.c64[2] &= 0x00000000FFFFFFFFULL ;

  return Fql_reduction(Z.c) ;
}

inline static Fql Fql_mul_1(Fql X, Fql Y){
  uint32_t Z[19] ;
  for(int i=0; i<19; i++) Z[i] = 0 ;

  for(int i=0; i<10; i++){
    for(int j=0; j<10; j++){
      uint32_t ZZ = (uint32_t)Fql2Fq(X,i) * (uint32_t)Fql2Fq(Y,j) ;
      Z[i+j  ] += (ZZ & 0x0000FFFF) ;
      Z[i+j+1] += (ZZ>>16) ;
    }
  }

  for(int i=0; i<=8; i++) Z[i] +=    Z[i+10] ;
  for(int i=3; i<=9; i++) Z[i] +=  5*Z[i+ 7] ;
  for(int i=0; i<=1; i++) Z[i] +=  5*Z[i+17] ;
  for(int i=3; i<=4; i++) Z[i] += 25*Z[i+14] ;

  uint16_t Z_[10] ;
  for(int i=0; i<10; i++) Z_[i] = (uint16_t)(Z[i] % QRUOV_q) ;

  return Fq2Fql(Z_) ;
}

inline static Fql Fql_accumulator_reduce(Fql_accumulator Z){
  return Fql_accumulator_reduce_1(Fql_accumulator_reduce_0(Z)) ;
}

inline static Fql Fql_mul_0(Fql X, Fql Y){
  Fql_accumulator Z = Fql_accumulator_mul(X, Y) ;
  return Fql_accumulator_reduce(Z) ;
}

inline static Fql Fql_mul(Fql X, Fql Y){ return Fql_mul_0(X, Y); }

#  else
#    error "unsupported QRUOV_q in Fql_accumulator_reduce_1()"
#  endif

#elif (QRUOV_q == 127 || QRUOV_q ==  31) && QRUOV_L ==  3

//  f(x) = x^3 - x - 1
//  g(x) = a0 + a1 * x + a2 * x^2 |-> a0 + a1 * 2^22 + a2 * (2^22)^2

typedef uint64_t  Fql ;
typedef UINT128_T Fql_accumulator ;

#define Fql_zero ((Fql)0)
#define Fql_accumulator_zero ((Fql_accumulator)0)

inline int Fql_eq(Fql A, Fql B){
  return (A==B) ;
}

inline static Fql Fq2Fql(Fql z0, Fql z1, Fql z2){
  return z0|(z1<<22)|(z2<<44) ;
}

inline static Fq Fql2Fq(Fql Z, int i){
  return ((Z >> (22*i)) & QRUOV_q) ;
}

#define Fql_mask0    ( (Fql)QRUOV_q     )
#define Fql_mask1    (((Fql)QRUOV_q)<<22)
#define Fql_mask2    (((Fql)QRUOV_q)<<44)
#define Fql_mask     (Fql_mask0|Fql_mask1|Fql_mask2)

inline static Fql Fql_reduction_0(Fql Z){
  uint32_t z0 = ((uint32_t)Z & 0x3FFFFF); Z >>= 22 ;
  uint32_t z1 = ((uint32_t)Z & 0x3FFFFF); Z >>= 22 ;
  uint32_t z2 =  (uint32_t)Z            ;
  z0 %= QRUOV_q ;
  z1 %= QRUOV_q ;
  z2 %= QRUOV_q ;
  return Fq2Fql(z0, z1, z2) ;
}

inline static Fql Fql_reduction(Fql Z){
  Z = (Z & Fql_mask) + ((Z & ~Fql_mask) >> QRUOV_ceil_log_2_q) ;
  if((Z&Fql_mask0)==Fql_mask0) Z^=Fql_mask0 ;
  if((Z&Fql_mask1)==Fql_mask1) Z^=Fql_mask1 ;
  if((Z&Fql_mask2)==Fql_mask2) Z^=Fql_mask2 ;
  return Z ;
}

inline static Fql Fql_add(Fql X, Fql Y){
  return Fql_reduction(X+Y) ;
}

inline static Fql Fql_sub(Fql X, Fql Y){
  return Fql_reduction(Fql_mask+X-Y) ;
}

inline static Fql_accumulator Fql_accumulator_add(Fql_accumulator X, Fql_accumulator Y){
  return X+Y ;
}

inline static Fql_accumulator Fql_accumulator_mul(Fql X, Fql Y){
  return (Fql_accumulator)X * (Fql_accumulator)Y ;
}

inline static Fql_accumulator Fql_accumulator_reduce_0(Fql_accumulator Z){
  uint32_t z0  = ((uint32_t)Z & 0x3FFFFF); Z >>= 22 ; z0 %= QRUOV_q ;
  uint32_t z1  = ((uint32_t)Z & 0x3FFFFF); Z >>= 22 ; z1 %= QRUOV_q ;
  uint32_t z2  = ((uint32_t)Z & 0x3FFFFF); Z >>= 22 ; z2 %= QRUOV_q ;
  uint32_t z3  = ((uint32_t)Z & 0x3FFFFF); Z >>= 22 ; z3 %= QRUOV_q ;
  uint32_t z4  =  (uint32_t)Z                       ; z4 %= QRUOV_q ;
  return ((Fql_accumulator)z0    )|
         ((Fql_accumulator)z1<<22)|
         ((Fql_accumulator)z2<<44)|
         ((Fql_accumulator)z3<<66)|
         ((Fql_accumulator)z4<<88);
}

inline static Fql Fql_accumulator_reduce(Fql_accumulator Z){
  uint32_t z0 = ((uint32_t)Z & 0x3FFFFF); Z >>= 22 ;     // * 1 (14 bit)
  uint32_t z1 = ((uint32_t)Z & 0x3FFFFF); Z >>= 22 ;     // * 2
  uint32_t z2 = ((uint32_t)Z & 0x3FFFFF); Z >>= 22 ;     // * 3
  uint32_t z3 = ((uint32_t)Z & 0x3FFFFF); Z >>= 22 ;     // * 2
  uint32_t z4 =  (uint32_t)Z                       ;     // * 1
           z0 += z3 ;                                    // * 3
           z1 += z3 ;                                    // * 4
           z1 += z4 ;                                    // * 5 (14 bit x 5)
           z2 += z4 ;                                    // * 4

  z0 %= QRUOV_q ;
  z1 %= QRUOV_q ;
  z2 %= QRUOV_q ;

  return Fq2Fql(z0, z1, z2) ;
}

inline static Fql Fql_mul_0(Fql X, Fql Y){
  Fql_accumulator Z = Fql_accumulator_mul(X, Y) ;
  return Fql_accumulator_reduce(Z);
}

inline static Fql Fql_mul(Fql X, Fql Y){
  UINT128_T Z = (UINT128_T)X * (UINT128_T)Y ;
  Fql Z0 =   Z       ;
  Fql Z1 =  (Z >>64) ;
      Z1 >>= 2 ;
      Z0 += Z1 ;
      Z1 <<=22 ;
      Z0 += Z1 ;

      Z0 = (Z0 & Fql_mask) + ((Z0 & ~Fql_mask) >> QRUOV_ceil_log_2_q) ;
      Z0 = (Z0 & Fql_mask) + ((Z0 & ~Fql_mask) >> QRUOV_ceil_log_2_q) ;
      Z0 = (Z0 & Fql_mask) + ((Z0 & ~Fql_mask) >> QRUOV_ceil_log_2_q) ;
  if((Z0&Fql_mask0)==Fql_mask0) Z0^=Fql_mask0 ;
  if((Z0&Fql_mask1)==Fql_mask1) Z0^=Fql_mask1 ;
  if((Z0&Fql_mask2)==Fql_mask2) Z0^=Fql_mask2 ;
  return Z0 ;
}

#else
#  error "unsupported QRUOV_q and QRUOV_L in Fql.h"
#endif

/* =====================================================================
   pseudo random number generator
   ===================================================================== */

TYPEDEF_STRUCT ( Fql_RANDOM_CTX,
  MGF_CTX   mgf_ctx ;
  unsigned  pool_bits ;
  uint64_t  pool ;
) ;

typedef uint8_t QRUOV_SEED  [QRUOV_SEED_LEN] ;

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

/* =====================================================================
   signature
   ===================================================================== */

typedef uint8_t QRUOV_SALT  [QRUOV_SALT_LEN] ;

TYPEDEF_STRUCT(QRUOV_SIGNATURE,
  QRUOV_SALT r           ;
  Fql        s [QRUOV_N] ;
) ;
