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

#include <stdint.h>
#include <stdlib.h>
#include "config.h"
#include "random.h"

#ifdef MIRITH_SUPERCOP

/* Nothing to do: 'randombytes' is defined by SUPERCOP. */

#else

#ifdef MIRITH_DETERMINISTIC

#include "prng.h"

static int prng_singleton_up = 0;
static prng_t prng_singleton;

#else

#ifdef __unix
#include <sys/random.h>
#endif

#ifdef _WIN32
#error "random_bytes on Windows not implemented yet!"
#endif

#endif /* #ifdef MIRITH_DETERMINISTIC */

#endif /* #ifdef MIRITH_SUPERCOP */


#ifndef MIRITH_SUPERCOP

void randombytes(uint8_t *target, size_t length)
{

#ifdef MIRITH_DETERMINISTIC

    if (!prng_singleton_up)
    {
        hash_t salt = {0};

        prng_init(&prng_singleton, salt, NULL);
        
        prng_singleton_up = 1;
    }
    
    prng_bytes(&prng_singleton, target, length);
    
#else

#ifdef  __unix
    getrandom(target, length, 0);
#endif

#ifdef _WIN32
#error "random_bytes on Windows not implemented yet!"
#endif

#endif /* #ifdef MIRITH_DETERMINISTIC */

}

#endif /* #ifndef MIRITH_SUPERCOP */
