/**
 * @file sign_mira_256_mpc.c
 * @brief Implementation of mpc.h
 */

#include <stdio.h>
#include "hash_fips202.h"
#include "seedexpander_shake.h"
#include "mpc.h"
#include "tree.h"



/**
* \fn void sign_mira_init_shares(sign_mira_shares_t *shares)
* \brief This function initializes the MPC shares as required in the signature scheme
*
* \param[in] shares sign_mira_shares_t Representation of the shares
*/
void sign_mira_256_init_shares(sign_mira_256_shares_t *shares) {
    gfqm_vec_init(&(shares->a_), SIGN_MIRA_256_PARAM_R);
    for(size_t i = 0; i < SIGN_MIRA_256_PARAM_N_MPC; i++ ) {
        gf16_mat_init(&(shares->x[i]), 1, SIGN_MIRA_256_PARAM_K);
        gfqm_vec_init(&(shares->beta[i]), SIGN_MIRA_256_PARAM_R);
        gfqm_vec_init(&(shares->a[i]), SIGN_MIRA_256_PARAM_R);

        gf16_mat_set_zero(shares->x[i], 1, SIGN_MIRA_256_PARAM_K);
        gfqm_vec_set_zero(shares->beta[i], SIGN_MIRA_256_PARAM_R);
        gfqm_vec_set_zero(shares->a[i], SIGN_MIRA_256_PARAM_R);
        gfqm_set_zero(shares->c[i]);
    }
}


/**
* \fn void sign_mira_init_main_shares(sign_mira_main_shares_t *main_shares)
* \brief This function initializes the main shares as required in the signature scheme (hypercube)
*
* \param[in] main_shares sign_mira_main_shares_t Representation of the main shares
*/
void sign_mira_256_init_main_shares(sign_mira_256_main_shares_t *main_shares) {
    for(size_t i = 0; i < SIGN_MIRA_256_PARAM_D; i++ ) {
        gf16_mat_init(&(main_shares->x[i]), 1, SIGN_MIRA_256_PARAM_K);
        gfqm_vec_init(&(main_shares->beta[i]), SIGN_MIRA_256_PARAM_R);
        gfqm_vec_init(&(main_shares->a[i]), SIGN_MIRA_256_PARAM_R);

        gf16_mat_set_zero(main_shares->x[i], 1, SIGN_MIRA_256_PARAM_K);
        gfqm_vec_set_zero(main_shares->beta[i], SIGN_MIRA_256_PARAM_R);
        gfqm_vec_set_zero(main_shares->a[i], SIGN_MIRA_256_PARAM_R);
        gfqm_set_zero(main_shares->c[i]);
    }
}



/**
* \fn void sign_mira_init_challenge1(sign_mira_challenge1_t *challenge)
* \brief This function initializes the first challenge as required in the signature scheme
*
* \param[in] challenge sign_mira_challenge1_t Representation of the challenge
*/
void sign_mira_256_init_challenge1(sign_mira_256_challenge1_t *challenge) {
    gfqm_vec_init(&(challenge->gamma), SIGN_MIRA_256_PARAM_N);

    gfqm_vec_set_zero(challenge->gamma, SIGN_MIRA_256_PARAM_N);
    gfqm_set_zero(challenge->epsilon);
}



/**
* \fn void sign_mira_init_response1(sign_mira_response1_t *response)
* \brief This function initializes the response as required in the signature scheme
*
* \param[in] response sign_mira_response1_t Representation of the response
*/
void sign_mira_256_init_response1(sign_mira_256_response1_t *response) {
    gfqm_vec_init(&(response->alpha_), SIGN_MIRA_256_PARAM_R);
    gfqm_vec_set_zero(response->alpha_, SIGN_MIRA_256_PARAM_R);
    for(size_t i = 0; i < SIGN_MIRA_256_PARAM_D; i++ ) {
        gfqm_vec_init(&(response->alpha[i]), SIGN_MIRA_256_PARAM_R);
        gfqm_vec_set_zero(response->alpha[i], SIGN_MIRA_256_PARAM_R);
        gfqm_set_zero(response->v[i]);
    }
}



/**
* \fn void sign_mira_clear_shares(sign_mira_shares_t *shares)
* \brief This function clears the MPC shares as required in the signature scheme
*
* \param[in] shares sign_mira_shares_t Representation of the shares
*/
void sign_mira_256_clear_shares(sign_mira_256_shares_t *shares) {
    gfqm_vec_clear(shares->a_);
    for(size_t i = 0; i < SIGN_MIRA_256_PARAM_N_MPC; i++ ) {
        gf16_mat_clear(shares->x[i]);
        gfqm_vec_clear(shares->beta[i]);
        gfqm_vec_clear(shares->a[i]);
        gfqm_set_zero(shares->c[i]);
    }
}



/**
* \fn void sign_mira_clear_main_shares(sign_mira_main_shares_t *main_shares)
* \brief This function clears the main shares as required in the signature scheme (hypercube)
*
* \param[in] main_shares sign_mira_main_shares_t Representation of the main_shares
*/
void sign_mira_256_clear_main_shares(sign_mira_256_main_shares_t *main_shares) {
    for(size_t i = 0; i < SIGN_MIRA_256_PARAM_D; i++ ) {
        gf16_mat_clear(main_shares->x[i]);
        gfqm_vec_clear(main_shares->beta[i]);
        gfqm_vec_clear(main_shares->a[i]);
        gfqm_set_zero(main_shares->c[i]);
    }
}



/**
* \fn void sign_mira_clear_challenge1(sign_mira_challenge1_t *challenge)
* \brief This function clears the first challenge as required in the signature scheme
*
* \param[in] challenge sign_mira_challenge1_t Representation of the challenge1
*/
void sign_mira_256_clear_challenge1(sign_mira_256_challenge1_t *challenge) {
    gfqm_vec_clear(challenge->gamma);
    gfqm_set_zero(challenge->epsilon);
}



/**
* \fn void sign_mira_clear_response1(sign_mira_response1_t *response)
* \brief This function clears the responses as required in the signature scheme
*
* \param[in] response sign_mira_response1_t Representation of the responses
*/
void sign_mira_256_clear_response1(sign_mira_256_response1_t *response) {
    gfqm_vec_clear(response->alpha_);
    for(size_t i = 0; i < SIGN_MIRA_256_PARAM_D; i++ ) {
        gfqm_vec_clear(response->alpha[i]);
        gfqm_set_zero(response->v[i]);
    }
}




