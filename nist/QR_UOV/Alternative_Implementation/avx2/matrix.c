#include "qruov.h"
#include "Fql_avx2.h"

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

Fql_vec_accumulator Fql_vec_accumulator_zero ;

void MATRIX_MUL_1(MATRIX_MxV A, MATRIX_VxV B, MATRIX_MxV C){
  const int N = QRUOV_M ;
  const int K = QRUOV_V ;
  const int M = QRUOV_V ;
  int i,j,k ;
  _Pragma("omp parallel for private(i,j,k) shared(A, B, C)")
  for(i=0;i<N;i++){
    for(j=0;j<M;j++){
      Fql_vec_accumulator t = Fql_vec_accumulator_zero ;
      for(k=0;k<K;k+=8){
        Fql_vec a QRUOV_aligned = Fql_array2vec(A[i],k,K) ;
        Fql_vec b QRUOV_aligned = Fql_array2vec(B[j],k,K) ;
        t = Fql_vec_accumulator_add(t, Fql_vec_mul_to_accumulator(a,b)) ;
      }
      C[i][j] = Fql_vec_accumulator_horizontal_sum(t) ;
    }
  }
}

void MATRIX_MUL_2(MATRIX_MxV A, MATRIX_VxM B, MATRIX_MxM C){
  const int N = QRUOV_M ;
  const int K = QRUOV_V ;
  const int M = QRUOV_M ;
  int i,j,k ;
  _Pragma("omp parallel for private(i,j,k) shared(A, B, C)")
  for(i=0;i<N;i++){
    for(j=0;j<M;j++){
      Fql_vec_accumulator t = Fql_vec_accumulator_zero ;
      for(k=0;k<K;k+=8){
        Fql BT[8] ; for(int l=0;l<8;l++) BT[l]=((k+l)<K)?B[k+l][j]:Fql_zero ;
        Fql_vec a QRUOV_aligned = Fql_array2vec(A[i],k,K) ;
        Fql_vec b QRUOV_aligned = Fql_array2vec(BT,0,(8<K)?8:K) ;
        t = Fql_vec_accumulator_add(t, Fql_vec_mul_to_accumulator(a,b)) ;
      }
      C[i][j] = Fql_vec_accumulator_horizontal_sum(t) ;
    }
  }
}

void MATRIX_MUL_ADD_2(MATRIX_MxV A, MATRIX_VxM B, MATRIX_MxM C){
  const int N = QRUOV_M ;
  const int K = QRUOV_V ;
  const int M = QRUOV_M ;
  int i,j,k ;
  _Pragma("omp parallel for private(i,j,k) shared(A, B, C)")
  for(i=0;i<N;i++){
    for(j=0;j<M;j++){
      Fql_vec_accumulator t = Fql_vec_accumulator_zero ;
      for(k=0;k<K;k+=8){
        Fql BT[8] ;
	for(int l=0;l<8;l++) BT[l]=((k+l)<K)?B[k+l][j]:Fql_zero;
        Fql_vec a QRUOV_aligned = Fql_array2vec(A[i],k,K) ;
        Fql_vec b QRUOV_aligned = Fql_array2vec(BT,0,(8<K)?8:K) ;
        t = Fql_vec_accumulator_add(t, Fql_vec_mul_to_accumulator(a,b)) ;
      }
      C[i][j] = Fql_add(C[i][j], Fql_vec_accumulator_horizontal_sum(t)) ;
    }
  }
}

#if 0
void MATRIX_MUL_DEBUG2(int N, int K, int M, MATRIX_MxV A, MATRIX_VxV B, MATRIX_MxV C){
  int _i,_j,_k ;
  _Pragma("omp parallel for private(_i,_j,_k) shared(A, B, C)")
  for(_i=0;_i<N;_i++){
    for(_j=0;_j<M;_j++){
      Fql _t = Fql_zero ;
      for(_k=0;_k<K;_k++){
      _t = Fql_add(_t, Fql_mul(A[_i][_k],B[_k][_j])) ;
//        _t = Fql_add(_t, Fql_mul(A[_i][_k],B[_j][_k])) ;
      }
      C[_i][_j] = _t ;
    }
  }
}

void MATRIX_MUL_DEBUG3(int N, int K, int M, MATRIX_MxV A, MATRIX_VxV B, MATRIX_MxV C){
  int _i,_j,_k ;
  _Pragma("omp parallel for private(_i,_j,_k) shared(A, B, C)")
  for(_i=0;_i<N;_i++){
    for(_j=0;_j<M;_j++){
      Fql _t = Fql_zero ;
      for(_k=0;_k<K;    ){
//      _t = Fql_add(_t, Fql_mul(A[_i][_k],B[_k][_j])) ;


        _t = Fql_add(_t, Fql_mul(A[_i][_k],B[_j][_k])) ; _k++ ; if(_k>=K) break ;
        _t = Fql_add(_t, Fql_mul(A[_i][_k],B[_j][_k])) ; _k++ ; if(_k>=K) break ;
        _t = Fql_add(_t, Fql_mul(A[_i][_k],B[_j][_k])) ; _k++ ; if(_k>=K) break ;
        _t = Fql_add(_t, Fql_mul(A[_i][_k],B[_j][_k])) ; _k++ ; if(_k>=K) break ;

        _t = Fql_add(_t, Fql_mul(A[_i][_k],B[_j][_k])) ; _k++ ; if(_k>=K) break ;
        _t = Fql_add(_t, Fql_mul(A[_i][_k],B[_j][_k])) ; _k++ ; if(_k>=K) break ;
        _t = Fql_add(_t, Fql_mul(A[_i][_k],B[_j][_k])) ; _k++ ; if(_k>=K) break ;
        _t = Fql_add(_t, Fql_mul(A[_i][_k],B[_j][_k])) ; _k++ ;
      }
      C[_i][_j] = _t ;
    }
  }
}
#endif

void MATRIX_MUL_MxV_VxV(MATRIX_MxV A, MATRIX_VxV B, MATRIX_MxV C){
//  MATRIX_MUL(Fql, QRUOV_M, QRUOV_V, QRUOV_V, A, B, C) ;
  MATRIX_MUL_1(A, B, C) ;
}

void MATRIX_MUL_MxV_VxM(MATRIX_MxV A, MATRIX_VxM B, MATRIX_MxM C){
//  MATRIX_MUL(Fql, QRUOV_M, QRUOV_V, QRUOV_M, A, B, C) ;
  MATRIX_MUL_2(A, B, C) ;
}

void MATRIX_MUL_ADD_MxV_VxM(MATRIX_MxV A, MATRIX_VxM B, MATRIX_MxM C){
//  MATRIX_MUL_ADD(Fql, QRUOV_M, QRUOV_V, QRUOV_M, A, B, C) ;
  MATRIX_MUL_ADD_2(A, B, C) ;
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
