/** 
 * @file ryde_256f_mpc.h
 * @brief Functions concerning the MPC part of the RYDE scheme
 */

#ifndef RYDE_256F_MPC_H
#define RYDE_256F_MPC_H

#include "rbc_43_vec.h"
#include "parameters.h"
#include "parsing.h"


typedef struct {
    rbc_43_vec x1[RYDE_256F_PARAM_N_MPC];
    rbc_43_vec x2[RYDE_256F_PARAM_N_MPC];
    rbc_43_vec a[RYDE_256F_PARAM_N_MPC];
    rbc_43_vec u[RYDE_256F_PARAM_N_MPC];
    rbc_43_elt c[RYDE_256F_PARAM_N_MPC];
    rbc_43_vec u_;
} ryde_256f_leaf_shares_t;

typedef struct {
    rbc_43_vec x1[RYDE_256F_PARAM_D];
    rbc_43_vec x2[RYDE_256F_PARAM_D];
    rbc_43_vec a[RYDE_256F_PARAM_D];
    rbc_43_vec u[RYDE_256F_PARAM_D];
    rbc_43_elt c[RYDE_256F_PARAM_D];
} ryde_256f_main_shares_t;

typedef struct {
    rbc_43_vec gamma;
    rbc_43_elt epsilon;
} ryde_256f_challenge1_t;

typedef struct {
    rbc_43_vec alpha_;
    rbc_43_vec alpha[RYDE_256F_PARAM_D];
    rbc_43_elt v[RYDE_256F_PARAM_D];
} ryde_256f_response1_t;



void ryde_256f_init_leaf_shares(ryde_256f_leaf_shares_t *leaf_shares);
void ryde_256f_init_main_shares(ryde_256f_main_shares_t *main_shares);
void ryde_256f_init_challenge1(ryde_256f_challenge1_t *challenge);
void ryde_256f_init_response1(ryde_256f_response1_t *response);

void ryde_256f_clear_leaf_shares(ryde_256f_leaf_shares_t *leaf_shares);
void ryde_256f_clear_main_shares(ryde_256f_main_shares_t *main_shares);
void ryde_256f_clear_challenge1(ryde_256f_challenge1_t *challenge);
void ryde_256f_clear_response1(ryde_256f_response1_t *response);

// Functions used for signing
void ryde_256f_compute_leaf_shares(ryde_256f_leaf_shares_t *leaf_shares, const rbc_43_vec y, const rbc_43_mat H, const rbc_43_vec x2, const rbc_43_vec a, const uint8_t *seed_input, const uint8_t *salt);
void ryde_256f_commit_to_shares(uint8_t *commits, uint8_t e, const uint8_t *salt, const ryde_256f_leaf_shares_t *leaf_shares, const uint8_t *seed_input);
void ryde_256f_compute_main_shares(ryde_256f_main_shares_t *main_shares, const ryde_256f_leaf_shares_t *leaf_shares);

void ryde_256f_compute_challenge1(ryde_256f_challenge1_t *challenge, const uint8_t *seed_input, const uint8_t *salt);
void ryde_256f_compute_response1(ryde_256f_response1_t *response, const ryde_256f_main_shares_t *main_shares, const ryde_256f_challenge1_t *challenge, const rbc_43_vec x1, const rbc_43_vec x2, const rbc_43_vec u);
void ryde_256f_reconstruct_alpha(ryde_256f_response1_t *response, rbc_43_vec x1, rbc_43_vec x2, rbc_43_vec u, const ryde_256f_challenge1_t *challenge);
void ryde_256f_reconstruct_share(rbc_43_vec x1, rbc_43_vec x2, rbc_43_vec u, const rbc_43_vec y, const rbc_43_mat H, const uint8_t *seed_input, uint8_t second_challenge, const uint8_t *salt);

// Functions used for verifying
void ryde_256f_recompute_commitments(uint8_t *commits, uint8_t *seed_inputs, uint8_t e, const uint8_t *salt, const uint8_t *state, uint8_t hidden);
void ryde_256f_recompute_shares(ryde_256f_leaf_shares_t *leaf_shares, const rbc_43_vec y, const rbc_43_mat H, const uint8_t *seed_input, const uint8_t *state, uint8_t second_challenge, const uint8_t *salt);
void ryde_256f_recompute_main_shares(ryde_256f_main_shares_t *main_shares, const ryde_256f_leaf_shares_t *leaf_shares, uint8_t second_challenge);

void ryde_256f_recompute_additional_main_share(rbc_43_vec x1, rbc_43_vec x2, rbc_43_vec a, rbc_43_vec u, rbc_43_elt c, const ryde_256f_leaf_shares_t *leaf_shares, uint8_t second_challenge);
void ryde_256f_add_alpha_ch2(ryde_256f_response1_t *response, const uint8_t *state, uint8_t second_challenge);
void ryde_256f_recompute_response1(ryde_256f_response1_t *response, const ryde_256f_main_shares_t *main_shares, const ryde_256f_challenge1_t *challenge, const rbc_43_vec x1, const rbc_43_vec x2, const rbc_43_vec a, const rbc_43_vec u, const rbc_43_elt c, uint8_t second_challenge);



#endif
