/** 
 * \file sign_mira_256_verify.c
 * \brief Implementation of verify.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "string.h"
#include "hash_fips202.h"
#include "parameters.h"
#include "parsing.h"
#include "mpc.h"
#include "verify.h"



/**
 * \fn int sign_mira_256_verify(const uint8_t* message, size_t message_size, const uint8_t* signature, size_t signature_size, const uint8_t* pk)
 * \brief Verify of the MIRA_256_SIGN scheme
 *
 * The public key is composed of the vector <b>y</b> as well as the seed used to generate matrix <b>H</b>.
 *
 * \param[in] signature String containing the signature
 * \param[in] signature_size Integer determining the signed message byte-length
 * \param[in] message String containing the message to be signed
 * \param[in] message_size Integer determining the message byte-length
 * \param[in] pk String containing the public key
 * \return EXIT_SUCCESS if verify is successful. Otherwise, it returns EXIT_FAILURE
 */
int sign_mira_256_verify(const uint8_t* signature, size_t signature_size, const uint8_t* message, size_t message_size, const uint8_t* pk) {

  #ifdef VERBOSE
    printf("\n### VERIFY ###\n");
  #endif

  // ------------------------------------------------------------------------------------------------------------- Setup

  if(signature == NULL || message == NULL || pk == NULL) { return EXIT_FAILURE; }
  if(signature_size != SIGN_MIRA_256_SIGNATURE_BYTES) { return EXIT_FAILURE; }

  // Setup variables related to randomness, salt, hash and challenges
  uint8_t salt[2 * SIGN_MIRA_256_SECURITY_BYTES] = {0};
  memcpy(salt, &signature[0], 2 * SIGN_MIRA_256_SECURITY_BYTES);

  uint8_t domain_separator;
  uint8_t h1[2 * SIGN_MIRA_256_SECURITY_BYTES] = {0};
  uint8_t h2[2 * SIGN_MIRA_256_SECURITY_BYTES] = {0};
  uint8_t h1_[2 * SIGN_MIRA_256_SECURITY_BYTES] = {0};
  uint8_t h2_[2 * SIGN_MIRA_256_SECURITY_BYTES] = {0};

  hash_sha3_ctx ctx_m, ctx_h1, ctx_h2;
  hash_sha3_init(&ctx_m);
  hash_sha3_init(&ctx_h1);
  hash_sha3_init(&ctx_h2);

  uint8_t m_digest[2 * SIGN_MIRA_256_SECURITY_BYTES] = {0};
  sign_mira_256_challenge1_t challenge1[SIGN_MIRA_256_PARAM_TAU];
  uint8_t challenge2[SIGN_MIRA_256_PARAM_TAU] = {0};

  // Setup variables related to pk
  gf16_mat M0;
  gf16_mat Mi[SIGN_MIRA_256_PARAM_K];

  gf16_mat_init(&M0, SIGN_MIRA_256_PARAM_M, SIGN_MIRA_256_PARAM_N);
  for(int i=0 ; i<SIGN_MIRA_256_PARAM_K ; i++) {
    gf16_mat_init(Mi + i, SIGN_MIRA_256_PARAM_M, SIGN_MIRA_256_PARAM_N);
  }

  // Setup variables related to MPC
  gf16_mat x[SIGN_MIRA_256_PARAM_TAU];
  gfqm_vec beta[SIGN_MIRA_256_PARAM_TAU];
  gfqm_vec a[SIGN_MIRA_256_PARAM_TAU];
  gfqm c[SIGN_MIRA_256_PARAM_TAU];

  for(size_t e = 0; e < SIGN_MIRA_256_PARAM_TAU; e++) {
      gf16_mat_init(&(x[e]), 1, SIGN_MIRA_256_PARAM_K);
      gfqm_vec_init(&(beta[e]), SIGN_MIRA_256_PARAM_R);
      gfqm_vec_init(&(a[e]), SIGN_MIRA_256_PARAM_R);
      gfqm_set_zero(c[e]);
  }

  uint8_t cmt[SIGN_MIRA_256_PARAM_COMMITMENT_BYTES * SIGN_MIRA_256_PARAM_TAU] = {0};

  sign_mira_256_shares_t shares;
  sign_mira_256_init_shares(&shares);
  sign_mira_256_main_shares_t main_shares[SIGN_MIRA_256_PARAM_TAU];

  uint8_t response1_str[(SIGN_MIRA_256_VEC_R_BYTES + SIGN_MIRA_256_PARAM_M_BYTES) * SIGN_MIRA_256_PARAM_D] = {0};
  sign_mira_256_response1_t response1[SIGN_MIRA_256_PARAM_TAU];

  // Parse public key
  sign_mira_256_public_key_from_string(M0, Mi, pk);

  // Hash message
  domain_separator = DOMAIN_SEPARATOR_MESSAGE;
  hash_sha3_absorb(&ctx_m, &domain_separator, sizeof(uint8_t));
  hash_sha3_absorb(&ctx_m, message, message_size);
  hash_sha3_finalize(m_digest, &ctx_m);

  // ---------------------------------------------------------------------------------------------- Recompute challenges

  // Recompute first challenge
  memcpy(h1_, &signature[2 * SIGN_MIRA_256_SECURITY_BYTES], 2 * SIGN_MIRA_256_SECURITY_BYTES);
  for(size_t e = 0; e < SIGN_MIRA_256_PARAM_TAU; e++) { sign_mira_256_init_challenge1(&(challenge1[e])); }
  sign_mira_256_compute_challenge1(challenge1, h1_, salt);

  // Recompute second challenge
  memcpy(h2_, &signature[4 * SIGN_MIRA_256_SECURITY_BYTES], 2 * SIGN_MIRA_256_SECURITY_BYTES);
  hash_shake((uint8_t *) challenge2, SIGN_MIRA_256_PARAM_TAU, h2_, 2 * SIGN_MIRA_256_SECURITY_BYTES);

  // ------------------------------------------- If the challenge2 is (RYDE_PARAM_N_MPC - 1) we verify the STATE is zero
  uint8_t zero_state[SIGN_MIRA_256_PARAM_STATE_BYTES] = {0};
  for(size_t e = 0; e < SIGN_MIRA_256_PARAM_TAU; e++) {
    uint32_t index = 6 * SIGN_MIRA_256_SECURITY_BYTES + (SIGN_MIRA_256_PARAM_RESPONSE_BYTES * e) + SIGN_MIRA_256_PARAM_TREE_PATH_BYTES + 2 * SIGN_MIRA_256_SECURITY_BYTES;
    challenge2[e] &= (uint8_t)(SIGN_MIRA_256_PARAM_N_MPC - 1);
    if (challenge2[e] == (uint8_t)(SIGN_MIRA_256_PARAM_N_MPC - 1)) {
      if (memcmp(zero_state,  &signature[index + SIGN_MIRA_256_VEC_R_BYTES], SIGN_MIRA_256_PARAM_STATE_BYTES) != 0) {

        sign_mira_256_clear_shares(&shares);

        for(size_t e = 0; e < SIGN_MIRA_256_PARAM_TAU; e++) {
          sign_mira_256_clear_challenge1(&(challenge1[e]));
          gf16_mat_clear(x[e]);
          gfqm_vec_clear(beta[e]);
          gfqm_vec_clear(a[e]);
          gfqm_set_zero(c[e]);
        }

        gf16_mat_clear(M0);

        return EXIT_FAILURE;
      }
    }
  }


  // ------------------------------------------------------------------------------------------------------ Recompute h1

  // Add m, pk and salt to h1
  domain_separator = DOMAIN_SEPARATOR_HASH1;
  hash_sha3_absorb(&ctx_h1, &domain_separator, sizeof(uint8_t));
  hash_sha3_absorb(&ctx_h1, m_digest, 2 * SIGN_MIRA_256_SECURITY_BYTES);
  hash_sha3_absorb(&ctx_h1, pk, SIGN_MIRA_256_PUBLIC_KEY_BYTES);
  hash_sha3_absorb(&ctx_h1, salt, 2 * SIGN_MIRA_256_SECURITY_BYTES);

  for(size_t e = 0; e < SIGN_MIRA_256_PARAM_TAU; e++) {
    uint8_t theta_i[SIGN_MIRA_256_PARAM_TREE_LEAF_BYTES] = {0};
    challenge2[e] &= (uint8_t)(SIGN_MIRA_256_PARAM_N_MPC - 1);

    // Recompute commitments com_i
    sign_mira_256_recompute_commitments(&cmt[SIGN_MIRA_256_PARAM_COMMITMENT_BYTES * e], theta_i, e, salt,
                                         &signature[6 * SIGN_MIRA_256_SECURITY_BYTES + (SIGN_MIRA_256_PARAM_RESPONSE_BYTES * e)], challenge2[e]);

    hash_sha3_absorb(&ctx_h1, &cmt[SIGN_MIRA_256_PARAM_COMMITMENT_BYTES * e], SIGN_MIRA_256_PARAM_COMMITMENT_BYTES);

    // Recompute (leaf party) shares
    sign_mira_256_recompute_shares(&shares, theta_i, &signature[6 * SIGN_MIRA_256_SECURITY_BYTES + (SIGN_MIRA_256_PARAM_RESPONSE_BYTES * e) + SIGN_MIRA_256_PARAM_TREE_PATH_BYTES + 2 * SIGN_MIRA_256_SECURITY_BYTES + SIGN_MIRA_256_VEC_R_BYTES], challenge2[e], salt);

    memset(theta_i, 0, SIGN_MIRA_256_PARAM_TREE_LEAF_BYTES);

    // Recompute (main party) shares
    sign_mira_256_init_main_shares(&(main_shares[e]));
    sign_mira_256_recompute_main_shares(&(main_shares[e]), &shares, challenge2[e]);

    // We additional vectors required to recover the plain alpha
    sign_mira_256_recompute_additional_main_share(x[e], beta[e], a[e], c[e], &shares, challenge2[e]);
  }
  sign_mira_256_clear_shares(&shares);

  // Recompute h1
  hash_sha3_finalize(h1, &ctx_h1);

  // ------------------------------------------------------------------------------------------------------ Recompute h2

  // Add m, pk, salt and h1 to h2
  domain_separator = DOMAIN_SEPARATOR_HASH2;
  hash_sha3_absorb(&ctx_h2, &domain_separator, sizeof(uint8_t));
  hash_sha3_absorb(&ctx_h2, m_digest, 2 * SIGN_MIRA_256_SECURITY_BYTES);
  hash_sha3_absorb(&ctx_h2, pk, SIGN_MIRA_256_PUBLIC_KEY_BYTES);
  hash_sha3_absorb(&ctx_h2, salt, 2 * SIGN_MIRA_256_SECURITY_BYTES);
  hash_sha3_absorb(&ctx_h2, h1, 2 * SIGN_MIRA_256_SECURITY_BYTES);

  for(size_t e = 0; e < SIGN_MIRA_256_PARAM_TAU; e++) {
    challenge2[e] &= (uint8_t)(SIGN_MIRA_256_PARAM_N_MPC - 1);

    // Recompute response1
    sign_mira_256_init_response1(&(response1[e]));
    sign_mira_256_add_alpha_ch2(&(response1[e]), &signature[6 * SIGN_MIRA_256_SECURITY_BYTES + (SIGN_MIRA_256_PARAM_RESPONSE_BYTES * e) +
                                                             SIGN_MIRA_256_PARAM_TREE_PATH_BYTES + 2 * SIGN_MIRA_256_SECURITY_BYTES ], challenge2[e]);

    sign_mira_256_recompute_response1(&(response1[e]), &(main_shares[e]), &(challenge1[e]), x[e], beta[e], a[e], c[e], M0, Mi, challenge2[e]);
   
    // Add alpha, [alpha] and [v] to h2
    for(size_t i = 0; i < SIGN_MIRA_256_PARAM_D; i++ ) {
        gfqm_vec_to_string(&response1_str[(SIGN_MIRA_256_VEC_R_BYTES + SIGN_MIRA_256_PARAM_M_BYTES) * i], response1[e].alpha[i], SIGN_MIRA_256_PARAM_R);
        gfqm_to_string(&response1_str[(SIGN_MIRA_256_VEC_R_BYTES + SIGN_MIRA_256_PARAM_M_BYTES) * i + SIGN_MIRA_256_VEC_R_BYTES], response1[e].v[i]);
    }

    uint8_t plain_alpha[SIGN_MIRA_256_VEC_R_BYTES] = {0};
    gfqm_vec_to_string(plain_alpha, response1[e].alpha_, SIGN_MIRA_256_PARAM_R);
    hash_sha3_absorb(&ctx_h2, plain_alpha, SIGN_MIRA_256_VEC_R_BYTES);
    hash_sha3_absorb(&ctx_h2, response1_str, (SIGN_MIRA_256_VEC_R_BYTES + SIGN_MIRA_256_PARAM_M_BYTES) * SIGN_MIRA_256_PARAM_D);
  }

  // Recompute h2
  hash_sha3_finalize(h2, &ctx_h2);

  #ifdef VERBOSE
    printf("\nchallenge1:");
    for(uint8_t e = 0; e < SIGN_MIRA_256_PARAM_TAU; e++) {
      printf("\n  Iteration %" PRIu8 ":", e);

      uint8_t challenge1_gamma_string[SIGN_MIRA_256_VEC_N_BYTES] = {0};
      gfqm_vec_to_string(challenge1_gamma_string, challenge1[e].gamma, SIGN_MIRA_256_PARAM_N);
      printf("\n    - gamma   : "); for(size_t i = 0 ; i < SIGN_MIRA_256_VEC_N_BYTES ; i++) { printf("%02x", challenge1_gamma_string[i]); }
      uint8_t challenge1_epsilon_string[SIGN_MIRA_256_PARAM_M_BYTES] = {0};
      gfqm_to_string(challenge1_epsilon_string, challenge1[e].epsilon);
      printf("\n    - epsilon : "); for(size_t i = 0 ; i < SIGN_MIRA_256_PARAM_M_BYTES ; i++) { printf("%02x", challenge1_epsilon_string[i]); }
    }

    printf("\nchallenge2: "); for(size_t i = 0 ; i < SIGN_MIRA_256_PARAM_TAU ; i++) printf("%" PRIu8 " ", challenge2[i]);

    printf("\nh1 : "); for(size_t i = 0; i < (2 * SIGN_MIRA_256_SECURITY_BYTES); i++) { printf("%02x", h1[i]); }
    printf("\nh1_: "); for(size_t i = 0; i < (2 * SIGN_MIRA_256_SECURITY_BYTES); i++) { printf("%02x", h1_[i]); }
    printf("\nh2 : "); for(size_t i = 0; i < (2 * SIGN_MIRA_256_SECURITY_BYTES); i++) { printf("%02x", h2[i]); }
    printf("\nh2_: "); for(size_t i = 0; i < (2 * SIGN_MIRA_256_SECURITY_BYTES); i++) { printf("%02x", h2_[i]); }
    printf("\n");
  #endif

  // -------------------------------------------------------------------------------------------------------- Clear Data

  memset(cmt, 0, SIGN_MIRA_256_PARAM_COMMITMENT_BYTES * SIGN_MIRA_256_PARAM_TAU);
  memset(response1_str, 0, (SIGN_MIRA_256_VEC_R_BYTES + SIGN_MIRA_256_PARAM_M_BYTES) * SIGN_MIRA_256_PARAM_D);

  for(size_t e = 0; e < SIGN_MIRA_256_PARAM_TAU; e++) {
    sign_mira_256_clear_challenge1(&(challenge1[e]));
    sign_mira_256_clear_main_shares(&(main_shares[e]));
    sign_mira_256_clear_response1(&(response1[e]));
    gf16_mat_clear(x[e]);
    gfqm_vec_clear(beta[e]);
    gfqm_vec_clear(a[e]);
    gfqm_set_zero(c[e]);
  }

  gf16_mat_clear(M0);
  for(int i=0 ; i<SIGN_MIRA_256_PARAM_K ; i++) { gf16_mat_clear(Mi[i]); }

  // ------------------------------------------------------------------------------ Validate reconstruction of h1 and h2
  if (memcmp(h1, h1_, 2*SIGN_MIRA_256_SECURITY_BYTES) != 0 || memcmp(h2, h2_, 2*SIGN_MIRA_256_SECURITY_BYTES) != 0) {
    memset(h1, 0, 2 * SIGN_MIRA_256_SECURITY_BYTES);
    memset(h2, 0, 2 * SIGN_MIRA_256_SECURITY_BYTES);
    memset(h1_, 0, 2 * SIGN_MIRA_256_SECURITY_BYTES);
    memset(h2_, 0, 2 * SIGN_MIRA_256_SECURITY_BYTES);

    return EXIT_FAILURE;
  }
  else {
    memset(h1, 0, 2 * SIGN_MIRA_256_SECURITY_BYTES);
    memset(h2, 0, 2 * SIGN_MIRA_256_SECURITY_BYTES);
    memset(h1_, 0, 2 * SIGN_MIRA_256_SECURITY_BYTES);
    memset(h2_, 0, 2 * SIGN_MIRA_256_SECURITY_BYTES);

    return EXIT_SUCCESS;
  }

  return EXIT_FAILURE;
}
