/** 
 * \file ryde_192f_verify.c
 * \brief Implementation of verify.h
 */

#include "string.h"
#include "hash_fips202.h"
#include "parameters.h"
#include "parsing.h"
#include "mpc.h"
#include "verification.h"



/**
 * \fn int ryde_192f_verify(const uint8_t* message, size_t message_size, const uint8_t* signature, size_t signature_size, const uint8_t* pk)
 * \brief Verify of the RYDE scheme
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
int ryde_192f_verify(const uint8_t* signature, size_t signature_size, const uint8_t* message, size_t message_size, const uint8_t* pk) {

  #ifdef VERBOSE
    printf("\n### VERIFY ###\n");
  #endif

  // ------------------------------------------------------------------------------------------------------------- Setup
  rbc_37_field_init();

  if(signature == NULL || message == NULL || pk == NULL) { return EXIT_FAILURE; }
  if(signature_size != RYDE_192F_SIGNATURE_BYTES) { return EXIT_FAILURE; }

  // Setup variables related to randomness, salt, hash and challenges
  uint8_t salt[2 * RYDE_192F_SECURITY_BYTES] = {0};
  memcpy(salt, &signature[0], 2 * RYDE_192F_SECURITY_BYTES);

  uint8_t domain_separator;
  uint8_t h1[2 * RYDE_192F_SECURITY_BYTES] = {0};
  uint8_t h2[2 * RYDE_192F_SECURITY_BYTES] = {0};
  uint8_t h1_[2 * RYDE_192F_SECURITY_BYTES] = {0};
  uint8_t h2_[2 * RYDE_192F_SECURITY_BYTES] = {0};

  hash_sha3_ctx ctx_m, ctx_h1, ctx_h2;
  hash_sha3_init(&ctx_m);
  hash_sha3_init(&ctx_h1);
  hash_sha3_init(&ctx_h2);

  uint8_t m_digest[2 * RYDE_192F_SECURITY_BYTES] = {0};
  ryde_192f_challenge1_t challenge1[RYDE_192F_PARAM_TAU];
  uint8_t challenge2[RYDE_192F_PARAM_TAU] = {0};

  // Setup variables related to pk
  rbc_37_mat H;
  rbc_37_vec y;

  rbc_37_mat_init(&H, RYDE_192F_PARAM_N - RYDE_192F_PARAM_K, RYDE_192F_PARAM_K);
  rbc_37_vec_init(&y, RYDE_192F_PARAM_N - RYDE_192F_PARAM_K);

  // Setup variables related to MPC: sum of all leaf shares excluding the challenge-th leaf share
  rbc_37_vec x1[RYDE_192F_PARAM_TAU];
  rbc_37_vec x2[RYDE_192F_PARAM_TAU];
  rbc_37_vec a[RYDE_192F_PARAM_TAU];
  rbc_37_vec u[RYDE_192F_PARAM_TAU];
  rbc_37_elt c[RYDE_192F_PARAM_TAU];

  for(size_t e = 0; e < RYDE_192F_PARAM_TAU; e++) {
      rbc_37_vec_init(&(x1[e]), RYDE_192F_PARAM_N - RYDE_192F_PARAM_K);
      rbc_37_vec_init(&(x2[e]), RYDE_192F_PARAM_K);
      rbc_37_vec_init(&(a[e]), RYDE_192F_PARAM_W - 1);
      rbc_37_vec_init(&(u[e]), RYDE_192F_PARAM_W - 1);
      rbc_37_elt_set_zero(c[e]);
  }

  uint8_t cmt[RYDE_192F_PARAM_COMMITMENT_BYTES * RYDE_192F_PARAM_TAU] = {0};

  ryde_192f_leaf_shares_t shares;
  ryde_192f_init_leaf_shares(&shares);
  ryde_192f_main_shares_t main_shares[RYDE_192F_PARAM_TAU];

  uint8_t response1_str[RYDE_192F_PARAM_W * RYDE_192F_PARAM_M_BYTES * RYDE_192F_PARAM_D] = {0};
  ryde_192f_response1_t response1[RYDE_192F_PARAM_TAU];

  // Parse public key
  ryde_192f_public_key_from_string(H, y, pk);

  // Hash message
  domain_separator = DOMAIN_SEPARATOR_MESSAGE;
  hash_sha3_absorb(&ctx_m, &domain_separator, sizeof(uint8_t));
  hash_sha3_absorb(&ctx_m, message, message_size);
  hash_sha3_finalize(m_digest, &ctx_m);

  // ---------------------------------------------------------------------------------------------- Recompute challenges

  // Recompute first challenge
  memcpy(h1_, &signature[2 * RYDE_192F_SECURITY_BYTES], 2 * RYDE_192F_SECURITY_BYTES);
  for(size_t e = 0; e < RYDE_192F_PARAM_TAU; e++) { ryde_192f_init_challenge1(&(challenge1[e])); }
  ryde_192f_compute_challenge1(challenge1, h1_, salt);

  // Recompute second challenge
  memcpy(h2_, &signature[4 * RYDE_192F_SECURITY_BYTES], 2 * RYDE_192F_SECURITY_BYTES);
  hash_shake((uint8_t *)challenge2, RYDE_192F_PARAM_TAU, h2_, 2 * RYDE_192F_SECURITY_BYTES);

  // ------------------------------------------- If the challenge2 is (RYDE_192F_PARAM_N_MPC - 1) we verify the STATE is zero
  uint8_t zero_state[RYDE_192F_PARAM_STATE_BYTES] = {0};
  for(size_t e = 0; e < RYDE_192F_PARAM_TAU; e++) {
    uint32_t index = 6 * RYDE_192F_SECURITY_BYTES + (RYDE_192F_PARAM_RESPONSE_BYTES * e) + RYDE_192F_PARAM_TREE_PATH_BYTES + 2 * RYDE_192F_SECURITY_BYTES;
    challenge2[e] &= (uint8_t)(RYDE_192F_PARAM_N_MPC - 1);
    if (challenge2[e] == (uint8_t)(RYDE_192F_PARAM_N_MPC - 1)) {
        if (memcmp(zero_state,  &signature[index + RYDE_192F_VEC_W_MINUS_ONE_BYTES], RYDE_192F_PARAM_STATE_BYTES) != 0) {

          ryde_192f_clear_leaf_shares(&shares);
          for(size_t i = 0; i < RYDE_192F_PARAM_TAU; i++) {
            ryde_192f_clear_challenge1(&(challenge1[i]));
            rbc_37_vec_clear(x1[i]);
            rbc_37_vec_clear(x2[i]);
            rbc_37_vec_clear(a[i]);
            rbc_37_vec_clear(u[i]);
            rbc_37_elt_set_zero(c[i]);
          }
          rbc_37_mat_clear(H);
          rbc_37_vec_clear(y);

          return EXIT_FAILURE;
        }
    }
  }

  // ------------------------------------------------------------------------------------------------------ Recompute h1

  // Add m, pk and salt to h1
  domain_separator = DOMAIN_SEPARATOR_HASH1;
  hash_sha3_absorb(&ctx_h1, &domain_separator, sizeof(uint8_t));
  hash_sha3_absorb(&ctx_h1, m_digest, 2 * RYDE_192F_SECURITY_BYTES);
  hash_sha3_absorb(&ctx_h1, pk, RYDE_192F_PUBLIC_KEY_BYTES);
  hash_sha3_absorb(&ctx_h1, salt, 2 * RYDE_192F_SECURITY_BYTES);

  for(size_t e = 0; e < RYDE_192F_PARAM_TAU; e++) {
    uint8_t theta_i[RYDE_192F_PARAM_TREE_LEAF_BYTES] = {0};
    challenge2[e] &= (uint8_t)(RYDE_192F_PARAM_N_MPC - 1);

    // Recompute commitments com_i
    ryde_192f_recompute_commitments(&cmt[RYDE_192F_PARAM_COMMITMENT_BYTES * e], theta_i, e, salt,
                                         &signature[6 * RYDE_192F_SECURITY_BYTES + (RYDE_192F_PARAM_RESPONSE_BYTES * e)], challenge2[e]);

    hash_sha3_absorb(&ctx_h1, &cmt[RYDE_192F_PARAM_COMMITMENT_BYTES * e], RYDE_192F_PARAM_COMMITMENT_BYTES);

    // Recompute (leaf party) shares
    ryde_192f_recompute_shares(&shares, y, H, theta_i, &signature[6 * RYDE_192F_SECURITY_BYTES + (RYDE_192F_PARAM_RESPONSE_BYTES * e) +
                                    RYDE_192F_PARAM_TREE_PATH_BYTES + 2 * RYDE_192F_SECURITY_BYTES + RYDE_192F_VEC_W_MINUS_ONE_BYTES], challenge2[e], salt);

    memset(theta_i, 0, RYDE_192F_PARAM_TREE_LEAF_BYTES);

    // Recompute (main party) shares
    ryde_192f_init_main_shares(&(main_shares[e]));
    ryde_192f_recompute_main_shares(&(main_shares[e]), &shares, challenge2[e]);

    // We additional vectors required to recover the plain alpha
    ryde_192f_recompute_additional_main_share(x1[e], x2[e], a[e], u[e], c[e], &shares, challenge2[e]);
  }
  ryde_192f_clear_leaf_shares(&shares);

  // Recompute h1
  hash_sha3_finalize(h1, &ctx_h1);

  // ------------------------------------------------------------------------------------------------------ Recompute h2

  // Add m, pk, salt and h1 to h2
  domain_separator = DOMAIN_SEPARATOR_HASH2;
  hash_sha3_absorb(&ctx_h2, &domain_separator, sizeof(uint8_t));
  hash_sha3_absorb(&ctx_h2, m_digest, 2 * RYDE_192F_SECURITY_BYTES);
  hash_sha3_absorb(&ctx_h2, pk, RYDE_192F_PUBLIC_KEY_BYTES);
  hash_sha3_absorb(&ctx_h2, salt, 2 * RYDE_192F_SECURITY_BYTES);
  hash_sha3_absorb(&ctx_h2, h1, 2 * RYDE_192F_SECURITY_BYTES);

  for(size_t e = 0; e < RYDE_192F_PARAM_TAU; e++) {
    challenge2[e] &= (uint8_t)(RYDE_192F_PARAM_N_MPC - 1);

    // Recompute response1
    ryde_192f_init_response1(&(response1[e]));
    ryde_192f_add_alpha_ch2(&(response1[e]), &signature[6 * RYDE_192F_SECURITY_BYTES + (RYDE_192F_PARAM_RESPONSE_BYTES * e) +
                                                             RYDE_192F_PARAM_TREE_PATH_BYTES + 2 * RYDE_192F_SECURITY_BYTES ], challenge2[e]);
    ryde_192f_recompute_response1(&(response1[e]), &(main_shares[e]), &(challenge1[e]), x1[e], x2[e], a[e], u[e], c[e], challenge2[e]);

    // Add alpha, [alpha] and [v] to h2
    for(size_t i = 0; i < RYDE_192F_PARAM_D; i++ ) {
        rbc_37_vec_to_bytes(&response1_str[RYDE_192F_PARAM_W * RYDE_192F_PARAM_M_BYTES * i], response1[e].alpha[i], RYDE_192F_PARAM_W - 1);
        memcpy(&response1_str[RYDE_192F_VEC_W_MINUS_ONE_BYTES + (RYDE_192F_PARAM_W * RYDE_192F_PARAM_M_BYTES * i)], response1[e].v[i], RYDE_192F_PARAM_M_BYTES);
    }

    uint8_t plain_alpha[(RYDE_192F_PARAM_W - 1) * RYDE_192F_PARAM_M_BYTES] = {0};
    rbc_37_vec_to_bytes(plain_alpha, response1[e].alpha_, RYDE_192F_PARAM_W - 1);
    hash_sha3_absorb(&ctx_h2, plain_alpha, (RYDE_192F_PARAM_W - 1) * RYDE_192F_PARAM_M_BYTES);
    hash_sha3_absorb(&ctx_h2, response1_str, RYDE_192F_PARAM_W * RYDE_192F_PARAM_M_BYTES * RYDE_192F_PARAM_D);
  }

  // Recompute h2
  hash_sha3_finalize(h2, &ctx_h2);

  // -------------------------------------------------------------------------------------------------------- Clear Data
  #ifdef VERBOSE
    printf("\nchallenge1:");
    for(uint8_t e = 0; e < RYDE_192F_PARAM_TAU; e++) {
      printf("\n  Iteration %" PRIu8 ":", e);

      uint8_t challenge1_gamma_string[RYDE_192F_VEC_N_BYTES] = {0};
      rbc_37_vec_to_string(challenge1_gamma_string, challenge1[e].gamma, RYDE_192F_PARAM_N);
      printf("\n    - gamma   : "); for(size_t i = 0 ; i < RYDE_192F_VEC_N_BYTES ; i++) { printf("%02x", challenge1_gamma_string[i]); }
      uint8_t challenge1_epsilon_string[RYDE_192F_PARAM_M_BYTES] = {0};
      rbc_37_elt_to_string(challenge1_epsilon_string, challenge1[e].epsilon);
      printf("\n    - epsilon : "); for(size_t i = 0 ; i < RYDE_192F_PARAM_M_BYTES ; i++) { printf("%02x", challenge1_epsilon_string[i]); }
    }

    printf("\nchallenge2: "); for(size_t i = 0 ; i < RYDE_192F_PARAM_TAU ; i++) printf("%" PRIu8 " ", challenge2[i]);

    printf("\nh1 : "); for(size_t i = 0; i < (2 * RYDE_192F_SECURITY_BYTES); i++) { printf("%02x", h1[i]); }
    printf("\nh1_: "); for(size_t i = 0; i < (2 * RYDE_192F_SECURITY_BYTES); i++) { printf("%02x", h1_[i]); }
    printf("\nh2 : "); for(size_t i = 0; i < (2 * RYDE_192F_SECURITY_BYTES); i++) { printf("%02x", h2[i]); }
    printf("\nh2_: "); for(size_t i = 0; i < (2 * RYDE_192F_SECURITY_BYTES); i++) { printf("%02x", h2_[i]); }
    printf("\n");
  #endif

  memset(cmt, 0, RYDE_192F_PARAM_COMMITMENT_BYTES * RYDE_192F_PARAM_TAU);
  memset(response1_str, 0, RYDE_192F_PARAM_W * RYDE_192F_PARAM_M_BYTES * RYDE_192F_PARAM_D);

  for(size_t e = 0; e < RYDE_192F_PARAM_TAU; e++) {
    ryde_192f_clear_challenge1(&(challenge1[e]));
    ryde_192f_clear_main_shares(&(main_shares[e]));
    ryde_192f_clear_response1(&(response1[e]));
    rbc_37_vec_clear(x1[e]);
    rbc_37_vec_clear(x2[e]);
    rbc_37_vec_clear(a[e]);
    rbc_37_vec_clear(u[e]);
    rbc_37_elt_set_zero(c[e]);
  }

  rbc_37_mat_clear(H);
  rbc_37_vec_clear(y);

  // ------------------------------------------------------------------------------ Validate reconstruction of h1 and h2
  if (memcmp(h1, h1_, 2*RYDE_192F_SECURITY_BYTES) != 0 || memcmp(h2, h2_, 2*RYDE_192F_SECURITY_BYTES) != 0) {
    memset(h1, 0, 2 * RYDE_192F_SECURITY_BYTES);
    memset(h2, 0, 2 * RYDE_192F_SECURITY_BYTES);
    memset(h1_, 0, 2 * RYDE_192F_SECURITY_BYTES);
    memset(h2_, 0, 2 * RYDE_192F_SECURITY_BYTES);

    return EXIT_FAILURE;
  }
  else {
    memset(h1, 0, 2 * RYDE_192F_SECURITY_BYTES);
    memset(h2, 0, 2 * RYDE_192F_SECURITY_BYTES);
    memset(h1_, 0, 2 * RYDE_192F_SECURITY_BYTES);
    memset(h2_, 0, 2 * RYDE_192F_SECURITY_BYTES);

    return EXIT_SUCCESS;
  }
}
