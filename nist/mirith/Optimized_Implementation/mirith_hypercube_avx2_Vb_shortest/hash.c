/*
 * Copyright 2023 Carlo Sanna, Javier Verbel, and Floyd Zweydinger.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "KeccakHash.h"
#include <string.h>
#include <stdlib.h>
#include "hash.h"

void hash_init(hash_ctx_t *ctx)
{
	*ctx = (hash_ctx_t)malloc(sizeof(Keccak_HashInstance));
#if HASH_SIZE == 32
	Keccak_HashInitialize_SHA3_256(*ctx);
#elif HASH_SIZE == 48
	Keccak_HashInitialize_SHA3_384(*ctx);
#elif HASH_SIZE == 64
	Keccak_HashInitialize_SHA3_512(*ctx);
#else
#error "HASH_SIZE not implemented!"
#endif
}

/// length in byte
void hash_update(hash_ctx_t ctx, const uint8_t *data, size_t length)
{
    Keccak_HashUpdate(ctx, data, length*8);
}

void hash_finalize(hash_ctx_t ctx, hash_t hash)
{
    Keccak_HashFinal(ctx, hash);
	free(ctx);
}

int hash_equal(hash_t hash1, hash_t hash2)
{
    return memcmp(hash1, hash2, HASH_SIZE) == 0;
}

void hash_digest0(hash_t hash, const hash_t salt, uint32_t l, uint32_t i, const seed_t seed)
{
    hash_ctx_t ctx;

    hash_init(&ctx);
    hash_update(ctx, salt, HASH_SIZE);
    hash_update(ctx, (uint8_t *)&l, sizeof(l));
    hash_update(ctx, (uint8_t *)&i, sizeof(i));
    hash_update(ctx, seed, SEED_SIZE);
    hash_finalize(ctx, hash);
}

void hash_digest0_aux(hash_t hash, 
    const hash_t salt, 
    uint32_t l,
    uint32_t i,
    const seed_t seed,
    const ff_t a_aux[matrix_bytes_size(PAR_K, 1)],
    const ff_t K_aux[matrix_bytes_size(PAR_R, PAR_N - PAR_R)],
    const ff_t C_aux[matrix_bytes_size(PAR_S, PAR_N - PAR_R)])
{

	hash_ctx_t ctx = (hash_ctx_t)malloc(sizeof(Keccak_HashInstance));
    
#if HASH_SIZE == 32
	Keccak_HashInitialize_SHA3_256(ctx);
#elif HASH_SIZE == 48
	Keccak_HashInitialize_SHA3_384(ctx);
#elif HASH_SIZE == 64
	Keccak_HashInitialize_SHA3_512(ctx);
#else
#error "HASH_SIZE not implemented!"
#endif

    Keccak_HashUpdate(ctx, salt, HASH_SIZE*8);
    Keccak_HashUpdate(ctx, (const BitSequence *)&l, sizeof(l)*8);
    Keccak_HashUpdate(ctx, (const BitSequence *)&i, sizeof(i)*8);
    Keccak_HashUpdate(ctx, seed, SEED_SIZE*8);
    Keccak_HashUpdate(ctx, a_aux, matrix_bytes_size(PAR_K, 1)*8);
    Keccak_HashUpdate(ctx, K_aux, matrix_bytes_size(PAR_R, PAR_N - PAR_R)*8);
    Keccak_HashUpdate(ctx, C_aux, matrix_bytes_size(PAR_S, PAR_N - PAR_R)*8);
	Keccak_HashFinal(ctx, hash);

	free(ctx);
}
