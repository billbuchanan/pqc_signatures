#ifndef _RAND_H__
#define _RAND_H__

#include <stddef.h>
#include <stdint.h>

/*
    Get random elements in field
    Using /dev/urandom for now
    TODO: consider directly use the same inner function, only use result variable name as distinguisher
*/
// int random_polynomial(uint64_t *random_poly, size_t random_poly_len);
// int random_verifier_message(uint64_t *random_message, size_t random_message_len); // Maybe no usage of this
int random_bytes(void *num, size_t len);
int sample_leaf_randomness(uint8_t *randomness, size_t randomness_size);
void set_prng_seed(unsigned char seed[48]); // this function will always assert in production

#endif