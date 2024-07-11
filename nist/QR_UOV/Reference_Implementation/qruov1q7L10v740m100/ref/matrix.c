#include "qruov.h"

// C = A*B
#define MATRIX_MUL(Element, N, K, M, A, B, C) {                    \
  int _i,_j,_k ;                                                   \
  _Pragma("omp parallel for private(_i,_j,_k) shared(A, B, C)")    \
  for(_i=0;_i<N;_i++){                                             \
    for(_j=0;_j<M;_j++){                                           \
      Element ## _ACCUMULATOR _t, _u ;                             \
      Element ## _ACCUMULATOR_ZERO(_t) ;                           \
      for(_k=0;_k<K;_k++){                                         \
        Element ## _MUL_TO_ACCUMULATOR(A[_i][_k], B[_k][_j], _u) ; \
        Element ## _ACCUMULATOR_ADD(_t, _u, _t) ;                  \
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
      Element ## _ACCUMULATOR _t, _u ;                             \
      Element ## _ACCUMULATOR_ZERO(_t) ;                           \
      for(_k=0;_k<K;_k++){                                         \
        Element ## _MUL_TO_ACCUMULATOR(A[_i][_k], B[_k][_j], _u) ; \
        Element ## _ACCUMULATOR_ADD(_t, _u, _t) ;                  \
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

#define Fql_ACCUMULATOR Fql

#define Fql_ACCUMULATOR_ZERO(a)           { a = Fql_zero ; }
#define Fql_ACCUMULATOR_ADD(a,b,c)        { c = Fql_add(a,b) ; }
#define Fql_MUL_TO_ACCUMULATOR(a,b,c)     { c = Fql_mul(a,b) ; }
#define Fql_ACCUMULATOR_REDUCE(a,c)       { c = a ; }

#define Fql_MUL_ADD_TO_ACCUMULATOR(a,b,c) { c = Fql_add(c, Fql_mul(a,b)) ; }

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
