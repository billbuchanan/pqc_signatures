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

#ifndef WAVE_VERIFY_H
#define WAVE_VERIFY_H

#include <stdint.h>

#include "types.h"

/**
 * Function to verify a signature
 * @param sig Signature (Truncated signature N-K)
 * @param sig_len  Size of the signature
 * @param salt Salt used to sign
 * @param message message that was signed
 * @param mlen size of the message
 * @param pk Public key
 * @return 1 if the verification is correct. Other values otherwise.
 */
int verify(vf3_e *sig, size_t *sig_len, uint8_t *salt, uint8_t const *message,
           size_t mlen, wave_pk_t *pk);

#endif  // WAVE_VERIFY_H
