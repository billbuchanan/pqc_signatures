// From HAETAE

#include <stdint.h>
#include <stdlib.h>

#include "fixpoint.h"

void fixpoint_square(fp96_76 *sqx, const fp96_76 *x) {
    uint64_t tmp[2];
    sq48(&sqx->limb48[0], x->limb48[0]);

    // shift right by 48, rounding
    sqx->limb48[0] += 1ULL << 47;
    sqx->limb48[0] >>= 48;
    sqx->limb48[0] += sqx->limb48[1];

    // mul
    mul48(tmp, x->limb48[0], x->limb48[1]);
    sqx->limb48[0] += tmp[0] << 1;
    sqx->limb48[1] = tmp[1] << 1;

    // shift right by 28, rounding
    sqx->limb48[0] += 1ULL << 27;
    sqx->limb48[0] >>= 28;
    sqx->limb48[0] += (sqx->limb48[1] << 20) & ((1ULL << 48) - 1);
    sqx->limb48[1] >>= 28;

    sq64(tmp, x->limb48[1]);
    sqx->limb48[0] += (tmp[0] << 20) & ((1ULL << 48) - 1);
    sqx->limb48[1] += (tmp[0] >> 28) + (tmp[1] << 36);

    renormalize(sqx);
}


void fixpoint_add(fp96_76 *xy, const fp96_76 *x, const fp96_76 *y) {
    xy->limb48[0] = x->limb48[0] + y->limb48[0];
    xy->limb48[1] = x->limb48[1] + y->limb48[1];
}
