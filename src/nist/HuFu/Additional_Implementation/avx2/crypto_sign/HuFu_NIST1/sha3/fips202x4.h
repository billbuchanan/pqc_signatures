#ifndef FIPS202X4_H
#define FIPS202X4_H

#include <immintrin.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

void shake256x4(uint8_t *out0, uint8_t *out1, uint8_t *out2, uint8_t *out3, unsigned long long outlen, const uint8_t *input, unsigned long long inlen);

#endif
