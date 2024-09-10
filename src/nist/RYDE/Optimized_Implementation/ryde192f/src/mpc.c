/**
 * @file ryde_192f_mpc.c
 * @brief Implementation of mpc.h
 */

#include "hash_fips202.h"
#include "seedexpander_shake.h"
#include "mpc.h"
#include "tree.h"



/**
* \fn void ryde_192f_init_leaf_shares(ryde_192f_leaf_shares_t *leaf_shares)
* \brief This function initializes the MPC shares as required in the signature scheme
*
* \param[in,out] leaf_shares ryde_192f_leaf_shares_t Representation of the leaf shares
*/
void ryde_192f_init_leaf_shares(ryde_192f_leaf_shares_t *leaf_shares) {
    rbc_37_vec_init(&(leaf_shares->u_), RYDE_192F_PARAM_W - 1);
    for(size_t i = 0; i < RYDE_192F_PARAM_N_MPC; i++ ) {
        rbc_37_vec_init(&(leaf_shares->x1[i]), RYDE_192F_PARAM_N - RYDE_192F_PARAM_K);
        rbc_37_vec_init(&(leaf_shares->x2[i]), RYDE_192F_PARAM_K);
        rbc_37_vec_init(&(leaf_shares->a[i]), RYDE_192F_PARAM_W - 1);
        rbc_37_vec_init(&(leaf_shares->u[i]), RYDE_192F_PARAM_W - 1);
        rbc_37_elt_set_zero(leaf_shares->c[i]);
    }
}



/**
* \fn void ryde_192f_init_main_shares(ryde_192f_main_shares_t *main_shares)
* \brief This function initializes the main shares as required in the signature scheme (hypercube)
*
* \param[in,out] main_shares ryde_192f_main_shares_t Representation of the main shares
*/
void ryde_192f_init_main_shares(ryde_192f_main_shares_t *main_shares) {
    for(size_t i = 0; i < RYDE_192F_PARAM_D; i++ ) {
        rbc_37_vec_init(&(main_shares->x1[i]), RYDE_192F_PARAM_N - RYDE_192F_PARAM_K);
        rbc_37_vec_init(&(main_shares->x2[i]), RYDE_192F_PARAM_K);
        rbc_37_vec_init(&(main_shares->a[i]), RYDE_192F_PARAM_W - 1);
        rbc_37_vec_init(&(main_shares->u[i]), RYDE_192F_PARAM_W - 1);
        rbc_37_elt_set_zero(main_shares->c[i]);
    }
}



/**
* \fn void ryde_192f_init_challenge1(ryde_192f_challenge1_t *challenge)
* \brief This function initializes the first challenge as required in the signature scheme
*
* \param[in,out] challenge ryde_192f_challenge1_t Representation of the challenge
*/
void ryde_192f_init_challenge1(ryde_192f_challenge1_t *challenge) {
    rbc_37_vec_init(&(challenge->gamma), RYDE_192F_PARAM_N);
    rbc_37_elt_set_zero(challenge->epsilon);
}



/**
* \fn void ryde_192f_init_response1(ryde_192f_response1_t *response)
* \brief This function initializes the response as required in the signature scheme
*
* \param[in,out] response ryde_192f_response1_t Representation of the response
*/
void ryde_192f_init_response1(ryde_192f_response1_t *response) {
    rbc_37_vec_init(&(response->alpha_), RYDE_192F_PARAM_W - 1);
    for(size_t i = 0; i < RYDE_192F_PARAM_D; i++ ) {
        rbc_37_vec_init(&(response->alpha[i]), RYDE_192F_PARAM_W - 1);
        rbc_37_elt_set_zero(response->v[i]);
    }
}



/**
* \fn void ryde_192f_clear_leaf_shares(ryde_192f_leaf_shares_t *leaf_shares)
* \brief This function clears the MPC shares as required in the signature scheme
*
* \param[in,out] leaf_shares ryde_192f_leaf_shares_t Representation of the leaf shares
*/
void ryde_192f_clear_leaf_shares(ryde_192f_leaf_shares_t *leaf_shares) {
    rbc_37_vec_clear(leaf_shares->u_);
    for(size_t i = 0; i < RYDE_192F_PARAM_N_MPC; i++ ) {
        rbc_37_vec_clear(leaf_shares->x1[i]);
        rbc_37_vec_clear(leaf_shares->x2[i]);
        rbc_37_vec_clear(leaf_shares->a[i]);
        rbc_37_vec_clear(leaf_shares->u[i]);
        rbc_37_elt_set_zero(leaf_shares->c[i]);
    }
}



/**
* \fn void ryde_192f_clear_main_shares(ryde_192f_main_shares_t *main_shares)
* \brief This function clears the main shares as required in the signature scheme (hypercube)
*
* \param[in,out] main_shares ryde_192f_main_shares_t Representation of the main_shares
*/
void ryde_192f_clear_main_shares(ryde_192f_main_shares_t *main_shares) {
    for(size_t i = 0; i < RYDE_192F_PARAM_D; i++ ) {
        rbc_37_vec_clear(main_shares->x1[i]);
        rbc_37_vec_clear(main_shares->x2[i]);
        rbc_37_vec_clear(main_shares->a[i]);
        rbc_37_vec_clear(main_shares->u[i]);
        rbc_37_elt_set_zero(main_shares->c[i]);
    }
}



/**
* \fn void ryde_192f_clear_challenge1(ryde_192f_challenge1_t *challenge)
* \brief This function clears the first challenge as required in the signature scheme
*
* \param[in,out] challenge ryde_192f_challenge1_t Representation of the challenge1
*/
void ryde_192f_clear_challenge1(ryde_192f_challenge1_t *challenge) {
    rbc_37_vec_clear(challenge->gamma);
    rbc_37_elt_set_zero(challenge->epsilon);
}



/**
* \fn void ryde_192f_clear_response1(ryde_192f_response1_t *response)
* \brief This function clears the responses as required in the signature scheme
*
* \param[in,out] response ryde_192f_response1_t Representation of the responses
*/
void ryde_192f_clear_response1(ryde_192f_response1_t *response) {
    rbc_37_vec_clear(response->alpha_);
    for(size_t i = 0; i < RYDE_192F_PARAM_D; i++ ) {
        rbc_37_vec_clear(response->alpha[i]);
        rbc_37_elt_set_zero(response->v[i]);
    }
}