/**
* \fn void sign_mira_compute_shares(sign_mira_shares_t *shares, const gf16_mat x, const gfqm_vec A, const uint8_t *theta_i, const uint8_t *salt)
*
* \brief This function computes the shares of the signature scheme
*
* The polynomial a determines the annihilator polynomial such that x = (x1, x2) is a root of a.
*
* \param[out] shares sign_mira_256_shares_t Shares of all parties of the protocol
* \param[in] x gf16_mat Secret x
* \param[in] A gfqm_vec Annulator polynomial A
* \param[in] theta_i uint8_t* Input seeds
* \param[in] salt uint8_t* Salt
*/
void sign_mira_256_compute_shares(sign_mira_256_shares_t *shares, const gf16_mat x, const gfqm_vec A, const uint8_t *theta_i, const uint8_t *salt) {
  gfqm tmp;

  uint8_t random[SIGN_MIRA_256_VEC_R_BYTES] = {0};
  seedexpander_shake_t seedexpander;

  uint8_t random0[2 * SIGN_MIRA_256_VEC_R_BYTES + SIGN_MIRA_256_PARAM_M_BYTES + SIGN_MIRA_256_VEC_K_BYTES] = {0};
  uint8_t random1[2 * SIGN_MIRA_256_VEC_R_BYTES + SIGN_MIRA_256_PARAM_M_BYTES + SIGN_MIRA_256_VEC_K_BYTES] = {0};
  uint8_t random2[2 * SIGN_MIRA_256_VEC_R_BYTES + SIGN_MIRA_256_PARAM_M_BYTES + SIGN_MIRA_256_VEC_K_BYTES] = {0};
  uint8_t random3[2 * SIGN_MIRA_256_VEC_R_BYTES + SIGN_MIRA_256_PARAM_M_BYTES + SIGN_MIRA_256_VEC_K_BYTES] = {0};
  uint8_t *random_x4[] = {random0, random1, random2, random3};
  seedexpander_shake_x4_t seedexpander_x4;
  const uint8_t *salt_x4[] = {salt, salt, salt, salt};

  gfqm_vec_set_zero(shares->a_, SIGN_MIRA_256_PARAM_R);
  gf16_mat_set_zero(shares->x[SIGN_MIRA_256_PARAM_N_MPC - 1], 1, SIGN_MIRA_256_PARAM_K);
  gfqm_vec_set_zero(shares->a[SIGN_MIRA_256_PARAM_N_MPC - 1], SIGN_MIRA_256_PARAM_R);
  gfqm_vec_set_zero(shares->beta[SIGN_MIRA_256_PARAM_N_MPC - 1], SIGN_MIRA_256_PARAM_R);
  gfqm_set_zero(shares->c[SIGN_MIRA_256_PARAM_N_MPC - 1]);

  // Compute the first N-1 shares
  for(size_t i = 0; i < SIGN_MIRA_256_PARAM_N_MPC; i+=4) {
    // Sample randomness
    const uint8_t *seed_x4[] = {&theta_i[SIGN_MIRA_256_SECURITY_BYTES * i], &theta_i[SIGN_MIRA_256_SECURITY_BYTES * (i + 1)],
                                &theta_i[SIGN_MIRA_256_SECURITY_BYTES * (i + 2)], &theta_i[SIGN_MIRA_256_SECURITY_BYTES * (i + 3)]};

    seedexpander_shake_x4_init(&seedexpander_x4, seed_x4, SIGN_MIRA_256_SECURITY_BYTES, salt_x4, 2 * SIGN_MIRA_256_SECURITY_BYTES);
    seedexpander_shake_x4_get_bytes(&seedexpander_x4, random_x4, (2 * SIGN_MIRA_256_VEC_R_BYTES + SIGN_MIRA_256_PARAM_M_BYTES + SIGN_MIRA_256_VEC_K_BYTES));

    for(size_t j = 0; j < 4; j++) {
      if ((i + j) < (SIGN_MIRA_256_PARAM_N_MPC - 1)) {
        gfqm_vec_set_zero(shares->a[i + j], SIGN_MIRA_256_PARAM_R);
        gfqm_set_zero(shares->c[i + j]);

        // Compute share [a]_i
        gfqm_vec_from_string(shares->a[i + j], SIGN_MIRA_256_PARAM_R, &random_x4[j][0]);
        gfqm_vec_add(shares->a_, shares->a_, shares->a[i + j], SIGN_MIRA_256_PARAM_R);

        // Compute share [x]_i
        gf16_mat_from_string(shares->x[i + j], 1, SIGN_MIRA_256_PARAM_K, &random_x4[j][SIGN_MIRA_256_VEC_R_BYTES]);
        gf16_mat_add(shares->x[SIGN_MIRA_256_PARAM_N_MPC - 1], shares->x[SIGN_MIRA_256_PARAM_N_MPC - 1], shares->x[i + j], 1, SIGN_MIRA_256_PARAM_K);

        // Compute share [beta]_i
        gfqm_vec_from_string(shares->beta[i + j], SIGN_MIRA_256_PARAM_R, &random_x4[j][SIGN_MIRA_256_VEC_K_BYTES + SIGN_MIRA_256_VEC_R_BYTES]);
        gfqm_vec_add(shares->beta[SIGN_MIRA_256_PARAM_N_MPC - 1], shares->beta[SIGN_MIRA_256_PARAM_N_MPC - 1], shares->beta[i + j], SIGN_MIRA_256_PARAM_R);

        // Compute share [c]_i
        gfqm_from_string(shares->c[i + j], &random_x4[j][2 * SIGN_MIRA_256_VEC_R_BYTES + SIGN_MIRA_256_VEC_K_BYTES]);
        gfqm_add(shares->c[SIGN_MIRA_256_PARAM_N_MPC - 1], shares->c[SIGN_MIRA_256_PARAM_N_MPC - 1], shares->c[i + j]);
      }
    }
  }

  // Sample randomness for share N
  seedexpander_shake_init(&seedexpander, &theta_i[SIGN_MIRA_256_SECURITY_BYTES * (SIGN_MIRA_256_PARAM_N_MPC - 1)], SIGN_MIRA_256_SECURITY_BYTES, salt, 2 * SIGN_MIRA_256_SECURITY_BYTES);
  seedexpander_shake_get_bytes(&seedexpander, random, SIGN_MIRA_256_VEC_R_BYTES);

  // Compute u and share [a]_N
  gfqm_vec_from_string(shares->a[SIGN_MIRA_256_PARAM_N_MPC - 1], SIGN_MIRA_256_PARAM_R, &random[0]);
  gfqm_vec_add(shares->a_, shares->a_, shares->a[SIGN_MIRA_256_PARAM_N_MPC - 1], SIGN_MIRA_256_PARAM_R);

  // Computing c and share [c]_N
  gfqm_vec_inner_product(tmp, shares->a_, A, SIGN_MIRA_256_PARAM_R);
  gfqm_add(shares->c[SIGN_MIRA_256_PARAM_N_MPC - 1], shares->c[SIGN_MIRA_256_PARAM_N_MPC - 1], tmp);

  // Compute shares [beta]_N, [x]_N
  gfqm_vec_add(shares->beta[SIGN_MIRA_256_PARAM_N_MPC - 1], shares->beta[SIGN_MIRA_256_PARAM_N_MPC - 1], A, SIGN_MIRA_256_PARAM_R);
  gf16_mat_add(shares->x[SIGN_MIRA_256_PARAM_N_MPC - 1], shares->x[SIGN_MIRA_256_PARAM_N_MPC - 1], x, 1, SIGN_MIRA_256_PARAM_K);

  memset(random, 0, SIGN_MIRA_256_VEC_R_BYTES);
}



