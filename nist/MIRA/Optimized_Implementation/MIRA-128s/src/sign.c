/** 
 * @file sign_mira_128_sign.c
 * @brief Implementation of sign.h
 */

#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include "string.h"
#include <assert.h>
#include "randombytes.h"
#include "hash_fips202.h"
#include "parameters.h"
#include "parsing.h"
#include "mpc.h"
#include "tree.h"
#include "sign.h"



/**
 * \fn int sign_mira_128_sign(uint8_t* signature, const uint8_t* message, size_t message_size, const uint8_t* sk)
 * \brief Sign algorithm of the MIRA_128_SIGN scheme
 *
 * \param[out] signature String containing the signature
 * \param[in] message String containing the message to be signed
 * \param[in] message_size Length of the message to be signed
 * \param[in] sk String containing the secret key
 * \return EXIT_SUCCESS if verify is successful. Otherwise, it returns EXIT_FAILURE.
 */
int sign_mira_128_sign(uint8_t* signature, const uint8_t* message, size_t message_size, const uint8_t* sk) {

  #ifdef VERBOSE
    printf("\n\n### SIGN ###\n");
  #endif

  // ------------------------------------------------------------------------------------------------------------- Setup
  if (message == NULL || signature == NULL || sk == NULL) { return EXIT_FAILURE; }
  if (message_size <= 0) { return EXIT_FAILURE; }
  memset(signature, 0, SIGN_MIRA_128_SIGNATURE_BYTES);

  // Setup variables related to randomness, hash and challenges
  uint8_t salt_and_mseed[3 * SIGN_MIRA_128_SECURITY_BYTES] = {0};
  uint8_t salt[2 * SIGN_MIRA_128_SECURITY_BYTES] = {0};

  uint8_t domain_separator;
  uint8_t h1[2 * SIGN_MIRA_128_SECURITY_BYTES] = {0};
  uint8_t h2[2 * SIGN_MIRA_128_SECURITY_BYTES] = {0};
  hash_sha3_ctx ctx_m, ctx_h1, ctx_h2;
  hash_sha3_init(&ctx_m);
  hash_sha3_init(&ctx_h1);
  hash_sha3_init(&ctx_h2);

  uint8_t m_digest[2 * SIGN_MIRA_128_SECURITY_BYTES] = {0};
  sign_mira_128_challenge1_t challenge1[SIGN_MIRA_128_PARAM_TAU];
  for(size_t e = 0; e < SIGN_MIRA_128_PARAM_TAU; e++) { sign_mira_128_init_challenge1(&(challenge1[e])); }
  uint8_t challenge2[SIGN_MIRA_128_PARAM_TAU] = {0};

  // Setup variables related to sk and pk
  uint8_t pk[SIGN_MIRA_128_PUBLIC_KEY_BYTES] = {0};
  gf16_mat M0;
  gf16_mat Mi[SIGN_MIRA_128_PARAM_K];
  gf16_mat x;
  gf16_mat E;
  gfqm A[SIGN_MIRA_128_PARAM_R+1];

  gf16_mat_init(&M0, SIGN_MIRA_128_PARAM_M, SIGN_MIRA_128_PARAM_N);
  for(int i=0 ; i<SIGN_MIRA_128_PARAM_K ; i++) gf16_mat_init(Mi + i, SIGN_MIRA_128_PARAM_M, SIGN_MIRA_128_PARAM_N);
  gf16_mat_init(&x, 1, SIGN_MIRA_128_PARAM_K);
  gf16_mat_init(&E, SIGN_MIRA_128_PARAM_M, SIGN_MIRA_128_PARAM_N);

  // Setup variables related to MPC
  uint8_t theta[SIGN_MIRA_128_PARAM_TAU * SIGN_MIRA_128_SECURITY_BYTES] = {0};
  uint8_t theta_i[SIGN_MIRA_128_PARAM_TREE_LEAF_BYTES * SIGN_MIRA_128_PARAM_TAU] = {0};
  uint8_t cmt_i[SIGN_MIRA_128_PARAM_COMMITMENT_BYTES * SIGN_MIRA_128_PARAM_TAU] = {0};

  sign_mira_128_shares_t shares;
  sign_mira_128_init_shares(&shares);
  sign_mira_128_main_shares_t main_shares[SIGN_MIRA_128_PARAM_TAU];

  gf16_mat x_ch2;
  gfqm_vec a_ch2;

  gf16_mat_init(&x_ch2, 1, SIGN_MIRA_128_PARAM_K);
  gfqm_vec_init(&a_ch2, SIGN_MIRA_128_PARAM_R);

  gf16_mat x_N[SIGN_MIRA_128_PARAM_TAU];
  gfqm_vec beta_N[SIGN_MIRA_128_PARAM_TAU];
  gfqm_vec a_N[SIGN_MIRA_128_PARAM_TAU];
  gfqm_vec a[SIGN_MIRA_128_PARAM_TAU];
  gfqm c_N[SIGN_MIRA_128_PARAM_TAU];
  gfqm c[SIGN_MIRA_128_PARAM_TAU];

  for(size_t e = 0; e < SIGN_MIRA_128_PARAM_TAU; e++) { gfqm_vec_init(&(a[e]), SIGN_MIRA_128_PARAM_R); }

  uint8_t response1_str[(SIGN_MIRA_128_VEC_R_BYTES + SIGN_MIRA_128_PARAM_M_BYTES) * SIGN_MIRA_128_PARAM_D] = {0};
  sign_mira_128_response1_t response1[SIGN_MIRA_128_PARAM_TAU];

  // Parse secret key and public key
  sign_mira_128_secret_key_from_string(M0, Mi, x, E, A, sk);
  sign_mira_128_public_key_to_string(pk, &sk[SIGN_MIRA_128_SECURITY_BYTES], M0);

  // Hash message
  domain_separator = DOMAIN_SEPARATOR_MESSAGE;
  hash_sha3_absorb(&ctx_m, &domain_separator, sizeof(uint8_t));
  hash_sha3_absorb(&ctx_m, message, message_size);
  hash_sha3_finalize(m_digest, &ctx_m);

  // -------------------------------------------------------------------------------------------------------- Commitment

  // Sample salt, theta and parse signature
  randombytes(salt_and_mseed, 3 * SIGN_MIRA_128_SECURITY_BYTES);
  hash_shake(theta, SIGN_MIRA_128_SECURITY_BYTES * SIGN_MIRA_128_PARAM_TAU, salt_and_mseed, 3 * SIGN_MIRA_128_SECURITY_BYTES);
  memcpy(&salt, salt_and_mseed, 2 * SIGN_MIRA_128_SECURITY_BYTES);
  memcpy(&signature[0], salt, 2 * SIGN_MIRA_128_SECURITY_BYTES);

  // Add m, pk and salt to h1
  domain_separator = DOMAIN_SEPARATOR_HASH1;
  hash_sha3_absorb(&ctx_h1, &domain_separator, sizeof(uint8_t));
  hash_sha3_absorb(&ctx_h1, m_digest, 2 * SIGN_MIRA_128_SECURITY_BYTES);
  hash_sha3_absorb(&ctx_h1, pk, SIGN_MIRA_128_PUBLIC_KEY_BYTES);
  hash_sha3_absorb(&ctx_h1, salt, 2 * SIGN_MIRA_128_SECURITY_BYTES);

  // Generate and commit to shares
  for(size_t e = 0; e < SIGN_MIRA_128_PARAM_TAU; e++) {
    // Generate seed tree
    sign_mira_128_seed_tree_t seed_tree = {0};
    memcpy(seed_tree[0], &theta[SIGN_MIRA_128_SECURITY_BYTES * e], SIGN_MIRA_128_SECURITY_BYTES);
    sign_mira_128_tree_expand(seed_tree, salt);
    memcpy(&theta_i[SIGN_MIRA_128_PARAM_TREE_LEAF_BYTES * e], &seed_tree[SIGN_MIRA_128_PARAM_N_MPC - 1], SIGN_MIRA_128_PARAM_TREE_LEAF_BYTES);

    // Generate (leaf party) shares
    sign_mira_128_compute_shares(&shares, x, A, &theta_i[SIGN_MIRA_128_PARAM_TREE_LEAF_BYTES * e], salt);
    
    // Commit to (leaf party) shares
    sign_mira_128_commit_to_shares(&cmt_i[SIGN_MIRA_128_PARAM_COMMITMENT_BYTES * e], e, salt, &shares, &theta_i[SIGN_MIRA_128_PARAM_TREE_LEAF_BYTES * e]);

    // Add cmt_i_i to h1
    hash_sha3_absorb(&ctx_h1, &cmt_i[SIGN_MIRA_128_PARAM_COMMITMENT_BYTES * e], SIGN_MIRA_128_PARAM_COMMITMENT_BYTES);
    
    // Generate (main party) shares
    sign_mira_128_init_main_shares(&(main_shares[e]));
    sign_mira_128_compute_main_shares(&(main_shares[e]), &shares);

    // Store last (leaf party) share and a
    gf16_mat_init(&(x_N[e]), 1, SIGN_MIRA_128_PARAM_K);
    gfqm_vec_init(&(beta_N[e]), SIGN_MIRA_128_PARAM_R);
    gfqm_vec_init(&(a_N[e]), SIGN_MIRA_128_PARAM_R);

    gf16_mat_set(x_N[e], shares.x[SIGN_MIRA_128_PARAM_N_MPC - 1], 1, SIGN_MIRA_128_PARAM_K);
    gfqm_vec_set(beta_N[e], shares.beta[SIGN_MIRA_128_PARAM_N_MPC - 1], SIGN_MIRA_128_PARAM_R);
    gfqm_vec_set(a_N[e], shares.a[SIGN_MIRA_128_PARAM_N_MPC - 1], SIGN_MIRA_128_PARAM_R);
    gfqm_set(c_N[e], shares.c[SIGN_MIRA_128_PARAM_N_MPC - 1]);
    gfqm_set_zero(c[e]);
    for(size_t index = 0; index < SIGN_MIRA_128_PARAM_N_MPC; index++) { gfqm_add(c[e], c[e], shares.c[index]); }
    gfqm_vec_set(a[e], shares.a_, SIGN_MIRA_128_PARAM_R);
  }
  sign_mira_128_clear_shares(&shares);

  // --------------------------------------------------------------------------------------------------- First Challenge

  // Generate h1 and parse signature
  hash_sha3_finalize(h1, &ctx_h1);
  memcpy(&signature[2 * SIGN_MIRA_128_SECURITY_BYTES], h1, 2 * SIGN_MIRA_128_SECURITY_BYTES);

  // Generate first challenge
  sign_mira_128_compute_challenge1(challenge1, h1, salt);

  // ---------------------------------------------------------------------------------------------------- First Response

  // Add m, pk, salt and h1 to h2
  domain_separator = DOMAIN_SEPARATOR_HASH2;
  hash_sha3_absorb(&ctx_h2, &domain_separator, sizeof(uint8_t));
  hash_sha3_absorb(&ctx_h2, m_digest, 2 * SIGN_MIRA_128_SECURITY_BYTES);
  hash_sha3_absorb(&ctx_h2, pk, SIGN_MIRA_128_PUBLIC_KEY_BYTES);
  hash_sha3_absorb(&ctx_h2, salt, 2 * SIGN_MIRA_128_SECURITY_BYTES);
  hash_sha3_absorb(&ctx_h2, h1, 2 * SIGN_MIRA_128_SECURITY_BYTES);

  // Compute first response
  for(size_t e = 0; e < SIGN_MIRA_128_PARAM_TAU; e++) {

    sign_mira_128_init_response1(&(response1[e]));
    sign_mira_128_compute_response1(&(response1[e]), &(main_shares[e]), &(challenge1[e]), M0, Mi, x, a[e]);

    // Add alpha, [alpha] and [v] to h2
    for(size_t i = 0; i < SIGN_MIRA_128_PARAM_D; i++ ) {
      gfqm_vec_to_string(&response1_str[(SIGN_MIRA_128_VEC_R_BYTES + SIGN_MIRA_128_PARAM_M_BYTES) * i], response1[e].alpha[i], SIGN_MIRA_128_PARAM_R);
      gfqm_to_string(&response1_str[(SIGN_MIRA_128_VEC_R_BYTES + SIGN_MIRA_128_PARAM_M_BYTES) * i + SIGN_MIRA_128_VEC_R_BYTES], response1[e].v[i]);
    }

    uint8_t plain_alpha[SIGN_MIRA_128_VEC_R_BYTES] = {0};
    gfqm_vec_to_string(plain_alpha, response1[e].alpha_, SIGN_MIRA_128_PARAM_R);
    hash_sha3_absorb(&ctx_h2, plain_alpha, SIGN_MIRA_128_VEC_R_BYTES);
    hash_sha3_absorb(&ctx_h2, response1_str, (SIGN_MIRA_128_VEC_R_BYTES + SIGN_MIRA_128_PARAM_M_BYTES) * SIGN_MIRA_128_PARAM_D);
  }

  // -------------------------------------------------------------------------------------------------- Second Challenge

  // Generate h2 and parse signature
  hash_sha3_finalize(h2, &ctx_h2);
  memcpy(&signature[4 * SIGN_MIRA_128_SECURITY_BYTES], h2, 2 * SIGN_MIRA_128_SECURITY_BYTES);

  // Generate second challenge
  hash_shake(challenge2, SIGN_MIRA_128_PARAM_TAU, h2, 2 * SIGN_MIRA_128_SECURITY_BYTES);

  // --------------------------------------------------------------------------------------------------- Second Response

  // Compute signature
  for(size_t e = 0; e < SIGN_MIRA_128_PARAM_TAU; e++) {
    challenge2[e] &= (uint8_t) (SIGN_MIRA_128_PARAM_N_MPC - 1);

    // Generate partial seed tree
    sign_mira_128_seed_tree_t seed_tree = {0};
    memcpy(seed_tree[0], &theta[SIGN_MIRA_128_SECURITY_BYTES * e], SIGN_MIRA_128_SECURITY_BYTES);
    sign_mira_128_tree_expand(seed_tree, salt);

    sign_mira_128_seed_tree_node_t partial_seed_tree[SIGN_MIRA_128_PARAM_N_MPC_LOG2] = {0};
    sign_mira_128_tree_compute_partial(partial_seed_tree, (const sign_mira_128_seed_tree_node_t *) seed_tree, challenge2[e]);

    memcpy(&signature[6 * SIGN_MIRA_128_SECURITY_BYTES + (SIGN_MIRA_128_PARAM_RESPONSE_BYTES * e)], partial_seed_tree[0], SIGN_MIRA_128_PARAM_TREE_PATH_BYTES);
    memcpy(&signature[6 * SIGN_MIRA_128_SECURITY_BYTES + (SIGN_MIRA_128_PARAM_RESPONSE_BYTES * e) + SIGN_MIRA_128_PARAM_TREE_PATH_BYTES],
           &cmt_i[(SIGN_MIRA_128_PARAM_COMMITMENT_BYTES * e) + (2 * SIGN_MIRA_128_SECURITY_BYTES * challenge2[e])], 2 * SIGN_MIRA_128_SECURITY_BYTES);

    // Compute share [alpha]_ch2
    if(challenge2[e] != (uint8_t) (SIGN_MIRA_128_PARAM_N_MPC - 1)) {
      sign_mira_128_reconstruct_share(x_ch2, a_ch2, &theta_i[SIGN_MIRA_128_PARAM_TREE_LEAF_BYTES * e], challenge2[e], salt);
    }
    else {
      gf16_mat_set(x_ch2, x_N[e], 1, SIGN_MIRA_128_PARAM_K);
      gfqm_vec_set(a_ch2, a_N[e], SIGN_MIRA_128_PARAM_R);
    }

    if(challenge2[e] == 0) sign_mira_128_reconstruct_alpha(&(response1[e]), M0, Mi, x_ch2, a_ch2, &(challenge1[e]), 1);
    else sign_mira_128_reconstruct_alpha(&(response1[e]), M0, Mi, x_ch2, a_ch2, &(challenge1[e]), 0);

    if(challenge2[e] == (uint8_t) (SIGN_MIRA_128_PARAM_N_MPC - 1)) {
        gf16_mat_set_zero(x_N[e], 1, SIGN_MIRA_128_PARAM_K);
        gfqm_vec_set_zero(beta_N[e], SIGN_MIRA_128_PARAM_R);
        gfqm_set_zero(c_N[e]);
    }

    // Add [alpha]_ch2, [x]_N, [beta]_N and [c]_N to the signature
    uint32_t index = 6 * SIGN_MIRA_128_SECURITY_BYTES + (SIGN_MIRA_128_PARAM_RESPONSE_BYTES * e) + SIGN_MIRA_128_PARAM_TREE_PATH_BYTES + 2 * SIGN_MIRA_128_SECURITY_BYTES;
    gfqm_vec_to_string(&signature[index], response1[e].alpha_, SIGN_MIRA_128_PARAM_R);
    gf16_mat_to_string(&signature[index + SIGN_MIRA_128_VEC_R_BYTES], x_N[e], 1, SIGN_MIRA_128_PARAM_K); 
    gfqm_vec_to_string(&signature[index + SIGN_MIRA_128_VEC_R_BYTES + SIGN_MIRA_128_VEC_K_BYTES], beta_N[e], SIGN_MIRA_128_PARAM_R);
    gfqm_to_string(&signature[index + 2 * SIGN_MIRA_128_VEC_R_BYTES + SIGN_MIRA_128_VEC_K_BYTES], c_N[e]);
  }

  // -------------------------------------------------------------------------------------------------------- Clear Data

  #ifdef VERBOSE
    printf("\nchallenge1:");
    for(uint8_t e = 0; e < SIGN_MIRA_128_PARAM_TAU; e++) {
      printf("\n  Iteration %" PRIu8 ":", e);

      uint8_t challenge1_gamma_string[SIGN_MIRA_128_VEC_N_BYTES] = {0};
      gfqm_vec_to_string(challenge1_gamma_string, challenge1[e].gamma, SIGN_MIRA_128_PARAM_N);
      printf("\n    - gamma   : "); for(size_t i = 0 ; i < SIGN_MIRA_128_VEC_N_BYTES ; i++) { printf("%02x", challenge1_gamma_string[i]); }
      uint8_t challenge1_epsilon_string[SIGN_MIRA_128_PARAM_M_BYTES] = {0};
      gfqm_to_string(challenge1_epsilon_string, challenge1[e].epsilon);
      printf("\n    - epsilon : "); for(size_t i = 0 ; i < SIGN_MIRA_128_PARAM_M_BYTES ; i++) { printf("%02x", challenge1_epsilon_string[i]); }
    }

    printf("\nchallenge2: "); for(size_t i = 0 ; i < SIGN_MIRA_128_PARAM_TAU ; i++) printf("%" PRIu8 " ", challenge2[i]);
    printf("\n");

    printf("\nsignature:"); size_t index = 0;
    printf("\n  salt:"); for(size_t i = 0 ; i < 2 * SIGN_MIRA_128_SECURITY_BYTES ; i++) { printf("%02x", signature[i]); } index+= 2 * SIGN_MIRA_128_SECURITY_BYTES;
    printf("\n  h1:  "); for(size_t i = 0 ; i < 2 * SIGN_MIRA_128_SECURITY_BYTES ; i++) { printf("%02x", signature[index + i]); } index+= 2 * SIGN_MIRA_128_SECURITY_BYTES;
    printf("\n  h2:  "); for(size_t i = 0 ; i < 2 * SIGN_MIRA_128_SECURITY_BYTES ; i++) { printf("%02x", signature[index + i]); } index+= 2 * SIGN_MIRA_128_SECURITY_BYTES;
    for(uint8_t e = 0; e < SIGN_MIRA_128_PARAM_TAU; e++) {
      printf("\n  Iteration %" PRIu8 ":", e);
      printf("\n    - tree : "); for(size_t i = 0 ; i < SIGN_MIRA_128_PARAM_TREE_PATH_BYTES ; i++) { printf("%02x", signature[index + i]); } index+= SIGN_MIRA_128_PARAM_TREE_PATH_BYTES;
      printf("\n    - cmt  : "); for(size_t i = 0 ; i < 2 * SIGN_MIRA_128_SECURITY_BYTES ; i++) { printf("%02x", signature[index + i]); } index+= 2 * SIGN_MIRA_128_SECURITY_BYTES;
      printf("\n    - alpha: "); for(size_t i = 0 ; i < SIGN_MIRA_128_VEC_R_BYTES ; i++) { printf("%02x", signature[index + i]); } index+= SIGN_MIRA_128_VEC_R_BYTES;
      printf("\n    - x    : "); for(size_t i = 0 ; i < SIGN_MIRA_128_VEC_K_BYTES ; i++) { printf("%02x", signature[index + i]); } index+= SIGN_MIRA_128_VEC_K_BYTES;
      printf("\n    - beta : "); for(size_t i = 0 ; i < SIGN_MIRA_128_VEC_R_BYTES ; i++) { printf("%02x", signature[index + i]); } index+= SIGN_MIRA_128_VEC_R_BYTES;
      printf("\n    - c    : "); for(size_t i = 0 ; i < SIGN_MIRA_128_PARAM_M_BYTES ; i++) { printf("%02x", signature[index + i]); } index+= SIGN_MIRA_128_PARAM_M_BYTES;
    }
    printf("\n");
  #endif

  memset(salt_and_mseed, 0, 3 * SIGN_MIRA_128_SECURITY_BYTES);
  memset(theta, 0, SIGN_MIRA_128_PARAM_TAU * SIGN_MIRA_128_SECURITY_BYTES);
  memset(theta_i, 0, SIGN_MIRA_128_PARAM_TREE_LEAF_BYTES * SIGN_MIRA_128_PARAM_TAU);
  memset(cmt_i, 0, SIGN_MIRA_128_PARAM_COMMITMENT_BYTES * SIGN_MIRA_128_PARAM_TAU);
  memset(h1, 0, 2 * SIGN_MIRA_128_SECURITY_BYTES);
  memset(h2, 0, 2 * SIGN_MIRA_128_SECURITY_BYTES);
  memset(response1_str, 0, (SIGN_MIRA_128_VEC_R_BYTES + SIGN_MIRA_128_PARAM_M_BYTES) * SIGN_MIRA_128_PARAM_D);

  for(size_t e = 0; e < SIGN_MIRA_128_PARAM_TAU; e++) {
    gfqm_vec_clear(a[e]);
    gf16_mat_clear(x_N[e]);
    gfqm_vec_clear(a_N[e]);
    gfqm_vec_clear(beta_N[e]);
    gfqm_set_zero(c_N[e]);
    sign_mira_128_clear_challenge1(&(challenge1[e]));
    sign_mira_128_clear_response1(&(response1[e]));
    sign_mira_128_clear_main_shares(&(main_shares[e]));
  }

  gf16_mat_clear(M0);
  for(int i=0 ; i<SIGN_MIRA_128_PARAM_K ; i++) gf16_mat_clear(Mi[i]);
  gf16_mat_clear(x);  
  gf16_mat_clear(x_ch2);
  gfqm_vec_clear(a_ch2);
  gf16_mat_clear(E);

  return EXIT_SUCCESS;
}
