#ifndef SDITH_GF251_AVX2_CT_H
#define SDITH_GF251_AVX2_CT_H

#include <stdint.h>

void p251_vec_mat16cols_muladd_avx2_ct(void *vz, uint8_t const *vx, void const *my, uint64_t m, uint64_t scaling);
void p251_vec_mat16cols_muladd_b16_avx2_ct(void *vz, uint8_t const *vx, void const *my, uint64_t m, uint64_t scaling);
void p251_vec_mat128cols_muladd_avx2_ct(void *vz, uint8_t const *vx, void const *my, uint64_t m);

#endif /* SDITH_GF251_AVX2_CT_H */