/**
* \fn void sign_mira_commit_to_shares(uint8_t *commits, uint8_t e, const uint8_t *salt, const sign_mira_256_shares_t *shares, const uint8_t *theta_i)
*
* \brief This function computes the commitments to the shares used in the signature scheme.
*
* \param[out] commits Commitments to the shares
* \param[in] e Current iteration of the protocol
* \param[in] salt Salt
* \param[in] shares Representation of the shares
* \param[in] theta_i Seed used to generate the shares
*/
void sign_mira_256_commit_to_shares(uint8_t *commits, uint8_t e, const uint8_t *salt, const sign_mira_256_shares_t *shares, const uint8_t *theta_i) {

  uint8_t domain_separator = DOMAIN_SEPARATOR_COMMITMENT;
  uint8_t state[SIGN_MIRA_256_PARAM_STATE_BYTES] = {0};

  uint8_t *domain_separator_x4[] = {&domain_separator, &domain_separator, &domain_separator, &domain_separator};
  const uint8_t *salt_x4[] = {salt, salt, salt, salt};
  uint8_t *e_x4[] = {&e, &e, &e, &e};
  uint8_t index_0 = 0, index_1 = 1, index_2 = 2, index_3 = 3;
  uint8_t *index_x4[] = {&index_0, &index_1, &index_2, &index_3};

  // Compute the first N-1 commitments (last commitment is a dummy computation that is rewritten after the loop)
  for(size_t i = 0; i < SIGN_MIRA_256_PARAM_N_MPC; i+=4) {
    const uint8_t *seed_x4[] = {&theta_i[SIGN_MIRA_256_SECURITY_BYTES * i], &theta_i[SIGN_MIRA_256_SECURITY_BYTES * (i + 1)],
                                &theta_i[SIGN_MIRA_256_SECURITY_BYTES * (i + 2)], &theta_i[SIGN_MIRA_256_SECURITY_BYTES * (i + 3)]};

    uint8_t *commit_x4[] = {&commits[2 * SIGN_MIRA_256_SECURITY_BYTES * i], &commits[2 * SIGN_MIRA_256_SECURITY_BYTES * (i + 1)],
                            &commits[2 * SIGN_MIRA_256_SECURITY_BYTES * (i + 2)], &commits[2 * SIGN_MIRA_256_SECURITY_BYTES * (i + 3)]};

    hash_sha3_x4_ctx ctx;
    hash_sha3_x4_init(&ctx);
    hash_sha3_x4_absorb(&ctx, (const uint8_t **) domain_separator_x4, sizeof(uint8_t));
    hash_sha3_x4_absorb(&ctx, (const uint8_t **) salt_x4, 2 * SIGN_MIRA_256_SECURITY_BYTES);
    hash_sha3_x4_absorb(&ctx, (const uint8_t **) e_x4, sizeof(uint8_t));
    hash_sha3_x4_absorb(&ctx, (const uint8_t **) index_x4, sizeof(uint8_t));
    hash_sha3_x4_absorb(&ctx, (const uint8_t **) seed_x4, SIGN_MIRA_256_SECURITY_BYTES);
    hash_sha3_x4_finalize(commit_x4, &ctx);

    index_0 += 4;
    index_1 += 4;
    index_2 += 4;
    index_3 += 4;
  }

  // Compute commitment N
  uint8_t i = (uint8_t) ((uint16_t)SIGN_MIRA_256_PARAM_N_MPC - 1);

  hash_sha3_ctx ctx;
  hash_sha3_init(&ctx);
  hash_sha3_absorb(&ctx, &domain_separator, sizeof(uint8_t));
  hash_sha3_absorb(&ctx, salt, 2 * SIGN_MIRA_256_SECURITY_BYTES);
  hash_sha3_absorb(&ctx, &e, sizeof(uint8_t));
  hash_sha3_absorb(&ctx, &i, sizeof(uint8_t));
  hash_sha3_absorb(&ctx, &theta_i[SIGN_MIRA_256_SECURITY_BYTES * i], SIGN_MIRA_256_SECURITY_BYTES);

  gf16_mat_to_string(&state[0], shares->x[i], 1, SIGN_MIRA_256_PARAM_K);
  gfqm_vec_to_string(&state[SIGN_MIRA_256_VEC_K_BYTES], shares->beta[i], SIGN_MIRA_256_PARAM_R);
  gfqm_to_string(&state[SIGN_MIRA_256_VEC_K_BYTES + SIGN_MIRA_256_VEC_R_BYTES], shares->c[i]);

  hash_sha3_absorb(&ctx, state, SIGN_MIRA_256_PARAM_STATE_BYTES);
  hash_sha3_finalize(&commits[2 * SIGN_MIRA_256_SECURITY_BYTES * i], &ctx);

  memset(state, 0, SIGN_MIRA_256_PARAM_STATE_BYTES);
}


/**
* \fn void sign_mira_compute_main_shares(sign_mira_main_shares_t *main_shares, const sign_mira_shares_t *shares)
*
* \brief This function generates the main main_shares from the leaves shares
*
* \param[out] challenge sign_mira_main_shares_t Representation of the main_shares
* \param[in] shares sign_mira_shares_t Representation of the shares
*/
void sign_mira_256_compute_main_shares(sign_mira_256_main_shares_t *main_shares, const sign_mira_256_shares_t *shares) {

    uint8_t bit = 0x1;
    for(size_t i = 0; i < SIGN_MIRA_256_PARAM_D; i++) {
        for(size_t j = 0; j < SIGN_MIRA_256_PARAM_N_MPC; j++) {
            if((j & bit) == 0) {
                gf16_mat_add(main_shares->x[i], main_shares->x[i], shares->x[j], 1, SIGN_MIRA_256_PARAM_K);
                gfqm_vec_add(main_shares->beta[i], main_shares->beta[i], shares->beta[j], SIGN_MIRA_256_PARAM_R);
                gfqm_vec_add(main_shares->a[i], main_shares->a[i], shares->a[j], SIGN_MIRA_256_PARAM_R);
                gfqm_add(main_shares->c[i], main_shares->c[i], shares->c[j]);
            }
        }
        bit <<= 1;
    }
}


/**
* \fn void sign_mira_compute_challenge1(sign_mira_challenge1_t *challenge, const uint8_t *seed_input, const uint8_t *salt)
* \brief This function generates challenges from an input seed
*
* \param[out] challenge Array of sign_mira_challenge1_t Representation of challenge
* \param[in] seed_input String containing the input seed
* \param[in] salt String containing the salt
*/
void sign_mira_256_compute_challenge1(sign_mira_256_challenge1_t *challenge, const uint8_t *seed_input, const uint8_t *salt) {

  uint8_t random[SIGN_MIRA_256_VEC_N_PLUS_ONE_BYTES] = {0};
  seedexpander_shake_t seedexpander;
  seedexpander_shake_init(&seedexpander, seed_input, 2 * SIGN_MIRA_256_SECURITY_BYTES, salt, 2 * SIGN_MIRA_256_SECURITY_BYTES);

  gfqm_vec tmp;
  gfqm_vec_init(&tmp, SIGN_MIRA_256_PARAM_N + 1);

  for(size_t e = 0; e < SIGN_MIRA_256_PARAM_TAU; e++) {
    seedexpander_shake_get_bytes(&seedexpander, random, SIGN_MIRA_256_VEC_N_PLUS_ONE_BYTES);
    gfqm_vec_from_string(tmp, SIGN_MIRA_256_PARAM_N + 1, random);
    gfqm_vec_set(challenge[e].gamma, tmp, SIGN_MIRA_256_PARAM_N);
    gfqm_set(challenge[e].epsilon, tmp[SIGN_MIRA_256_PARAM_N]);
  }

  gfqm_vec_clear(tmp);
  memset(random, 0, SIGN_MIRA_256_VEC_N_PLUS_ONE_BYTES);
}



