/*
Copyright (c) 2023 Team HAETAE
Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

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
