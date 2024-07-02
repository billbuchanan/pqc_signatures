/******************************************************************************
WAVE -- Code-Based Digital Signature Scheme
Copyright (c) 2023 The Wave Team
contact: wave-contact@inria.fr

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
******************************************************************************/


#ifndef WAVE_RANDPERM_H
#define WAVE_RANDPERM_H

#include <stdint.h>

#include "fq_arithmetic/types_f3.h"

/**
 * // Algorithm 14, page 15 of specs
        randperm() draws a permutation by sorting:
        1 - draw random data r_i, 0 <= i < n
        2 - sort (r_i,i) according to r_i -> (x_i,pi(i)) with x_i ordered
        3 - the mapping (i -> pi(i)) is a permutation

        This permutation is uniformly random if all the r_i drawn at step 1
        are distinct. If not, to enforce uniformity, reject and start again.
 * @param sigma permutation
 * @param n  size of random data
 * @param y permutation map
 * @param z  permuation map
 */
void randperm(uint16_t *sigma, int n, vf3_e *y, vf3_e *z);

void randpermV(uint16_t *sigma, int n, int k, int t, vf3_e *y);

void randpermU(uint16_t *sigma, int n, int k, int l, vf3_e *y, vf3_e *v,
               vf2_e *s);

void vectperminv(vf3_e *res, vf3_e *y, uint16_t *pi);

#endif  // WAVE_RANDPERM_H