/**
* \fn void sign_mira_compute_response1(sign_mira_response1_t *response, const sign_mira_main_shares_t *shares, const sign_mira_challenge1_t *challenge)
*
* \brief This function computes the first response used in the signature scheme
*
* \param[out] response sign_mira_response1_t Representation of the response
* \param[in] shares sign_mira_shares_t Representation of the shares
* \param[in] challenge sign_mira_challenge1_t Representation of the challenge
*/
void sign_mira_256_compute_response1(sign_mira_256_response1_t *response, const sign_mira_256_main_shares_t *shares, const sign_mira_256_challenge1_t *challenge, const gf16_mat M0, const gf16_mat *Mi, const gf16_mat x, const gfqm_vec a) {

    gf16_mat Ei, tmp;
    gf16_mat_init(&Ei, SIGN_MIRA_256_PARAM_M, SIGN_MIRA_256_PARAM_N);
    gf16_mat_init(&tmp, SIGN_MIRA_256_PARAM_M, SIGN_MIRA_256_PARAM_N);

    gfqm_vec Ei_vec;
    gfqm_vec Ei_vec_pow_k;
    gfqm_vec_init(&Ei_vec, SIGN_MIRA_256_PARAM_N);
    gfqm_vec_init(&Ei_vec_pow_k, SIGN_MIRA_256_PARAM_N);

    gfqm z[SIGN_MIRA_256_PARAM_D];
    gfqm gamma_e;
    gfqm_vec w;
    gfqm_vec_init(&w, SIGN_MIRA_256_PARAM_R);

    for(size_t i = 0; i < SIGN_MIRA_256_PARAM_D; i++) {
        // Compute [E]
        // M0 on leaf 0 => present in every main share
        gf16_mat_set(Ei, M0, SIGN_MIRA_256_PARAM_M, SIGN_MIRA_256_PARAM_N);

        for(int j=0 ; j<SIGN_MIRA_256_PARAM_K ; j++) {
            gf16_mat_scalar_mul(tmp, Mi[j], shares->x[i][j], SIGN_MIRA_256_PARAM_M, SIGN_MIRA_256_PARAM_N);
            gf16_mat_add(Ei, Ei, tmp, SIGN_MIRA_256_PARAM_M, SIGN_MIRA_256_PARAM_N);
        }

        //"Transpose" into elements of Fqm
        for(int j=0 ; j<SIGN_MIRA_256_PARAM_N ; j++) {
            for(int k=0 ; k<SIGN_MIRA_256_PARAM_M ; k++) {
                Ei_vec[j][k] = Ei[k*SIGN_MIRA_256_PARAM_N + j];
            }
            gfqm_set(Ei_vec_pow_k[j], Ei_vec[j]);
        }

        // Compute [w] and [z]
        for(size_t k=0 ; k < SIGN_MIRA_256_PARAM_R ; k++) {
            gfqm_set_zero(w[k]);
            //w[k] = <gamma . (e_ji^q^k)>
            for(int j=0 ; j<SIGN_MIRA_256_PARAM_N ; j++) {
                gfqm_mul(gamma_e, Ei_vec_pow_k[j], challenge->gamma[j]);
                gfqm_add(w[k], w[k], gamma_e);
            }
            for(size_t j = 0; j < SIGN_MIRA_256_PARAM_N; j++) {
                gfqm_pow16(Ei_vec_pow_k[j], Ei_vec_pow_k[j]);
            }
        }
        //Compute [z]
        gfqm_set_zero(z[i]);
        //z = <gamma . (e_ji^q^r)>
        for(int j=0 ; j<SIGN_MIRA_256_PARAM_N ; j++) {
            gfqm_mul(gamma_e, Ei_vec_pow_k[j], challenge->gamma[j]);
            gfqm_add(z[i], z[i], gamma_e);
        }

        // Compute [alpha]
        gfqm_vec_scalar_mul(response->alpha[i], w, challenge->epsilon, SIGN_MIRA_256_PARAM_R);
        gfqm_vec_add(response->alpha[i], response->alpha[i], shares->a[i], SIGN_MIRA_256_PARAM_R);
    }

    // Compute alpha
    sign_mira_256_reconstruct_alpha(response, M0, Mi, x, a, challenge, 1);

    // Compute [v]
    for(size_t i = 0; i < SIGN_MIRA_256_PARAM_D; i++) {
        gfqm_mul(response->v[i], challenge->epsilon, z[i]);
        gfqm_vec_inner_product(gamma_e, response->alpha_, shares->beta[i], SIGN_MIRA_256_PARAM_R); //Use gamma_e as a tmp buffer
        gfqm_add(response->v[i], response->v[i], gamma_e);
        gfqm_add(response->v[i], response->v[i], shares->c[i]);
        gfqm_set_zero(gamma_e);
    }

    gfqm_vec_clear(Ei_vec);
    gfqm_vec_clear(Ei_vec_pow_k);
    gfqm_vec_clear(w);

    gf16_mat_clear(Ei);
    gf16_mat_clear(tmp);
}



/**
* \fn void sign_mira_reconstruct_alpha(sign_mira_response1_t *response, const gf16_mat M0, const gf16_mat *Mi, const gf16_mat x, const gfqm_vec a, const sign_mira_challenge1_t *challenge, int include_M0)
*
* \brief This function computes alpha given x and a
*
* \param[out] response sign_mira_response1_t representation of response
* \param[in] M0 gf16_mat Matrix M0
* \param[in] Mi gf16_mat* Matrices Mi
* \param[in] x gf16_mat Vector [x]
* \param[in] a gfqm_vec Values of [a]
* \param[in] challenge sign_mira_challenge1_t representation of challenge
* \param[in] include_M0 int If not 0, M0 will be included in the computation of [E]
*/
void sign_mira_256_reconstruct_alpha(sign_mira_256_response1_t *response, const gf16_mat M0, const gf16_mat *Mi, const gf16_mat x, const gfqm_vec a, const sign_mira_256_challenge1_t *challenge, int include_M0) {
    gfqm_vec w, aux;
    gfqm_vec_init(&w, SIGN_MIRA_256_PARAM_R);
    gfqm_vec_init(&aux, SIGN_MIRA_256_PARAM_R);

    gf16_mat E, tmp;
    gf16_mat_init(&E, SIGN_MIRA_256_PARAM_M, SIGN_MIRA_256_PARAM_N);
    gf16_mat_init(&tmp, SIGN_MIRA_256_PARAM_M, SIGN_MIRA_256_PARAM_N);

    //Recompute E
    if(include_M0 != 0) gf16_mat_set(E, M0, SIGN_MIRA_256_PARAM_M, SIGN_MIRA_256_PARAM_N);
    else gf16_mat_set_zero(E, SIGN_MIRA_256_PARAM_M, SIGN_MIRA_256_PARAM_N);

    for(int i=0 ; i<SIGN_MIRA_256_PARAM_K ; i++) {
        gf16_mat_scalar_mul(tmp, Mi[i], x[i], SIGN_MIRA_256_PARAM_M, SIGN_MIRA_256_PARAM_N);
        gf16_mat_add(E, E, tmp, SIGN_MIRA_256_PARAM_M, SIGN_MIRA_256_PARAM_N);
    }

    //"Transpose" into elements of Fqm
    gfqm_vec E_vec;
    gfqm_vec E_vec_pow_k;
    gfqm_vec_init(&E_vec, SIGN_MIRA_256_PARAM_N);
    gfqm_vec_init(&E_vec_pow_k, SIGN_MIRA_256_PARAM_N);

    for(int j=0 ; j<SIGN_MIRA_256_PARAM_N ; j++) {
        for(int k=0 ; k<SIGN_MIRA_256_PARAM_M ; k++) {
            E_vec[j][k] = E[k*SIGN_MIRA_256_PARAM_N + j];
        }
        gfqm_set(E_vec_pow_k[j], E_vec[j]);
    }

    //Compute w[i] = Ej^q^(i)
    for(size_t k=0 ; k<SIGN_MIRA_256_PARAM_R ; k++) {
        gfqm_set_zero(w[k]);
        gfqm gamma_e;
        //w[k] = <gamma . (e_ji^q^k)>
        for(int j=0 ; j<SIGN_MIRA_256_PARAM_N ; j++) {
            gfqm_mul(gamma_e, E_vec_pow_k[j], challenge->gamma[j]);
            gfqm_add(w[k], w[k], gamma_e);
        }
        for(size_t j = 0; j < SIGN_MIRA_256_PARAM_N; j++) {
            gfqm_pow16(E_vec_pow_k[j], E_vec_pow_k[j]);
        }
    }

    // Compute [alpha]_ch2
    gfqm_vec_set_zero(response->alpha_, SIGN_MIRA_256_PARAM_R);
    gfqm_vec_scalar_mul(response->alpha_, w, challenge->epsilon, SIGN_MIRA_256_PARAM_R);
    gfqm_vec_add(response->alpha_, response->alpha_, a, SIGN_MIRA_256_PARAM_R);

    gf16_mat_clear(E);
    gf16_mat_clear(tmp);

    gfqm_vec_clear(E_vec);
    gfqm_vec_clear(E_vec_pow_k);
    gfqm_vec_clear(w);
    gfqm_vec_clear(aux);
}



