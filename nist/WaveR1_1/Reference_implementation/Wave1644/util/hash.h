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


#ifndef WAVE2_HASH_H
#define WAVE2_HASH_H
#include <stdlib.h>
#include <string.h>

#include "fq_arithmetic/vf3.h"
#include "params.h"
#include "prng/fips202.h"
#include "types.h"

/**
 * Function to compute the hash of a message and transform into trits.
 * @param m_hash high and low part of the representation of the hash
 * (representation in trits)
 * @param message message that we will take the hash
 * @param mlen size of the message
 * @param salt salt that it will be included to hash
 * @param msalt size of the salt
 */
void hash_message(vf3_e *m_hash, const uint8_t *message, const size_t mlen,
                  const uint8_t *salt, const size_t msalt);

#endif /* HASH_H_ */
