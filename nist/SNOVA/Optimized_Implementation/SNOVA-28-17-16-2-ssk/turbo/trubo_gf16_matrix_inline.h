#ifndef TURBO_GF16M_INLINE_H
#define TURBO_GF16M_INLINE_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../deriv_params.h"
#include "../gf16_init.h"
#include "../gf16_matrix.h"
#include "turbo_gf16_init.h"

// POD -> entry[a][b] * (entry[c][d] * entry[e][f] + entry[g][h] * entry[i][j])
#define POD(entry, a, b, c, d, e, f, g, h, i, j)                          \
    gf16_get_mul(                                                         \
        get_gf16m(entry, a, b),                                           \
        gf16_get_add(                                                     \
            gf16_get_mul(get_gf16m(entry, c, d), get_gf16m(entry, e, f)), \
            gf16_get_mul(get_gf16m(entry, g, h), get_gf16m(entry, i, j))))

#ifdef __cplusplus
extern "C" {
#endif

#if rank == 2 || rank == 4
static inline uint32_t gf16_get_mul_4(uint8_t a, uint8_t b, uint8_t c,
                                      uint8_t d) {
    return turbo_mt4b[(d << 12) ^ (c << 8) ^ (b << 4) ^ a];
}

static inline uint32_t gf16m_2x2_kernel(gf16_t* a, gf16_t* b) {
    return gf16_get_mul_4(a[0], b[0], a[2], b[1]) ^
           gf16_get_mul_4(a[1], b[2], a[3], b[3]);
}

#else  // rank == 3
static inline uint32_t gf16m_1x3_kernel(gf16_t a, gf16_t* b) {
    return turbo_mt4b[(b[2] << 12) ^ (b[1] << 8) ^ (b[0] << 4) ^ a];
}
#endif

#if rank == 2
static inline void gf16m_mul(gf16m_t a, gf16m_t b, gf16m_t c) {
    *((uint32_t*)c) = gf16m_2x2_kernel(a, b);
}
#elif rank == 4
static inline void gf16m_mul(gf16m_t a, gf16m_t b, gf16m_t c) {
    uint32_t sum[rank];
    gf16_t A0[4] = {get_gf16m(a, 0, 0), get_gf16m(a, 0, 1), get_gf16m(a, 1, 0),
                    get_gf16m(a, 1, 1)};
    gf16_t A1[4] = {get_gf16m(a, 0, 2), get_gf16m(a, 0, 3), get_gf16m(a, 1, 2),
                    get_gf16m(a, 1, 3)};
    gf16_t A2[4] = {get_gf16m(a, 2, 0), get_gf16m(a, 2, 1), get_gf16m(a, 3, 0),
                    get_gf16m(a, 3, 1)};
    gf16_t A3[4] = {get_gf16m(a, 2, 2), get_gf16m(a, 2, 3), get_gf16m(a, 3, 2),
                    get_gf16m(a, 3, 3)};
    gf16_t B0[4] = {get_gf16m(b, 0, 0), get_gf16m(b, 0, 1), get_gf16m(b, 1, 0),
                    get_gf16m(b, 1, 1)};
    gf16_t B1[4] = {get_gf16m(b, 0, 2), get_gf16m(b, 0, 3), get_gf16m(b, 1, 2),
                    get_gf16m(b, 1, 3)};
    gf16_t B2[4] = {get_gf16m(b, 2, 0), get_gf16m(b, 2, 1), get_gf16m(b, 3, 0),
                    get_gf16m(b, 3, 1)};
    gf16_t B3[4] = {get_gf16m(b, 2, 2), get_gf16m(b, 2, 3), get_gf16m(b, 3, 2),
                    get_gf16m(b, 3, 3)};

    sum[0] = gf16m_2x2_kernel(A0, B0) ^ gf16m_2x2_kernel(A1, B2);
    sum[1] = gf16m_2x2_kernel(A0, B1) ^ gf16m_2x2_kernel(A1, B3);
    sum[2] = gf16m_2x2_kernel(A2, B0) ^ gf16m_2x2_kernel(A3, B2);
    sum[3] = gf16m_2x2_kernel(A2, B1) ^ gf16m_2x2_kernel(A3, B3);

    uint16_t* s_u16 = (uint16_t*)sum;
    uint16_t* c_u16 = (uint16_t*)c;

    c_u16[0] = s_u16[0];
    c_u16[1] = s_u16[2];
    c_u16[2] = s_u16[1];
    c_u16[3] = s_u16[3];
    c_u16[4] = s_u16[4];
    c_u16[5] = s_u16[6];
    c_u16[6] = s_u16[5];
    c_u16[7] = s_u16[7];
}

#else  // rank == 3
static inline void gf16m_mul(gf16m_t a, gf16m_t b, gf16m_t c) {
    uint8_t* pt_c = c;
    uint32_t tmp = 0;
    for (int i = 0; i < rank; ++i) {
        tmp = gf16m_1x3_kernel(get_gf16m(a, i, 0), b + 0);
        tmp ^= gf16m_1x3_kernel(get_gf16m(a, i, 1), b + 3);
        tmp ^= gf16m_1x3_kernel(get_gf16m(a, i, 2), b + 6);
        pt_c[0] = *(((uint8_t*)&tmp) + 0);
        pt_c[1] = *(((uint8_t*)&tmp) + 1);
        pt_c[2] = *(((uint8_t*)&tmp) + 2);
        pt_c += rank;
    }
}

#endif

/**
 * Zeroing the GF16 Matrix a.
 */
static inline void gf16m_set_zero(gf16m_t a) { memset(a, 0, sq_rank); }

/**
 * Adding GF16 Matrices. c = a + b
 */
static inline void gf16m_add(gf16m_t a, gf16m_t b, gf16m_t c) {
    for (int i = 0; i < rank; ++i) {
        for (int j = 0; j < rank; ++j) {
            set_gf16m(c, i, j,
                      gf16_get_add(get_gf16m(a, i, j), get_gf16m(b, i, j)));
        }
    }
}

/**
 * Scaling the GF16 Matrix. c = Scaling "a" by a factor of "k"
 */
static inline void gf16m_scale(gf16m_t a, gf16_t k, gf16m_t c) {
    for (int i = 0; i < rank; ++i) {
        for (int j = 0; j < rank; ++j) {
            set_gf16m(c, i, j, gf16_get_mul(get_gf16m(a, i, j), k));
        }
    }
}

/**
 * Transposing the GF16 Matrix. ap = aT
 */
static inline void gf16m_transpose(gf16m_t a, gf16m_t ap) {
    for (int i = 0; i < rank; ++i) {
        for (int j = 0; j < rank; ++j) {
            set_gf16m(ap, i, j, get_gf16m(a, j, i));
        }
    }
}

/**
 * Cloning the GF16 Matrix target = source
 */
static inline void gf16m_clone(gf16m_t target, gf16m_t source) {
    memcpy(target, source, sq_rank);
}

/**
 * be_aI
 */
static inline void be_aI(gf16m_t target, gf16_t a) {
    for (int i = 0; i < rank; ++i) {
        for (int j = 0; j < rank; ++j) {
            set_gf16m(target, i, j, (i == j) ? a : 0);
        }
    }
}

/**
 * be_the_S
 */
static inline void be_the_S(gf16m_t target) {
    for (int i = 0; i < rank; ++i) {
        for (int j = 0; j < rank; ++j) {
            set_gf16m(target, i, j, (8 - (i + j)));
        }
    }
}

/**
 * gf16m_det
 */
static inline gf16_t gf16m_det(gf16m_t entry) {
#if rank == 2
    return gf16_get_add(
        gf16_get_mul(get_gf16m(entry, 0, 0), get_gf16m(entry, 1, 1)),
        gf16_get_mul(get_gf16m(entry, 0, 1), get_gf16m(entry, 1, 0)));
    // (entry[0][0] * entry[1][1] + entry[0][1] * entry[1][0]);
#elif rank == 3
    return gf16_get_add(
        gf16_get_add(
            gf16_get_mul(get_gf16m(entry, 0, 0),
                         gf16_get_add(gf16_get_mul(get_gf16m(entry, 1, 1),
                                                   get_gf16m(entry, 2, 2)),
                                      gf16_get_mul(get_gf16m(entry, 1, 2),
                                                   get_gf16m(entry, 2, 1)))),
            // AAAAA(entry, 0, 0, 1, 1, 2, 2, 1, 2, 2, 1),
            gf16_get_mul(get_gf16m(entry, 0, 1),
                         gf16_get_add(gf16_get_mul(get_gf16m(entry, 1, 0),
                                                   get_gf16m(entry, 2, 2)),
                                      gf16_get_mul(get_gf16m(entry, 1, 2),
                                                   get_gf16m(entry, 2, 0))))),
        gf16_get_mul(
            get_gf16m(entry, 0, 2),
            gf16_get_add(
                gf16_get_mul(get_gf16m(entry, 1, 0), get_gf16m(entry, 2, 1)),
                gf16_get_mul(get_gf16m(entry, 1, 1), get_gf16m(entry, 2, 0)))));

    /*
    (
            (entry[0][0] * (entry[1][1] * entry[2][2] + entry[1][2] *
    entry[2][1])) + (entry[0][1] * (entry[1][0] * entry[2][2] + entry[1][2] *
    entry[2][0])) + (entry[0][2] * (entry[1][0] * entry[2][1] + entry[1][1] *
    entry[2][0]))
    )

    */

    // gf16_get_mul(gf16_get_mul(get_gf16m(entry, 0, 1),
    // gf16_get_add(gf16_get_mul(get_gf16m(entry, 1, 0), get_gf16m(entry, 2,
    // 2)), gf16_get_mul(get_gf16m(entry, 1, 2), get_gf16m(entry, 2, 0)))))),
    // gf16_get_mul(gf16_get_mul(get_gf16m(entry, 0, 2),
    // gf16_get_add(gf16_get_mul(get_gf16m(entry, 1, 0), get_gf16m(entry, 2,
    // 1)), gf16_get_mul(get_gf16m(entry, 1, 1), get_gf16m(entry, 2, 0))))));

#elif rank == 4

    gf16_t d0 = gf16_get_mul(
        get_gf16m(entry, 0, 0),
        gf16_get_add(gf16_get_add(POD(entry, 1, 1, 2, 2, 3, 3, 2, 3, 3, 2),
                                  POD(entry, 1, 2, 2, 1, 3, 3, 2, 3, 3, 1)),
                     POD(entry, 1, 3, 2, 1, 3, 2, 2, 2, 3, 1)));

    gf16_t d1 = gf16_get_mul(
        get_gf16m(entry, 0, 1),
        gf16_get_add(gf16_get_add(POD(entry, 1, 0, 2, 2, 3, 3, 2, 3, 3, 2),
                                  POD(entry, 1, 2, 2, 0, 3, 3, 2, 3, 3, 0)),
                     POD(entry, 1, 3, 2, 0, 3, 2, 2, 2, 3, 0)));

    gf16_t d2 = gf16_get_mul(
        get_gf16m(entry, 0, 2),
        gf16_get_add(gf16_get_add(POD(entry, 1, 0, 2, 1, 3, 3, 2, 3, 3, 1),
                                  POD(entry, 1, 1, 2, 0, 3, 3, 2, 3, 3, 0)),
                     POD(entry, 1, 3, 2, 0, 3, 1, 2, 1, 3, 0)));

    gf16_t d3 = gf16_get_mul(
        get_gf16m(entry, 0, 3),
        gf16_get_add(gf16_get_add(POD(entry, 1, 0, 2, 1, 3, 2, 2, 2, 3, 1),
                                  POD(entry, 1, 1, 2, 0, 3, 2, 2, 2, 3, 0)),
                     POD(entry, 1, 2, 2, 0, 3, 1, 2, 1, 3, 0)));

    return gf16_get_add(gf16_get_add(gf16_get_add(d0, d1), d2), d3);
    /*
    (
            entry[0][0] * (
                    (entry[1][1] * (entry[2][2] * entry[3][3] + entry[2][3] *
    entry[3][2])) + (entry[1][2] * (entry[2][1] * entry[3][3] + entry[2][3] *
    entry[3][1])) + (entry[1][3] * (entry[2][1] * entry[3][2] + entry[2][2] *
    entry[3][1]))

            ) +

            entry[0][1] * (
                    (entry[1][0] * (entry[2][2] * entry[3][3] + entry[2][3] *
    entry[3][2])) + (entry[1][2] * (entry[2][0] * entry[3][3] + entry[2][3] *
    entry[3][0])) + (entry[1][3] * (entry[2][0] * entry[3][2] + entry[2][2] *
    entry[3][0])) ) +

            entry[0][2] * (
                    (entry[1][0] * (entry[2][1] * entry[3][3] + entry[2][3] *
    entry[3][1])) + (entry[1][1] * (entry[2][0] * entry[3][3] + entry[2][3] *
    entry[3][0])) + (entry[1][3] * (entry[2][0] * entry[3][1] + entry[2][1] *
    entry[3][0])) ) +

            entry[0][3] * (
                    (entry[1][0] * (entry[2][1] * entry[3][2] + entry[2][2] *
    entry[3][1])) + (entry[1][1] * (entry[2][0] * entry[3][2] + entry[2][2] *
    entry[3][0])) + (entry[1][2] * (entry[2][0] * entry[3][1] + entry[2][1] *
    entry[3][0]))
    )
    */
#endif
    return 0;
}

/**
 * be_aS
 */
static inline void be_aS(gf16m_t target, gf16_t a) {
    for (int i = 0; i < rank; ++i) {
        for (int j = 0; j < rank; ++j) {
            set_gf16m(target, i, j, gf16_get_mul((8 - (i + j)), a));
        }
    }
}

/**
 * be_invertible_by_add_aS
 */
static inline void be_invertible_by_add_aS(gf16m_t source) {
    gf16m_t temp;
    if (gf16m_det(source) == 0) {
        for (uint8_t a = 1; a < 16; ++a) {
            be_aS(temp, a);
            gf16m_add(temp, source, source);
            if (gf16m_det(source) != 0) {
                return;
            }
        }
    }
}

#ifdef __cplusplus
}
#endif

#endif