/**
* \fn void sign_mira_reconstruct_share(gf16_mat x, gfqm_vec a, const uint8_t *theta_i, uint8_t challenge2, const uint8_t *salt)
*
* \brief This function reconstructs the shares of index challenge2
*
* \param[in] x gf16_mat Vector [x]
* \param[in] a gfqm_vec Values of [a]
* \param[in] theta_i String containing the input seed
* \param[in] challenge2 Integer concerning the second challenge
* \param[in] salt String containing the salt
*/
void sign_mira_256_reconstruct_share(gf16_mat x, gfqm_vec a, const uint8_t *theta_i, uint8_t challenge2, const uint8_t *salt) {

  uint8_t random[2 * SIGN_MIRA_256_VEC_R_BYTES + SIGN_MIRA_256_PARAM_M_BYTES + SIGN_MIRA_256_VEC_K_BYTES] = {0};
  seedexpander_shake_t seedexpander;

  // Sample randomness from seed theta
  seedexpander_shake_init(&seedexpander, &theta_i[SIGN_MIRA_256_SECURITY_BYTES * challenge2], SIGN_MIRA_256_SECURITY_BYTES, salt, 2 * SIGN_MIRA_256_SECURITY_BYTES);
  seedexpander_shake_get_bytes(&seedexpander, random, 2 * SIGN_MIRA_256_VEC_R_BYTES + SIGN_MIRA_256_PARAM_M_BYTES + SIGN_MIRA_256_VEC_K_BYTES);

  // Recompute shares [a], [x] of index challenge2
  gfqm_vec_from_string(a, SIGN_MIRA_256_PARAM_R, &random[0]);

  // Compute share [x]_i
  gf16_mat_from_string(x, 1, SIGN_MIRA_256_PARAM_K, &random[SIGN_MIRA_256_VEC_R_BYTES]);

  memset(random, 0, (2 * SIGN_MIRA_256_VEC_R_BYTES + SIGN_MIRA_256_PARAM_M_BYTES + SIGN_MIRA_256_VEC_K_BYTES));
}





/**
* \fn void sign_mira_recompute_commitments(uint8_t *commits, uint8_t *theta_i, uint8_t e, const uint8_t *salt, const uint8_t *state, uint8_t hidden)
*
* \brief This function commits the MPC shares as required in the signature scheme (when verifying)
*
* \param[out] commits String commitment determined by the input data
* \param[in] theta_i String containing the input seed used when generating the MPC shares (excluding the hidden position)
* \param[in] e Integer concerning the current iteration of the protocol
* \param[in] salt String containing the salt
* \param[in] state String containing the public data determined by the used shares
* \param[in] hidden String containing the public data determined by the hidden private data
*/
void sign_mira_256_recompute_commitments(uint8_t *commits, uint8_t *theta_i, uint8_t e, const uint8_t *salt, const uint8_t *state, uint8_t hidden) {

  uint8_t domain_separator = DOMAIN_SEPARATOR_COMMITMENT;
  sign_mira_256_seed_tree_node_t partial_tree_seeds[SIGN_MIRA_256_PARAM_N_MPC_LOG2] = {0};
  sign_mira_256_seed_tree_t theta_tree = {0};

  memcpy(&partial_tree_seeds, &state[0], SIGN_MIRA_256_PARAM_TREE_PATH_BYTES);
  sign_mira_256_tree_expand_partial(theta_tree, (const sign_mira_256_seed_tree_node_t *) partial_tree_seeds, salt, hidden);
  memcpy(theta_i, &theta_tree[SIGN_MIRA_256_PARAM_N_MPC - 1], SIGN_MIRA_256_PARAM_TREE_LEAF_BYTES);

  const uint8_t *salt_x4[] = {salt, salt, salt, salt};
  uint8_t *e_x4[] = {&e, &e, &e, &e};
  uint8_t *domain_separator_x4[] = {&domain_separator, &domain_separator, &domain_separator, &domain_separator};
  uint8_t index_0 = 0, index_1 = 1, index_2 = 2, index_3 = 3;
  uint8_t *index_x4[] = {&index_0, &index_1, &index_2, &index_3};

  // Recompute the first N-1 commitments (last commitment and index hidden are overwrited after)
  for(size_t i = 0; i < SIGN_MIRA_256_PARAM_N_MPC; i+=4) {

    const uint8_t *seed_x4[] = {&theta_i[SIGN_MIRA_256_SECURITY_BYTES * i], &theta_i[SIGN_MIRA_256_SECURITY_BYTES * (i + 1)],
                                &theta_i[SIGN_MIRA_256_SECURITY_BYTES * (i + 2)], &theta_i[SIGN_MIRA_256_SECURITY_BYTES * (i + 3)]};

    uint8_t *commit_x4[] = {&commits[2 * SIGN_MIRA_256_SECURITY_BYTES * i], &commits[2 * SIGN_MIRA_256_SECURITY_BYTES * (i + 1)],
                            &commits[2 * SIGN_MIRA_256_SECURITY_BYTES * (i + 2)], &commits[2 * SIGN_MIRA_256_SECURITY_BYTES * (i + 3)]};

    hash_sha3_x4_ctx ctx;
    hash_sha3_x4_init(&ctx);
    hash_sha3_x4_absorb(&ctx, (const uint8_t **) domain_separator_x4, sizeof(uint8_t));
    hash_sha3_x4_absorb(&ctx, (const uint8_t **) salt_x4, 2 * SIGN_MIRA_256_SECURITY_BYTES);
    hash_sha3_x4_absorb(&ctx, (const uint8_t **) e_x4, sizeof(uint8_t));
    hash_sha3_x4_absorb(&ctx, (const uint8_t **) index_x4, sizeof(uint8_t));
    hash_sha3_x4_absorb(&ctx, (const uint8_t **) seed_x4, SIGN_MIRA_256_SECURITY_BYTES);
    hash_sha3_x4_finalize(commit_x4, &ctx);

    index_0 += 4;
    index_1 += 4;
    index_2 += 4;
    index_3 += 4;
  }

  memcpy(&commits[2 * SIGN_MIRA_256_SECURITY_BYTES * hidden], &state[SIGN_MIRA_256_PARAM_TREE_PATH_BYTES], 2 * SIGN_MIRA_256_SECURITY_BYTES);

  // Recompute commitment N
  uint8_t i = (uint8_t) ((uint16_t)SIGN_MIRA_256_PARAM_N_MPC - 1);

  if (i == hidden) {
    memcpy(&commits[2 * SIGN_MIRA_256_SECURITY_BYTES * i], &state[SIGN_MIRA_256_PARAM_TREE_PATH_BYTES], 2 * SIGN_MIRA_256_SECURITY_BYTES);
  }
  else {
    hash_sha3_ctx ctx;
    hash_sha3_init(&ctx);
    hash_sha3_absorb(&ctx, &domain_separator, sizeof(uint8_t));
    hash_sha3_absorb(&ctx, salt, 2 * SIGN_MIRA_256_SECURITY_BYTES);
    hash_sha3_absorb(&ctx, &e, sizeof(uint8_t));
    hash_sha3_absorb(&ctx, &i, sizeof(uint8_t));
    hash_sha3_absorb(&ctx, &theta_i[SIGN_MIRA_256_SECURITY_BYTES * i], SIGN_MIRA_256_SECURITY_BYTES);
    hash_sha3_absorb(&ctx, &state[SIGN_MIRA_256_PARAM_TREE_PATH_BYTES + (2 * SIGN_MIRA_256_SECURITY_BYTES) + SIGN_MIRA_256_VEC_R_BYTES], SIGN_MIRA_256_PARAM_STATE_BYTES);
    hash_sha3_finalize(&commits[2 * SIGN_MIRA_256_SECURITY_BYTES * i], &ctx);
  }
}



