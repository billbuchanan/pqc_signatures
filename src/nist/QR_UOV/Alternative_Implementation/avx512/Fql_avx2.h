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

typedef __m512i Fq_vec ;

inline static int _m512i_eq(__m512i a, __m512i b){ return (_mm512_cmpeq_epi64_mask(a, b) == 0xFF) ; }
inline static int _m512i_ne(__m512i a, __m512i b){ return (_mm512_cmpneq_epi64_mask(a, b) == 0xFF) ; }

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
  printf("%s%08x %08x %08x %08x %08x %08x %08x %08x"
           "%08x %08x %08x %08x %08x %08x %08x %08x\n",
    header, a[15],  a[14],  a[13],  a[12], a[11], a[10], a[9], a[8],
    a[7],  a[6],  a[5],  a[4], a[3], a[2], a[1], a[0]
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
    __m512i c = _mm512_add_epi32(a, b) ;
    __m512i d = _mm512_set1_epi32(QRUOV_q-1) ;
//    __m512i e = _mm512_cmpgt_epi32(c,d) ;
    __m512i e = _mm512_movm_epi32(_mm512_cmpgt_epi32_mask(c,d)) ;
    __m512i f = _mm512_srli_epi32(e, 32-QRUOV_ceil_log_2_q) ;
    __m512i g = _mm512_sub_epi32(c, f) ;
    return g ;
}

inline static Fql_vec Fql_vec_add(Fql_vec A, Fql_vec B){
  Fql_vec C ;
  for(int i=0; i<QRUOV_L; i++) C.c[i] = Fq_vec_add(A.c[i], B.c[i]) ;
  return C ;
}

inline static Fql_vec_accumulator Fql_vec_accumulator_add(Fql_vec_accumulator A, Fql_vec_accumulator B){
  Fql_vec_accumulator C ;
  for(int i=0; i<2*QRUOV_L-1; i++) C.c[i] = _mm512_add_epi32(A.c[i], B.c[i]) ;
  return C ;
}

inline static Fql_vec_accumulator Fql_vec_mul_to_accumulator(Fql_vec X, Fql_vec Y){
  Fql_vec_accumulator Z ;
  for(int i=0; i<2*QRUOV_L-1 ; i++) Z.c[i] = _mm512_setzero_si512() ;

  for(int i=0; i<QRUOV_L; i++){
    for(int j=0; j<QRUOV_L; j++){
      Z.c[i+j] = _mm512_add_epi32(Z.c[i+j], _mm512_mullo_epi32(X.c[i], Y.c[j]));
    }
  }
  return Z ;
}

inline static uint32_t extract_Fq(Fq_vec a, int n) {
  int m = (n >> 2) & 0x3 ; n &= 0x3 ;
  __m128i b = _mm512_extracti32x4_epi32(a, m) ;
  return (uint32_t) _mm_extract_epi32(b, n) ;
}

inline static __m512i insert_Fq(Fq_vec a, uint32_t ins, int n) {
  int m = (n >> 2) & 0x3 ; n &= 0x3 ;
  __m128i b = _mm512_extracti32x4_epi32(a, m) ;
  __m128i c = _mm_insert_epi32(b, ins, n) ;
  return _mm512_inserti32x4 (a, c, m) ;
}

inline static Fql_vec_accumulator Fql_vec_mul_to_accumulator_0(Fql_vec X, Fql_vec Y){
  Fql_vec_accumulator Z ;
  for(int i=0; i<2*QRUOV_L-1 ; i++) Z.c[i] = _mm512_setzero_si512() ;
  for(int i=0; i<QRUOV_L;i++){
    for(int j=0; j<QRUOV_L;j++){
      for(int k=0; k<16; k++){
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

inline static uint32_t vec_horizontal_sum_u32(Fq_vec X) {
  __m256i lo256 = _mm512_castsi512_si256(X) ;
  __m512i hi512 = _mm512_maskz_compress_epi64(0xF0, X) ;
  __m256i hi256 = _mm512_castsi512_si256(hi512) ;
  __m256i s0    = _mm256_hadd_epi32(lo256, hi256) ;
  __m128i lo = _mm256_castsi256_si128(s0) ;
  __m128i hi = _mm256_extracti128_si256(s0, 1) ;
  __m128i s1 = _mm_hadd_epi32(lo, hi) ;
  __m128i s2 = _mm_hadd_epi32(s1, s1) ;
  __m128i s3 = _mm_hadd_epi32(s2, s2) ;
  return ((uint32_t)_mm_extract_epi32(s3, 0)) ;
}

inline static uint32_t vec_horizontal_sum_u32_0(Fq_vec a_) {
  uint32_t * a = (uint32_t *) & a_ ;
  return a[15]+  a[14]+  a[13]+  a[12]+ a[11]+ a[10]+ a[9]+ a[8] +
         a[7]+  a[6]+  a[5]+  a[4]+ a[3]+ a[2]+ a[1]+ a[0] ;
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

inline static Fql_vec Fql_array2vec(Fql A[], int k, int K) {
  Fql_vec Z ;
  for(int j = 0 ; j < QRUOV_L ; j++){
    __m256i lo = _mm256_setr_epi32(
      (k+0<K)?A[k+0].c[j]:0,
      (k+1<K)?A[k+1].c[j]:0,
      (k+2<K)?A[k+2].c[j]:0,
      (k+3<K)?A[k+3].c[j]:0,
      (k+4<K)?A[k+4].c[j]:0,
      (k+5<K)?A[k+5].c[j]:0,
      (k+6<K)?A[k+6].c[j]:0,
      (k+7<K)?A[k+7].c[j]:0
      ) ;
    __m256i hi = _mm256_setr_epi32(
      (k+8<K)?A[k+8].c[j]:0,
      (k+9<K)?A[k+9].c[j]:0,
      (k+10<K)?A[k+10].c[j]:0,
      (k+11<K)?A[k+11].c[j]:0,
      (k+12<K)?A[k+12].c[j]:0,
      (k+13<K)?A[k+13].c[j]:0,
      (k+14<K)?A[k+14].c[j]:0,
      (k+15<K)?A[k+15].c[j]:0
    ) ;
    Z.c[j] = _mm512_inserti64x4(Z.c[j],hi,1) ;
    Z.c[j] = _mm512_inserti64x4(Z.c[j],lo,0) ;
  }
  return Z ;
}

inline static uint32_t Fq_RANDOM(){ return random() % QRUOV_q ; }

inline static Fql Fql_RANDOM(){
  Fql Z ;
  for(int i = 0; i< QRUOV_L; i++) Z.c[i] = Fq_RANDOM() ;
  return Z ;
}

inline static __m512i Fq_vec_random(){
    __m256i lo = _mm256_setr_epi32(
      random() % QRUOV_q, random() % QRUOV_q, 
      random() % QRUOV_q, random() % QRUOV_q,
      random() % QRUOV_q, random() % QRUOV_q, 
      random() % QRUOV_q, random() % QRUOV_q
    );
    __m256i hi = _mm256_setr_epi32(
      random() % QRUOV_q, random() % QRUOV_q, 
      random() % QRUOV_q, random() % QRUOV_q,
      random() % QRUOV_q, random() % QRUOV_q, 
      random() % QRUOV_q, random() % QRUOV_q
    ) ;
    __m512i r = _mm512_setzero_si512() ;
    __m512i s = _mm512_inserti64x4(r,hi,1) ;
    return _mm512_inserti64x4(s,lo,0) ;
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
