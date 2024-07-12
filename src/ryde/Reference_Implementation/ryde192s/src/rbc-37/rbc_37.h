#ifndef RBC_37_H
#define RBC_37_H

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <x86intrin.h>

#define RBC_37_FIELD_Q 2
#define RBC_37_FIELD_M 37

#define RBC_37_ELT_SIZE 1
#define RBC_37_ELT_DATA_SIZE 1

#define RBC_37_ELT_UR_SIZE 2
#define RBC_37_ELT_UR_DATA_SIZE 2

#define RBC_37_ELT_UINT8_SIZE 5
#define RBC_37_ELT_UR_UINT8_SIZE 10


#define RBC_37_ELT_MASK 31


#define RBC_37_INTEGER_LENGTH 64

typedef int64_t rbc_37_elt_int;
typedef uint64_t rbc_37_elt_uint;
typedef uint64_t rbc_37_elt[RBC_37_ELT_SIZE];
typedef uint64_t rbc_37_elt_ur[RBC_37_ELT_UR_SIZE];
typedef uint64_t* rbc_37_elt_ptr;

typedef rbc_37_elt* rbc_37_vec;
typedef rbc_37_elt* rbc_37_vspace;

typedef struct {
  rbc_37_vec v;
  int32_t max_degree;
  // Do not use degree, it is deprecated and will be removed later
  // Kept temporarily for compatibility with rbc_37_qpoly
  int32_t degree;
} rbc_37_poly_struct;

typedef struct {
	 uint32_t coeffs_nb;
	 uint32_t* coeffs;
} rbc_37_poly_sparse_struct;

typedef rbc_37_poly_struct* rbc_37_poly;
typedef rbc_37_poly_sparse_struct* rbc_37_poly_sparse;

typedef rbc_37_poly rbc_37_qre;

typedef rbc_37_vec* rbc_37_mat;
typedef uint64_t** rbc_37_mat_fq;
typedef uint64_t* rbc_37_perm;

typedef struct {
  rbc_37_mat_fq P;
  rbc_37_mat_fq Q;
  uint32_t n;
} rbc_37_isometry;

void rbc_37_field_init(void);
extern uint64_t RBC_37_SQR_LOOKUP_TABLE[256];
static const rbc_37_elt RBC_37_ELT_MODULUS = {0x0000002000000053};

#ifndef min
  #define min(a,b) (((a)<(b))?(a):(b))
#endif

#ifndef max
  #define max(a,b) (((a)>(b))?(a):(b))
#endif

#endif
