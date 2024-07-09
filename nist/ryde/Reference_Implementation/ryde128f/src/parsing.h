/** 
 * @file ryde_128f_parsing.h
 * @brief Functions to parse secret key, public key, commitment, and response of the SIGN-RSD_MPC_ONE scheme
 */

#ifndef RYDE_128F_PARSING_H
#define RYDE_128F_PARSING_H

#include "rbc_31_vec.h"
#include "rbc_31_mat.h"
#include "rbc_31_qpoly.h"

void ryde_128f_public_key_to_string(uint8_t* pk, const uint8_t* pk_seed, const rbc_31_vec y);
void ryde_128f_public_key_from_string(rbc_31_mat H, rbc_31_vec y, const uint8_t* pk);

void ryde_128f_secret_key_to_string(uint8_t* sk, const uint8_t* sk_seed, const uint8_t* pk_seed);
void ryde_128f_secret_key_from_string(rbc_31_vec y, rbc_31_mat H, rbc_31_vec x1, rbc_31_vec x2, rbc_31_qpoly A, const uint8_t* sk);

#endif

