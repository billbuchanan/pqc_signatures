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

#include <string.h>
#include "prng.h"

void prng_init(prng_t *prng, const hash_t salt, const seed_t seed)
{
    uint8_t input[HASH_SIZE + SEED_SIZE];

    memset(input, 0, HASH_SIZE + SEED_SIZE);

    Keccak_HashInitialize_SHAKE256(prng);

    /* Set 'buffer = salt | seed'. */
    if (salt != NULL)
    {
        memcpy(input, salt, HASH_SIZE);
    }

    if (seed != NULL)
    {
        memcpy(input + HASH_SIZE, seed, SEED_SIZE);
    }
    /* * */

    Keccak_HashUpdate(prng, input, (HASH_SIZE + SEED_SIZE)*8);
    Keccak_HashFinal(prng, NULL);
}

void prng_bytes(prng_t *prng, void *target, size_t length)
{
    Keccak_HashSqueeze(prng, (uint8_t*) target, length*8);
}
