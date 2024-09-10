/** 
 * @file ryde_128s_mpc.h
 * @brief Functions concerning the MPC part of the RYDE scheme
 */

#ifndef RYDE_128S_MPC_H
#define RYDE_128S_MPC_H

#include "rbc_31_vec.h"
#include "parameters.h"
#include "parsing.h"


typedef struct {
    rbc_31_vec x1[RYDE_128S_PARAM_N_MPC];
    rbc_31_vec x2[RYDE_128S_PARAM_N_MPC];
    rbc_31_vec a[RYDE_128S_PARAM_N_MPC];
    rbc_31_vec u[RYDE_128S_PARAM_N_MPC];
    rbc_31_elt c[RYDE_128S_PARAM_N_MPC];
    rbc_31_vec u_;
} ryde_128s_leaf_shares_t;

typedef struct {
    rbc_31_vec x1[RYDE_128S_PARAM_D];
    rbc_31_vec x2[RYDE_128S_PARAM_D];
    rbc_31_vec a[RYDE_128S_PARAM_D];
    rbc_31_vec u[RYDE_128S_PARAM_D];
    rbc_31_elt c[RYDE_128S_PARAM_D];
} ryde_128s_main_shares_t;

typedef struct {
    rbc_31_vec gamma;
    rbc_31_elt epsilon;
} ryde_128s_challenge1_t;

typedef struct {
    rbc_31_vec alpha_;
    rbc_31_vec alpha[RYDE_128S_PARAM_D];
    rbc_31_elt v[RYDE_128S_PARAM_D];
} ryde_128s_response1_t;



void ryde_128s_init_leaf_shares(ryde_128s_leaf_shares_t *leaf_shares);
void ryde_128s_init_main_shares(ryde_128s_main_shares_t *main_shares);
void ryde_128s_init_challenge1(ryde_128s_challenge1_t *challenge);
void ryde_128s_init_response1(ryde_128s_response1_t *response);

void ryde_128s_clear_leaf_shares(ryde_128s_leaf_shares_t *leaf_shares);
void ryde_128s_clear_main_shares(ryde_128s_main_shares_t *main_shares);
void ryde_128s_clear_challenge1(ryde_128s_challenge1_t *challenge);
void ryde_128s_clear_response1(ryde_128s_response1_t *response);

// Functions used for signing
void ryde_128s_compute_leaf_shares(ryde_128s_leaf_shares_t *leaf_shares, const rbc_31_vec y, const rbc_31_mat H, const rbc_31_vec x2, const rbc_31_vec a, const uint8_t *seed_input, const uint8_t *salt);
void ryde_128s_commit_to_shares(uint8_t *commits, uint8_t e, const uint8_t *salt, const ryde_128s_leaf_shares_t *leaf_shares, const uint8_t *seed_input);
void ryde_128s_compute_main_shares(ryde_128s_main_shares_t *main_shares, const ryde_128s_leaf_shares_t *leaf_shares);

void ryde_128s_compute_challenge1(ryde_128s_challenge1_t *challenge, const uint8_t *seed_input, const uint8_t *salt);
void ryde_128s_compute_response1(ryde_128s_response1_t *response, const ryde_128s_main_shares_t *main_shares, const ryde_128s_challenge1_t *challenge, const rbc_31_vec x1, const rbc_31_vec x2, const rbc_31_vec u);
void ryde_128s_reconstruct_alpha(ryde_128s_response1_t *response, rbc_31_vec x1, rbc_31_vec x2, rbc_31_vec u, const ryde_128s_challenge1_t *challenge);
void ryde_128s_reconstruct_share(rbc_31_vec x1, rbc_31_vec x2, rbc_31_vec u, const rbc_31_vec y, const rbc_31_mat H, const uint8_t *seed_input, uint8_t second_challenge, const uint8_t *salt);

// Functions used for verifying
void ryde_128s_recompute_commitments(uint8_t *commits, uint8_t *seed_inputs, uint8_t e, const uint8_t *salt, const uint8_t *state, uint8_t hidden);
void ryde_128s_recompute_shares(ryde_128s_leaf_shares_t *leaf_shares, const rbc_31_vec y, const rbc_31_mat H, const uint8_t *seed_input, const uint8_t *state, uint8_t second_challenge, const uint8_t *salt);
void ryde_128s_recompute_main_shares(ryde_128s_main_shares_t *main_shares, const ryde_128s_leaf_shares_t *leaf_shares, uint8_t second_challenge);

void ryde_128s_recompute_additional_main_share(rbc_31_vec x1, rbc_31_vec x2, rbc_31_vec a, rbc_31_vec u, rbc_31_elt c, const ryde_128s_leaf_shares_t *leaf_shares, uint8_t second_challenge);
void ryde_128s_add_alpha_ch2(ryde_128s_response1_t *response, const uint8_t *state, uint8_t second_challenge);
void ryde_128s_recompute_response1(ryde_128s_response1_t *response, const ryde_128s_main_shares_t *main_shares, const ryde_128s_challenge1_t *challenge, const rbc_31_vec x1, const rbc_31_vec x2, const rbc_31_vec a, const rbc_31_vec u, const rbc_31_elt c, uint8_t second_challenge);



#endif