/**
* \fn void ryde_192f_compute_leaf_shares(ryde_192f_leaf_shares_t *leaf_shares, const rbc_37_vec y, const rbc_37_mat H, const rbc_37_vec x2, const rbc_37_vec a, const uint8_t *theta_i, const uint8_t *salt)
*
* \brief This function computes the shares of the signature scheme
*
* The polynomial a determines the annihilator polynomial such that x = (x1, x2) is a root of a.
*
* \param[out] leaf_shares ryde_192f_leaf_shares_t Representation of the leaf shares
* \param[in] y rbc_37_vec Syndrome y
* \param[in] H rbc_37_mat Parity-check matrix H
* \param[in] x2 rbc_37_vec Secret x2
* \param[in] a rbc_37_qpoly Annihilator polynomial cancelling (x1, x2)
* \param[in] theta_i uint8_t* Input seeds
* \param[in] salt uint8_t* Salt
*/
#ifdef SHAKE_TIMES4
void ryde_192f_compute_leaf_shares(ryde_192f_leaf_shares_t *leaf_shares, const rbc_37_vec y, const rbc_37_mat H, const rbc_37_vec x2, const rbc_37_vec a, const uint8_t *theta_i, const uint8_t *salt) {

  rbc_37_vec beta;
  rbc_37_vec_init(&beta, RYDE_192F_PARAM_W - 1);
  for(size_t i = 1; i < RYDE_192F_PARAM_W; i++) {
    rbc_37_elt_set(beta[i - 1], a[i]);
  }

  uint8_t random[(RYDE_192F_PARAM_W - 1) * RYDE_192F_PARAM_M_BYTES] = {0};
  seedexpander_shake_t seedexpander;

  uint8_t random0[(2 * RYDE_192F_PARAM_W - 1 + RYDE_192F_PARAM_K) * RYDE_192F_PARAM_M_BYTES] = {0};
  uint8_t random1[(2 * RYDE_192F_PARAM_W - 1 + RYDE_192F_PARAM_K) * RYDE_192F_PARAM_M_BYTES] = {0};
  uint8_t random2[(2 * RYDE_192F_PARAM_W - 1 + RYDE_192F_PARAM_K) * RYDE_192F_PARAM_M_BYTES] = {0};
  uint8_t random3[(2 * RYDE_192F_PARAM_W - 1 + RYDE_192F_PARAM_K) * RYDE_192F_PARAM_M_BYTES] = {0};
  uint8_t *random_x4[] = {random0, random1, random2, random3};
  seedexpander_shake_x4_t seedexpander_x4;
  const uint8_t *salt_x4[] = {salt, salt, salt, salt};

  rbc_37_elt tmp;

  rbc_37_vec_set_zero( leaf_shares->u_, RYDE_192F_PARAM_W - 1);
  rbc_37_vec_set_zero( leaf_shares->x1[RYDE_192F_PARAM_N_MPC - 1], RYDE_192F_PARAM_N - RYDE_192F_PARAM_K);
  rbc_37_vec_set_zero( leaf_shares->x2[RYDE_192F_PARAM_N_MPC - 1], RYDE_192F_PARAM_K);
  rbc_37_vec_set_zero( leaf_shares->a[RYDE_192F_PARAM_N_MPC - 1], RYDE_192F_PARAM_W - 1);
  rbc_37_elt_set_zero( leaf_shares->c[RYDE_192F_PARAM_N_MPC - 1]);

  // Compute the first N-1 shares
  for(size_t i = 0; i < RYDE_192F_PARAM_N_MPC; i+=4) {
    // Sample randomness
    const uint8_t *seed_x4[] = {&theta_i[RYDE_192F_SECURITY_BYTES * i], &theta_i[RYDE_192F_SECURITY_BYTES * (i + 1)],
                                &theta_i[RYDE_192F_SECURITY_BYTES * (i + 2)], &theta_i[RYDE_192F_SECURITY_BYTES * (i + 3)]};

    seedexpander_shake_x4_init(&seedexpander_x4, seed_x4, RYDE_192F_SECURITY_BYTES, salt_x4, 2 * RYDE_192F_SECURITY_BYTES);
    seedexpander_shake_x4_get_bytes(&seedexpander_x4, random_x4, (2 * RYDE_192F_PARAM_W - 1 + RYDE_192F_PARAM_K) * RYDE_192F_PARAM_M_BYTES);

    for(size_t j = 0; j < 4; j++) {
      if ((i + j) < (RYDE_192F_PARAM_N_MPC - 1)) {
        rbc_37_vec_set_zero( leaf_shares->a[i + j], RYDE_192F_PARAM_W - 1);
        rbc_37_elt_set_zero( leaf_shares->c[i + j]);

        // Compute share [u]_i
        rbc_37_vec_from_bytes( leaf_shares->u[i + j], RYDE_192F_PARAM_W - 1, &random_x4[j][0]);
        rbc_37_vec_add( leaf_shares->u_,  leaf_shares->u_,  leaf_shares->u[i + j], RYDE_192F_PARAM_W - 1);

        // Compute share [x1]_i and [x2]_i
        rbc_37_vec_from_bytes( leaf_shares->x2[i + j], RYDE_192F_PARAM_K, &random_x4[j][(RYDE_192F_PARAM_W - 1) * RYDE_192F_PARAM_M_BYTES]);
        rbc_37_mat_vec_mul( leaf_shares->x1[i + j], H,  leaf_shares->x2[i + j], RYDE_192F_PARAM_N - RYDE_192F_PARAM_K, RYDE_192F_PARAM_K);
        rbc_37_vec_add( leaf_shares->x1[i + j], y,  leaf_shares->x1[i + j], RYDE_192F_PARAM_N - RYDE_192F_PARAM_K);
        rbc_37_vec_add( leaf_shares->x2[RYDE_192F_PARAM_N_MPC - 1],  leaf_shares->x2[RYDE_192F_PARAM_N_MPC - 1],  leaf_shares->x2[i + j], RYDE_192F_PARAM_K);

        // Compute share [a]_i
        rbc_37_vec_from_bytes( leaf_shares->a[i + j], RYDE_192F_PARAM_W - 1, &random_x4[j][(RYDE_192F_PARAM_W - 1 + RYDE_192F_PARAM_K) * RYDE_192F_PARAM_M_BYTES]);
        rbc_37_vec_add( leaf_shares->a[RYDE_192F_PARAM_N_MPC - 1],  leaf_shares->a[RYDE_192F_PARAM_N_MPC - 1],  leaf_shares->a[i + j], RYDE_192F_PARAM_W - 1);

        // Compute share [c]_i
        random_x4[j][(2 * RYDE_192F_PARAM_W - 1 + RYDE_192F_PARAM_K) * RYDE_192F_PARAM_M_BYTES - 1] &= RYDE_192F_PARAM_M_MASK;
        memcpy( leaf_shares->c[i + j], &random_x4[j][(2 * RYDE_192F_PARAM_W - 1 + RYDE_192F_PARAM_K - 1) * RYDE_192F_PARAM_M_BYTES], RYDE_192F_PARAM_M_BYTES);
        rbc_37_elt_add( leaf_shares->c[RYDE_192F_PARAM_N_MPC - 1],  leaf_shares->c[RYDE_192F_PARAM_N_MPC - 1],  leaf_shares->c[i + j]);
      }
    }
  }

  // Sample randomness for share N
  seedexpander_shake_init(&seedexpander, &theta_i[RYDE_192F_SECURITY_BYTES * (RYDE_192F_PARAM_N_MPC - 1)], RYDE_192F_SECURITY_BYTES, salt, 2 * RYDE_192F_SECURITY_BYTES);
  seedexpander_shake_get_bytes(&seedexpander, random, (RYDE_192F_PARAM_W - 1) * RYDE_192F_PARAM_M_BYTES);

  // Compute u and share [u]_N
  rbc_37_vec_from_bytes( leaf_shares->u[RYDE_192F_PARAM_N_MPC - 1], RYDE_192F_PARAM_W - 1, &random[0]);
  rbc_37_vec_add( leaf_shares->u_,  leaf_shares->u_,  leaf_shares->u[RYDE_192F_PARAM_N_MPC - 1], RYDE_192F_PARAM_W - 1);

  // Computing c and share [c]_N
  rbc_37_vec_inner_product(tmp,  leaf_shares->u_, beta, RYDE_192F_PARAM_W - 1);
  rbc_37_elt_add( leaf_shares->c[RYDE_192F_PARAM_N_MPC - 1],  leaf_shares->c[RYDE_192F_PARAM_N_MPC - 1], tmp);

  // Compute shares [a]_N, [x1]_N and [x2]_N
  rbc_37_vec_add( leaf_shares->a[RYDE_192F_PARAM_N_MPC - 1],  leaf_shares->a[RYDE_192F_PARAM_N_MPC - 1], beta, RYDE_192F_PARAM_W - 1);
  rbc_37_vec_add( leaf_shares->x2[RYDE_192F_PARAM_N_MPC - 1],  leaf_shares->x2[RYDE_192F_PARAM_N_MPC - 1], x2, RYDE_192F_PARAM_K);
  rbc_37_mat_vec_mul( leaf_shares->x1[RYDE_192F_PARAM_N_MPC - 1], H,  leaf_shares->x2[RYDE_192F_PARAM_N_MPC - 1], RYDE_192F_PARAM_N - RYDE_192F_PARAM_K, RYDE_192F_PARAM_K);

  memset(random, 0, (RYDE_192F_PARAM_W - 1) * RYDE_192F_PARAM_M_BYTES);
  rbc_37_vec_clear(beta);
}
#else
void ryde_192f_compute_leaf_shares(ryde_192f_leaf_shares_t *leaf_shares, const rbc_37_vec y, const rbc_37_mat H, const rbc_37_vec x2, const rbc_37_vec a, const uint8_t *theta_i, const uint8_t *salt) {

  rbc_37_vec beta;
  rbc_37_vec_init(&beta, RYDE_192F_PARAM_W - 1);
  for(size_t i = 1; i < RYDE_192F_PARAM_W; i++) {
    rbc_37_elt_set(beta[i - 1], a[i]);
  }

  uint8_t random[(2 * RYDE_192F_PARAM_W - 1 + RYDE_192F_PARAM_K) * RYDE_192F_PARAM_M_BYTES] = {0};
  seedexpander_shake_t seedexpander;

  rbc_37_elt tmp;

  rbc_37_vec_set_zero( leaf_shares->u_, RYDE_192F_PARAM_W - 1);
  rbc_37_vec_set_zero( leaf_shares->x1[RYDE_192F_PARAM_N_MPC - 1], RYDE_192F_PARAM_N - RYDE_192F_PARAM_K);
  rbc_37_vec_set_zero( leaf_shares->x2[RYDE_192F_PARAM_N_MPC - 1], RYDE_192F_PARAM_K);
  rbc_37_vec_set_zero( leaf_shares->a[RYDE_192F_PARAM_N_MPC - 1], RYDE_192F_PARAM_W - 1);
  rbc_37_elt_set_zero( leaf_shares->c[RYDE_192F_PARAM_N_MPC - 1]);

  // Compute the first N-1 shares
  for(size_t i = 0; i < (RYDE_192F_PARAM_N_MPC - 1); i++) {
    rbc_37_vec_set_zero( leaf_shares->a[i], RYDE_192F_PARAM_W - 1);
    rbc_37_elt_set_zero( leaf_shares->c[i]);

    // Sample randomness
    seedexpander_shake_init(&seedexpander, &theta_i[RYDE_192F_SECURITY_BYTES * i], RYDE_192F_SECURITY_BYTES, salt, 2 * RYDE_192F_SECURITY_BYTES);
    seedexpander_shake_get_bytes(&seedexpander, random, (2 * RYDE_192F_PARAM_W - 1 + RYDE_192F_PARAM_K) * RYDE_192F_PARAM_M_BYTES);

    // Compute share [u]_i
    rbc_37_vec_from_bytes( leaf_shares->u[i], RYDE_192F_PARAM_W - 1, &random[0]);
    rbc_37_vec_add( leaf_shares->u_,  leaf_shares->u_,  leaf_shares->u[i], RYDE_192F_PARAM_W - 1);

    // Compute share [x1]_i and [x2]_i
    rbc_37_vec_from_bytes( leaf_shares->x2[i], RYDE_192F_PARAM_K, &random[(RYDE_192F_PARAM_W - 1) * RYDE_192F_PARAM_M_BYTES]);
    rbc_37_mat_vec_mul( leaf_shares->x1[i], H,  leaf_shares->x2[i], RYDE_192F_PARAM_N - RYDE_192F_PARAM_K, RYDE_192F_PARAM_K);
    rbc_37_vec_add( leaf_shares->x1[i], y,  leaf_shares->x1[i], RYDE_192F_PARAM_N - RYDE_192F_PARAM_K);
    rbc_37_vec_add( leaf_shares->x2[RYDE_192F_PARAM_N_MPC - 1],  leaf_shares->x2[RYDE_192F_PARAM_N_MPC - 1],  leaf_shares->x2[i], RYDE_192F_PARAM_K);

    // Compute share [a]_i
    rbc_37_vec_from_bytes( leaf_shares->a[i], RYDE_192F_PARAM_W - 1, &random[(RYDE_192F_PARAM_W - 1 + RYDE_192F_PARAM_K) * RYDE_192F_PARAM_M_BYTES]);
    rbc_37_vec_add( leaf_shares->a[RYDE_192F_PARAM_N_MPC - 1],  leaf_shares->a[RYDE_192F_PARAM_N_MPC - 1],  leaf_shares->a[i], RYDE_192F_PARAM_W - 1);

    // Compute share [c]_i
    random[(2 * RYDE_192F_PARAM_W - 1 + RYDE_192F_PARAM_K) * RYDE_192F_PARAM_M_BYTES - 1] &= RYDE_192F_PARAM_M_MASK;
    memcpy( leaf_shares->c[i], &random[(2 * RYDE_192F_PARAM_W - 1 + RYDE_192F_PARAM_K - 1) * RYDE_192F_PARAM_M_BYTES], RYDE_192F_PARAM_M_BYTES);
    rbc_37_elt_add( leaf_shares->c[RYDE_192F_PARAM_N_MPC - 1],  leaf_shares->c[RYDE_192F_PARAM_N_MPC - 1],  leaf_shares->c[i]);
  }

  // Sample randomness for share N
  seedexpander_shake_init(&seedexpander, &theta_i[RYDE_192F_SECURITY_BYTES * (RYDE_192F_PARAM_N_MPC - 1)], RYDE_192F_SECURITY_BYTES, salt, 2 * RYDE_192F_SECURITY_BYTES);
  seedexpander_shake_get_bytes(&seedexpander, random, (RYDE_192F_PARAM_W - 1) * RYDE_192F_PARAM_M_BYTES);

  // Compute u and share [u]_N
  rbc_37_vec_from_bytes( leaf_shares->u[RYDE_192F_PARAM_N_MPC - 1], RYDE_192F_PARAM_W - 1, &random[0]);
  rbc_37_vec_add( leaf_shares->u_,  leaf_shares->u_,  leaf_shares->u[RYDE_192F_PARAM_N_MPC - 1], RYDE_192F_PARAM_W - 1);

  // Computing c and share [c]_N
  rbc_37_vec_inner_product(tmp,  leaf_shares->u_, beta, RYDE_192F_PARAM_W - 1);
  rbc_37_elt_add( leaf_shares->c[RYDE_192F_PARAM_N_MPC - 1],  leaf_shares->c[RYDE_192F_PARAM_N_MPC - 1], tmp);

  // Compute shares [a]_N, [x1]_N and [x2]_N
  rbc_37_vec_add( leaf_shares->a[RYDE_192F_PARAM_N_MPC - 1],  leaf_shares->a[RYDE_192F_PARAM_N_MPC - 1], beta, RYDE_192F_PARAM_W - 1);
  rbc_37_vec_add( leaf_shares->x2[RYDE_192F_PARAM_N_MPC - 1],  leaf_shares->x2[RYDE_192F_PARAM_N_MPC - 1], x2, RYDE_192F_PARAM_K);
  rbc_37_mat_vec_mul( leaf_shares->x1[RYDE_192F_PARAM_N_MPC - 1], H,  leaf_shares->x2[RYDE_192F_PARAM_N_MPC - 1], RYDE_192F_PARAM_N - RYDE_192F_PARAM_K, RYDE_192F_PARAM_K);

  memset(random, 0, (2 * RYDE_192F_PARAM_W - 1 + RYDE_192F_PARAM_K) * RYDE_192F_PARAM_M_BYTES);
  rbc_37_vec_clear(beta);
}
#endif



