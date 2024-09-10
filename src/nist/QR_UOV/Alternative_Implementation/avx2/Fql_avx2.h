#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <x86intrin.h>

#define Fql_avx2_h_DEBUG 0
#if     Fql_avx2_h_DEBUG

#define QRUOV_security_strength_category 1
#define QRUOV_q                          127
#define QRUOV_v                          156
#define QRUOV_m                          54
#define QRUOV_L                          3
#define QRUOV_fc                         1
#define QRUOV_fe                         1
#define QRUOV_fc0                        1
#define QRUOV_PLATFORM                   portable64

#if   (QRUOV_q == 7)
#  define QRUOV_ceil_log_2_q 3
#elif (QRUOV_q == 31)
#  define QRUOV_ceil_log_2_q 5
#elif (QRUOV_q == 127)
#  define QRUOV_ceil_log_2_q 7
#else
#  error "Unsupported: QRUOV_q == " # QRUOV_q
#endif

#include "Fql.h"

/*
typedef uint32_t Fq ;
typedef struct Fql_t {
  Fq c[QRUOV_L] ;
} Fql ;
*/

#else

#include "qruov.h"

#endif

typedef __m256i Fq_vec ;

inline static int _m256i_eq(__m256i a, __m256i b){
  __m256i  c = _mm256_cmpeq_epi64(a,b) ;
  __m128i  d = _mm256_castsi256_si128(c) ;
  __m128i  e = _mm256_extracti128_si256(c, 1) ;
  __m128i  f = _mm_or_si128(d,e) ;
  __m128i  g = _mm_shuffle_epi32(f, _MM_SHUFFLE(1,0,3,2)) ;
  __m128i  h = _mm_or_si128(f,g) ;
  __m128i  i = _mm_hadd_epi32(h, h) ;
  return _mm_extract_epi32(i, 0) ;
}

inline static int _m256i_ne(__m256i a_, __m256i b_){ return ! _m256i_eq(a_, b_) ; }

typedef struct Fql_vec_t {
  Fq_vec c[QRUOV_L] ;
} Fql_vec ;

typedef struct Fql_vec_accumulator_t {
  Fq_vec c[2*QRUOV_L-1] ;
} Fql_vec_accumulator ;

extern Fq                  Fq_zero ;
extern Fql                 Fql_zero ;
extern Fql_vec             Fql_vec_zero ;
extern Fql_vec_accumulator Fql_vec_accumulator_zero ;

inline static void Fq_vec_print(char * header, Fq_vec a_){
  uint32_t * a = (uint32_t *) & a_ ;
  printf("%s%08x %08x %08x %08x %08x %08x %08x %08x\n",
    header, a[7],  a[6],  a[5],  a[4], a[3], a[2], a[1], a[0]
  );
}

inline static void Fql_vec_print(char * header, Fql_vec A){
  printf("%s",header) ;
  for(int i=0; i < QRUOV_L; i++) Fq_vec_print("",A.c[i]) ;
}

inline static void Fql_vec_accumulator_print(char * header, Fql_vec_accumulator A){
  printf("%s",header) ;
  for(int i=0; i < 2*QRUOV_L-1; i++) Fq_vec_print("",A.c[i]) ;
}

#define   Fq_vec_PRINT(a)    Fq_vec_print( #a " =\n", a )
#define   Fql_vec_PRINT(a)   Fql_vec_print( #a " =\n", a )
#define   Fql_vec_accumulator_PRINT(a)   Fql_vec_accumulator_print( #a " =\n", a )

inline static Fq_vec Fq_vec_add(Fq_vec a, Fq_vec b){
    __m256i c = _mm256_add_epi32(a, b) ;
    __m256i d = _mm256_set1_epi32(QRUOV_q-1) ;
    __m256i e = _mm256_cmpgt_epi32(c,d) ;
    __m256i f = _mm256_srli_epi32(e, 32-QRUOV_ceil_log_2_q) ;
    __m256i g = _mm256_sub_epi32(c, f) ;
    return g ;
}

inline static Fql_vec Fql_vec_add(Fql_vec A, Fql_vec B){
  Fql_vec C ;
  for(int i=0; i<QRUOV_L; i++) C.c[i] = Fq_vec_add(A.c[i], B.c[i]) ;
  return C ;
}

