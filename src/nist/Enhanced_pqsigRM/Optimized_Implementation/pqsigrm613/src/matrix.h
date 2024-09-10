#ifndef __MATRIX_H
#define __MATRIX_H

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define MATRIX_NULL 0 

#define INV_SUCCESS 1
#define INV_FAIL 0

#define get_element(A, i, j) (!!(((A)->elem[(i)][(j) / 64]) & (1ULL << ((j) % 64))))
#define set_element(A, i, j, val) ((A)->elem[(i)][(j) / 64] = ((A)->elem[(i)][(j) / 64] & ~(1ULL << ((j) % 64))) | ((uint64_t)(val) << ((j) % 64)))

typedef struct {
    uint32_t nrows;// number of rows.
    uint32_t ncols;// number of columns.
    uint32_t colsize; // number of 64-bit int in each row
    uint64_t** elem;// elements.
    uint8_t made_with_import;
} matrix;

matrix* new_matrix(uint32_t nrows, uint32_t ncols);
matrix* new_matrix_with_pool (uint32_t nrows, uint32_t ncols, const uint8_t* src);
void init_zero(matrix *self);
void delete_matrix(matrix *self) ;

void copy_matrix(matrix* self, matrix* src);
void export_matrix(matrix* self, uint8_t* dest);
void import_matrix(matrix* self, const uint8_t* src);

void rref(matrix* self);

void get_pivot(matrix* self, uint16_t *lead, uint16_t *lead_diff);

void vec_mat_prod(matrix* self, matrix* mat,  matrix *vec);
void vec_vec_add(matrix* self, matrix* vec);

uint8_t vec_vec_is_equal(matrix* self, matrix *vec);

void dual(matrix* self, matrix* dual_sys);
void row_interchange(matrix* mtx, uint32_t row_idx1, uint32_t row_idx2);
void partial_replace(matrix* self, const uint32_t r1, const uint32_t r2,
        const uint32_t c1, const uint32_t c2, 
        matrix* src, const int r3, const int c3);
void col_permute(matrix* self, const int r1, const int r2
	, const int c1, const int c2, uint16_t* Q);

void codeword(matrix* self, uint8_t* seed, matrix* dest);
uint8_t is_zero(matrix* self);

#endif