/**
* \fn void ryde_192f_commit_to_shares(uint8_t *commits, uint8_t e, const uint8_t *salt, const ryde_192f_leaf_shares_t *leaf_shares, const uint8_t *theta_i)
*
* \brief This function computes the commitments to the shares used in the signature scheme.
*
* \param[out] commits Commitments to the shares
* \param[in] e Current iteration of the protocol
* \param[in] salt Salt
* \param[in] leaf_shares ryde_192f_leaf_shares_t Representation of the leaf shares
* \param[in] theta_i Seed used to generate the shares
*/
#ifdef SHAKE_TIMES4
void ryde_192f_commit_to_shares(uint8_t *commits, uint8_t e, const uint8_t *salt, const ryde_192f_leaf_shares_t *leaf_shares, const uint8_t *theta_i) {

  uint8_t domain_separator = DOMAIN_SEPARATOR_COMMITMENT;
  uint8_t state[RYDE_192F_PARAM_STATE_BYTES] = {0};

  uint8_t *domain_separator_x4[] = {&domain_separator, &domain_separator, &domain_separator, &domain_separator};
  const uint8_t *salt_x4[] = {salt, salt, salt, salt};
  uint8_t *e_x4[] = {&e, &e, &e, &e};
  uint8_t index_0 = 0, index_1 = 1, index_2 = 2, index_3 = 3;
  uint8_t *index_x4[] = {&index_0, &index_1, &index_2, &index_3};

  // Compute the first N-1 commitments (last commitment is a dummy computation that is rewritten after the loop)
  for(size_t i = 0; i < RYDE_192F_PARAM_N_MPC; i+=4) {
    const uint8_t *seed_x4[] = {&theta_i[RYDE_192F_SECURITY_BYTES * i], &theta_i[RYDE_192F_SECURITY_BYTES * (i + 1)],
                                &theta_i[RYDE_192F_SECURITY_BYTES * (i + 2)], &theta_i[RYDE_192F_SECURITY_BYTES * (i + 3)]};

    uint8_t *commit_x4[] = {&commits[2 * RYDE_192F_SECURITY_BYTES * i], &commits[2 * RYDE_192F_SECURITY_BYTES * (i + 1)],
                            &commits[2 * RYDE_192F_SECURITY_BYTES * (i + 2)], &commits[2 * RYDE_192F_SECURITY_BYTES * (i + 3)]};

    hash_sha3_x4_ctx ctx;
    hash_sha3_x4_init(&ctx);
    hash_sha3_x4_absorb(&ctx, (const uint8_t **) domain_separator_x4, sizeof(uint8_t));
    hash_sha3_x4_absorb(&ctx, salt_x4, 2 * RYDE_192F_SECURITY_BYTES);
    hash_sha3_x4_absorb(&ctx, (const uint8_t **) e_x4, sizeof(uint8_t));
    hash_sha3_x4_absorb(&ctx, (const uint8_t **) index_x4, sizeof(uint8_t));
    hash_sha3_x4_absorb(&ctx, (const uint8_t **) seed_x4, RYDE_192F_SECURITY_BYTES);
    hash_sha3_x4_finalize(commit_x4, &ctx);

    index_0 += 4;
    index_1 += 4;
    index_2 += 4;
    index_3 += 4;
  }

  // Compute commitment N
  uint8_t i = (uint8_t) ((uint16_t)RYDE_192F_PARAM_N_MPC - 1);

  hash_sha3_ctx ctx;
  hash_sha3_init(&ctx);
  hash_sha3_absorb(&ctx, salt, 2 * RYDE_192F_SECURITY_BYTES);
  hash_sha3_absorb(&ctx, &e, sizeof(uint8_t));
  hash_sha3_absorb(&ctx, &i, sizeof(uint8_t));
  hash_sha3_absorb(&ctx, &theta_i[RYDE_192F_SECURITY_BYTES * i], RYDE_192F_SECURITY_BYTES);

  rbc_37_vec_to_string(&state[0],  leaf_shares->x2[i], RYDE_192F_PARAM_K);
  rbc_37_vec_to_string(&state[RYDE_192F_VEC_K_BYTES],  leaf_shares->a[i], RYDE_192F_PARAM_W - 1);
  memcpy(&state[RYDE_192F_VEC_K_BYTES + RYDE_192F_VEC_W_MINUS_ONE_BYTES],  leaf_shares->c[i], RYDE_192F_PARAM_M_BYTES);

  hash_sha3_absorb(&ctx, state, RYDE_192F_PARAM_STATE_BYTES);
  hash_sha3_absorb(&ctx, &domain_separator, sizeof(uint8_t));
  hash_sha3_finalize(&commits[2 * RYDE_192F_SECURITY_BYTES * i], &ctx);

  memset(state, 0, RYDE_192F_PARAM_STATE_BYTES);
}
#else
void ryde_192f_commit_to_shares(uint8_t *commits, uint8_t e, const uint8_t *salt, const ryde_192f_leaf_shares_t *leaf_shares, const uint8_t *theta_i)  {

  uint8_t domain_separator = DOMAIN_SEPARATOR_COMMITMENT;
  uint8_t state[RYDE_192F_PARAM_STATE_BYTES] = {0};

  // Compute the first N-1 commitments
  for(size_t i = 0; i < (RYDE_192F_PARAM_N_MPC - 1); i++) {
    hash_sha3_ctx ctx;
    hash_sha3_init(&ctx);
    hash_sha3_absorb(&ctx, &domain_separator, sizeof(uint8_t));
    hash_sha3_absorb(&ctx, salt, 2 * RYDE_192F_SECURITY_BYTES);
    hash_sha3_absorb(&ctx, &e, sizeof(uint8_t));
    hash_sha3_absorb(&ctx, (uint8_t *) &i, sizeof(uint8_t));
    hash_sha3_absorb(&ctx, &theta_i[RYDE_192F_SECURITY_BYTES * i], RYDE_192F_SECURITY_BYTES);
    hash_sha3_finalize(&commits[2 * RYDE_192F_SECURITY_BYTES * i], &ctx);
  }

  // Compute commitment N
  uint8_t i = (uint8_t) ((uint16_t) RYDE_192F_PARAM_N_MPC - 1);

  hash_sha3_ctx ctx;
  hash_sha3_init(&ctx);
  hash_sha3_absorb(&ctx, salt, 2 * RYDE_192F_SECURITY_BYTES);
  hash_sha3_absorb(&ctx, &e, sizeof(uint8_t));
  hash_sha3_absorb(&ctx, &i, sizeof(uint8_t));
  hash_sha3_absorb(&ctx, &theta_i[RYDE_192F_SECURITY_BYTES * i], RYDE_192F_SECURITY_BYTES);

  rbc_37_vec_to_string(&state[0],  leaf_shares->x2[i], RYDE_192F_PARAM_K);
  rbc_37_vec_to_string(&state[RYDE_192F_VEC_K_BYTES],  leaf_shares->a[i], RYDE_192F_PARAM_W - 1);
  memcpy(&state[RYDE_192F_VEC_K_BYTES + RYDE_192F_VEC_W_MINUS_ONE_BYTES],  leaf_shares->c[i], RYDE_192F_PARAM_M_BYTES);

  hash_sha3_absorb(&ctx, state, RYDE_192F_PARAM_STATE_BYTES);
  hash_sha3_absorb(&ctx, &domain_separator, sizeof(uint8_t));
  hash_sha3_finalize(&commits[2 * RYDE_192F_SECURITY_BYTES * i], &ctx);

  memset(state, 0, RYDE_192F_PARAM_STATE_BYTES);
}
#endif