inline static Fql_vec_accumulator Fql_vec_accumulator_add(Fql_vec_accumulator A, Fql_vec_accumulator B){
  Fql_vec_accumulator C ;
  for(int i=0; i<2*QRUOV_L-1; i++) C.c[i] = _mm256_add_epi32(A.c[i], B.c[i]) ;
                                // C.c[i] = Fq_vec_add(A.c[i], B.c[i]) ;
  return C ;
}

#if 0

inline static Fq_vec Fq_vec_sub(Fq_vec a, Fq_vec b){
    __m256i c = _mm256_sub_epi32(a, b) ;
    __m256i d = _mm256_setzero_si256() ;
    __m256i e = _mm256_cmpgt_epi32(d,c);
    __m256i f = _mm256_srli_epi32(e, 32-QRUOV_ceil_log_2_q) ;
    __m256i g = _mm256_add_epi32(c, f) ;
    return g ;
}

inline static Fql_vec Fql_vec_sub(Fql_vec A, Fql_vec B){
  Fql_vec C ;
  for(int i=0; i<QRUOV_L; i++) C.c[i] = Fq_vec_sub(A.c[i], B.c[i]) ;
  return C ;
}

#endif

inline static Fql_vec_accumulator Fql_vec_mul_to_accumulator(Fql_vec X, Fql_vec Y){
  Fql_vec_accumulator Z ;
  for(int i=0; i<2*QRUOV_L-1 ; i++) Z.c[i] = _mm256_setzero_si256() ;

  for(int i=0; i<QRUOV_L; i++){
    for(int j=0; j<QRUOV_L; j++){
      Z.c[i+j] = _mm256_add_epi32(Z.c[i+j], _mm256_mullo_epi32(X.c[i], Y.c[j]));
    }
  }
  return Z ;
}

inline static uint32_t extract_Fq(Fq_vec a, int n) {
  int m = (n >> 2) & 1 ; n &= 0x3 ;
  __m128i b = _mm256_extracti128_si256(a, m) ;
  return (uint32_t) _mm_extract_epi32(b, n) ;
}

inline static __m256i insert_Fq(Fq_vec a, uint32_t ins, int n) {
  int m = (n >> 2) & 1 ; n &= 0x3 ;
  __m128i b = _mm256_extracti128_si256(a, m) ;
  __m128i c = _mm_insert_epi32(b, ins, n) ;
  return _mm256_inserti128_si256 (a, c, m) ;
}

inline static Fql_vec_accumulator Fql_vec_mul_to_accumulator_0(Fql_vec X, Fql_vec Y){
  Fql_vec_accumulator Z ;
  for(int i=0; i<2*QRUOV_L-1 ; i++) Z.c[i] = _mm256_setzero_si256() ;
  for(int i=0; i<QRUOV_L;i++){
    for(int j=0; j<QRUOV_L;j++){
      for(int k=0; k<8; k++){
        uint32_t x = extract_Fq(X.c[i], k) ;
        uint32_t y = extract_Fq(Y.c[j], k) ;
        uint32_t z = extract_Fq(Z.c[i+j], k) ;
	         z+= x*y ;
		 Z.c[i+j] = insert_Fq(Z.c[i+j], z, k) ;
      }
    }
  }
  return Z ;
}

#if 0

