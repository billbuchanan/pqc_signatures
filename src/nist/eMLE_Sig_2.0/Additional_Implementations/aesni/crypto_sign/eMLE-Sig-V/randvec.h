#ifndef _RANDVEC_H
#define _RANDVEC_H

#include <stdint.h>
#include <stddef.h>
#include "aes256ctr.h"

void randVec_xmax(int64_t *a, const size_t n, aes256ctr_ctx *ctx);
void randVec_y(int64_t *a, const size_t n, const int64_t sumXn, const int64_t sumXp, aes256ctr_ctx *ctx);
void randVec_loc(size_t *a, const size_t n, aes256ctr_ctx *ctx);
void randVec_dist(int64_t *a, const size_t loc, size_t num, aes256ctr_ctx *ctx);
void randVec_noise(int64_t *a, const size_t n, const int flag, aes256ctr_ctx *ctx);

#endif

