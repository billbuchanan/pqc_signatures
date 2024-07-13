/** 
 * @file ryde_192f_mpc.h
 * @brief Functions concerning the MPC part of the RYDE scheme
 */

#ifndef RYDE_192F_MPC_H
#define RYDE_192F_MPC_H

#include "rbc_37_vec.h"
#include "parameters.h"
#include "parsing.h"


typedef struct {
    rbc_37_vec x1[RYDE_192F_PARAM_N_MPC];
    rbc_37_vec x2[RYDE_192F_PARAM_N_MPC];
    rbc_37_vec a[RYDE_192F_PARAM_N_MPC];
    rbc_37_vec u[RYDE_192F_PARAM_N_MPC];
    rbc_37_elt c[RYDE_192F_PARAM_N_MPC];
    rbc_37_vec u_;
} ryde_192f_leaf_shares_t;

typedef struct {
    rbc_37_vec x1[RYDE_192F_PARAM_D];
    rbc_37_vec x2[RYDE_192F_PARAM_D];
    rbc_37_vec a[RYDE_192F_PARAM_D];
    rbc_37_vec u[RYDE_192F_PARAM_D];
    rbc_37_elt c[RYDE_192F_PARAM_D];
} ryde_192f_main_shares_t;

typedef struct {
    rbc_37_vec gamma;
    rbc_37_elt epsilon;
} ryde_192f_challenge1_t;

typedef struct {
    rbc_37_vec alpha_;
    rbc_37_vec alpha[RYDE_192F_PARAM_D];
    rbc_37_elt v[RYDE_192F_PARAM_D];
} ryde_192f_response1_t;



void ryde_192f_init_leaf_shares(ryde_192f_leaf_shares_t *leaf_shares);
void ryde_192f_init_main_shares(ryde_192f_main_shares_t *main_shares);
void ryde_192f_init_challenge1(ryde_192f_challenge1_t *challenge);
void ryde_192f_init_response1(ryde_192f_response1_t *response);

void ryde_192f_clear_leaf_shares(ryde_192f_leaf_shares_t *leaf_shares);
void ryde_192f_clear_main_shares(ryde_192f_main_shares_t *main_shares);
void ryde_192f_clear_challenge1(ryde_192f_challenge1_t *challenge);
void ryde_192f_clear_response1(ryde_192f_response1_t *response);

// Functions used for signing
void ryde_192f_compute_leaf_shares(ryde_192f_leaf_shares_t *leaf_shares, const rbc_37_vec y, const rbc_37_mat H, const rbc_37_vec x2, const rbc_37_vec a, const uint8_t *seed_input, const uint8_t *salt);
void ryde_192f_commit_to_shares(uint8_t *commits, uint8_t e, const uint8_t *salt, const ryde_192f_leaf_shares_t *leaf_shares, const uint8_t *seed_input);
void ryde_192f_compute_main_shares(ryde_192f_main_shares_t *main_shares, const ryde_192f_leaf_shares_t *leaf_shares);

void ryde_192f_compute_challenge1(ryde_192f_challenge1_t *challenge, const uint8_t *seed_input, const uint8_t *salt);
void ryde_192f_compute_response1(ryde_192f_response1_t *response, const ryde_192f_main_shares_t *main_shares, const ryde_192f_challenge1_t *challenge, const rbc_37_vec x1, const rbc_37_vec x2, const rbc_37_vec u);
void ryde_192f_reconstruct_alpha(ryde_192f_response1_t *response, rbc_37_vec x1, rbc_37_vec x2, rbc_37_vec u, const ryde_192f_challenge1_t *challenge);
void ryde_192f_reconstruct_share(rbc_37_vec x1, rbc_37_vec x2, rbc_37_vec u, const rbc_37_vec y, const rbc_37_mat H, const uint8_t *seed_input, uint8_t second_challenge, const uint8_t *salt);

// Functions used for verifying
void ryde_192f_recompute_commitments(uint8_t *commits, uint8_t *seed_inputs, uint8_t e, const uint8_t *salt, const uint8_t *state, uint8_t hidden);
void ryde_192f_recompute_shares(ryde_192f_leaf_shares_t *leaf_shares, const rbc_37_vec y, const rbc_37_mat H, const uint8_t *seed_input, const uint8_t *state, uint8_t second_challenge, const uint8_t *salt);
void ryde_192f_recompute_main_shares(ryde_192f_main_shares_t *main_shares, const ryde_192f_leaf_shares_t *leaf_shares, uint8_t second_challenge);

void ryde_192f_recompute_additional_main_share(rbc_37_vec x1, rbc_37_vec x2, rbc_37_vec a, rbc_37_vec u, rbc_37_elt c, const ryde_192f_leaf_shares_t *leaf_shares, uint8_t second_challenge);
void ryde_192f_add_alpha_ch2(ryde_192f_response1_t *response, const uint8_t *state, uint8_t second_challenge);
void ryde_192f_recompute_response1(ryde_192f_response1_t *response, const ryde_192f_main_shares_t *main_shares, const ryde_192f_challenge1_t *challenge, const rbc_37_vec x1, const rbc_37_vec x2, const rbc_37_vec a, const rbc_37_vec u, const rbc_37_elt c, uint8_t second_challenge);



#endif
