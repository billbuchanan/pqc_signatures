
/**
 * @file common.c
 * @brief Common functions
 */

#include "common.h"
#include <string.h>
#include "crypto_memset.h"
#include "symmetric_times4.h"

void sig_perk_gen_pi_i_and_v_i(perm_t *pi_i, vect1_t *v_i, const salt_t salt,
                               const perk_theta_seeds_tree_t theta_tree) {
    uint16_t rnd_buffer_pi_i[4][PARAM_N1];
    uint8_t *rnd_buffer_times4_pi_i[] = {(uint8_t *)rnd_buffer_pi_i[0], (uint8_t *)rnd_buffer_pi_i[1],
                                         (uint8_t *)rnd_buffer_pi_i[2], (uint8_t *)rnd_buffer_pi_i[3]};

    uint16_t rnd_buff_v_i[4][PRNG_BLOCK_SIZE / 2];
    uint8_t *rnd_buff_times4_v_i[] = {(uint8_t *)rnd_buff_v_i[0], (uint8_t *)rnd_buff_v_i[1],
                                      (uint8_t *)rnd_buff_v_i[2], (uint8_t *)rnd_buff_v_i[3]};

    sig_perk_prg_times4_state_t state;
    // pi_i generation
    for (int i = 0; i < PARAM_N / 4; ++i) {
        const uint8_t *theta_i_times4[] = {
            theta_tree[THETA_SEEDS_OFFSET + i * 4 + 0], theta_tree[THETA_SEEDS_OFFSET + i * 4 + 1],
            theta_tree[THETA_SEEDS_OFFSET + i * 4 + 2], theta_tree[THETA_SEEDS_OFFSET + i * 4 + 3]};
        sig_perk_prg_times4_init(&state, PRG1, salt, theta_i_times4);
        // generate permutations
        int status_perm_0 = EXIT_FAILURE;
        int status_perm_1 = EXIT_FAILURE;
        int status_perm_2 = EXIT_FAILURE;
        int status_perm_3 = EXIT_FAILURE;
        if (i == 0) {  // skip first permutation
            status_perm_0 = EXIT_SUCCESS;
        }
        while ((status_perm_0 != EXIT_SUCCESS) || (status_perm_1 != EXIT_SUCCESS) || (status_perm_2 != EXIT_SUCCESS) ||
               (status_perm_3 != EXIT_SUCCESS)) {
            sig_perk_prg_times4(&state, rnd_buffer_times4_pi_i, sizeof(rnd_buffer_pi_i[0]));
            if (status_perm_0 != EXIT_SUCCESS) {
                status_perm_0 = sig_perk_perm_gen_given_random_input(pi_i[i * 4 + 0], rnd_buffer_pi_i[0]);
            }
            if (status_perm_1 != EXIT_SUCCESS) {
                status_perm_1 = sig_perk_perm_gen_given_random_input(pi_i[i * 4 + 1], rnd_buffer_pi_i[1]);
            }
            if (status_perm_2 != EXIT_SUCCESS) {
                status_perm_2 = sig_perk_perm_gen_given_random_input(pi_i[i * 4 + 2], rnd_buffer_pi_i[2]);
            }
            if (status_perm_3 != EXIT_SUCCESS) {
                status_perm_3 = sig_perk_perm_gen_given_random_input(pi_i[i * 4 + 3], rnd_buffer_pi_i[3]);
            }
        }
        // To make easier the times4 optimized implementation
        // we reinitialise the PRNG with a different domain separator, thus we use
        // PRG1 to sample the pi_i and PRG2 to sample the v_i
        sig_perk_prg_times4_init(&state, PRG2, salt, theta_i_times4);
        // v_i generation
        int j0 = 0;
        int j1 = 0;
        int j2 = 0;
        int j3 = 0;
        while ((j0 < PARAM_N1) || (j1 < PARAM_N1) || (j2 < PARAM_N1) || (j3 < PARAM_N1)) {
            int k0 = 0;
            int k1 = 0;
            int k2 = 0;
            int k3 = 0;
            sig_perk_prg_times4(&state, rnd_buff_times4_v_i, sizeof(rnd_buff_v_i[0]));
            while ((j0 < PARAM_N1) && (k0 < PRNG_BLOCK_SIZE / 2)) {
                v_i[i * 4 + 0][j0] = PARAM_Q_MASK & rnd_buff_v_i[0][k0++];
                if (v_i[i * 4 + 0][j0] < PARAM_Q) {  // accept the sample
                    j0++;
                }
            }
            while ((j1 < PARAM_N1) && (k1 < PRNG_BLOCK_SIZE / 2)) {
                v_i[i * 4 + 1][j1] = PARAM_Q_MASK & rnd_buff_v_i[1][k1++];
                if (v_i[i * 4 + 1][j1] < PARAM_Q) {  // accept the sample
                    j1++;
                }
            }
            while ((j2 < PARAM_N1) && (k2 < PRNG_BLOCK_SIZE / 2)) {
                v_i[i * 4 + 2][j2] = PARAM_Q_MASK & rnd_buff_v_i[2][k2++];
                if (v_i[i * 4 + 2][j2] < PARAM_Q) {  // accept the sample
                    j2++;
                }
            }
            while ((j3 < PARAM_N1) && (k3 < PRNG_BLOCK_SIZE / 2)) {
                v_i[i * 4 + 3][j3] = PARAM_Q_MASK & rnd_buff_v_i[3][k3++];
                if (v_i[i * 4 + 3][j3] < PARAM_Q) {  // accept the sample
                    j3++;
                }
            }
        }
    }
    memset_zero(rnd_buffer_pi_i, sizeof(rnd_buffer_pi_i));
    memset_zero(rnd_buff_v_i, sizeof(rnd_buff_v_i));
}

