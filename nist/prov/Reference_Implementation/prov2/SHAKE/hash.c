/*  This file is originally part of the reference implementation of the Picnic signature scheme. It is under MIT license, see  LICENSE for
 *  more details.
 *  SPDX-License-Identifier: MIT
 */

#include "hash.h"
#include <stdio.h>
#include <assert.h>

void HashUpdate(HashInstance* ctx, const uint8_t* data, size_t byteLen)
{
    HashReturn ret = Keccak_HashUpdate(ctx, data, byteLen * 8);

    if (ret != SUCCESS) {
        fprintf(stderr, "%s: Keccak_HashUpdate failed (returned %d)\n", __func__, ret);
        assert(!"Keccak_HashUpdate failed");
    }
}

void HashInit(HashInstance* ctx, uint8_t hashPrefix)
{
    Keccak_HashInitialize_SHAKE256(ctx);

    if (hashPrefix != HASH_PREFIX_NONE) {
        HashUpdate(ctx, &hashPrefix, 1);
    }
}

void HashFinal(HashInstance* ctx)
{
    HashReturn ret = Keccak_HashFinal(ctx, NULL);

    if (ret != SUCCESS) {
        fprintf(stderr, "%s: Keccak_HashFinal failed (returned %d)\n", __func__, ret);
    }
}


void HashSqueeze(HashInstance* ctx, uint8_t* digest, size_t byteLen)
{
    HashReturn ret = Keccak_HashSqueeze(ctx, digest, byteLen * 8);

    if (ret != SUCCESS) {
        fprintf(stderr, "%s: Keccak_HashSqueeze failed (returned %d)\n", __func__, ret);
    }
}

