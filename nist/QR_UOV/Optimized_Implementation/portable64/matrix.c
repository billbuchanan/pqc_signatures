#include "qruov.h"

// C = A*B
#define MATRIX_MUL(Element, N, K, M, A, B, C) {                    \
  int _i,_j,_k ;                                                   \
  _Pragma("omp parallel for private(_i,_j,_k) shared(A, B, C)")    \
  for(_i=0;_i<N;_i++){                                             \
    for(_j=0;_j<M;_j++){                                           \
      Element ## _ACCUMULATOR _t ;                                 \
      Element ## _ACCUMULATOR_ZERO(_t) ;                           \
      int k_POOL = k_THRESHOLD ;                                   \
      for(_k=0;_k<K;_k++){                                         \
        Element ## _ACCUMULATE_MUL(A[_i][_k], B[_k][_j], _t) ;     \
      }                                                            \
      Element ## _ACCUMULATOR_REDUCE(_t, C[_i][_j]) ;              \
    }                                                              \
  }                                                                \
}

// C += A*B
#define MATRIX_MUL_ADD(Element, N, K, M, A, B, C) {                \
  int _i,_j,_k ;                                                   \
  _Pragma("omp parallel for private(_i,_j,_k) shared(A, B, C)")    \
  for(_i=0;_i<N;_i++){                                             \
    for(_j=0;_j<M;_j++){                                           \
      Element ## _ACCUMULATOR _t ;                                 \
      Element ## _ACCUMULATOR_ZERO(_t) ;                           \
      int k_POOL = k_THRESHOLD ;                                   \
      for(_k=0;_k<K;_k++){                                         \
        Element ## _ACCUMULATE_MUL(A[_i][_k], B[_k][_j], _t) ;     \
      }                                                            \
      Element _v ;                                                 \
      Element ## _ACCUMULATOR_REDUCE(_t, _v) ;                     \
      Element ## _ADD(_v, C[_i][_j], C[_i][_j]) ;                  \
    }                                                              \
  }                                                                \
}

// C = A+B
#define MATRIX_ADD(Element, N, M, A, B, C) {                 \
  int _i,_j ;                                                \
  _Pragma("omp parallel for private(_i,_j) shared(A, B, C)") \
  for(_i=0;_i<N;_i++){                                       \
    for(_j=0;_j<M;_j++){                                     \
      Element ## _ADD(A[_i][_j], B[_i][_j], C[_i][_j]) ;     \
    }                                                        \
  }                                                          \
}

// C = A-B
#define MATRIX_SUB(Element, N, M, A, B, C) {                 \
  int _i,_j ;                                                \
  _Pragma("omp parallel for private(_i,_j) shared(A, B, C)") \
  for(_i=0;_i<N;_i++){                                       \
    for(_j=0;_j<M;_j++){                                     \
      Element ## _SUB(A[_i][_j], B[_i][_j], C[_i][_j]) ;     \
    }                                                        \
  }                                                          \
}

// C = A^T
#define MATRIX_TRANSPOSE(Element, N, M, A, C) {           \
  int _i,_j ;                                             \
  _Pragma("omp parallel for private(_i,_j) shared(A, C)") \
  for(_i=0;_i<N;_i++){                                    \
    for(_j=0;_j<M;_j++){                                  \
      Element ## _COPY(A[_i][_j], C[_j][_i]) ;            \
    }                                                     \
  }                                                       \
}

#if (QRUOV_L == 3)

#  define Fql_ACCUMULATOR                 UINT128_T
#  define Fql_ACCUMULATOR_ZERO(a)         { a = Fql_accumulator_zero ; }

#    define k_THRESHOLD                     (1<<(22-2*QRUOV_ceil_log_2_q))

#  if (QRUOV_v > k_THRESHOLD)
#    define Fql_ACCUMULATE_MUL(a,b,c)        \
     {                                       \
        c += (UINT128_T)a * (UINT128_T)b ;   \
        k_POOL -= QRUOV_L ;                  \
        if(k_POOL < QRUOV_L){                \
          c = Fql_accumulator_reduce_0(c) ;  \
          k_POOL = k_THRESHOLD ;             \
        } ;                                  \
     }