/**
* \fn void ryde_192f_compute_main_shares(ryde_192f_main_shares_t *main_shares, const ryde_192f_leaf_shares_t *leaf_shares)
*
* \brief This function generates the main main_shares from the leaves shares
*
* \param[out] main_shares ryde_192f_main_shares_t Representation of the main shares
* \param[in] leaf_shares ryde_192f_leaf_shares_t Representation of the leaf shares
*/
void ryde_192f_compute_main_shares(ryde_192f_main_shares_t *main_shares, const ryde_192f_leaf_shares_t *leaf_shares) {

    uint8_t bit = 0x1;
    for(size_t i = 0; i < RYDE_192F_PARAM_D; i++) {
        for(size_t j = 0; j < RYDE_192F_PARAM_N_MPC; j++) {
            if((j & bit) == 0) {
                rbc_37_vec_add(main_shares->x1[i], main_shares->x1[i], leaf_shares->x1[j], RYDE_192F_PARAM_N - RYDE_192F_PARAM_K);
                rbc_37_vec_add(main_shares->x2[i], main_shares->x2[i], leaf_shares->x2[j], RYDE_192F_PARAM_K);
                rbc_37_vec_add(main_shares->a[i], main_shares->a[i], leaf_shares->a[j], RYDE_192F_PARAM_W - 1);
                rbc_37_vec_add(main_shares->u[i], main_shares->u[i], leaf_shares->u[j], RYDE_192F_PARAM_W - 1);
                rbc_37_elt_add(main_shares->c[i], main_shares->c[i], leaf_shares->c[j]);
            }
        }
        bit <<= 1;
    }
}



/**
* \fn void ryde_192f_compute_challenge1(ryde_192f_challenge1_t *challenge, const uint8_t *seed_input, const uint8_t *salt)
* \brief This function generates challenges from an input seed
*
* \param[out] challenge Array of ryde_192f_challenge1_t Representation of challenge
* \param[in] seed_input String containing the input seed
* \param[in] salt String containing the salt
*/
void ryde_192f_compute_challenge1(ryde_192f_challenge1_t *challenge, const uint8_t *seed_input, const uint8_t *salt) {

  uint8_t random[(RYDE_192F_PARAM_N + 1) * RYDE_192F_PARAM_M_BYTES] = {0};
  seedexpander_shake_t seedexpander;
  seedexpander_shake_init(&seedexpander, seed_input, 2 * RYDE_192F_SECURITY_BYTES, salt, 2 * RYDE_192F_SECURITY_BYTES);

  rbc_37_vec tmp;
  rbc_37_vec_init(&tmp, RYDE_192F_PARAM_N + 1);

  for(size_t e = 0; e < RYDE_192F_PARAM_TAU; e++) {
    seedexpander_shake_get_bytes(&seedexpander, random, (RYDE_192F_PARAM_N + 1) * RYDE_192F_PARAM_M_BYTES);
    rbc_37_vec_from_bytes(tmp, RYDE_192F_PARAM_N + 1, random);
    rbc_37_vec_set(challenge[e].gamma, tmp, RYDE_192F_PARAM_N);
    rbc_37_elt_set(challenge[e].epsilon, tmp[RYDE_192F_PARAM_N]);
  }

  rbc_37_vec_clear(tmp);
  memset(random, 0, (RYDE_192F_PARAM_N + 1) * RYDE_192F_PARAM_M_BYTES);
}



/**
* \fn void ryde_192f_compute_response1(ryde_192f_response1_t *response, const ryde_192f_main_shares_t *main_shares, const ryde_192f_challenge1_t *challenge, const rbc_37_vec x1, const rbc_37_vec x2, const rbc_37_vec u)
*
* \brief This function computes the first response used in the signature scheme (sign)
*
* \param[out] response ryde_192f_response1_t Representation of the response
* \param[in] main_shares ryde_192f_leaf_shares_t Representation of the main shares
* \param[in] challenge ryde_192f_challenge1_t Representation of the challenge
* \param[in] x1 rbc_37_vec Representation of the secret vector x1
* \param[in] x2 rbc_37_vec Representation of the secret vector x2
* \param[in] u rbc_37_vec Representation of the secret vector u
*/
void ryde_192f_compute_response1(ryde_192f_response1_t *response, const ryde_192f_main_shares_t *main_shares, const ryde_192f_challenge1_t *challenge, const rbc_37_vec x1, const rbc_37_vec x2, const rbc_37_vec u) {

    rbc_37_vec x_qj; // x^[j]
    rbc_37_vec x, w;
    rbc_37_vec diff; // x^[j] - x
    rbc_37_vec_init(&x, RYDE_192F_PARAM_N);
    rbc_37_vec_init(&x_qj, RYDE_192F_PARAM_N);
    rbc_37_vec_init(&diff, RYDE_192F_PARAM_N);
    rbc_37_vec_init(&w, RYDE_192F_PARAM_W - 1);
    rbc_37_elt z[RYDE_192F_PARAM_D];

    for(size_t i = 0; i < RYDE_192F_PARAM_D; i++) {
        // Compute [x] = ([x1], [x2])
        for(size_t k = 0; k < (RYDE_192F_PARAM_N - RYDE_192F_PARAM_K); k++) {
            rbc_37_elt_set(x[k], ( main_shares->x1[i])[k]);
            rbc_37_elt_set(x_qj[k], ( main_shares->x1[i])[k]);
        }

        for(size_t k = 0; k < RYDE_192F_PARAM_K; k++) {
            rbc_37_elt_set(x[k + RYDE_192F_PARAM_N - RYDE_192F_PARAM_K], ( main_shares->x2[i])[k]);
            rbc_37_elt_set(x_qj[k + RYDE_192F_PARAM_N - RYDE_192F_PARAM_K], ( main_shares->x2[i])[k]);
        }

        // Compute [w]
        for(size_t k = 0; k < (RYDE_192F_PARAM_W - 1); k++) {
            for(size_t j = 0; j < RYDE_192F_PARAM_N; j++) {
                rbc_37_elt_sqr(x_qj[j], x_qj[j]);
            }
            rbc_37_vec_add(diff, x_qj, x, RYDE_192F_PARAM_N);
            rbc_37_vec_inner_product(w[k], challenge->gamma, diff, RYDE_192F_PARAM_N);
        }

        // Compute [alpha]
        for(size_t j = 0; j < RYDE_192F_PARAM_N; j++) {
            rbc_37_elt_sqr(x_qj[j], x_qj[j]);
        }
        rbc_37_vec_add(diff, x_qj, x, RYDE_192F_PARAM_N);
        rbc_37_vec_inner_product(z[i], challenge->gamma, diff, RYDE_192F_PARAM_N);
        rbc_37_vec_scalar_mul(response->alpha[i], w, challenge->epsilon, RYDE_192F_PARAM_W - 1);
        rbc_37_vec_add(response->alpha[i], response->alpha[i],  main_shares->u[i], RYDE_192F_PARAM_W - 1);
    }

    // Compute alpha
    ryde_192f_reconstruct_alpha(response, x1, x2, u, challenge);

    #ifdef VERBOSE
    printf("ryde_192f_compute_response1:alpha:\t"); rbc_37_vec_print(response->alpha_, RYDE_192F_PARAM_W - 1); printf("\n");
    #endif

    // Compute [z] and [v]
    for(size_t i = 0; i < RYDE_192F_PARAM_D; i++) {
        #ifdef VERBOSE
        printf("ryde_192f_compute_response1:c[%zu]:\t", i); rbc_37_elt_print( main_shares->c[i]); printf("\n");
        printf("ryde_192f_compute_response1:z[%zu]:\t", i); rbc_37_elt_print(z[i]);printf("\n");
        printf("ryde_192f_compute_response1:a[%zu]:\t", i); rbc_37_vec_print( main_shares->a[i], RYDE_192F_PARAM_W - 1);
        #endif

        rbc_37_elt_mul(response->v[i], challenge->epsilon, z[i]);
        rbc_37_vec_inner_product(z[i], response->alpha_,  main_shares->a[i], RYDE_192F_PARAM_W - 1);
        rbc_37_elt_add(response->v[i], response->v[i], z[i]);
        rbc_37_elt_add(response->v[i], response->v[i],  main_shares->c[i]);
        rbc_37_elt_set_zero(z[i]);

        #ifdef VERBOSE
        printf("ryde_192f_compute_response1:v[%zu]:\t", i); rbc_37_elt_print(response->v[i]); printf("\n\n");
        #endif
    }

    rbc_37_vec_clear(w);
    rbc_37_vec_clear(x);
    rbc_37_vec_clear(x_qj);
    rbc_37_vec_clear(diff);
}



