#ifndef SDITH_COMMIT_H
#define SDITH_COMMIT_H

#include "parameters.h"
#include "mpc.h"
#include <stdint.h>

// Commitment x1
void commit_share(uint8_t* digest, const mpc_share_t* share, const uint8_t* salt, uint16_t e, uint16_t i);
void commit_seed(uint8_t* digest, const uint8_t* seed, const uint8_t* salt, uint16_t e, uint16_t i);
void commit_seed_and_aux(uint8_t* digest, const uint8_t* seed, const mpc_wit_t* wit, const mpc_corr_t* corr, const uint8_t* salt, uint16_t e, uint16_t i);

// Commitment x4
void commit_share_x4(uint8_t** digest, mpc_share_t const* const* share, const uint8_t* salt, uint16_t e, const uint16_t* i);
void commit_seed_x4(uint8_t** digest, uint8_t const* const* seed, const uint8_t* salt, uint16_t e, const uint16_t* i);
void commit_seed_and_aux_x4(uint8_t** digest, uint8_t const* const* seed, mpc_wit_t const* const* wit, mpc_corr_t const* const* corr, const uint8_t* salt, uint16_t e, const uint16_t* i);

#endif /* SDITH_COMMIT_H */