void sig_perk_gen_vect_v(vect1_t o, perm_t *pi_i, vect1_t *v_i, const perm_t pi) {
    vect1_t tmp_vect;
    perm_t tmp_compose_perm;

    sig_perk_perm_compose_inv(tmp_compose_perm, pi, pi_i[0]);
    sig_perk_perm_vect_permute(o, tmp_compose_perm, v_i[0]);
    for (int i = 1; i < PARAM_N - 1; ++i) {
        sig_perk_perm_compose_inv(tmp_compose_perm, tmp_compose_perm, pi_i[i]);
        sig_perk_perm_vect_permute(tmp_vect, tmp_compose_perm, v_i[i]);
        sig_perk_vect1_add(o, o, tmp_vect);
    }
    sig_perk_vect1_add(o, o, v_i[PARAM_N - 1]);
}

void sig_perk_gen_commitment_cmt_1_i(perk_instance_t *instance, salt_t salt, uint8_t tau) {
    uint8_t *cmt_i_times4[4];
    for (int i = 0; i < PARAM_N / 4; ++i) {
        sig_perk_hash_times4_state_t state4;
        sig_perk_hash_state_t state;
        uint8_t idx4[4] = {i * 4 + 0, i * 4 + 1, i * 4 + 2, i * 4 + 3};
        uint8_t tau4[4] = {tau, tau, tau, tau};
        const uint8_t *theta_i_times4[] = {
            instance->theta_tree[THETA_SEEDS_OFFSET + i * 4 + 0], instance->theta_tree[THETA_SEEDS_OFFSET + i * 4 + 1],
            instance->theta_tree[THETA_SEEDS_OFFSET + i * 4 + 2], instance->theta_tree[THETA_SEEDS_OFFSET + i * 4 + 3]};
        if (i == 0) {
            uint8_t pi_1_bytes[PARAM_N1];
            for (int j = 0; j < PARAM_N1; ++j) {
                pi_1_bytes[j] = (uint8_t)instance->pi_i[0][j];
            }
            sig_perk_hash_init(&state, salt, &tau, &idx4[0]);
            sig_perk_hash_update(&state, pi_1_bytes, PARAM_N1);
            sig_perk_hash_update(&state, instance->theta_tree[THETA_SEEDS_OFFSET], sizeof(theta_t));
            sig_perk_hash_final(&state, instance->cmt_1_i[0], H0);
            // avoid to clobber cmt_i[0] - implies PARAM_N >= 8
            cmt_i_times4[0] = instance->cmt_1_i[4];
        } else {
            cmt_i_times4[0] = instance->cmt_1_i[i * 4 + 0];
        }
        cmt_i_times4[1] = instance->cmt_1_i[i * 4 + 1];
        cmt_i_times4[2] = instance->cmt_1_i[i * 4 + 2];
        cmt_i_times4[3] = instance->cmt_1_i[i * 4 + 3];

        sig_perk_hash_times4_init(&state4, salt, tau4, idx4);
        sig_perk_hash_times4_update(&state4, theta_i_times4, sizeof(theta_t));
        sig_perk_hash_times4_final(&state4, cmt_i_times4, H0);
    }
}

void sig_perk_gen_instance_commitments(perk_instance_t *instance, salt_t salt, uint8_t tau, const perm_t pi) {
    sig_perk_gen_pi_i_and_v_i(instance->pi_i, instance->v_i, salt, (const theta_t *)instance->theta_tree);
    sig_perk_perm_gen_pi_1(instance->pi_i, pi);
    sig_perk_gen_commitment_cmt_1_i(instance, salt, tau);
}

void sig_perk_gen_instance_commitment_cmt_1(perk_instance_t *instance, const salt_t salt, uint8_t tau, const mat_t H,
                                            const vect1_t v) {
    sig_perk_hash_state_t state;
    vect2_t tmp;
    sig_perk_mat_vect_mul(tmp, H, v);
    sig_perk_hash_init(&state, salt, &tau, NULL);
    sig_perk_hash_update(&state, (uint8_t *)tmp, sizeof(tmp));
    sig_perk_hash_final(&state, instance->cmt_1, H0);
}