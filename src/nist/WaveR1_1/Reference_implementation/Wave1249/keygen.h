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

#ifndef WAVE2_KEYGEN_H
#define WAVE2_KEYGEN_H

#include "types.h"

/**
 * Allocation of the elements of secret key
 * @param sk pointer to be allocated
 * @return
 */
int wave_sk_alloc(wave_sk_t *sk);
/**
 * Allocation of the elements of public key
 * @param pk pointer to be allocated
 * @return
 */
int wave_pk_alloc(wave_pk_t *pk);
/**
 * Function to free the secret key.
 * @param sk valid pointer
 */
void wave_sk_free(wave_sk_t *sk);
/**
 * Function to free the public key
 * @param pk  Valid pointer
 */
void wave_pk_free(wave_pk_t *pk);

/**
 * Function to generate secret and public keys
 * @param sk Secret key (it should be previously allocated)
 * @param pk Public key (it should be previously allocated)
 * @return 1 if the process is correct, otherwise 0
 */
int keygen(wave_sk_t *sk, wave_pk_t *pk);

#endif  // WAVE2_KEYGEN_H
