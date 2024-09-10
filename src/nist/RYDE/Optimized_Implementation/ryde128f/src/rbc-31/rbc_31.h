#ifndef RBC_31_H
#define RBC_31_H

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <x86intrin.h>

#define RBC_31_FIELD_Q 2
#define RBC_31_FIELD_M 31

#define RBC_31_ELT_SIZE 1
#define RBC_31_ELT_DATA_SIZE 1

#define RBC_31_ELT_UR_SIZE 1
#define RBC_31_ELT_UR_DATA_SIZE 1

#define RBC_31_ELT_UINT8_SIZE 4
#define RBC_31_ELT_UR_UINT8_SIZE 8


#define RBC_31_ELT_MASK 127


#define RBC_31_INTEGER_LENGTH 64

typedef int64_t rbc_31_elt_int;
typedef uint64_t rbc_31_elt_uint;
typedef uint64_t rbc_31_elt[RBC_31_ELT_SIZE];
typedef uint64_t rbc_31_elt_ur[RBC_31_ELT_UR_SIZE];
typedef uint64_t* rbc_31_elt_ptr;

typedef rbc_31_elt* rbc_31_vec;
typedef rbc_31_elt* rbc_31_vspace;

typedef struct {
  rbc_31_vec v;
  int32_t max_degree;
  // Do not use degree, it is deprecated and will be removed later
  // Kept temporarily for compatibility with rbc_31_qpoly
  int32_t degree;
} rbc_31_poly_struct;

typedef struct {
	 uint32_t coeffs_nb;
	 uint32_t* coeffs;
} rbc_31_poly_sparse_struct;

typedef rbc_31_poly_struct* rbc_31_poly;
typedef rbc_31_poly_sparse_struct* rbc_31_poly_sparse;

typedef rbc_31_poly rbc_31_qre;

typedef rbc_31_vec* rbc_31_mat;
typedef uint64_t** rbc_31_mat_fq;
typedef uint64_t* rbc_31_perm;

typedef struct {
  rbc_31_mat_fq P;
  rbc_31_mat_fq Q;
  uint32_t n;
} rbc_31_isometry;

void rbc_31_field_init(void);
static const __m128i RBC_31_ELT_SQR_MASK_128 = {0x0F0F0F0F0F0F0F0F, 0x0F0F0F0F0F0F0F0F};
static const __m128i RBC_31_ELT_SQR_LOOKUP_TABLE_128 = {0x1514111005040100, 0x5554515045444140};
static const rbc_31_elt RBC_31_ELT_MODULUS = {0x0000000080000009};

#ifndef min
  #define min(a,b) (((a)<(b))?(a):(b))
#endif

#ifndef max
  #define max(a,b) (((a)>(b))?(a):(b))
#endif

#endif
