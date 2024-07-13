/** 
 * @file sign_mira_code_128s_parsing.h
 * @brief Functions to parse secret key, public key, commitment, and response of the MIRA scheme
 */

#ifndef SIGN_MIRA_CODE_128S_PARSING_H
#define SIGN_MIRA_CODE_128S_PARSING_H

#include "finite_fields.h"

void sign_mira_128_public_key_to_string(uint8_t* pk, const uint8_t* pk_seed, const gf16_mat M0);
void sign_mira_128_public_key_from_string(gf16_mat M0, gf16_mat *Mi, const uint8_t* pk);

void sign_mira_128_secret_key_to_string(uint8_t* sk, const uint8_t* sk_seed, const uint8_t* pk_seed);
void sign_mira_128_secret_key_from_string(gf16_mat M0, gf16_mat *Mi, gf16_mat x, gf16_mat E, gfqm *A, const uint8_t* sk);

#endif

