#pragma once
#include "qruov_misc.h"

typedef Fql        VECTOR_V        [aligned_V]        ;
typedef Fql        VECTOR_M        [aligned_M]        ;
typedef VECTOR_V   MATRIX_VxV      [QRUOV_V]        ;
typedef VECTOR_V   MATRIX_MxV      [QRUOV_M]        ;
typedef VECTOR_M   MATRIX_VxM      [QRUOV_V]        ;
typedef VECTOR_M   MATRIX_MxM      [QRUOV_M]        ;

extern void MATRIX_MUL_MxV_VxV(MATRIX_MxV A, MATRIX_VxV B, MATRIX_MxV C) ;
extern void MATRIX_MUL_MxV_VxM(MATRIX_MxV A, MATRIX_VxM B, MATRIX_MxM C) ;
extern void MATRIX_MUL_ADD_MxV_VxM(MATRIX_MxV A, MATRIX_VxM B, MATRIX_MxM C) ; // C += A*B
extern void MATRIX_SUB_MxV(MATRIX_MxV A, MATRIX_MxV B, MATRIX_MxV C) ;
extern void MATRIX_ADD_MxM(MATRIX_MxM A, MATRIX_MxM B, MATRIX_MxM C) ;
extern void MATRIX_TRANSPOSE_VxM(MATRIX_VxM A, MATRIX_MxV C) ;

typedef MATRIX_VxM QRUOV_Sd                         ;
typedef MATRIX_MxV QRUOV_SdT                        ;
typedef MATRIX_VxV QRUOV_P1        [QRUOV_m]        ;
typedef MATRIX_VxM QRUOV_P2        [QRUOV_m]        ;
typedef MATRIX_MxV QRUOV_P2T       [QRUOV_m]        ;
typedef MATRIX_MxM QRUOV_P3        [QRUOV_m]        ;
