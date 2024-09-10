/** 
 * @file ryde_192f_parsing.h
 * @brief Functions to parse secret key, public key, commitment, and response of the SIGN-RSD_MPC_ONE scheme
 */

#ifndef RYDE_192F_PARSING_H
#define RYDE_192F_PARSING_H

#include "rbc_37_vec.h"
#include "rbc_37_mat.h"
#include "rbc_37_qpoly.h"

void ryde_192f_public_key_to_string(uint8_t* pk, const uint8_t* pk_seed, const rbc_37_vec y);
void ryde_192f_public_key_from_string(rbc_37_mat H, rbc_37_vec y, const uint8_t* pk);

void ryde_192f_secret_key_to_string(uint8_t* sk, const uint8_t* sk_seed, const uint8_t* pk_seed);
void ryde_192f_secret_key_from_string(rbc_37_vec y, rbc_37_mat H, rbc_37_vec x1, rbc_37_vec x2, rbc_37_qpoly A, const uint8_t* sk);

#endif