/**
* \fn void sign_mira_recompute_shares(sign_mira_shares_t *shares, const uint8_t *theta_i, const uint8_t *state, uint8_t second_challenge, const uint8_t *salt)
*
* \brief This function generates the random MPC shares determined by the signature scheme (when verifying)
*
* \param[out] shares sign_mira_shares_t representation of shares such that the sum of all of them gives the input data
* \param[in] theta_i String containing the input seed used when generating the MPC shares (excluding the hidden position)
* \param[in] state String containing the public data determined by the used shares
* \param[in] second_challenge Integer concerning the second challenge
* \param[in] salt String containing the salt
*/
void sign_mira_256_recompute_shares(sign_mira_256_shares_t *shares, const uint8_t *theta_i, const uint8_t *state, uint8_t second_challenge, const uint8_t *salt) {

  uint8_t random[SIGN_MIRA_256_VEC_R_BYTES] = {0};
  seedexpander_shake_x4_t seedexpander_x4;
  seedexpander_shake_t seedexpander;

  const uint8_t *salt_x4[] = {salt, salt, salt, salt};

  for(size_t i = 0; i < SIGN_MIRA_256_PARAM_N_MPC; i+=4) {
    const uint8_t *seed_x4[] = {&theta_i[SIGN_MIRA_256_SECURITY_BYTES * i], &theta_i[SIGN_MIRA_256_SECURITY_BYTES * (i + 1)],
                                    &theta_i[SIGN_MIRA_256_SECURITY_BYTES * (i + 2)], &theta_i[SIGN_MIRA_256_SECURITY_BYTES * (i + 3)]};

    uint8_t random0[2 * SIGN_MIRA_256_VEC_R_BYTES + SIGN_MIRA_256_PARAM_M_BYTES + SIGN_MIRA_256_VEC_K_BYTES] = {0};
    uint8_t random1[2 * SIGN_MIRA_256_VEC_R_BYTES + SIGN_MIRA_256_PARAM_M_BYTES + SIGN_MIRA_256_VEC_K_BYTES] = {0};
    uint8_t random2[2 * SIGN_MIRA_256_VEC_R_BYTES + SIGN_MIRA_256_PARAM_M_BYTES + SIGN_MIRA_256_VEC_K_BYTES] = {0};
    uint8_t random3[2 * SIGN_MIRA_256_VEC_R_BYTES + SIGN_MIRA_256_PARAM_M_BYTES + SIGN_MIRA_256_VEC_K_BYTES] = {0};
    uint8_t *random_x4[] = {random0, random1, random2, random3};

    seedexpander_shake_x4_init(&seedexpander_x4, seed_x4, SIGN_MIRA_256_SECURITY_BYTES, salt_x4, 2 * SIGN_MIRA_256_SECURITY_BYTES);
    seedexpander_shake_x4_get_bytes(&seedexpander_x4, random_x4, 2 * SIGN_MIRA_256_VEC_R_BYTES + SIGN_MIRA_256_PARAM_M_BYTES + SIGN_MIRA_256_VEC_K_BYTES);

    for(size_t j = 0; j < 4; j++) {
      if (((i + j) < (SIGN_MIRA_256_PARAM_N_MPC - 1)) && ((i + j) != second_challenge)) {
        // Recompute share [a]_i
        gfqm_vec_from_string(shares->a[i + j], SIGN_MIRA_256_PARAM_R, &random_x4[j][0]);

        // Recompute share [x]_i
        gf16_mat_from_string(shares->x[i + j], 1, SIGN_MIRA_256_PARAM_K, &random_x4[j][SIGN_MIRA_256_VEC_R_BYTES]);

        // Recompute share [beta]_i
        gfqm_vec_from_string(shares->beta[i + j], SIGN_MIRA_256_PARAM_R, &random_x4[j][SIGN_MIRA_256_VEC_K_BYTES + SIGN_MIRA_256_VEC_R_BYTES]);

        // Recompute share [c]_i
        gfqm_from_string(shares->c[i + j], &random_x4[j][2 * SIGN_MIRA_256_VEC_R_BYTES + SIGN_MIRA_256_VEC_K_BYTES]);
      }
    }
  }

  // Recompute share N
  if ((SIGN_MIRA_256_PARAM_N_MPC - 1) != second_challenge) {
    // Sample randomness
    seedexpander_shake_init(&seedexpander, &theta_i[SIGN_MIRA_256_SECURITY_BYTES * (SIGN_MIRA_256_PARAM_N_MPC - 1)], SIGN_MIRA_256_SECURITY_BYTES, salt, 2 * SIGN_MIRA_256_SECURITY_BYTES);
    seedexpander_shake_get_bytes(&seedexpander, random, SIGN_MIRA_256_VEC_R_BYTES);

    // Recompute [a]_N
    gfqm_vec_from_string(shares->a[SIGN_MIRA_256_PARAM_N_MPC - 1], SIGN_MIRA_256_PARAM_R, &random[0]);
  }

  // Recompute [x]_N
  gf16_mat_from_string(shares->x[SIGN_MIRA_256_PARAM_N_MPC - 1], 1, SIGN_MIRA_256_PARAM_K, &state[0]);

  // Recompute [beta]_N
  gfqm_vec_from_string(shares->beta[SIGN_MIRA_256_PARAM_N_MPC - 1], SIGN_MIRA_256_PARAM_R, &state[SIGN_MIRA_256_VEC_K_BYTES]);

  // Recompute [c]_N
  gfqm_from_string(shares->c[SIGN_MIRA_256_PARAM_N_MPC - 1], &state[SIGN_MIRA_256_VEC_K_BYTES + SIGN_MIRA_256_VEC_R_BYTES]);

  memset(random, 0, SIGN_MIRA_256_VEC_R_BYTES);
}



