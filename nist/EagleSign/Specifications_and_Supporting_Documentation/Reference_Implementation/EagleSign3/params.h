/*
 * Implementors: EagleSign Team
 * This implementation is highly inspired from Dilithium and
 * Falcon Signatures' implementations
 */

#ifndef PARAMS_H
#define PARAMS_H

#include "config.h"

/*
 * Computation of Constants for NTT.
 *
 *   phi = X^n + 1
 *   Q = 12289
 *   Q0I = -1/Q mod 2^16
 *   R = 2^16 mod Q
 *   R2 = 2^32 mod Q
 */

/* ---------First Level Parameters--------- */
#if EAGLESIGN_MODE == 3
// Fixed Parameters
#define K 1
#define L 1
#define N 1024
#define Q 12289
#define ETAY1 1
#define ETAY2 64
#define ETAG 1
#define ETAD 1
#define T 140
#define TAU 38

// Dependencies
#define Q_EXACT_SIZE_HIGHEST 0x1FFF
#define Q0I 12287
#define QINV 53249
#define R 4091
#define R2 10952
#define LOG_N 10
#define NBYTES 128
#define LOGQ 14
#define LOGETAG 2
#define LOGETAD 2
#define DELTA 178
#define DELTA_PRIME 242
#define LOGDELTA 9
#define LOGDELTA_PRIME 9

#elif EAGLESIGN_MODE == 5
// Fixed Parameters
#define K 1
#define L 2
#define N 1024
#define Q 12289
#define ETAY1 1
#define ETAY2 32
#define ETAG 1
#define ETAD 1
#define T 86
#define TAU 18

// Dependencies
#define Q_EXACT_SIZE_HIGHEST 0x1FFF
#define Q0I 12287
#define R 4091
#define R2 10952
#define LOG_N 10
#define NBYTES 128
#define LOGQ 14
#define LOGETAG 2
#define LOGETAD 2
#define DELTA 208
#define DELTA_PRIME 240
#define LOGDELTA 9
#define LOGDELTA_PRIME 9
#endif
// ------------------------------------------

/* ---------Common Parameters-------------- */
#define Q_SIZE uint16_t
#define S_Q_SIZE int16_t
#define Q_BIT_SIZE 16
#define DOUBLE_Q_BIT_SIZE 32
#define DOUBLE_Q_SIZE uint32_t
#define S_DOUBLE_Q_SIZE int32_t
#define TWO_POWER_SIZE_Q_MINUS_ONE 0xFFFF
#define SEEDBYTES 32
#define CRHBYTES 48

#define T_(s) T_##s

#define CRYPTO_EAGLESIGN_PUBLICKEYBYTES (SEEDBYTES + NBYTES * L * K * LOGQ)
#define CRYPTO_EAGLESIGN_SECRETKEYBYTES (2 * SEEDBYTES + NBYTES * L * L * LOGETAG + NBYTES * K * L * LOGETAD)
#define CRYPTO_EAGLESIGN_BYTES (SEEDBYTES + NBYTES * (L * LOGDELTA + K * LOGDELTA_PRIME))
// ------------------------------------------
#endif
