#ifndef WAVE_DECODE_H
#define WAVE_DECODE_H

#include <stdint.h>

#include "types.h"

int decode_v(vf3_e *e_v, vf3_e *y_v, wave_sk_t *sk);

int decode_u(vf3_e *e_u, vf3_e *y_u, vf3_e *e_v, wave_sk_t *sk);

int decode(vf3_e *e, vf3_e *s, wave_sk_t *sk);

void mf3_genmatperm(mf3_e *H, uint16_t *pi, uint8_t *seed, int seed_len,
                    uint32_t domain);

#endif  // WAVE_DECODE_H
