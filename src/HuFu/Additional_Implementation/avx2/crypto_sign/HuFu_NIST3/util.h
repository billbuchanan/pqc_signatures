#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>

void cbd(int32_t *R, uint8_t *seed_R);

int cholesky(double *L, double *A, int dim);

int32_t modq(int32_t a);

void tri_mat_mul_avx2_con1(double *C, const double *A, const double *B, int row);

void mat_mul_avx2_con1(int32_t *C, const int32_t *A, const int32_t *B, int l1, int l2);

void mat_mul_avx2(int32_t *C, const int32_t *A, const int32_t *B, int l1, int l2, int l3);

void mat_add_avx2(int32_t *C, const int32_t *A, const int32_t *B, int row, int col);

void mat_sub_avx2(int32_t *C, const int32_t *A, const int32_t *B, int row, int col);

void real_mat_mul_avx2_con1(double *C, const double *A, const double *B, int l1, int l2);

void real_mat_mul_avx2_con1_int32(double *C, const int32_t *A, const double *B, int l1, int l2);

void real_mat_add(double *C, const double *A, const double *B, int row, int col);

void mat_sub(int32_t *C, const int32_t *A, const int32_t *B, int row, int col);

void mat_neg(int32_t *A, int row, int col);

int generate_auxiliary_mat(double *L_22, double *L_32, double *L_33, int32_t *R);

double norm_square(const int32_t *v, int n);

void tri_mat_inv(double *OUT, const double *IN);

void multiply_fixed_float_avx2(double *E, size_t size, double factor, double *result);
#endif