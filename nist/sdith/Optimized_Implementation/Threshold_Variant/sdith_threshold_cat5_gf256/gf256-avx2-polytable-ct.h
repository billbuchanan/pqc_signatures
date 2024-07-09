#ifndef SDITH_GF256_AVX2_POLYTABLE_CT_H
#define SDITH_GF256_AVX2_POLYTABLE_CT_H

#include <stdint.h>

void gf256_vec_mat16cols_muladd_polytable_avx2_ct(void* vz, void const* vx, void const* my, uint64_t m, uint64_t scaling);
void gf256_vec_mat128cols_muladd_polytable_avx2_ct(void* vz, void const* vx, void const* my, uint64_t m, uint64_t scaling);

#endif /* SDITH_GF256_AVX2_POLYTABLE_CT_H */
