#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>

void cbd(int16_t * R, uint8_t * seed_R);

int cholesky(double * L, double * A, int dim);

int16_t modq(int32_t a);

void tri_mat_mul(double * C, const double * A, const double * B, int row, int col);

void mat_mul(int16_t * C, const int16_t * A, const int16_t * B, int l1, int l2, int l3);

void mat_add(int16_t * C, const int16_t * A, const int16_t * B, int row, int col);

void real_mat_mul(double * C, const double * A, const double * B, int l1, int l2, int l3);
void real_mat_mul_int16(double *C, const int16_t *A, const double *B, int l1, int l2, int l3);
void real_mat_add(double *C, const double *A, const double *B, int row, int col);

void mat_sub(int16_t * C, const int16_t * A, const int16_t * B, int row, int col);

void mat_neg(int16_t * A, int row, int col);

int generate_auxiliary_mat(double * L_22, double * L_32, double * L_33, int16_t * R);

double norm_square(const int16_t * v, int n);

void tri_mat_inv(double *OUT, const double *IN);

void pack_mat_r(uint8_t *buf, const int16_t *data);

void unpack_mat_r(int16_t *data, const uint8_t *buf);


#endif




