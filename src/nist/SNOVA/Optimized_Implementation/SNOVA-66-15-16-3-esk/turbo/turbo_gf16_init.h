#include <stdint.h>
#ifndef TURBO_GF16_INIT_H
#define TURBO_GF16_INIT_H
#include "../gf16.h"

#if rank == 2 || rank == 4
static uint32_t turbo_mt4b[65536] = {0};
#else
static uint32_t turbo_mt4b[65536] = {0};
#endif

int gf16_trubo_tables_is_init = 0;

#ifdef __cplusplus
extern "C" {
#endif

/**
 *
 */
void init_turbo_gf16_tables() {
    if (gf16_trubo_tables_is_init) {
        return;
    }
    gf16_trubo_tables_is_init = 1;

#if rank == 2 || rank == 4
    for (int a = 0; a < 16; ++a) {
        for (int b = 0; b < 16; ++b) {
            for (int c = 0; c < 16; ++c) {
                for (int d = 0; d < 16; ++d) {
                    *((uint8_t *)(&(
                          turbo_mt4b[(d << 12) ^ (c << 8) ^ (b << 4) ^ a])) +
                      0) = mt(a, b);
                    *((uint8_t *)(&(
                          turbo_mt4b[(d << 12) ^ (c << 8) ^ (b << 4) ^ a])) +
                      1) = mt(a, d);
                    *((uint8_t *)(&(
                          turbo_mt4b[(d << 12) ^ (c << 8) ^ (b << 4) ^ a])) +
                      2) = mt(c, b);
                    *((uint8_t *)(&(
                          turbo_mt4b[(d << 12) ^ (c << 8) ^ (b << 4) ^ a])) +
                      3) = mt(c, d);
                }
            }
        }
    }
#else
    for (int a = 0; a < 16; ++a) {
        for (int b = 0; b < 16; ++b) {
            for (int c = 0; c < 16; ++c) {
                for (int d = 0; d < 16; ++d) {
                    *((uint8_t *)(&(
                          turbo_mt4b[(d << 12) ^ (c << 8) ^ (b << 4) ^ a])) +
                      0) = mt(a, b);
                    *((uint8_t *)(&(
                          turbo_mt4b[(d << 12) ^ (c << 8) ^ (b << 4) ^ a])) +
                      1) = mt(a, c);
                    *((uint8_t *)(&(
                          turbo_mt4b[(d << 12) ^ (c << 8) ^ (b << 4) ^ a])) +
                      2) = mt(a, d);
                }
            }
        }
    }

#endif
}

#ifdef __cplusplus
}
#endif

#endif