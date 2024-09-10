#ifndef EXPAND_H_
#define EXPAND_H_

#include <string.h>
#include "api.h"
#include "compress.h"
#include "aes/aes256.h"

extern void randomSeed(uint8_t *seed);

extern void expandSeeds(uint8_t *seeds, const uint8_t *seed, const int nSeeds);

extern void expandChallenge(uint64_t *chg_c, uint64_t *chg_nc, uint64_t *chg_val, const uint8_t *seed);

extern void expandATF(uint64_t *atf, const uint8_t *seed);

extern void expandColumns(uint64_t *col, const uint8_t *seed);

#endif