inline static Fql_vec Fql_vec_accumulator_reduction(Fql_vec_accumulator T){
  for(int i = 2*QRUOV_L-2; i >= QRUOV_L; i--){
    { // T.c[i-QRUOV_L]          += QRUOV_fc0 * T.c[i] ;
#     if QRUOV_fc0 == 1
        T.c[i-QRUOV_L] = _mm256_add_epi32(T.c[i-QRUOV_L], T.c[i]);
#     else
        __m256i t0     = _mm_setzero_si128() ;
        __m128i t1     = _mm_insert_epi32(t0,QRUOV_fc0,0) ;
        __m256i fc0    = _mm256_broadcastw_epi32(t1) ;
        T.c[i-QRUOV_L] = _mm256_add_epi32(T.c[i-QRUOV_L],
	                         _mm256_mullo_epi32(fc0,T.c[i]));
#     endif
    }{ // T.c[i-QRUOV_L+QRUOV_fe] += QRUOV_fc  * T.c[i] ;
#     if QRUOV_fc0 == 1
        T.c[i-QRUOV_L+QRUOV_fe]=_mm256_add_epi32(T.c[i-QRUOV_L+QRUOV_fe],T.c[i]);
#     elif QRUOV_fc0 == 2
        T.c[i-QRUOV_L+QRUOV_fe]=_mm256_add_epi32(T.c[i-QRUOV_L+QRUOV_fe],T.c[i]);
        T.c[i-QRUOV_L+QRUOV_fe]=_mm256_add_epi32(T.c[i-QRUOV_L+QRUOV_fe],T.c[i]);
#     elif QRUOV_fc0 == 5
        T.c[i-QRUOV_L+QRUOV_fe]=_mm256_add_epi32(T.c[i-QRUOV_L+QRUOV_fe],T.c[i]);
        __m256i ti4 = _mm256_srli_epi32(T.c[i],2) ;
        T.c[i-QRUOV_L+QRUOV_fe]=_mm256_add_epi32(T.c[i-QRUOV_L+QRUOV_fe],ti4);
#     else
        __m256i t0  = _mm_setzero_si128() ;
        __m128i t1  = _mm_insert_epi32(t0,QRUOV_fc,0) ;
        __m256i fc  = _mm256_broadcastw_epi32(t1) ;
        T.c[i-QRUOV_L+QRUOV_fe] = _mm256_add_epi32(
          T.c[i-QRUOV_L+QRUOV_fe],_mm256_mullo_epi32(fc,T.c[i]));
#     endif
    }
  }
  __m256i a         = _mm256_setzero_si256() ;
  __m256i b         = _mm256_cmpeq_epi8(a, a) ;
#if QRUOV_q == 7
  __m256i q4_mask   = _mm256_srli_epi32(b, 32-4*QRUOV_ceil_log_2_q) ;
#endif
  __m256i q2_mask   = _mm256_srli_epi32(b, 32-2*QRUOV_ceil_log_2_q) ;
  __m256i q1_mask   = _mm256_srli_epi32(b, 32-  QRUOV_ceil_log_2_q) ;
  __m256i one       = _mm256_srli_epi32(b, 31) ;
  __m256i q_minus_1 = _mm256_sub_epi32(q1_mask, one) ;

  for(int i = 0; i<QRUOV_L; i++){
#if QRUOV_q == 7
    __m256i hi4 = _mm256_srli_epi32(T.c[i], 4*QRUOV_ceil_log_2_q) ;
    __m256i lo4 = _mm256_and_si256(T.c[i], q4_mask) ;
    __m256i rs4 = _mm256_add_epi32(hi4, lo4) ;
    __m256i hi2 = _mm256_srli_epi32(rs4, 2*QRUOV_ceil_log_2_q) ;
    __m256i lo2 = _mm256_and_si256(rs4, q2_mask) ;
#else
    __m256i hi2 = _mm256_srli_epi32(T.c[i], 2*QRUOV_ceil_log_2_q) ;
    __m256i lo2 = _mm256_and_si256(T.c[i], q2_mask) ;
#endif
    __m256i rs2 = _mm256_add_epi32(hi2, lo2) ;

    __m256i hi1 = _mm256_srli_epi32(rs2, QRUOV_ceil_log_2_q) ;
    __m256i lo1 = _mm256_and_si256(rs2, q1_mask) ;
    __m256i rs1 = _mm256_add_epi32(hi1, lo1) ;

    __m256i msk = _mm256_cmpgt_epi32(rs1, q_minus_1) ;
    __m256i msq = _mm256_srli_epi32(msk, 32-QRUOV_ceil_log_2_q) ;
           T.c[i] = _mm256_sub_epi32(rs1, msq) ;
  }
  return *(Fql_vec*)&T ;
}

inline static Fql_vec Fql_vec_mul(Fql_vec X, Fql_vec Y){
  return Fql_vec_accumulator_reduction(Fql_vec_mul_to_accumulator(X, Y)) ;
}

#endif

inline static uint32_t vec_horizontal_sum_u32(Fq_vec X) {
  __m128i lo = _mm256_castsi256_si128(X) ;
  __m128i hi = _mm256_extracti128_si256(X, 1) ;
  __m128i s1 = _mm_hadd_epi32(lo, hi) ;
  __m128i s2 = _mm_hadd_epi32(s1, s1) ;
  __m128i s3 = _mm_hadd_epi32(s2, s2) ;
  return ((uint32_t)_mm_extract_epi32(s3, 0)) ;
}

inline static uint32_t vec_horizontal_sum_u32_0(Fq_vec a_) {
  uint32_t * a = (uint32_t *) & a_ ;
  return a[7]+  a[6]+  a[5]+  a[4]+ a[3]+ a[2]+ a[1]+ a[0] ;
}