/**
* \fn void ryde_192f_reconstruct_alpha(ryde_192f_response1_t *response, const rbc_37_vec x1, const rbc_37_vec x2, const rbc_37_vec u, const ryde_192f_challenge1_t *challenge)
*
* \brief This function updates the response according to the signature scheme (when signing)
*
* \param[out] response ryde_192f_response1_t Representation of response
* \param[in] x1 rbc_37_vec Representation of x1 concerning the second challenge
* \param[in] x2 rbc_37_vec Representation of x2 concerning the second challenge
* \param[in] u rbc_37_vec Representation of u concerning the second challenge
* \param[in] challenge ryde_192f_challenge1_t Representation of challenge
*/
void ryde_192f_reconstruct_alpha(ryde_192f_response1_t *response, const rbc_37_vec x1, const rbc_37_vec x2, const rbc_37_vec u, const ryde_192f_challenge1_t *challenge) {

    rbc_37_vec x_qj; // x^[j]
    rbc_37_vec x, w;
    rbc_37_vec diff; // x^[j] - x
    rbc_37_vec_init(&x, RYDE_192F_PARAM_N);
    rbc_37_vec_init(&x_qj, RYDE_192F_PARAM_N);
    rbc_37_vec_init(&diff, RYDE_192F_PARAM_N);
    rbc_37_vec_init(&w, RYDE_192F_PARAM_W - 1);

    // Compute [x]_ch2 = ([x1]_ch2, [x2]_ch2)
    for(size_t k = 0; k < (RYDE_192F_PARAM_N - RYDE_192F_PARAM_K); k++) {
        rbc_37_elt_set(x[k], x1[k]);
        rbc_37_elt_set(x_qj[k], x1[k]);
    }

    for(size_t k = 0; k < RYDE_192F_PARAM_K; k++) {
        rbc_37_elt_set(x[k + RYDE_192F_PARAM_N - RYDE_192F_PARAM_K], x2[k]);
        rbc_37_elt_set(x_qj[k + RYDE_192F_PARAM_N - RYDE_192F_PARAM_K], x2[k]);
    }

    // Compute [w]_ch2
    for(size_t k = 0; k < (RYDE_192F_PARAM_W - 1); k++) {
        for(size_t j = 0; j < RYDE_192F_PARAM_N; j++) {
            rbc_37_elt_sqr(x_qj[j], x_qj[j]);
        }
        rbc_37_vec_add(diff, x_qj, x, RYDE_192F_PARAM_N);
        rbc_37_vec_inner_product(w[k], challenge->gamma, diff, RYDE_192F_PARAM_N);
    }

    // Compute [alpha]_ch2
    rbc_37_vec_scalar_mul(response->alpha_, w, challenge->epsilon, RYDE_192F_PARAM_W - 1);
    rbc_37_vec_add(response->alpha_, response->alpha_, u, RYDE_192F_PARAM_W - 1);

    rbc_37_vec_clear(w);
    rbc_37_vec_clear(x);
    rbc_37_vec_clear(x_qj);
    rbc_37_vec_clear(diff);
}



/**
* \fn void ryde_192f_reconstruct_share(rbc_37_vec x1, rbc_37_vec x2, rbc_37_vec u, const rbc_37_vec y, const rbc_37_mat H, const uint8_t *theta_i, uint8_t challenge2, const uint8_t *salt)
*
* \brief This function reconstructs the shares of index challenge2
*
* \param[out] x1 rbc_37_vec Representation of x1 concerning the second challenge
* \param[out] x2 rbc_37_vec Representation of x2 concerning the second challenge
* \param[out] u rbc_37_vec Representation of u concerning the second challenge
* \param[in] y rbc_37_vec Representation of the public key y
* \param[in] H rbc_37_mat Representation of public matrix H
* \param[in] theta_i String containing the input seed
* \param[in] challenge2 Integer concerning the second challenge
* \param[in] salt String containing the salt
*/
void ryde_192f_reconstruct_share(rbc_37_vec x1, rbc_37_vec x2, rbc_37_vec u, const rbc_37_vec y, const rbc_37_mat H, const uint8_t *theta_i, uint8_t challenge2, const uint8_t *salt) {

  uint8_t random[(2 * RYDE_192F_PARAM_W - 1 + RYDE_192F_PARAM_K) * RYDE_192F_PARAM_M_BYTES] = {0};
  seedexpander_shake_t seedexpander;

  // Sample randomness from seed theta
  seedexpander_shake_init(&seedexpander, &theta_i[RYDE_192F_SECURITY_BYTES * challenge2], RYDE_192F_SECURITY_BYTES, salt, 2 * RYDE_192F_SECURITY_BYTES);
  seedexpander_shake_get_bytes(&seedexpander, random, (2 * RYDE_192F_PARAM_W - 1 + RYDE_192F_PARAM_K) * RYDE_192F_PARAM_M_BYTES);

  // Recompute shares [x1], [x2] and [u] of index challenge2
  rbc_37_vec_from_bytes(u, RYDE_192F_PARAM_W - 1, &random[0]);
  rbc_37_vec_from_bytes(x2, RYDE_192F_PARAM_K, &random[(RYDE_192F_PARAM_W - 1) * RYDE_192F_PARAM_M_BYTES]);
  rbc_37_mat_vec_mul(x1, H, x2, RYDE_192F_PARAM_N - RYDE_192F_PARAM_K, RYDE_192F_PARAM_K);
  rbc_37_vec_add(x1, y, x1, RYDE_192F_PARAM_N - RYDE_192F_PARAM_K);

  memset(random, 0, (2 * RYDE_192F_PARAM_W - 1 + RYDE_192F_PARAM_K) * RYDE_192F_PARAM_M_BYTES);
}







