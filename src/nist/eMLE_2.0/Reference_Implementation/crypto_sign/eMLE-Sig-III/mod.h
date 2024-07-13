#ifndef _MOD_H
#define _MOD_H

#include <stdint.h>

/* Plantard's modular reductions */
static inline uint32_t mod_5(uint32_t a)
{
    uint32_t c = a * 858993460;
    return (((c >> 16) + 1) * 5) >> 16;
}

static inline uint64_t mod_557(uint64_t a)
{
    uint64_t c = a * 33118032448311583ULL;
    return (((c >> 32) + 1) * 557) >> 32;
}

static inline uint64_t mod_823(uint64_t a)
{
    uint64_t c = a * 22414026821032262ULL;
    return (((c >> 32) + 1) * 823) >> 32;
}

static inline uint64_t mod_1097(uint64_t a)
{
    uint64_t c = a * 16815628143764405ULL;
    return (((c >> 32) + 1) * 1097) >> 32;
}

static inline uint16_t mod_9(uint16_t a)
{
    uint16_t c = a * 7282;
    return (((c >> 8) + 1) * 9) >> 8;
}

/* return (x < y) at the most significant bit */
static inline uint64_t ct_lt(uint64_t x, uint64_t y)
{
    return x - y;
}

/* return (x != y) at the most significant bit */
static inline uint64_t ct_neq(uint64_t x, uint64_t y)
{
    return (x - y) | (y - x);
}

#endif
