#ifndef SAMPLING_H
#define SAMPLING_H

#include <stdint.h>
#include "../sha3/shake.h"

void samplep(int32_t * p, const uint8_t * R, double * L_22, double * L_32, double * L_33,uint8_t seed[32]);

void sampleF(int32_t * z, int32_t * v,uint8_t seed[32]);

void samplef(int32_t * z, int32_t v, prng *rng);

#endif
