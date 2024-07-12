#ifndef RBC_43_H
#define RBC_43_H

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <x86intrin.h>

#define RBC_43_FIELD_Q 2
#define RBC_43_FIELD_M 43

#define RBC_43_ELT_SIZE 1
#define RBC_43_ELT_DATA_SIZE 1

#define RBC_43_ELT_UR_SIZE 2
#define RBC_43_ELT_UR_DATA_SIZE 2

#define RBC_43_ELT_UINT8_SIZE 6
#define RBC_43_ELT_UR_UINT8_SIZE 11


#define RBC_43_ELT_MASK 7


#define RBC_43_INTEGER_LENGTH 64

typedef int64_t rbc_43_elt_int;
typedef uint64_t rbc_43_elt_uint;
typedef uint64_t rbc_43_elt[RBC_43_ELT_SIZE];
typedef uint64_t rbc_43_elt_ur[RBC_43_ELT_UR_SIZE];
typedef uint64_t* rbc_43_elt_ptr;

typedef rbc_43_elt* rbc_43_vec;
typedef rbc_43_elt* rbc_43_vspace;

typedef struct {
  rbc_43_vec v;
  int32_t max_degree;
  // Do not use degree, it is deprecated and will be removed later
  // Kept temporarily for compatibility with rbc_43_qpoly
  int32_t degree;
} rbc_43_poly_struct;

typedef struct {
	 uint32_t coeffs_nb;
	 uint32_t* coeffs;
} rbc_43_poly_sparse_struct;

typedef rbc_43_poly_struct* rbc_43_poly;
typedef rbc_43_poly_sparse_struct* rbc_43_poly_sparse;

typedef rbc_43_poly rbc_43_qre;

typedef rbc_43_vec* rbc_43_mat;
typedef uint64_t** rbc_43_mat_fq;
typedef uint64_t* rbc_43_perm;

typedef struct {
  rbc_43_mat_fq P;
  rbc_43_mat_fq Q;
  uint32_t n;
} rbc_43_isometry;

void rbc_43_field_init(void);
extern uint64_t RBC_43_SQR_LOOKUP_TABLE[256];
static const rbc_43_elt RBC_43_ELT_MODULUS = {0x0000080000000059};

#ifndef min
  #define min(a,b) (((a)<(b))?(a):(b))
#endif

#ifndef max
  #define max(a,b) (((a)>(b))?(a):(b))
#endif

#endif
