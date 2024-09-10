/** 
 * @file ryde_256f_parsing.h
 * @brief Functions to parse secret key, public key, commitment, and response of the SIGN-RSD_MPC_ONE scheme
 */

#ifndef RYDE_256F_PARSING_H
#define RYDE_256F_PARSING_H

#include "rbc_43_vec.h"
#include "rbc_43_mat.h"
#include "rbc_43_qpoly.h"

void ryde_256f_public_key_to_string(uint8_t* pk, const uint8_t* pk_seed, const rbc_43_vec y);
void ryde_256f_public_key_from_string(rbc_43_mat H, rbc_43_vec y, const uint8_t* pk);

void ryde_256f_secret_key_to_string(uint8_t* sk, const uint8_t* sk_seed, const uint8_t* pk_seed);
void ryde_256f_secret_key_from_string(rbc_43_vec y, rbc_43_mat H, rbc_43_vec x1, rbc_43_vec x2, rbc_43_qpoly A, const uint8_t* sk);

#endif

