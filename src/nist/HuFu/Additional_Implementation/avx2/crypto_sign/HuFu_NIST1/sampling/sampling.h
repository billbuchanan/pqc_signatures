#ifndef SAMPLING_H
#define SAMPLING_H

#include <stdint.h>
#include "../sha3/shake.h"
void samplep(int16_t *p, const uint8_t *sk, double *L_22, double *L_32, double *L_33, uint8_t seed[32]);

void sampleF(int16_t *z, int16_t *v, uint8_t seed[32]);

void samplef(int16_t *z, int16_t v, prng *rng);

#endif
