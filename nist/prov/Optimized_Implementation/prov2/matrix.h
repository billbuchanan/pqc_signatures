#ifndef MATRIX_H
#define MATRIX_H

#include <stdio.h>

#include "field.h"

#define MATRIX_ASSIGN 0
#define MATRIX_ADD    1

#define MATRIX_SIZE_T uint16_t

void matrix_print(MATRIX_SIZE_T nrows, MATRIX_SIZE_T ncols, uint8_t M[nrows][ncols]);

void matrix_print_vec(MATRIX_SIZE_T nrows, uint8_t v[nrows]);

void matrix_copy(MATRIX_SIZE_T nrows, MATRIX_SIZE_T ncols, uint8_t B[nrows][ncols], uint8_t A[nrows][ncols]);

void matrix_transpose(MATRIX_SIZE_T nrows, MATRIX_SIZE_T ncols, uint8_t B[nrows][ncols], uint8_t A[ncols][nrows]);

void matrix_add(MATRIX_SIZE_T nrows, MATRIX_SIZE_T ncols, uint8_t C[nrows][ncols], uint8_t A[nrows][ncols], uint8_t B[nrows][ncols]);

void matrix_mul(MATRIX_SIZE_T nrows, MATRIX_SIZE_T ncols, MATRIX_SIZE_T nlayers, uint8_t C[nrows][ncols], uint8_t A[nrows][nlayers], uint8_t B[nlayers][ncols],unsigned char add);

void matrix_mul_transpose(MATRIX_SIZE_T nrows, MATRIX_SIZE_T ncols, MATRIX_SIZE_T nlayers, uint8_t C[nrows][ncols], uint8_t A[nrows][nlayers], uint8_t B_transpose[ncols][nlayers], unsigned char add);

void matrix_mul_vec(MATRIX_SIZE_T nrows, MATRIX_SIZE_T ncols, uint8_t y[nrows], uint8_t A[nrows][ncols], uint8_t x[ncols]);

int matrix_pivot(MATRIX_SIZE_T nrows, MATRIX_SIZE_T ncols, uint8_t A[nrows][ncols]);

int matrix_solve(MATRIX_SIZE_T nrows, MATRIX_SIZE_T ncols, uint8_t x[ncols], uint8_t A[nrows][ncols], uint8_t y[nrows]);

void matrix_lower_to_full(MATRIX_SIZE_T dim, uint8_t matrix[dim][dim], uint8_t *lower);

void matrix_full_to_lower(MATRIX_SIZE_T dim, uint8_t *lower, uint8_t matrix[dim][dim]);

void matrix_sym(MATRIX_SIZE_T dim, uint8_t A[dim][dim]);

void matrix_add_own_transpose(MATRIX_SIZE_T dim, uint8_t A[dim][dim]);

uint8_t matrix_lower_bilin(MATRIX_SIZE_T dim, uint8_t *lower, uint8_t v[dim]);

uint8_t matrix_full_bilin(MATRIX_SIZE_T nrows, MATRIX_SIZE_T ncols, uint8_t M[nrows][ncols], uint8_t x[nrows], uint8_t y[nrows]);

#endif /*MATRIX_H*/
