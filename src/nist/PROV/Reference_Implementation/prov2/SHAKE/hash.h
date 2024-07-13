    /*  This file is originally part of the reference implementation of the Picnic signature scheme. It is under MIT license, see  LICENSE for
 *  more details.
 *  SPDX-License-Identifier: MIT
 */

#ifndef HASH_H
#define HASH_H

#include <stdint.h>

#include "sha3/KeccakHash.h"

/* Wrap the Keccak API, checking return values, logging errors, and working
 * with byte lengths instead of bitlengths. */


/* Prefix values for domain separation. */
static const uint8_t HASH_PREFIX_NONE = -1;
static const uint8_t HASH_PREFIX_SECRET = 0;
static const uint8_t HASH_PREFIX_P1_P2 = 1;
static const uint8_t HASH_PREFIX_OIL = 2;
static const uint8_t HASH_PREFIX_V = 3;
static const uint8_t HASH_PREFIX_MESSAGE = 4;
static const uint8_t HASH_PREFIX_PUBLIC = 5;

typedef Keccak_HashInstance HashInstance;

void HashUpdate(HashInstance* ctx, const uint8_t* data, size_t byteLen);

void HashInit(HashInstance* ctx, uint8_t hashPrefix);

void HashFinal(HashInstance* ctx);

void HashSqueeze(HashInstance* ctx, uint8_t* digest, size_t byteLen);

#endif /* HASH_H */