#  else
#    define Fql_ACCUMULATE_MUL(a,b,c)       { c += (UINT128_T)a * (UINT128_T)b ; }
#  endif

#  define Fql_ACCUMULATOR_REDUCE(a,c)     { c = Fql_accumulator_reduce(a) ; }

#elif (QRUOV_L == 10)

#  define Fql_ACCUMULATOR                 Fql_accumulator 
#  define Fql_ACCUMULATOR_ZERO(a)         { a = Fql_accumulator_zero ; }

#  define k_THRESHOLD                     (1<<(16-2*QRUOV_ceil_log_2_q))
#  define k_MASK                          (k_THRESHOLD-1)

#  if (QRUOV_v > k_THRESHOLD)
#    define Fql_ACCUMULATE_MUL(a,b,c)        \
     {                                       \
        c = Fql_accumulator_add(c, Fql_accumulator_mul(a,b)) ; \
        k_POOL -= QRUOV_L ;                  \
        if(k_POOL < QRUOV_L){                \
          c = Fql_accumulator_reduce_0(c) ;  \
          k_POOL = k_THRESHOLD ;             \
        } ;                                  \
     }
#  else
#    define Fql_ACCUMULATE_MUL(a,b,c)       { c = Fql_accumulator_add(c, Fql_accumulator_mul(a,b)) ; }
#  endif

#  define Fql_ACCUMULATOR_REDUCE(a,c)     { c = Fql_accumulator_reduce(a) ; }

#define Fql_ACCUMULATOR_DEBUG                   Fql
#define Fql_ACCUMULATOR_ZERO_DEBUG(a)           { a = Fql_zero ; }
#define Fql_ACCUMULATE_MUL_DEBUG(a,b,c)         { c = Fql_add(c, Fql_mul(a,b)) ; }
#define Fql_ACCUMULATOR_REDUCE_DEBUG(a,c)       { c = a ; }

#else
#  error "unsupported QRUOV_L for matrix.h"
#endif

#define Fql_ADD(a,b,c)                    { c = Fql_add(a,b) ; }
#define Fql_SUB(a,b,c)                    { c = Fql_sub(a,b) ; }
#define Fql_COPY(a,c)                     { c = a ; }

void MATRIX_MUL_MxV_VxV(MATRIX_MxV A, MATRIX_VxV B, MATRIX_MxV C){
  MATRIX_MUL(Fql, QRUOV_M, QRUOV_V, QRUOV_V, A, B, C) ;
}

void MATRIX_MUL_MxV_VxM(MATRIX_MxV A, MATRIX_VxM B, MATRIX_MxM C){
  MATRIX_MUL(Fql, QRUOV_M, QRUOV_V, QRUOV_M, A, B, C) ;
}

void MATRIX_MUL_ADD_MxV_VxM(MATRIX_MxV A, MATRIX_VxM B, MATRIX_MxM C){
  MATRIX_MUL_ADD(Fql, QRUOV_M, QRUOV_V, QRUOV_M, A, B, C) ;
}

void MATRIX_SUB_MxV(MATRIX_MxV A, MATRIX_MxV B, MATRIX_MxV C){
  MATRIX_SUB(Fql, QRUOV_M, QRUOV_V, A, B, C) ;
}

void MATRIX_ADD_MxM(MATRIX_MxM A, MATRIX_MxM B, MATRIX_MxM C){
  MATRIX_ADD(Fql, QRUOV_M, QRUOV_M, A, B, C) ;
}

void MATRIX_TRANSPOSE_VxM(MATRIX_VxM A, MATRIX_MxV C){
  MATRIX_TRANSPOSE(Fql, QRUOV_V, QRUOV_M, A, C) ;
}

void EQN_GEN(VECTOR_V vineger, MATRIX_MxV F2T[QRUOV_m], Fq eqn[QRUOV_m][QRUOV_m]){
  int i,j,k ;
#pragma omp parallel for private(i,j,k) shared(vineger, F2T, eqn)
  for(i=0; i<QRUOV_m; i++){
    for(j=0; j<QRUOV_M; j++){
      Fql_accumulator t = Fql_accumulator_zero ;
      int k_POOL = k_THRESHOLD ;
      for(k=0; k<QRUOV_V; k++){
        Fql_ACCUMULATE_MUL(vineger[k], F2T[i][j][k], t) ;
      }
      Fql u ;
      Fql_ACCUMULATOR_REDUCE(t, u) ;
      u = Fql_add(u,u) ;
      for(int l=0; l<QRUOV_L; l++){
        eqn[i][QRUOV_L*j+l] = Fql2Fq(u, QRUOV_perm(l)) ; // <- unpack_1(...)
      }
    }
  }
}

