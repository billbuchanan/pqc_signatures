/* DannyNiu/NJF, 2018-02-07. Public Domain. */

#ifndef MySuiteA_sponge_h
#define MySuiteA_sponge_h 1

#include "mysuitea-common.h"

// The structure size is a multiply of 8
// under S16ILP32 and S16I32LP64 environments. 
typedef struct sponge {
    unsigned        rate;
    short           lopad, hipad;
    int             finalized;
    unsigned        filled;

    // `offset' is the offset of the state buffer
    // from the beginning of the `sponge_t' structure.
    // Usually, the state buffer is directly appended
    // to the `sponge_t' struct, plus the struct being
    // correctly aligned, the value is usually the
    // size of the `sponge_t' struct.
    //
    // The state buffer is initialized by
    // functions or macros associated with
    // the structure that embeds `sponge_t'.
    //
    ptrdiff_t       offset;
    PermuteFunc_t   permute;
} sponge_t;

#define SPONGE_INIT(r,lo,hi,p)                  \
    ((sponge_t){                                \
        .rate = r, .lopad = lo, .hipad = hi,    \
        .finalized = 0, .filled = 0,            \
        .offset = sizeof(sponge_t),             \
        .permute = PERMUTE_FUNC(p),             \
    })

void Sponge_Update(sponge_t *restrict s, void const *restrict data, size_t len);
void Sponge_Final(sponge_t *restrict s);
void Sponge_Read(sponge_t *restrict s, void *restrict data, size_t len);

#endif /* MySuiteA_sponge_h */
