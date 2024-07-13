// clang-format off
#ifndef HAETAE_PARAMS_H
#define HAETAE_PARAMS_H

#include "config.h"

#define SEEDBYTES 32
#define CRHBYTES 64
#define N 256
#define ROOT_OF_UNITY 3

#define Q 64513
#define DQ (Q << 1)// 2Q

#define SCALE_BITS 16
#define SCALE (1u << SCALE_BITS)
#define MAX_ENC_H_BYTES ((N*K*SCALE_BITS)/8+4)
#define MAX_ENC_HB_Z1_BYTES ((N*L*SCALE_BITS)/8+4)

#if HAETAE_MODE == 2
#define K 2
#define L 4
#define ETA 1
#define TAU 58
#define B0 9388.96
#define B1 9382.26
#define B2 12320.79
#define GAMMA 48.83
#define LN 8192 // Large N
#define SQNM 39.191835884530846 // \sqrt(n * m)
#define D 1
#define CRYPTO_BYTES 1463

#define ALPHA_HINT 512
#define LOG_ALPHA_HINT 9

#define POLYB1_PACKEDBYTES 480     // 15bits * N / 8bits
#define POLYQ_PACKEDBYTES 480  // 16bits * N / 8bits

#elif HAETAE_MODE == 3
#define K 3
#define L 6
#define ETA 1
#define TAU 80
#define B0 17773.20
#define B1 17766.15
#define B2 21365.10
#define GAMMA 57.68
#define LN 8192 // Large N
#define SQNM 48.0
#define D 1
#define CRYPTO_BYTES 2337

#define ALPHA_HINT 512
#define LOG_ALPHA_HINT 9

#define POLYB1_PACKEDBYTES 480     // 15bits * N / 8bits
#define POLYQ_PACKEDBYTES 480  // 16bits * N / 8bits

#elif HAETAE_MODE == 5
#define K 4
#define L 7
#define ETA 1
#define TAU 60
#define B0 20614.9815
#define B1 20609.9152
#define B2 23740.4482
#define GAMMA 55.13
#define LN 8192 // Large N
#define SQNM 53.0659966456864
#define D 0
#define CRYPTO_BYTES 2908

#define ALPHA_HINT 256
#define LOG_ALPHA_HINT 8

#define POLYB1_PACKEDBYTES 512     // 16bits * N / 8bits
#define POLYQ_PACKEDBYTES 512  // 16bits * N / 8bits

#endif // HAETAE_MODE

#define HALF_ALPHA_HINT (ALPHA_HINT >> 1) // ALPHA / 2

#define B0SQ ((uint64_t)(B0*B0))
#define B1SQ ((uint64_t)(B1*B1))
#define B2SQ ((uint64_t)(B2*B2))

#define M (L-1)

#if ETA == 1
#define POLYETA_PACKEDBYTES 64
#define POLY2ETA_PACKEDBYTES 96
#elif ETA == 2
#define POLYETA_PACKEDBYTES 96
#endif

#define POLYC_PACKEDBYTES 32       // 1bit * N / 8bits
#define POLY_HIGHBITS_PACKEDBYTES (N * 9 / 8)
#define POLYVECK_HIGHBITS_PACKEDBYTES (POLY_HIGHBITS_PACKEDBYTES * K)
#define POLYVECK_BYTES (K * N * sizeof(int32_t))
#define POLYVECL_BYTES (L * N * sizeof(int32_t))

#define CRYPTO_PUBLICKEYBYTES (SEEDBYTES + K * POLYQ_PACKEDBYTES)                                      // seed + b
#if D == 1
#define CRYPTO_SECRETKEYBYTES (CRYPTO_PUBLICKEYBYTES + M * POLYETA_PACKEDBYTES + K * POLY2ETA_PACKEDBYTES + SEEDBYTES)  // pk + s + K
#elif D == 0
#define CRYPTO_SECRETKEYBYTES (CRYPTO_PUBLICKEYBYTES + (M + K) * POLYETA_PACKEDBYTES + SEEDBYTES)  // pk + s + K
#else
#error
#endif
#endif
// clang-format on