void C_GEN(VECTOR_V vineger, MATRIX_VxV F1[QRUOV_m], Fq c[QRUOV_m]){
  int i,j,k ;
#pragma omp parallel for private(i,j,k) shared(vineger, F1, c)
  for(i=0; i<QRUOV_m; i++){
    Fql tmp [QRUOV_V] ;
    for(j=0; j<QRUOV_V; j++){
      Fql_accumulator t = Fql_accumulator_zero ;
      int k_POOL = k_THRESHOLD ;
      for(k=0; k<QRUOV_V; k++){
        Fql_ACCUMULATE_MUL(vineger[k], F1[i][j][k], t) ;
      }
      Fql_ACCUMULATOR_REDUCE(t, tmp[j]) ;
    }
    uint64_t c_i = 0 ;
    for(k=0; k<QRUOV_V; k++){
      c_i += (uint64_t) Fql2Fq(Fql_mul(tmp[k],vineger[k]), QRUOV_perm(0)) ; // <-- shrink
    }
    c[i] = (Fq)(c_i % QRUOV_q) ;
  }
}

void SIG_GEN(VECTOR_M oil, MATRIX_MxV SdT, VECTOR_V vineger, QRUOV_SIGNATURE sig){
  int i,j ;
#pragma omp parallel for private(i,j) shared(oil, SdT, vineger, sig)
  for(i=0;i<QRUOV_V;i++){
    Fql_accumulator t = Fql_accumulator_zero ;
    int k_POOL = k_THRESHOLD ;
    for(j=0;j<QRUOV_M;j++){
      Fql_ACCUMULATE_MUL(oil[j], SdT[j][i], t) ;
    }
    Fql u ;
    Fql_ACCUMULATOR_REDUCE(t, u) ;
    sig->s[i] = Fql_sub(vineger[i], u) ;
  }
  for(i=QRUOV_V;i<QRUOV_N;i++){
    sig->s[i] = oil[i-QRUOV_V] ;
  }
}

void RESULT_GEN(const QRUOV_P1 P1, const QRUOV_P2T P2T, const QRUOV_P3 P3, const VECTOR_M oil, const VECTOR_V vineger, const Fq msg [QRUOV_m], uint8_t result[QRUOV_m]) {
  int i,j,k ;
#pragma omp parallel for private(i,j,k) shared(P1, P2T, P3, oil, vineger, msg, result)
  for(i=0; i<QRUOV_m; i++){
    Fql t ;
    Fql tmp_v [QRUOV_V] ;
    Fql tmp_o [QRUOV_M] ;
    for(j=0;j<QRUOV_V;j++){
      t = Fql_zero ;
      for(k=0;k<QRUOV_M;k++){
        t = Fql_add(t, Fql_mul(P2T[i][k][j],oil[k])) ; // <-
      }
      t = Fql_add(t,t) ;
      for(k=0;k<QRUOV_V;k++){
        t = Fql_add(t, Fql_mul(P1[i][j][k],vineger[k])) ;
      }
      tmp_v[j] = t ;
    }

    for(j=0;j<QRUOV_M;j++){
      t = Fql_zero ;
      for(k=0;k<QRUOV_M;k++){
        t = Fql_add(t, Fql_mul(P3[i][j][k],oil[k])) ;
      }
      tmp_o[j] = t ;
    }

    t = Fql_zero ;
    for(j=0;j<QRUOV_V;j++){
      t = Fql_add(t, Fql_mul(vineger[j],tmp_v[j])) ;
    }
    for(j=0;j<QRUOV_M;j++){
      t = Fql_add(t, Fql_mul(oil[j],tmp_o[j])) ;
    }
    if(msg[i] != Fql2Fq(t,QRUOV_perm(0))){ // <-- shrink
      result[i] = 0 ;
    }else{
      result[i] = 1 ;
    }
  }
}