/**
* \fn void ryde_192f_recompute_commitments(uint8_t *commits, uint8_t *theta_i, uint8_t e, const uint8_t *salt, const uint8_t *state, uint8_t hidden)
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
#ifdef SHAKE_TIMES4
void ryde_192f_recompute_commitments(uint8_t *commits, uint8_t *theta_i, uint8_t e, const uint8_t *salt, const uint8_t *state, uint8_t hidden) {

  uint8_t domain_separator = DOMAIN_SEPARATOR_COMMITMENT;
  ryde_192f_seed_tree_node_t partial_tree_seeds[RYDE_192F_PARAM_N_MPC_LOG2] = {0};
  ryde_192f_seed_tree_t theta_tree = {0};

  memcpy(&partial_tree_seeds, &state[0], RYDE_192F_PARAM_TREE_PATH_BYTES);
  ryde_192f_tree_expand_partial(theta_tree, (const ryde_192f_seed_tree_node_t *) partial_tree_seeds, salt, e, hidden);
  memcpy(theta_i, &theta_tree[RYDE_192F_PARAM_N_MPC - 1], RYDE_192F_PARAM_TREE_LEAF_BYTES);

  const uint8_t *salt_x4[] = {salt, salt, salt, salt};
  uint8_t *e_x4[] = {&e, &e, &e, &e};
  uint8_t *domain_separator_x4[] = {&domain_separator, &domain_separator, &domain_separator, &domain_separator};
  uint8_t index_0 = 0, index_1 = 1, index_2 = 2, index_3 = 3;

  // Recompute the first N-1 commitments (last commitment and index hidden are overwrited after)
  for(size_t i = 0; i < RYDE_192F_PARAM_N_MPC; i+=4) {
    uint8_t *index_x4[] = {&index_0, &index_1, &index_2, &index_3};

    const uint8_t *seed_x4[] = {&theta_i[RYDE_192F_SECURITY_BYTES * i], &theta_i[RYDE_192F_SECURITY_BYTES * (i + 1)],
                                &theta_i[RYDE_192F_SECURITY_BYTES * (i + 2)], &theta_i[RYDE_192F_SECURITY_BYTES * (i + 3)]};

    uint8_t *commit_x4[] = {&commits[2 * RYDE_192F_SECURITY_BYTES * i], &commits[2 * RYDE_192F_SECURITY_BYTES * (i + 1)],
                            &commits[2 * RYDE_192F_SECURITY_BYTES * (i + 2)], &commits[2 * RYDE_192F_SECURITY_BYTES * (i + 3)]};

    hash_sha3_x4_ctx ctx;
    hash_sha3_x4_init(&ctx);
    hash_sha3_x4_absorb(&ctx, (const uint8_t **) domain_separator_x4, sizeof(uint8_t));
    hash_sha3_x4_absorb(&ctx, (const uint8_t **) salt_x4, 2 * RYDE_192F_SECURITY_BYTES);
    hash_sha3_x4_absorb(&ctx, (const uint8_t **) e_x4, sizeof(uint8_t));
    hash_sha3_x4_absorb(&ctx, (const uint8_t **) index_x4, sizeof(uint8_t));
    hash_sha3_x4_absorb(&ctx, (const uint8_t **) seed_x4, RYDE_192F_SECURITY_BYTES);
    hash_sha3_x4_finalize(commit_x4, &ctx);

    index_0 += 4;
    index_1 += 4;
    index_2 += 4;
    index_3 += 4;
  }

  memcpy(&commits[2 * RYDE_192F_SECURITY_BYTES * hidden], &state[RYDE_192F_PARAM_TREE_PATH_BYTES], 2 * RYDE_192F_SECURITY_BYTES);

  // Recompute commitment N
  uint8_t i = (uint8_t) ((uint16_t)RYDE_192F_PARAM_N_MPC - 1);

  if (i == hidden) {
    memcpy(&commits[2 * RYDE_192F_SECURITY_BYTES * i], &state[RYDE_192F_PARAM_TREE_PATH_BYTES], 2 * RYDE_192F_SECURITY_BYTES);
  }
  else {
    hash_sha3_ctx ctx;
    hash_sha3_init(&ctx);
    hash_sha3_absorb(&ctx, salt, 2 * RYDE_192F_SECURITY_BYTES);
    hash_sha3_absorb(&ctx, &e, sizeof(uint8_t));
    hash_sha3_absorb(&ctx, &i, sizeof(uint8_t));
    hash_sha3_absorb(&ctx, &theta_i[RYDE_192F_SECURITY_BYTES * i], RYDE_192F_SECURITY_BYTES);
    hash_sha3_absorb(&ctx, &state[RYDE_192F_PARAM_TREE_PATH_BYTES + (2 * RYDE_192F_SECURITY_BYTES) + RYDE_192F_VEC_W_MINUS_ONE_BYTES], RYDE_192F_PARAM_STATE_BYTES);
    hash_sha3_absorb(&ctx, &domain_separator, sizeof(uint8_t));
    hash_sha3_finalize(&commits[2 * RYDE_192F_SECURITY_BYTES * i], &ctx);
  }
}
#else
void ryde_192f_recompute_commitments(uint8_t *commits, uint8_t *theta_i, uint8_t e, const uint8_t *salt, const uint8_t *state, uint8_t hidden) {

  uint8_t domain_separator = DOMAIN_SEPARATOR_COMMITMENT;
  ryde_192f_seed_tree_node_t partial_tree_seeds[RYDE_192F_PARAM_N_MPC_LOG2] = {0};
  ryde_192f_seed_tree_t theta_tree = {0};

  memcpy(&partial_tree_seeds, &state[0], RYDE_192F_PARAM_TREE_PATH_BYTES);
  ryde_192f_tree_expand_partial(theta_tree, (const ryde_192f_seed_tree_node_t *)partial_tree_seeds, salt, e, hidden);
  memcpy(theta_i, &theta_tree[RYDE_192F_PARAM_N_MPC - 1], RYDE_192F_PARAM_TREE_LEAF_BYTES);

  // Recompute the first N - 1 commitments
  for(size_t i = 0; i < (RYDE_192F_PARAM_N_MPC - 1); i++) {
    if(i == hidden) {
      memcpy(&commits[2 * RYDE_192F_SECURITY_BYTES * i], &state[RYDE_192F_PARAM_TREE_PATH_BYTES], 2 * RYDE_192F_SECURITY_BYTES);
    }
    else {
      hash_sha3_ctx ctx;
      hash_sha3_init(&ctx);
      hash_sha3_absorb(&ctx, &domain_separator, sizeof(uint8_t));
      hash_sha3_absorb(&ctx, salt, 2 * RYDE_192F_SECURITY_BYTES);
      hash_sha3_absorb(&ctx, &e, sizeof(uint8_t));
      hash_sha3_absorb(&ctx, (uint8_t *)&i, sizeof(uint8_t));
      hash_sha3_absorb(&ctx, &theta_i[RYDE_192F_SECURITY_BYTES * i], RYDE_192F_SECURITY_BYTES);
      hash_sha3_finalize(&commits[2 * RYDE_192F_SECURITY_BYTES * i], &ctx);
    }
  }

  // Recompute commitment N
  uint8_t i = (uint8_t) ((uint16_t)RYDE_192F_PARAM_N_MPC - 1);
  if (i == hidden) {
    memcpy(&commits[2 * RYDE_192F_SECURITY_BYTES * i], &state[RYDE_192F_PARAM_TREE_PATH_BYTES], 2 * RYDE_192F_SECURITY_BYTES);
  }
  else {
    hash_sha3_ctx ctx;
    hash_sha3_init(&ctx);
    hash_sha3_absorb(&ctx, salt, 2 * RYDE_192F_SECURITY_BYTES);
    hash_sha3_absorb(&ctx, &e, sizeof(uint8_t));
    hash_sha3_absorb(&ctx, &i, sizeof(uint8_t));
    hash_sha3_absorb(&ctx, &theta_i[RYDE_192F_SECURITY_BYTES * i], RYDE_192F_SECURITY_BYTES);
    hash_sha3_absorb(&ctx, &state[RYDE_192F_PARAM_TREE_PATH_BYTES + (2 * RYDE_192F_SECURITY_BYTES) + RYDE_192F_VEC_W_MINUS_ONE_BYTES], RYDE_192F_PARAM_STATE_BYTES);
    hash_sha3_absorb(&ctx, &domain_separator, sizeof(uint8_t));
    hash_sha3_finalize(&commits[2 * RYDE_192F_SECURITY_BYTES * i], &ctx);
  }
}
#endif



/**
* \fn void ryde_192f_recompute_shares(ryde_192f_leaf_shares_t *leaf_shares, const rbc_37_vec y, const rbc_37_mat H, const uint8_t *theta_i, const uint8_t *state, uint8_t second_challenge, const uint8_t *salt)
*
* \brief This function generates the random MPC shares determined by the signature scheme (when verifying)
*
* \param[out] leaf_shares ryde_192f_leaf_shares_t Representation of the leaf shares such that the sum of all of them gives the input data
* \param[in] y rbc_37_vec Representation of vector y
* \param[in] H rbc_37_mat Representation of matrix H
* \param[in] theta_i String containing the input seed used when generating the MPC shares (excluding the hidden position)
* \param[in] state String containing the public data determined by the used shares
* \param[in] second_challenge Integer concerning the second challenge
* \param[in] salt String containing the salt
*/
#ifdef SHAKE_TIMES4
void ryde_192f_recompute_shares(ryde_192f_leaf_shares_t *leaf_shares, const rbc_37_vec y, const rbc_37_mat H, const uint8_t *theta_i, const uint8_t *state, uint8_t second_challenge, const uint8_t *salt) {

  uint8_t random[(RYDE_192F_PARAM_W - 1) * RYDE_192F_PARAM_M_BYTES] = {0};
  seedexpander_shake_x4_t seedexpander_x4;
  seedexpander_shake_t seedexpander;

  const uint8_t *salt_x4[] = {salt, salt, salt, salt};

  for(size_t i = 0; i < RYDE_192F_PARAM_N_MPC; i+=4) {
    const uint8_t *seed_x4[] = {&theta_i[RYDE_192F_SECURITY_BYTES * i], &theta_i[RYDE_192F_SECURITY_BYTES * (i + 1)],
                                    &theta_i[RYDE_192F_SECURITY_BYTES * (i + 2)], &theta_i[RYDE_192F_SECURITY_BYTES * (i + 3)]};

    uint8_t random0[(2 * RYDE_192F_PARAM_W - 1 + RYDE_192F_PARAM_K) * RYDE_192F_PARAM_M_BYTES] = {0};
    uint8_t random1[(2 * RYDE_192F_PARAM_W - 1 + RYDE_192F_PARAM_K) * RYDE_192F_PARAM_M_BYTES] = {0};
    uint8_t random2[(2 * RYDE_192F_PARAM_W - 1 + RYDE_192F_PARAM_K) * RYDE_192F_PARAM_M_BYTES] = {0};
    uint8_t random3[(2 * RYDE_192F_PARAM_W - 1 + RYDE_192F_PARAM_K) * RYDE_192F_PARAM_M_BYTES] = {0};
    uint8_t *random_x4[] = {random0, random1, random2, random3};

    seedexpander_shake_x4_init(&seedexpander_x4, seed_x4, RYDE_192F_SECURITY_BYTES, salt_x4, 2 * RYDE_192F_SECURITY_BYTES);
    seedexpander_shake_x4_get_bytes(&seedexpander_x4, random_x4, (2 * RYDE_192F_PARAM_W - 1 + RYDE_192F_PARAM_K) * RYDE_192F_PARAM_M_BYTES);

    for(size_t j = 0; j < 4; j++) {
      rbc_37_elt_set_zero( leaf_shares->c[i + j]);
      rbc_37_vec_set_zero( leaf_shares->a[i + j], RYDE_192F_PARAM_W - 1);
      if (((i + j) < (RYDE_192F_PARAM_N_MPC - 1)) && ((i + j) != second_challenge)) {
        // Recompute share [u]_i
        rbc_37_vec_from_bytes( leaf_shares->u[i + j], RYDE_192F_PARAM_W - 1, &random_x4[j][0]);

        // Recompute share [x1]_i and [x2]_i
        rbc_37_vec_from_bytes( leaf_shares->x2[i + j], RYDE_192F_PARAM_K, &random_x4[j][(RYDE_192F_PARAM_W - 1) * RYDE_192F_PARAM_M_BYTES]);
        rbc_37_mat_vec_mul( leaf_shares->x1[i + j], H,  leaf_shares->x2[i + j], RYDE_192F_PARAM_N - RYDE_192F_PARAM_K, RYDE_192F_PARAM_K);
        rbc_37_vec_add( leaf_shares->x1[i + j], y,  leaf_shares->x1[i + j], RYDE_192F_PARAM_N - RYDE_192F_PARAM_K);

        // Recompute share [a]_i
        rbc_37_vec_from_bytes( leaf_shares->a[i + j], RYDE_192F_PARAM_W - 1, &random_x4[j][(RYDE_192F_PARAM_W - 1 + RYDE_192F_PARAM_K) * RYDE_192F_PARAM_M_BYTES]);

        // Recompute share [c]_i
        random_x4[j][(2 * RYDE_192F_PARAM_W - 1 + RYDE_192F_PARAM_K) * RYDE_192F_PARAM_M_BYTES - 1] &= RYDE_192F_PARAM_M_MASK;
        memcpy( leaf_shares->c[i + j], &random_x4[j][(2 * RYDE_192F_PARAM_W - 1 + RYDE_192F_PARAM_K - 1) * RYDE_192F_PARAM_M_BYTES], RYDE_192F_PARAM_M_BYTES);
      }
    }
  }

  // Recompute share N
  if ((RYDE_192F_PARAM_N_MPC - 1) != second_challenge) {
    // Sample randomness
    seedexpander_shake_init(&seedexpander, &theta_i[RYDE_192F_SECURITY_BYTES * (RYDE_192F_PARAM_N_MPC - 1)], RYDE_192F_SECURITY_BYTES, salt, 2 * RYDE_192F_SECURITY_BYTES);
    seedexpander_shake_get_bytes(&seedexpander, random, (RYDE_192F_PARAM_W - 1) * RYDE_192F_PARAM_M_BYTES);

    // Recompute [u]_N
    rbc_37_vec_from_bytes( leaf_shares->u[RYDE_192F_PARAM_N_MPC - 1], RYDE_192F_PARAM_W - 1, &random[0]);
  }

  // Recompute [x1]_N and [x2]_N
  rbc_37_vec_from_string( leaf_shares->x2[RYDE_192F_PARAM_N_MPC - 1], RYDE_192F_PARAM_K, &state[0]);
  rbc_37_mat_vec_mul( leaf_shares->x1[RYDE_192F_PARAM_N_MPC - 1], H,  leaf_shares->x2[RYDE_192F_PARAM_N_MPC - 1], RYDE_192F_PARAM_N - RYDE_192F_PARAM_K, RYDE_192F_PARAM_K);

  // Recompute [a]_N
  rbc_37_vec_from_string( leaf_shares->a[RYDE_192F_PARAM_N_MPC - 1], RYDE_192F_PARAM_W - 1, &state[RYDE_192F_VEC_K_BYTES]);

  // Recompute [c]_N
  memcpy( leaf_shares->c[RYDE_192F_PARAM_N_MPC - 1], &state[RYDE_192F_VEC_K_BYTES + RYDE_192F_VEC_W_MINUS_ONE_BYTES], RYDE_192F_PARAM_M_BYTES);

  memset(random, 0, (RYDE_192F_PARAM_W - 1) * RYDE_192F_PARAM_M_BYTES);
}
#else
void ryde_192f_recompute_shares(ryde_192f_leaf_shares_t *leaf_shares, const rbc_37_vec y, const rbc_37_mat H, const uint8_t *theta_i, const uint8_t *state, uint8_t second_challenge, const uint8_t *salt) {

  seedexpander_shake_t seedexpander;
  uint8_t random[(2 * RYDE_192F_PARAM_W - 1 + RYDE_192F_PARAM_K) * RYDE_192F_PARAM_M_BYTES] = {0};

  // Recompute the first N-1 shares
  for(size_t i = 0; i < (RYDE_192F_PARAM_N_MPC - 1); i++) {
    rbc_37_elt_set_zero( leaf_shares->c[i]);
    rbc_37_vec_set_zero( leaf_shares->a[i], RYDE_192F_PARAM_W - 1);
    if (i != second_challenge) {
      seedexpander_shake_init(&seedexpander, &theta_i[RYDE_192F_SECURITY_BYTES * i], RYDE_192F_SECURITY_BYTES, salt, 2 * RYDE_192F_SECURITY_BYTES);
      seedexpander_shake_get_bytes(&seedexpander, random, (2 * RYDE_192F_PARAM_W - 1 + RYDE_192F_PARAM_K) * RYDE_192F_PARAM_M_BYTES);

      // Recompute [u]_i
      rbc_37_vec_from_bytes( leaf_shares->u[i], RYDE_192F_PARAM_W - 1, &random[0]);

      // Recompute [x1]_i and [x2]_i
      rbc_37_vec_from_bytes( leaf_shares->x2[i], RYDE_192F_PARAM_K, &random[(RYDE_192F_PARAM_W - 1) * RYDE_192F_PARAM_M_BYTES]);
      rbc_37_mat_vec_mul( leaf_shares->x1[i], H,  leaf_shares->x2[i], RYDE_192F_PARAM_N - RYDE_192F_PARAM_K, RYDE_192F_PARAM_K);
      rbc_37_vec_add( leaf_shares->x1[i], y,  leaf_shares->x1[i], RYDE_192F_PARAM_N - RYDE_192F_PARAM_K);

      // Recompute [a]_i,
      rbc_37_vec_from_bytes( leaf_shares->a[i], RYDE_192F_PARAM_W - 1, &random[(RYDE_192F_PARAM_W - 1 + RYDE_192F_PARAM_K) * RYDE_192F_PARAM_M_BYTES]);

      // Recompute [c]_i
      random[(2 * RYDE_192F_PARAM_W - 1 + RYDE_192F_PARAM_K) * RYDE_192F_PARAM_M_BYTES - 1] &= RYDE_192F_PARAM_M_MASK;
      memcpy( leaf_shares->c[i], &random[(2 * RYDE_192F_PARAM_W - 1 + RYDE_192F_PARAM_K - 1) * RYDE_192F_PARAM_M_BYTES], RYDE_192F_PARAM_M_BYTES);
    }
  }

    // Recompute share N
  if ((RYDE_192F_PARAM_N_MPC - 1) != second_challenge) {
    // Sample randomness
    seedexpander_shake_init(&seedexpander, &theta_i[RYDE_192F_SECURITY_BYTES * (RYDE_192F_PARAM_N_MPC - 1)], RYDE_192F_SECURITY_BYTES, salt, 2 * RYDE_192F_SECURITY_BYTES);
    seedexpander_shake_get_bytes(&seedexpander, random, (RYDE_192F_PARAM_W - 1) * RYDE_192F_PARAM_M_BYTES);

    // Recompute [u]_N
    rbc_37_vec_from_bytes( leaf_shares->u[RYDE_192F_PARAM_N_MPC - 1], RYDE_192F_PARAM_W - 1, &random[0]);
  }

  // Recompute [x1]_N and [x2]_N
  rbc_37_vec_from_string( leaf_shares->x2[RYDE_192F_PARAM_N_MPC - 1], RYDE_192F_PARAM_K, &state[0]);
  rbc_37_mat_vec_mul( leaf_shares->x1[RYDE_192F_PARAM_N_MPC - 1], H,  leaf_shares->x2[RYDE_192F_PARAM_N_MPC - 1], RYDE_192F_PARAM_N - RYDE_192F_PARAM_K, RYDE_192F_PARAM_K);

  // Recompute [a]_N
  rbc_37_vec_from_string( leaf_shares->a[RYDE_192F_PARAM_N_MPC - 1], RYDE_192F_PARAM_W - 1, &state[RYDE_192F_VEC_K_BYTES]);

  // Recompute [c]_N
  memcpy( leaf_shares->c[RYDE_192F_PARAM_N_MPC - 1], &state[RYDE_192F_VEC_K_BYTES + RYDE_192F_VEC_W_MINUS_ONE_BYTES], RYDE_192F_PARAM_M_BYTES);

  memset(random, 0, (2 * RYDE_192F_PARAM_W - 1 + RYDE_192F_PARAM_K) * RYDE_192F_PARAM_M_BYTES);
}
#endif