/**
* \fn void sign_mira_recompute_main_shares(sign_mira_main_shares_t *main_shares, const sign_mira_shares_t *shares, uint8_t second_challenge)
*
* \brief This function generates the main main_shares from the leaves main_shares as required in the signature scheme (when verifying)
*
* \param[out] challenge sign_mira_main_shares_t representation of main_shares
* \param[in] shares sign_mira_shares_t representation of shares
* \param[in] second_challenge Integer concerning the second challenge
*/
void sign_mira_256_recompute_main_shares(sign_mira_256_main_shares_t *main_shares, const sign_mira_256_shares_t *shares, uint8_t second_challenge) {

  uint8_t bit = 0x1;
  for(size_t i = 0; i < SIGN_MIRA_256_PARAM_D; i++) {
    for(size_t j = 0; j < SIGN_MIRA_256_PARAM_N_MPC; j++) {
      if (j != second_challenge) {
        if ((j & bit) == 0) {
          gf16_mat_add(main_shares->x[i], main_shares->x[i], shares->x[j], 1, SIGN_MIRA_256_PARAM_K);
          gfqm_vec_add(main_shares->beta[i], main_shares->beta[i], shares->beta[j], SIGN_MIRA_256_PARAM_R);
          gfqm_vec_add(main_shares->a[i], main_shares->a[i], shares->a[j], SIGN_MIRA_256_PARAM_R);
          gfqm_add(main_shares->c[i], main_shares->c[i], shares->c[j]);
        }
      }
    }
    bit <<= 1;
  }
}



/**
* \fn void sign_mira_recompute_additional_main_share(gf16_mat x, gfqm_vec beta, gfqm_vec a, gfqm c, const sign_mira_shares_t *shares, uint8_t second_challenge)
*
* \brief This function generates the quasi-complement data concerning the first main share as required in the signature scheme (when verifying)
*        Notice that such a complement does not include the challenge leaf share.
*
* \param[out] x gf16_mat representation of the quasi-complement x concerning the first main share
* \param[out] beta gfqm_vec representation of the quasi-complement beta concerning the first main share
* \param[out] a gfqm_vec representation of the quasi-complement a concerning the first main share
* \param[out] c gfqm representation of the quasi-complement c concerning the first main share
* \param[in] shares sign_mira_256_shares_t representation of shares
* \param[in] second_challenge Integer concerning the second challenge
*/
void sign_mira_256_recompute_additional_main_share(gf16_mat x, gfqm_vec beta, gfqm_vec a, gfqm c, const sign_mira_256_shares_t *shares, uint8_t second_challenge) {

  gf16_mat_set_zero(x, 1, SIGN_MIRA_256_PARAM_K);
  gfqm_vec_set_zero(beta, SIGN_MIRA_256_PARAM_R);
  gfqm_vec_set_zero(a, SIGN_MIRA_256_PARAM_R);
  gfqm_set_zero(c);

  for(size_t j = 0; j < SIGN_MIRA_256_PARAM_N_MPC; j++) {
    if (j != second_challenge) {
      if ((j & 0x1) != 0) {
        gf16_mat_add(x, x, shares->x[j], 1, SIGN_MIRA_256_PARAM_K);
        gfqm_vec_add(beta, beta, shares->beta[j], SIGN_MIRA_256_PARAM_R);
        gfqm_vec_add(a, a, shares->a[j], SIGN_MIRA_256_PARAM_R);
        gfqm_add(c, c, shares->c[j]);
      }
    }
  }
}



/**
* \fn void sign_mira_add_alpha_ch2(sign_mira_response1_t *response, const uint8_t *state, uint8_t second_challenge)
*
* \brief This function updates the response according to the signature scheme (when verifying)
*
* \param[out] response sign_mira_response1_t representation of response
* \param[in] state String containing the public data determined by the used shares
* \param[in] second_challenge Integer concerning the second challenge
*/
void sign_mira_256_add_alpha_ch2(sign_mira_256_response1_t *response, const uint8_t *state, uint8_t second_challenge) {

  gfqm_vec_from_string(response->alpha_, SIGN_MIRA_256_PARAM_R, state);

  uint8_t bit = 0x1;
  for(size_t i = 0; i < SIGN_MIRA_256_PARAM_D; i++) {
    if((second_challenge & bit) == 0) {
      gfqm_vec_add(response->alpha[i], response->alpha[i], response->alpha_, SIGN_MIRA_256_PARAM_R);
    }
    bit <<= 1;
  }
}



