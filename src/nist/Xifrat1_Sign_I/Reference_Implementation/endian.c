/* DannyNiu/NJF, 2018-01-30. Public Domain. */

#include "endian.h"

#ifndef MySuiteA_UsePlatformEndianFuncs

#define htobe(b)                                        \
    uint##b##_t htobe##b(uint##b##_t x)                 \
    {                                                   \
        int i;                                          \
        union { uint8_t o[b/8]; uint##b##_t h; } v;     \
        uint##b##_t ret = 0;                            \
        v.h = x;                                        \
        for(i=0; i<b/8; i++) {                          \
            ret <<= 8;                                  \
            ret |= v.o[i];                              \
        }                                               \
        return ret;                                     \
    }

#define betoh(b)                                        \
    uint##b##_t be##b##toh(uint##b##_t x)               \
    {                                                   \
        int i;                                          \
        union { uint8_t o[b/8]; uint##b##_t h; } v;     \
        for(i=0; i<b/8; i++) {                      \
            v.o[i] = (uint8_t)(x >> (b-i*8-8));         \
        }                                               \
        return v.h;                                     \
    }

#define htole(b)                                        \
    uint##b##_t htole##b(uint##b##_t x)                 \
    {                                                   \
        int i;                                          \
        union { uint8_t o[b/8]; uint##b##_t h; } v;     \
        uint##b##_t ret = 0;                            \
        v.h = x;                                        \
        for(i=0; i<b/8; i++) {                          \
            ret |= (uint##b##_t)v.o[i] << i*8;          \
        }                                               \
        return ret;                                     \
    }

#define letoh(b)                                        \
    uint##b##_t le##b##toh(uint##b##_t x)               \
    {                                                   \
        int i;                                          \
        union { uint8_t o[b/8]; uint##b##_t h; } v;     \
        for(i=0; i<b/8; i++) {                          \
            v.o[i] = (uint8_t)(x >> (i*8));             \
        }                                               \
        return v.h;                                     \
    }

htobe(16)
htobe(32)
htobe(64)

betoh(16)
betoh(32)
betoh(64)

htole(16)
htole(32)
htole(64)

letoh(16)
letoh(32)
letoh(64)

#endif /* MySuiteA_UsePlatformEndianFuncs */
