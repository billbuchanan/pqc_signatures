#ifndef P251_H
#define P251_H

#ifdef __cplusplus
#include <cstdint>
#define EXPORT extern "C"
#define EXPORT_DECL extern "C"
#else
#define EXPORT
#define EXPORT_DECL extern
#include <stdint.h>
#endif

#define SDITH_GEN_P251 0x6 // multiplicative generator of p251

#define p251_init() p251_init_(P251_VERSION)

EXPORT int p251_init_(__attribute__((unused)) __attribute__((unused)) int version);

/// Performs "z[16] += vx[m] * my[m][16]"
/// parallelizaion over m
EXPORT_DECL void (*p251_vec_mat16cols_muladd)(void* vz, void const* vx, void const* my, uint64_t m);
EXPORT_DECL void (*p251_vec_mat16cols_muladd_ct)(void* vz, void const* vx, void const* my, uint64_t m);
/// Performs "z[n] += vx[m] * my[m][n]" where n is a multiple of 32
/// parallelization over n
EXPORT_DECL void (*p251_vec_mat128cols_muladd)(void* vz, void const* vx, void const* my, uint64_t m);
EXPORT_DECL void (*p251_vec_mat128cols_muladd_ct)(void* vz, void const* vx, void const* my, uint64_t m);

/** @brief pure C constant time implementation */
EXPORT void p251_vec_mat16cols_muladd_ref_ct(void* vz, void const* vx, void const* my, uint64_t m);
EXPORT void p251_vec_mat128cols_muladd_ref_ct(void* vz, void const* vx, void const* my, uint64_t m);

/** @brief avx2 constant time (floating point) implementation) */
EXPORT void p251_vec_mat16cols_muladd_avx2_ct(void *vz, void const *vx, void const *my, uint64_t m);
EXPORT void p251_vec_mat16cols_muladd_b16_avx2_ct(void* vz, void const* vx, void const* my, uint64_t m);
EXPORT void p251_vec_mat128cols_muladd_avx2_ct(void* vz, void const* vx, void const* my, uint64_t m);

/** @brief reduction in P251. */
#define p251_reduce_16(x) ((x) - 251 * (((uint32_t)(x) * 33421) >> 23))
#define p251_reduce_32(x) ((x) - 251 * (((uint64_t)(x) * 2190262207) >> 39))
/** @brief multiplication in P251. */
#define p251_mul_naive(x, y) p251_reduce_16((uint16_t)(x) * (uint16_t)(y))
/** @brief multiplication using LUT in P251. */
EXPORT uint8_t p251_mul_table(uint8_t x, uint8_t y);
/** @brief discrete log in P251. */
EXPORT uint8_t p251_dlog(uint8_t x);
/** @brief extended power in P251. */
EXPORT uint8_t p251_pow(uint8_t x, uint8_t p);

EXPORT_DECL uint8_t const* sdith_p251_dexp_table;
EXPORT_DECL uint8_t const* sdith_p251_dlog_table;

EXPORT void p251_create_log_tables();

#endif // P251_H