inline static Fql Fql_vec_accumulator_horizontal_sum (Fql_vec_accumulator X){
  Fq T[2*QRUOV_L - 1] ;
  for(int i=0; i<2*QRUOV_L-1; i++) T[i] = vec_horizontal_sum_u32(X.c[i]) ;

  for(int i=2*QRUOV_L-2; i>= QRUOV_L; i--){
      T[i-QRUOV_L]          += QRUOV_fc0 * T[i] ;
      T[i-QRUOV_L+QRUOV_fe] += QRUOV_fc  * T[i] ;
  }

  Fql Z ;
  for(int i=0; i<QRUOV_L; i++) Z.c[i] = (Fq)(T[i] % QRUOV_q) ;
  return Z ;
}

#if 0

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

inline static Fql_vec Fql_vec_mul_0(Fql_vec X, Fql_vec Y){
  Fql_vec Z ;
  for(int i = 0 ; i < 8 ; i++){
    Fql x, y ;
    for(int j = 0 ; j < QRUOV_L ; j++){
      x.c[j] = extract_Fq(X.c[j],i) ;
      y.c[j] = extract_Fq(Y.c[j],i) ;
    }
    Fql z = Fql_mul(x,y) ;
    for(int j = 0 ; j < QRUOV_L ; j++){
      Z.c[j] = insert_Fq(Z.c[j],z.c[j],i) ;
    }
  }
  return Z ;
}

#endif

inline static Fql_vec Fql_array2vec(Fql A[], int k, int K) {
  Fql_vec Z ;
  for(int j = 0 ; j < QRUOV_L ; j++){
    Z.c[j] = _mm256_setr_epi32(
      (k+0<K)?A[k+0].c[j]:0,
      (k+1<K)?A[k+1].c[j]:0,
      (k+2<K)?A[k+2].c[j]:0,
      (k+3<K)?A[k+3].c[j]:0,
      (k+4<K)?A[k+4].c[j]:0,
      (k+5<K)?A[k+5].c[j]:0,
      (k+6<K)?A[k+6].c[j]:0,
      (k+7<K)?A[k+7].c[j]:0
    ) ;
  }
  return Z ;
}

inline static uint32_t Fq_RANDOM(){ return random() % QRUOV_q ; }

inline static Fql Fql_RANDOM(){
  Fql Z ;
  for(int i = 0; i< QRUOV_L; i++) Z.c[i] = Fq_RANDOM() ;
  return Z ;
}

inline static __m256i Fq_vec_random(){
    return _mm256_setr_epi32(
      random() % QRUOV_q, random() % QRUOV_q, 
      random() % QRUOV_q, random() % QRUOV_q,
      random() % QRUOV_q, random() % QRUOV_q, 
      random() % QRUOV_q, random() % QRUOV_q
    ) ;
}

inline static Fql_vec Fql_vec_random(){
  Fql_vec Z ;
  for(int i=0;i<QRUOV_L;i++) Z.c[i] = Fq_vec_random() ;
  return Z ;
}

#if Fql_avx2_h_DEBUG

int Fql_vec_accumulator_eq(Fql_vec_accumulator X, Fql_vec_accumulator Y){
  int flag = 1 ;
  for(int i=0; i<2*QRUOV_L-1; i++) flag = (flag && _m256i_eq(X.c[i], Y.c[i])) ;
  return flag ;
}

Fql_vec_accumulator Fql_vec_accumulator_zero ;


int main(){

  for(int i=0;i<10000000; i++){
    Fql A[8] ;
    Fql B[8] ;
    for(int j=0; j<8; j++) A[j] = Fql_RANDOM() ;
    for(int j=0; j<8; j++) B[j] = Fql_RANDOM() ;

    Fql_vec C = Fql_array2vec(A) ;
    Fql_vec D = Fql_array2vec(B) ;
    Fql_vec_accumulator Z = Fql_vec_mul_to_accumulator(C,D) ;
    Fql_vec_accumulator Y = Fql_vec_mul_to_accumulator_0(C,D) ;
    if(! Fql_vec_accumulator_eq(Y,Z)){
      Fql_vec_PRINT(C) ;
      Fql_vec_PRINT(D) ;
      Fql_vec_accumulator_PRINT(Z) ;
      Fql_vec_accumulator_PRINT(Y) ;
    }
  }
  return 0 ;
}

#endif
