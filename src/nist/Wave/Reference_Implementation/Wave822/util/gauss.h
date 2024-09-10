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


#ifndef WAVE2_GAUSS_H
#define WAVE2_GAUSS_H

#include "fq_arithmetic/types_f3.h"

/**
 * Extended gauss elimination. Function that perform gauss elimination.
 * extended Gaussian elimination, Algorithm 21, page 19
 * @param H Matrix to be reduced
 * @param gap The number of rows to be stacked into H
 * @return number of pivot success
 */
int extended_gauss_elimination(mf3_e *H, int gap);
/**
 * Gauss elimination
 * /NS 27/04/23 Gaussian elimination for KeyGen, Algorithm 18, page 18
 * The pivot positions are not secret and can be revealed
 * @param H Matrix to be reduced
 * @param pivot Elements for the pivot
 * @return number of pivot success
 */
int gauss_elimination(mf3_e *H, int *pivot);

void normalize(vf3_e *e, int pos_l);
void reduce(vf3_e *x, vf3_e *y, int pos_l);

int partial_gauss_elimination(mf3_e *G, int gap);
int gauss_elimination_with_abort(mf3_e *H);

#endif  // WAVE2_GAUSS_H
