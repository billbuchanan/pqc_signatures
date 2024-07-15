#include "rand.h"

#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>

#include "params.h"

#ifdef USE_PRNG
#include "rng.h"
#endif

int random_bytes(void *num, size_t len)
{
    if (len == 0)
    {
        return 0;
    }
#ifndef USE_PRNG
    int f = open("/dev/urandom", O_RDONLY);
    if (f >= 0)
    {
        while (len > 0)
        {
            ssize_t rlen = read(f, num, len);
            if (rlen < 0)
            {
                if (errno == EINTR)
                    continue;
                break;
            }
            num = (uint8_t *)num + rlen;
            len -= (size_t)rlen;
        }
        close(f);
        if (len == 0)
        {
            return 0;
        }
    }

    return 1;
#else
    return !(randombytes(num, len) == RNG_SUCCESS);
#endif
}

void set_prng_seed(unsigned char seed[48])
{
#ifdef USE_PRNG
    randombytes_init(seed, NULL, 256);
#else
    assert(0 && "set_prng_seed is not available");
#endif
}

int random_polynomial(uint64_t *random_poly, size_t random_poly_len)
{
    // Arranging memory should be the duty of caller?
    random_poly = (uint64_t *)malloc(random_poly_len * sizeof(uint64_t));
    return random_bytes(random_poly, random_poly_len * 8);
}

int random_verifier_message(uint64_t *random_message, size_t random_message_len)
{
    random_message = (uint64_t *)malloc(random_message_len * sizeof(uint64_t));
    return random_bytes(random_message, random_message_len * 8);
}

// int get_random_numbers(uint64_t *random_numbers, size_t random_numbers_len)
// {
// }

// int sample_leaf_randomness(uint8_t *randomness, size_t randomness_size)
// {
//     randomness = (uint8_t *) malloc(randomness_size * BTQ_AURORA_192_HASH_ZK_BYTESIZE);
//     return random_bytes(randomness, randomness_size * BTQ_AURORA_192_HASH_ZK_BYTESIZE);
// }
