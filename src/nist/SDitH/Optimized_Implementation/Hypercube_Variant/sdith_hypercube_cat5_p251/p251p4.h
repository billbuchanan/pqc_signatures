#ifndef SIMPLE_EXAMPLE_SDITH_ARITHMETIC_P251P4_H_
#define SIMPLE_EXAMPLE_SDITH_ARITHMETIC_P251P4_H_

#include "p251.h"

#ifdef __cplusplus
#include <cstdint>
#define EXPORT extern "C"
#define EXPORT_DECL extern "C"
#else
#include <stdint.h>
#define EXPORT
#define EXPORT_DECL extern
#endif

// We are going to represent p251p2 as P(X) mod X^2-X-7 mod over GF256.
// A field element a+bX is represented in binary by the concatenation of two bytes (a,b)
// We are going to represent p251p4 as Q(Y) mod Y^2-0x100 over GF2p16
// A field element c+dY is represented in binary by the concatenation of two 16-bytes (c,d)

#define SDITH_IRRED_CST_P251P2 2
#define SDITH_IRRED_CST_P251P4 0x101
#define SDITH_GEN_P251P4 0x703d9e62
#define SDITH_GEN_P251P2 0x2984
//Generator of p251p4: 0x703d9e62
//Generator of p251p2: 0x2984
//Generator of p251  : 0x06
#define SDITH_ORDER_P251P2 63000U
#define SDITH_ORDER_P251P4 3969126000U
#define SDITH_PP_POWER_P251P4 63002U    // x^PP_POWER is in p251p2 for all x in p251p4
#define SDITH_PP_POWER_P251P2 252U      // x^PP_POWER is in p251 for all x in p251p2

/** @brief multiplication in p251p4 constant time */
EXPORT_DECL uint32_t (*p251p4_mul_ct)(uint32_t x, uint32_t y);

/** @brief multiplication in p251p4 non constant time */
EXPORT_DECL uint32_t (*p251p4_mul)(uint32_t x, uint32_t y);

/** @brief initialize material for multiplication in p251p4 */
EXPORT void p251p4_mul_init();

// naive constant time implementation

/** @brief naive multiplication in p251p2 (constant time) */
EXPORT uint16_t p251p2_mul_naive(uint16_t x, uint16_t y);

/** @brief naive multiplication in p251p4 (constant time) */
EXPORT uint32_t p251p4_mul_naive(uint32_t x, uint32_t y);

/** @brief naive (divide and conquer) pow in p251p4 (constant time in x, not in n) */
EXPORT uint32_t p251p4_pow_naive(uint32_t x, uint64_t n);

/** @brief naive addition in p251p4 (mod X^2-Y) */
uint32_t p251p4_add_naive(uint32_t x, uint32_t y);

/** @brief naive addition in p251p4 (mod X^2-Y) */
uint32_t p251p4_sub_naive(uint32_t x, uint32_t y);

EXPORT_DECL uint16_t sdith_p251p2_logtable[65536];      // full domain logtable.
EXPORT_DECL uint16_t sdith_p251p2_exptable[SDITH_ORDER_P251P2+1];
EXPORT_DECL uint32_t sdith_p251p4_log1table[SDITH_ORDER_P251P2+1];     // for x=u+Y: table of log16(u) -> log32(u+Y)
EXPORT_DECL uint16_t sdith_p251p4_exp1table[SDITH_PP_POWER_P251P4][2];  // table of p -> (log16(u),log16(v))  where gen32^p = u+vY

/** @brief lookup-table based discrete log in p251p4 (non constant time)
 * Please initialize the p251p4 log tables once before using this function */
EXPORT uint32_t p251p4_dlog(uint32_t x);

/** @brief lookup-table based discrete exp in p251p4 (non constant time)
 * Please initialize the p251p4 log tables once before using this function */
EXPORT uint32_t p251p4_dexp(uint32_t x);

EXPORT uint32_t p251p4_pow(uint32_t x, uint32_t p);
EXPORT uint32_t p251p4_dlog_pow_l32(uint32_t logx, uint32_t p);
EXPORT uint32_t p251p4_dlog_mul_l32_l32(uint32_t logx, uint32_t logy);

/** @brief initialize the p251p4 log tables, needed for gf2p32 dlog and dexp. */
EXPORT void create_p251p4_log_tables();

#endif  // SIMPLE_EXAMPLE_SDITH_ARITHMETIC_P251P4_H_