/**
* \fn void ryde_192f_recompute_main_shares(ryde_192f_main_shares_t *main_shares, const ryde_192f_leaf_shares_t *leaf_shares, uint8_t second_challenge)
*
* \brief This function generates the main main_shares from the leaves main_shares as required in the signature scheme (when verifying)
*
* \param[out] main_shares ryde_192f_main_shares_t Representation of the main shares
* \param[in] leaf_shares ryde_192f_leaf_shares_t Representation of the leaf shares
* \param[in] second_challenge Integer concerning the second challenge
*/
void ryde_192f_recompute_main_shares(ryde_192f_main_shares_t *main_shares, const ryde_192f_leaf_shares_t *leaf_shares, uint8_t second_challenge) {

  uint8_t bit = 0x1;
  for(size_t i = 0; i < RYDE_192F_PARAM_D; i++) {
    for(size_t j = 0; j < RYDE_192F_PARAM_N_MPC; j++) {
      if (j != second_challenge) {
        if ((j & bit) == 0) {
          rbc_37_vec_add(main_shares->x1[i], main_shares->x1[i], leaf_shares->x1[j], RYDE_192F_PARAM_N - RYDE_192F_PARAM_K);
          rbc_37_vec_add(main_shares->x2[i], main_shares->x2[i], leaf_shares->x2[j], RYDE_192F_PARAM_K);
          rbc_37_vec_add(main_shares->a[i], main_shares->a[i], leaf_shares->a[j], RYDE_192F_PARAM_W - 1);
          rbc_37_vec_add(main_shares->u[i], main_shares->u[i], leaf_shares->u[j], RYDE_192F_PARAM_W - 1);
          rbc_37_elt_add(main_shares->c[i], main_shares->c[i], leaf_shares->c[j]);
        }
      }
    }
    bit <<= 1;
  }
}



/**
* \fn void ryde_192f_recompute_additional_main_share(rbc_37_vec x1, rbc_37_vec x2, rbc_37_vec a, rbc_37_vec u, rbc_37_elt c, const ryde_192f_leaf_shares_t *leaf_shares, uint8_t second_challenge)
*
* \brief This function generates the quasi-complement data concerning the first main share as required in the signature scheme (when verifying)
*        Notice that such a complement does not include the challenge leaf share.
*
* \param[out] x1 rbc_37_vec Representation of the quasi-complement x1 concerning the first main share
* \param[out] x2 rbc_37_vec Representation of the quasi-complement x2 concerning the first main share
* \param[out] a rbc_37_vec Representation of the quasi-complement a concerning the first main share
* \param[out] u rbc_37_vec Representation of the quasi-complement u concerning the first main share
* \param[out] c rbc_37_elt Representation of the quasi-complement c concerning the first main share
* \param[in] leaf_shares ryde_192f_leaf_shares_t Representation of the leaf shares
* \param[in] second_challenge Integer concerning the second challenge
*/
void ryde_192f_recompute_additional_main_share(rbc_37_vec x1, rbc_37_vec x2, rbc_37_vec a, rbc_37_vec u, rbc_37_elt c, const ryde_192f_leaf_shares_t *leaf_shares, uint8_t second_challenge) {

  rbc_37_vec_set_zero(x1, RYDE_192F_PARAM_N - RYDE_192F_PARAM_K);
  rbc_37_vec_set_zero(x2, RYDE_192F_PARAM_K);
  rbc_37_vec_set_zero(a, RYDE_192F_PARAM_W - 1);
  rbc_37_vec_set_zero(u, RYDE_192F_PARAM_W - 1);
  rbc_37_elt_set_zero(c);

  for(size_t j = 0; j < RYDE_192F_PARAM_N_MPC; j++) {
    if (j != second_challenge) {
      if ((j & 0x1) != 0) {
        rbc_37_vec_add(x1, x1, leaf_shares->x1[j], RYDE_192F_PARAM_N - RYDE_192F_PARAM_K);
        rbc_37_vec_add(x2, x2, leaf_shares->x2[j], RYDE_192F_PARAM_K);
        rbc_37_vec_add(a, a, leaf_shares->a[j], RYDE_192F_PARAM_W - 1);
        rbc_37_vec_add(u, u, leaf_shares->u[j], RYDE_192F_PARAM_W - 1);
        rbc_37_elt_add(c, c, leaf_shares->c[j]);
      }
    }
  }
}



