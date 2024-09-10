/** 
 * @file sign_mira_128_mpc.h
 * @brief Functions concerning the MPC part of the SIGN_MARS_CODE scheme
 */

#ifndef SIGN_MIRA_128_MPC_H
#define SIGN_MIRA_128_MPC_H

#include "parameters.h"
#include "parsing.h"


typedef struct {
    gf16_mat x[SIGN_MIRA_128_PARAM_N_MPC];
    gfqm_vec beta[SIGN_MIRA_128_PARAM_N_MPC];
    gfqm_vec a[SIGN_MIRA_128_PARAM_N_MPC];
    gfqm c[SIGN_MIRA_128_PARAM_N_MPC];
    gfqm_vec a_;
} sign_mira_128_shares_t;

typedef struct {
    gf16_mat x[SIGN_MIRA_128_PARAM_N_MPC];
    gfqm_vec beta[SIGN_MIRA_128_PARAM_N_MPC];
    gfqm_vec a[SIGN_MIRA_128_PARAM_N_MPC];
    gfqm c[SIGN_MIRA_128_PARAM_N_MPC];
} sign_mira_128_main_shares_t;

typedef struct {
    gfqm_vec gamma;
    gfqm epsilon;
} sign_mira_128_challenge1_t;

typedef struct {
    gfqm_vec alpha_;
    gfqm_vec alpha[SIGN_MIRA_128_PARAM_D];
    gfqm v[SIGN_MIRA_128_PARAM_D];
} sign_mira_128_response1_t;


void sign_mira_128_init_shares(sign_mira_128_shares_t *shares);
void sign_mira_128_init_main_shares(sign_mira_128_main_shares_t *parties);
void sign_mira_128_init_challenge1(sign_mira_128_challenge1_t *challenge);
void sign_mira_128_init_response1(sign_mira_128_response1_t *response);

void sign_mira_128_clear_shares(sign_mira_128_shares_t *shares);
void sign_mira_128_clear_main_shares(sign_mira_128_main_shares_t *parties);
void sign_mira_128_clear_challenge1(sign_mira_128_challenge1_t *challenge);
void sign_mira_128_clear_response1(sign_mira_128_response1_t *response);


// Functions used for signing
void sign_mira_128_compute_shares(sign_mira_128_shares_t *shares, const gf16_mat x, const gfqm_vec A, const uint8_t *theta_i, const uint8_t *salt);
void sign_mira_128_commit_to_shares(uint8_t *commits, uint8_t e, const uint8_t *salt, const sign_mira_128_shares_t *shares, const uint8_t *seed_input);
void sign_mira_128_compute_main_shares(sign_mira_128_main_shares_t *parties, const sign_mira_128_shares_t *shares);

void sign_mira_128_compute_challenge1(sign_mira_128_challenge1_t *challenge, const uint8_t *seed_input, const uint8_t *salt);
void sign_mira_128_compute_response1(sign_mira_128_response1_t *response, const sign_mira_128_main_shares_t *shares, const sign_mira_128_challenge1_t *challenge, const gf16_mat M0, const gf16_mat *Mi, const gf16_mat x, const gfqm_vec a);
void sign_mira_128_reconstruct_alpha(sign_mira_128_response1_t *response, const gf16_mat M0, const gf16_mat *Mi, const gf16_mat x, const gfqm_vec a, const sign_mira_128_challenge1_t *challenge, int include_M0);
void sign_mira_128_reconstruct_share(gf16_mat x, gfqm_vec a, const uint8_t *theta_i, uint8_t challenge2, const uint8_t *salt);

// Functions used for verifying
void sign_mira_128_recompute_commitments(uint8_t *commits, uint8_t *seed_inputs, uint8_t e, const uint8_t *salt, const uint8_t *state, uint8_t hidden);
void sign_mira_128_recompute_shares(sign_mira_128_shares_t *shares, const uint8_t *seed_input, const uint8_t *state, uint8_t second_challenge, const uint8_t *salt);
void sign_mira_128_recompute_main_shares(sign_mira_128_main_shares_t *parties, const sign_mira_128_shares_t *shares, uint8_t second_challenge);

void sign_mira_128_recompute_additional_main_share(gf16_mat x, gfqm_vec beta, gfqm_vec a, gfqm c, const sign_mira_128_shares_t *shares, uint8_t second_challenge);
void sign_mira_128_add_alpha_ch2(sign_mira_128_response1_t *response, const uint8_t *state, uint8_t second_challenge);
void sign_mira_128_recompute_response1(sign_mira_128_response1_t *response, const sign_mira_128_main_shares_t *shares, const sign_mira_128_challenge1_t *challenge, const gf16_mat x, const gfqm_vec beta, const gfqm_vec a, const gfqm c, const gf16_mat M0, const gf16_mat* Mi, uint8_t second_challenge);


#endif