/**
* \fn void sign_mira_recompute_response1(sign_mira_response1_t *response, const sign_mira_main_shares_t *shares, const sign_mira_challenge1_t *challenge, const gf16_mat x, const gfqm_vec beta, const gfqm_vec a, const gfqm c, const gf16_mat M0, const gf16_mat* Mi, uint8_t second_challenge)
*
* \brief This function computes the first response used in the signature scheme
*
* \param[out] response sign_mira_response1_t Representation of the response
* \param[in] shares sign_mira_shares_t Representation of the shares
* \param[in] challenge sign_mira_challenge1_t Representation of the challenge
* \param[in] x gf16_mat representation of the quasi-complement x concerning the first main share
* \param[in] beta gfqm_vec representation of the quasi-complement x2 concerning the first main share
* \param[in] a gfqm_vec representation of the quasi-complement a concerning the first main share
* \param[in] c gfqm representation of the quasi-complement c concerning the first main share
* \param[in] M0 gf16_mat Matrix M0
* \param[in] Mi gf16_mat* Matrices Mi
* \param[in] second_challenge Integer concerning the second challenge
*/
void sign_mira_256_recompute_response1(sign_mira_256_response1_t *response, const sign_mira_256_main_shares_t *shares, const sign_mira_256_challenge1_t *challenge, const gf16_mat x, const gfqm_vec beta, const gfqm_vec a, const gfqm c, const gf16_mat M0, const gf16_mat* Mi, uint8_t second_challenge) {

  gfqm_vec alpha, w, beta_c, beta_t, aux;

  gfqm z[SIGN_MIRA_256_PARAM_D];
  gfqm z_c, c_c, c_t;
  gfqm gamma_e;

  gf16_mat Ei, tmp;
  gf16_mat x_;
  gfqm_vec Ei_vec;
  gfqm_vec Ei_vec_pow_k;

  gfqm_vec_init(&alpha, SIGN_MIRA_256_PARAM_R);
  gfqm_vec_init(&aux, SIGN_MIRA_256_PARAM_R);
  gfqm_vec_init(&w, SIGN_MIRA_256_PARAM_R);
  gfqm_vec_init(&beta_c, SIGN_MIRA_256_PARAM_R);
  gfqm_vec_init(&beta_t, SIGN_MIRA_256_PARAM_R);

  gf16_mat_init(&Ei, SIGN_MIRA_256_PARAM_M, SIGN_MIRA_256_PARAM_N);
  gf16_mat_init(&tmp, SIGN_MIRA_256_PARAM_M, SIGN_MIRA_256_PARAM_N);
  gf16_mat_init(&x_, 1, SIGN_MIRA_256_PARAM_K);
  gfqm_vec_init(&Ei_vec, SIGN_MIRA_256_PARAM_N);
  gfqm_vec_init(&Ei_vec_pow_k, SIGN_MIRA_256_PARAM_N);

  // Compute [z], [beta] and [c] for everything except missing leaf (z_c, beta_c, c_c)
  // Compute [E]
  if(second_challenge == 0) gf16_mat_set_zero(Ei, SIGN_MIRA_256_PARAM_M, SIGN_MIRA_256_PARAM_N);
  else gf16_mat_set(Ei, M0, SIGN_MIRA_256_PARAM_M, SIGN_MIRA_256_PARAM_N);
  gf16_mat_add(x_, x, shares->x[0], 1, SIGN_MIRA_256_PARAM_K);

  for(int j=0 ; j<SIGN_MIRA_256_PARAM_K ; j++) {
      gf16_mat_scalar_mul(tmp, Mi[j], x_[j], SIGN_MIRA_256_PARAM_M, SIGN_MIRA_256_PARAM_N);
      gf16_mat_add(Ei, Ei, tmp, SIGN_MIRA_256_PARAM_M, SIGN_MIRA_256_PARAM_N);
  }

  //"Transpose" into elements of Fqm
  for(int j=0 ; j<SIGN_MIRA_256_PARAM_N ; j++) {
      for(int k=0 ; k<SIGN_MIRA_256_PARAM_M ; k++) {
          Ei_vec[j][k] = Ei[k*SIGN_MIRA_256_PARAM_N + j];
      }
      gfqm_set(Ei_vec_pow_k[j], Ei_vec[j]);
  }

  // Compute [w] and [z]
  for(size_t k=0 ; k<SIGN_MIRA_256_PARAM_R ; k++) {
      gfqm_set_zero(w[k]);
      //w[k] = <gamma . (e_ji^q^k)>
      for(int j=0 ; j<SIGN_MIRA_256_PARAM_N ; j++) {
          gfqm_mul(gamma_e, Ei_vec_pow_k[j], challenge->gamma[j]);
          gfqm_add(w[k], w[k], gamma_e);
      }
      for(size_t j = 0; j < SIGN_MIRA_256_PARAM_N; j++) {
          gfqm_pow16(Ei_vec_pow_k[j], Ei_vec_pow_k[j]);
      }
  }
  //Compute [z]
  gfqm_set_zero(z_c);
  //z = <gamma . (e_ji^q^r)>
  for(int j=0 ; j<SIGN_MIRA_256_PARAM_N ; j++) {
      gfqm_mul(gamma_e, Ei_vec_pow_k[j], challenge->gamma[j]);
      gfqm_add(z_c, z_c, gamma_e);
  }

  gfqm_vec_add(beta_c, beta, shares->beta[0], SIGN_MIRA_256_PARAM_R);
  gfqm_add(c_c, c, shares->c[0]);

  // Compute share [alpha] for main shares
  uint8_t bit = 0x1;
  for(size_t i = 0; i < SIGN_MIRA_256_PARAM_D; i++) {
    // Compute [E]
    if(second_challenge == 0) gf16_mat_set_zero(Ei, SIGN_MIRA_256_PARAM_M, SIGN_MIRA_256_PARAM_N);
    else gf16_mat_set(Ei, M0, SIGN_MIRA_256_PARAM_M, SIGN_MIRA_256_PARAM_N);

    for(int j=0 ; j<SIGN_MIRA_256_PARAM_K ; j++) {
        gf16_mat_scalar_mul(tmp, Mi[j], shares->x[i][j], SIGN_MIRA_256_PARAM_M, SIGN_MIRA_256_PARAM_N);
        gf16_mat_add(Ei, Ei, tmp, SIGN_MIRA_256_PARAM_M, SIGN_MIRA_256_PARAM_N);
    }

    //"Transpose" into elements of Fqm
    for(int j=0 ; j<SIGN_MIRA_256_PARAM_N ; j++) {
        for(int k=0 ; k<SIGN_MIRA_256_PARAM_M ; k++) {
            Ei_vec[j][k] = Ei[k*SIGN_MIRA_256_PARAM_N + j];
        }
        gfqm_set(Ei_vec_pow_k[j], Ei_vec[j]);
    }

    // Compute [w] and [z]
    for(size_t k=0 ; k<SIGN_MIRA_256_PARAM_R ; k++) {
        gfqm_set_zero(w[k]);
        gfqm gamma_e;
        //w[k] = <gamma . (e_ji^q^k)>
        for(int j=0 ; j<SIGN_MIRA_256_PARAM_N ; j++) {
            gfqm_mul(gamma_e, Ei_vec_pow_k[j], challenge->gamma[j]);
            gfqm_add(w[k], w[k], gamma_e);
        }
        for(size_t j = 0; j < SIGN_MIRA_256_PARAM_N; j++) {
            gfqm_pow16(Ei_vec_pow_k[j], Ei_vec_pow_k[j]);
        }
    }
    //Compute [z]
    gfqm_set_zero(z[i]);
    gfqm gamma_e;
    //z = <gamma . (e_ji^q^r - e_ji)>
    for(int j=0 ; j<SIGN_MIRA_256_PARAM_N ; j++) {
        gfqm_mul(gamma_e, Ei_vec_pow_k[j], challenge->gamma[j]);
        gfqm_add(z[i], z[i], gamma_e);
    }

    // Compute [alpha]
    gfqm_vec_scalar_mul(aux, w, challenge->epsilon, SIGN_MIRA_256_PARAM_R);
    gfqm_vec_add(aux, aux, shares->a[i], SIGN_MIRA_256_PARAM_R);
    gfqm_vec_add(response->alpha[i], response->alpha[i], aux, SIGN_MIRA_256_PARAM_R);

    bit <<= 1;
  }

    // Compute plain alpha
    gfqm_vec_set(alpha, response->alpha_, SIGN_MIRA_256_PARAM_R);
    sign_mira_256_reconstruct_alpha(response, M0, Mi, x, a, challenge, 0);

    if ((second_challenge & 0x1) != 0) {
      gfqm_vec_add(response->alpha_, response->alpha_, alpha, SIGN_MIRA_256_PARAM_R);
    }
    gfqm_vec_add(response->alpha_, response->alpha_, response->alpha[0], SIGN_MIRA_256_PARAM_R);


    // Compute [v] for main shares
    bit = 0x1;
    for(size_t i = 0; i < SIGN_MIRA_256_PARAM_D; i++) {
      if ((second_challenge & bit) == 0) {
        gfqm_add(z[i], z[i], z_c);
        gfqm_vec_add(beta_t, beta_c, shares->beta[i], SIGN_MIRA_256_PARAM_R);
        gfqm_add(c_t, c_c, shares->c[i]);
      }
      else {
        gfqm_vec_set(beta_t, shares->beta[i], SIGN_MIRA_256_PARAM_R);
        gfqm_set(c_t, shares->c[i]);
      }

      gfqm_mul(response->v[i], challenge->epsilon, z[i]);
      gfqm_vec_inner_product(gamma_e, response->alpha_, beta_t, SIGN_MIRA_256_PARAM_R);
      gfqm_add(response->v[i], response->v[i], gamma_e);
      gfqm_add(response->v[i], response->v[i], c_t);
      gfqm_set_zero(z[i]);
      bit <<= 1;
    }

    gfqm_set_zero(z_c);
    gfqm_set_zero(c_c);
    gfqm_set_zero(c_t);

    gfqm_vec_clear(alpha);
    gfqm_vec_clear(aux);
    gfqm_vec_clear(w);
    gfqm_vec_clear(beta_c);
    gfqm_vec_clear(beta_t);

    gf16_mat_clear(Ei);
    gf16_mat_clear(tmp);
    gf16_mat_clear(x_);
    gfqm_vec_clear(Ei_vec);
    gfqm_vec_clear(Ei_vec_pow_k);
}