/**
* \fn void ryde_192f_add_alpha_ch2(ryde_192f_response1_t *response, const uint8_t *state, uint8_t second_challenge)
*
* \brief This function updates the response according to the signature scheme (when verifying)
*
* \param[out] response ryde_192f_response1_t Representation of response
* \param[in] state String containing the public data determined by the used shares
* \param[in] second_challenge Integer concerning the second challenge
*/
void ryde_192f_add_alpha_ch2(ryde_192f_response1_t *response, const uint8_t *state, uint8_t second_challenge) {

  rbc_37_vec_from_string(response->alpha_, RYDE_192F_PARAM_W - 1, state);

  uint8_t bit = 0x1;
  for(size_t i = 0; i < RYDE_192F_PARAM_D; i++) {
    if((second_challenge & bit) == 0) {
      rbc_37_vec_add(response->alpha[i], response->alpha[i], response->alpha_, RYDE_192F_PARAM_W - 1);
    }
    bit <<= 1;
  }
}



/**
* \fn void ryde_192f_recompute_response1(ryde_192f_response1_t *response, const ryde_192f_main_shares_t *main_shares, const ryde_192f_challenge1_t *challenge, const rbc_37_vec x1, const rbc_37_vec x2, const rbc_37_vec a, const rbc_37_vec u, const rbc_37_elt c, uint8_t second_challenge)
*
* \brief This function computes the first response used in the signature scheme (verify)
*
* \param[out] response ryde_192f_response1_t Representation of the response
* \param[in] main_shares ryde_192f_leaf_shares_t Representation of the main shares
* \param[in] challenge ryde_192f_challenge1_t Representation of the challenge
* \param[in] x1 rbc_37_vec Representation of the quasi-complement x1 concerning the first main share
* \param[in] x2 rbc_37_vec Representation of the quasi-complement x2 concerning the first main share
* \param[in] a rbc_37_vec Representation of the quasi-complement a concerning the first main share
* \param[in] u rbc_37_vec Representation of the quasi-complement u concerning the first main share
* \param[in] c rbc_37_elt Representation of the quasi-complement c concerning the first main share
* \param[in] second_challenge Integer concerning the second challenge
*/
void ryde_192f_recompute_response1(ryde_192f_response1_t *response, const ryde_192f_main_shares_t *main_shares, const ryde_192f_challenge1_t *challenge,
                                        const rbc_37_vec x1, const rbc_37_vec x2, const rbc_37_vec a, const rbc_37_vec u, const rbc_37_elt c, uint8_t second_challenge) {

  rbc_37_vec x_qj; // x^[j]
  rbc_37_vec x, alpha, w, aux, b, a_t;
  rbc_37_vec diff; // x^[j] - x
  rbc_37_elt z[RYDE_192F_PARAM_D];
  rbc_37_elt c_, c_t, z_;

  rbc_37_vec_init(&alpha, RYDE_192F_PARAM_W - 1);
  rbc_37_vec_init(&aux, RYDE_192F_PARAM_W - 1);
  rbc_37_vec_init(&x, RYDE_192F_PARAM_N);
  rbc_37_vec_init(&x_qj, RYDE_192F_PARAM_N);
  rbc_37_vec_init(&diff, RYDE_192F_PARAM_N);
  rbc_37_vec_init(&w, RYDE_192F_PARAM_W - 1);
  rbc_37_vec_init(&b, RYDE_192F_PARAM_W - 1);
  rbc_37_vec_init(&a_t, RYDE_192F_PARAM_W - 1);

  // Compute [z], [a] and [c] concerning the sum of the first main share and its complement
  for(size_t k = 0; k < (RYDE_192F_PARAM_N - RYDE_192F_PARAM_K); k++) {
    rbc_37_elt_add(x[k], x1[k], ( main_shares->x1[0])[k]);
    rbc_37_elt_add(x_qj[k], x1[k], ( main_shares->x1[0])[k]);
  }
  for(size_t k = 0; k < RYDE_192F_PARAM_K; k++) {
    rbc_37_elt_add(x[k + RYDE_192F_PARAM_N - RYDE_192F_PARAM_K], x2[k], ( main_shares->x2[0])[k]);
    rbc_37_elt_add(x_qj[k + RYDE_192F_PARAM_N - RYDE_192F_PARAM_K], x2[k], ( main_shares->x2[0])[k]);
  }
  for(size_t k = 0; k < RYDE_192F_PARAM_W; k++) {
    for(size_t j = 0; j < RYDE_192F_PARAM_N; j++) {
      rbc_37_elt_sqr(x_qj[j], x_qj[j]);
    }
  }
  rbc_37_vec_add(diff, x_qj, x, RYDE_192F_PARAM_N);
  rbc_37_vec_inner_product(z_, challenge->gamma, diff, RYDE_192F_PARAM_N);
  rbc_37_vec_add(b, a,  main_shares->a[0], RYDE_192F_PARAM_W - 1);
  rbc_37_elt_add(c_, c,  main_shares->c[0]);

  // Compute share [alpha] for main shares
  uint8_t bit = 0x1;
  for(size_t i = 0; i < RYDE_192F_PARAM_D; i++) {
    // Compute share [x]
    for(size_t k = 0; k < (RYDE_192F_PARAM_N - RYDE_192F_PARAM_K); k++) {
      rbc_37_elt_set(x[k], ( main_shares->x1[i])[k]);
      rbc_37_elt_set(x_qj[k], ( main_shares->x1[i])[k]);
    }
    for(size_t k = 0; k < RYDE_192F_PARAM_K; k++) {
      rbc_37_elt_set(x[k + RYDE_192F_PARAM_N - RYDE_192F_PARAM_K], ( main_shares->x2[i])[k]);
      rbc_37_elt_set(x_qj[k + RYDE_192F_PARAM_N - RYDE_192F_PARAM_K], ( main_shares->x2[i])[k]);
    }

    // Compute share [w]
    for(size_t k = 0; k < (RYDE_192F_PARAM_W - 1); k++) {
      for(size_t j = 0; j < RYDE_192F_PARAM_N; j++) {
        rbc_37_elt_sqr(x_qj[j], x_qj[j]);
      }
      rbc_37_vec_add(diff, x_qj, x, RYDE_192F_PARAM_N);
      rbc_37_vec_inner_product(w[k], challenge->gamma, diff, RYDE_192F_PARAM_N);
    }

    // Compute [alpha]
    rbc_37_vec_scalar_mul(aux, w, challenge->epsilon, RYDE_192F_PARAM_W - 1);
    rbc_37_vec_add(aux, aux,  main_shares->u[i], RYDE_192F_PARAM_W - 1);
    rbc_37_vec_add(response->alpha[i], response->alpha[i], aux, RYDE_192F_PARAM_W - 1);

    for(size_t j = 0; j < RYDE_192F_PARAM_N; j++) {
      rbc_37_elt_sqr(x_qj[j], x_qj[j]);
    }
    rbc_37_vec_add(diff, x_qj, x, RYDE_192F_PARAM_N);
    rbc_37_vec_inner_product(z[i], challenge->gamma, diff, RYDE_192F_PARAM_N);

    bit <<= 1;
  }

  // Compute plain alpha
  rbc_37_vec_set(alpha, response->alpha_, RYDE_192F_PARAM_W - 1);
  ryde_192f_reconstruct_alpha(response, x1, x2, u, challenge);
  if ((second_challenge & 0x1) != 0) {
    rbc_37_vec_add(response->alpha_, response->alpha_, alpha, RYDE_192F_PARAM_W - 1);
  }
  rbc_37_vec_add(response->alpha_, response->alpha_, response->alpha[0], RYDE_192F_PARAM_W - 1);
  #ifdef VERBOSE
  printf("ryde_192f_recompute_response1:alpha:\t");rbc_37_vec_print(response->alpha_, RYDE_192F_PARAM_W - 1);printf("\n");
  #endif

  // Compute [z] and [v] for main shares
  bit = 0x1;
  for(size_t i = 0; i < RYDE_192F_PARAM_D; i++) {
    if ((second_challenge & bit) == 0) {
      rbc_37_elt_add(z[i], z_, z[i]);
      rbc_37_vec_add(a_t, b,  main_shares->a[i], RYDE_192F_PARAM_W - 1);
      rbc_37_elt_add(c_t, c_,  main_shares->c[i]);
    }
    else {
      rbc_37_vec_set(a_t,  main_shares->a[i], RYDE_192F_PARAM_W - 1);
      rbc_37_elt_set(c_t,  main_shares->c[i]);
    }

    #ifdef VERBOSE
    printf("ryde_192f_recompute_response1:c[%zu]:\t", i);rbc_37_elt_print(c_t);printf("\n");
    printf("ryde_192f_recompute_response1:z[%zu]:\t", i);rbc_37_elt_print(z[i]);printf("\n");
    printf("ryde_192f_recompute_response1:a[%zu]:\t", i);rbc_37_vec_print(a_t, RYDE_192F_PARAM_W - 1);
    #endif

    rbc_37_elt_mul(response->v[i], challenge->epsilon, z[i]);
    rbc_37_vec_inner_product(z[i], response->alpha_, a_t, RYDE_192F_PARAM_W - 1);
    rbc_37_elt_add(response->v[i], response->v[i], z[i]);
    rbc_37_elt_add(response->v[i], response->v[i], c_t);
    rbc_37_elt_set_zero(z[i]);
    bit <<= 1;

    #ifdef VERBOSE
    printf("ryde_192f_recompute_response1:v[%zu]:\t", i);rbc_37_elt_print(response->v[i]);printf("\n\n");
    #endif
  }

  rbc_37_elt_set_zero(c_);
  rbc_37_elt_set_zero(c_t);
  rbc_37_vec_clear(b);
  rbc_37_vec_clear(a_t);
  rbc_37_vec_clear(w);
  rbc_37_vec_clear(x);
  rbc_37_vec_clear(x_qj);
  rbc_37_vec_clear(diff);
  rbc_37_vec_clear(alpha);
  rbc_37_vec_clear(aux);
}
