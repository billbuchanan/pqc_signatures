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

#ifndef WAVE2_PRNG_H
#define WAVE2_PRNG_H

#include <stdint.h>

#include "fq_arithmetic/types_f3.h"

void seed_prng(uint8_t *seed, int seed_len);
void close_prng();
void rng_bytes(uint8_t *output, int outlen);
/**
 *
 * Function to generate trits of the size of y->alloc
 * @param y The output vector (trits)
 * @param seed The seed since we will call a Pseudo Random generator (SHAKE)
 * @param seed_len Length of the SEDD
 * @param custom custom value to avoid attacks of "Domain" Separation
 */
void vf3_random_from_seed(vf3_e *y, uint8_t const *seed, int seed_len,
                          uint32_t custom);

#endif  // WAVE2_PRNG_H
