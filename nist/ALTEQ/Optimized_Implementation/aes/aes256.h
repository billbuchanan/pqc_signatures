/* also modified from the files provided by the NIST */

#ifndef aes256_h
#define aes256_h

#include <stdio.h>
#include <stdint.h>

void
randombytes_init(uint8_t *entropy_input,
                 uint8_t *personalization_string,
                 int security_strength);

int
randombytes(uint8_t *x, size_t xlen);

#endif
