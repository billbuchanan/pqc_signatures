
/**
 * @file symmetric_times4.h
 * @brief Header file for symmetric_times4.c
 */

#ifndef SIG_PERK_SYMMETRIC_TIMES4_H
#define SIG_PERK_SYMMETRIC_TIMES4_H

#include <stdint.h>
#include "KeccakHashtimes4.h"
#include "parameters.h"
#include "symmetric.h"

typedef Keccak_HashInstancetimes4 sig_perk_prg_times4_state_t;
typedef Keccak_HashInstancetimes4 sig_perk_hash_times4_state_t;

/**
 * @brief Initialize a PRNG times4
 *        absorb the salt if not NULL and the seeds
 *
 * A variant that uses 4 parallel instances
 *
 * @param [out,in] state a pointer to the state of the PRNG
 * @param [in] domain a byte that is the domain separator.
 * @param [in] salt a string containing the salt. If Null no salt is absorbed.
 * @param [in] seed4 an array of 4 string containing the seeds. If Null no seed is absorbed.
 */
void sig_perk_prg_times4_init(sig_perk_prg_times4_state_t *state, const uint8_t domain, const salt_t salt,
                              const uint8_t *seed4[4]);

/**
 * @brief PRNG times4
 *
 * A variant that uses 4 parallel instances
 *
 * @param [out,in] state a pointer to the state of the PRNG
 * @param [out] output4 an array of 4 pointers to the buffer to be filled
 * @param [in] outlen size of the output
 */
void sig_perk_prg_times4(sig_perk_prg_times4_state_t *state, uint8_t *output4[4], size_t outlen);

/**
 * @brief initialize the HASH times4 function
 *        absorb the salt and ctr if != 0
 *
 * @param [out,in] state a pointer to the state of the HASH.
 * @param [in] salt a string containing the salt.
 * @param [in] tau4 an array of 4 uint8_t absorbed after the salt. If NULL tau is not absorbed.
 * @param [in] n4 an array of 4 uint8_t absorbed after the salt. If NULL n is not absorbed.
 */
void sig_perk_hash_times4_init(sig_perk_hash_times4_state_t *state, const salt_t salt, const uint8_t tau4[4],
                               const uint8_t n4[4]);

/**
 * @brief HASH update times4
 *
 * @param [out,in] state a pointer to the state of the HASH.
 * @param [in] message4 an array of 4 pointers to the message to be absorbed.
 * @param [in] message_size size of the messages.
 */
void sig_perk_hash_times4_update(sig_perk_hash_times4_state_t *state, const uint8_t *message4[4],
                                 const size_t message_size);

/**
 * @brief output the 4 digests for the chosen hash function (domain)
 *
 * @param [out,in] state a pointer to the state of the HASH.
 * @param [out] digest4 an array of 4 pointers to the output digests.
 * @param [in] domain domain: H0, H1 or H2.
 */
void sig_perk_hash_times4_final(sig_perk_hash_times4_state_t *state, uint8_t *digest4[4], const uint8_t domain);

#endif