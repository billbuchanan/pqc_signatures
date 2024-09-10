/** 
 * @file ryde_192s_sign.c
 * @brief Implementation of sign.h
 */

#include "string.h"
#include <assert.h>
#include "randombytes.h"
#include "hash_fips202.h"
#include "parameters.h"
#include "parsing.h"
#include "mpc.h"
#include "tree.h"
#include "signature.h"



/**
 * \fn int ryde_192s_sign(uint8_t* signature, const uint8_t* message, size_t message_size, const uint8_t* sk)
 * \brief Sign algorithm of the RYDE scheme
 *
 * \param[out] signature String containing the signature
 * \param[in] message String containing the message to be signed
 * \param[in] message_size Length of the message to be signed
 * \param[in] sk String containing the secret key
 * \return EXIT_SUCCESS if verify is successful. Otherwise, it returns EXIT_FAILURE.
 */
int ryde_192s_sign(uint8_t* signature, const uint8_t* message, size_t message_size, const uint8_t* sk) {

  #ifdef VERBOSE
    printf("\n\n### SIGN ###\n");
  #endif

  // ------------------------------------------------------------------------------------------------------------- Setup
  rbc_37_field_init();

  if (message == NULL || signature == NULL || sk == NULL) { return EXIT_FAILURE; }
  if (message_size <= 0) { return EXIT_FAILURE; }
  memset(signature, 0, RYDE_192S_SIGNATURE_BYTES);

  // Setup variables related to randomness, hash and challenges
  uint8_t salt_and_mseed[3 * RYDE_192S_SECURITY_BYTES] = {0};
  uint8_t salt[2 * RYDE_192S_SECURITY_BYTES] = {0};

  uint8_t domain_separator;
  uint8_t h1[2 * RYDE_192S_SECURITY_BYTES] = {0};
  uint8_t h2[2 * RYDE_192S_SECURITY_BYTES] = {0};
  hash_sha3_ctx ctx_m, ctx_h1, ctx_h2;
  hash_sha3_init(&ctx_m);
  hash_sha3_init(&ctx_h1);
  hash_sha3_init(&ctx_h2);

  uint8_t m_digest[2 * RYDE_192S_SECURITY_BYTES] = {0};
  ryde_192s_challenge1_t challenge1[RYDE_192S_PARAM_TAU];
  for(size_t e = 0; e < RYDE_192S_PARAM_TAU; e++) { ryde_192s_init_challenge1(&(challenge1[e])); }
  uint8_t challenge2[RYDE_192S_PARAM_TAU] = {0};

  // Setup variables related to sk and pk
  uint8_t pk[RYDE_192S_PUBLIC_KEY_BYTES] = {0};
  rbc_37_vec x1, x2;
  rbc_37_qpoly A;
  rbc_37_mat H;
  rbc_37_vec y;

  rbc_37_vec_init(&x1, RYDE_192S_PARAM_N - RYDE_192S_PARAM_K);
  rbc_37_vec_init(&x2, RYDE_192S_PARAM_K);
  rbc_37_qpoly_init(&A, RYDE_192S_PARAM_W);
  rbc_37_mat_init(&H, RYDE_192S_PARAM_N - RYDE_192S_PARAM_K, RYDE_192S_PARAM_K);
  rbc_37_vec_init(&y, RYDE_192S_PARAM_N - RYDE_192S_PARAM_K);

  // Setup variables related to MPC
  uint8_t theta[RYDE_192S_PARAM_TAU * RYDE_192S_SECURITY_BYTES] = {0};
  uint8_t theta_i[RYDE_192S_PARAM_TREE_LEAF_BYTES * RYDE_192S_PARAM_TAU] = {0};
  uint8_t cmt_i[RYDE_192S_PARAM_COMMITMENT_BYTES * RYDE_192S_PARAM_TAU] = {0};

  ryde_192s_leaf_shares_t shares;
  ryde_192s_init_leaf_shares(&shares);
  ryde_192s_main_shares_t main_shares[RYDE_192S_PARAM_TAU];

  rbc_37_vec x1_N[RYDE_192S_PARAM_TAU];
  rbc_37_vec x2_N[RYDE_192S_PARAM_TAU];
  rbc_37_vec a_N[RYDE_192S_PARAM_TAU];
  rbc_37_vec u_N[RYDE_192S_PARAM_TAU];
  rbc_37_vec u[RYDE_192S_PARAM_TAU];
  rbc_37_elt c_N[RYDE_192S_PARAM_TAU];

  rbc_37_vec tmp1, tmp2, tmp3;
  rbc_37_vec_init(&tmp1, RYDE_192S_PARAM_N - RYDE_192S_PARAM_K);
  rbc_37_vec_init(&tmp2, RYDE_192S_PARAM_K);
  rbc_37_vec_init(&tmp3, RYDE_192S_PARAM_W - 1);
  for(size_t e = 0; e < RYDE_192S_PARAM_TAU; e++) { rbc_37_vec_init(&(u[e]), RYDE_192S_PARAM_W - 1); }

  uint8_t response1_str[RYDE_192S_PARAM_W * RYDE_192S_PARAM_M_BYTES * RYDE_192S_PARAM_D] = {0};
  ryde_192s_response1_t response1[RYDE_192S_PARAM_TAU];

  // Parse secret key and public key
  ryde_192s_secret_key_from_string(y, H, x1, x2, A, sk);
  ryde_192s_public_key_to_string(pk, &sk[RYDE_192S_SECURITY_BYTES], y);

  // Hash message
  domain_separator = DOMAIN_SEPARATOR_MESSAGE;
  hash_sha3_absorb(&ctx_m, &domain_separator, sizeof(uint8_t));
  hash_sha3_absorb(&ctx_m, message, message_size);
  hash_sha3_finalize(m_digest, &ctx_m);

  // -------------------------------------------------------------------------------------------------------- Commitment

  // Sample salt, theta and parse signature
  if (randombytes(salt_and_mseed, 3 * RYDE_192S_SECURITY_BYTES) != EXIT_SUCCESS) {

    ryde_192s_clear_leaf_shares(&shares);
    for(size_t e = 0; e < RYDE_192S_PARAM_TAU; e++) {
      rbc_37_vec_clear(u[e]);
      ryde_192s_clear_challenge1(&(challenge1[e]));
    }

    rbc_37_vec_clear(tmp1);
    rbc_37_vec_clear(tmp2);
    rbc_37_vec_clear(tmp3);

    rbc_37_vec_clear(x1);
    rbc_37_vec_clear(x2);
    rbc_37_qpoly_clear(A);
    rbc_37_mat_clear(H);
    rbc_37_vec_clear(y);

    memset(salt_and_mseed, 0, 3 * RYDE_192S_SECURITY_BYTES);
    return EXIT_FAILURE;
  }

  hash_shake(theta, RYDE_192S_SECURITY_BYTES * RYDE_192S_PARAM_TAU, salt_and_mseed, 3 * RYDE_192S_SECURITY_BYTES);
  memcpy(&salt, salt_and_mseed, 2 * RYDE_192S_SECURITY_BYTES);
  memcpy(&signature[0], salt, 2 * RYDE_192S_SECURITY_BYTES);

  // Add m, pk and salt to h1
  domain_separator = DOMAIN_SEPARATOR_HASH1;
  hash_sha3_absorb(&ctx_h1, &domain_separator, sizeof(uint8_t));
  hash_sha3_absorb(&ctx_h1, m_digest, 2 * RYDE_192S_SECURITY_BYTES);
  hash_sha3_absorb(&ctx_h1, pk, RYDE_192S_PUBLIC_KEY_BYTES);
  hash_sha3_absorb(&ctx_h1, salt, 2 * RYDE_192S_SECURITY_BYTES);

  // Generate and commit to shares
  rbc_37_elt c[RYDE_192S_PARAM_TAU];
  for(size_t e = 0; e < RYDE_192S_PARAM_TAU; e++) {
    // Generate seed tree
    ryde_192s_seed_tree_t seed_tree = {0};
    memcpy(seed_tree[0], &theta[RYDE_192S_SECURITY_BYTES * e], RYDE_192S_SECURITY_BYTES);
    ryde_192s_tree_expand(seed_tree, salt, e);
    memcpy(&theta_i[RYDE_192S_PARAM_TREE_LEAF_BYTES * e], &seed_tree[RYDE_192S_PARAM_N_MPC - 1], RYDE_192S_PARAM_TREE_LEAF_BYTES);

    // Generate (leaf party) shares
    ryde_192s_compute_leaf_shares(&shares, y, H, x2, A->values, &theta_i[RYDE_192S_PARAM_TREE_LEAF_BYTES * e], salt);

    // Commit to (leaf party) shares
    ryde_192s_commit_to_shares(&cmt_i[RYDE_192S_PARAM_COMMITMENT_BYTES * e], e, salt, &shares, &theta_i[RYDE_192S_PARAM_TREE_LEAF_BYTES * e]);

    // Add cmt_i_i to h1
    hash_sha3_absorb(&ctx_h1, &cmt_i[RYDE_192S_PARAM_COMMITMENT_BYTES * e], RYDE_192S_PARAM_COMMITMENT_BYTES);

    // Generate (main party) shares
    ryde_192s_init_main_shares(&(main_shares[e]));
    ryde_192s_compute_main_shares(&(main_shares[e]), &shares);

    // Store last (leaf party) share and u
    rbc_37_vec_init(&(x1_N[e]), RYDE_192S_PARAM_N - RYDE_192S_PARAM_K);
    rbc_37_vec_init(&(x2_N[e]), RYDE_192S_PARAM_K);
    rbc_37_vec_init(&(a_N[e]), RYDE_192S_PARAM_W - 1);
    rbc_37_vec_init(&(u_N[e]), RYDE_192S_PARAM_W - 1);

    rbc_37_vec_set(x1_N[e], shares.x1[RYDE_192S_PARAM_N_MPC - 1], RYDE_192S_PARAM_N - RYDE_192S_PARAM_K);
    rbc_37_vec_set(x2_N[e], shares.x2[RYDE_192S_PARAM_N_MPC - 1], RYDE_192S_PARAM_K);
    rbc_37_vec_set(a_N[e], shares.a[RYDE_192S_PARAM_N_MPC - 1], RYDE_192S_PARAM_W - 1);
    rbc_37_vec_set(u_N[e], shares.u[RYDE_192S_PARAM_N_MPC - 1], RYDE_192S_PARAM_W - 1);
    rbc_37_elt_set(c_N[e], shares.c[RYDE_192S_PARAM_N_MPC - 1]);
    rbc_37_elt_set_zero(c[e]);
    for(size_t index = 0; index < RYDE_192S_PARAM_N_MPC; index++) { rbc_37_elt_add(c[e], c[e], shares.c[index]); }
    rbc_37_vec_set(u[e], shares.u_, RYDE_192S_PARAM_W - 1);
  }
  ryde_192s_clear_leaf_shares(&shares);

  // --------------------------------------------------------------------------------------------------- First Challenge

  // Generate h1 and parse signature
  hash_sha3_finalize(h1, &ctx_h1);
  memcpy(&signature[2 * RYDE_192S_SECURITY_BYTES], h1, 2 * RYDE_192S_SECURITY_BYTES);

  // Generate first challenge
  ryde_192s_compute_challenge1(challenge1, h1, salt);

  // ---------------------------------------------------------------------------------------------------- First Response

  // Add m, pk, salt and h1 to h2
  domain_separator = DOMAIN_SEPARATOR_HASH2;
  hash_sha3_absorb(&ctx_h2, &domain_separator, sizeof(uint8_t));
  hash_sha3_absorb(&ctx_h2, m_digest, 2 * RYDE_192S_SECURITY_BYTES);
  hash_sha3_absorb(&ctx_h2, pk, RYDE_192S_PUBLIC_KEY_BYTES);
  hash_sha3_absorb(&ctx_h2, salt, 2 * RYDE_192S_SECURITY_BYTES);
  hash_sha3_absorb(&ctx_h2, h1, 2 * RYDE_192S_SECURITY_BYTES);

  // Compute first response
  for(size_t e = 0; e < RYDE_192S_PARAM_TAU; e++) {
    ryde_192s_init_response1(&(response1[e]));
    ryde_192s_compute_response1(&(response1[e]), &(main_shares[e]), &(challenge1[e]), x1, x2, u[e]);

    // Add alpha, [alpha] and [v] to h2
    for(size_t i = 0; i < RYDE_192S_PARAM_D; i++ ) {
      rbc_37_vec_to_bytes(&response1_str[RYDE_192S_PARAM_W * RYDE_192S_PARAM_M_BYTES * i], response1[e].alpha[i], RYDE_192S_PARAM_W - 1);
      memcpy(&response1_str[RYDE_192S_VEC_W_MINUS_ONE_BYTES + (RYDE_192S_PARAM_W * RYDE_192S_PARAM_M_BYTES * i)], response1[e].v[i], RYDE_192S_PARAM_M_BYTES);
    }

    uint8_t plain_alpha[(RYDE_192S_PARAM_W - 1) * RYDE_192S_PARAM_M_BYTES] = {0};
    rbc_37_vec_to_bytes(plain_alpha, response1[e].alpha_, RYDE_192S_PARAM_W - 1);
    hash_sha3_absorb(&ctx_h2, plain_alpha, (RYDE_192S_PARAM_W - 1) * RYDE_192S_PARAM_M_BYTES);
    hash_sha3_absorb(&ctx_h2, response1_str, RYDE_192S_PARAM_W * RYDE_192S_PARAM_M_BYTES * RYDE_192S_PARAM_D);
  }

  // -------------------------------------------------------------------------------------------------- Second Challenge

  // Generate h2 and parse signature
  hash_sha3_finalize(h2, &ctx_h2);
  memcpy(&signature[4 * RYDE_192S_SECURITY_BYTES], h2, 2 * RYDE_192S_SECURITY_BYTES);

  // Generate second challenge
  hash_shake((uint8_t *)challenge2, RYDE_192S_PARAM_TAU, h2, 2 * RYDE_192S_SECURITY_BYTES);

  // --------------------------------------------------------------------------------------------------- Second Response

  // Compute signature
  for(size_t e = 0; e < RYDE_192S_PARAM_TAU; e++) {
    challenge2[e] &= (uint8_t)(RYDE_192S_PARAM_N_MPC - 1);

    // Generate partial seed tree
    ryde_192s_seed_tree_t seed_tree = {0};
    memcpy(seed_tree[0], &theta[RYDE_192S_SECURITY_BYTES * e], RYDE_192S_SECURITY_BYTES);
    ryde_192s_tree_expand(seed_tree, salt, e);

    ryde_192s_seed_tree_node_t partial_seed_tree[RYDE_192S_PARAM_N_MPC_LOG2] = {0};
    ryde_192s_tree_compute_partial(partial_seed_tree, (const ryde_192s_seed_tree_node_t *) seed_tree, challenge2[e]);

    // Add partial seed tree and (commitment)_ch2 to the signature
    memcpy(&signature[6 * RYDE_192S_SECURITY_BYTES + (RYDE_192S_PARAM_RESPONSE_BYTES * e)], partial_seed_tree[0], RYDE_192S_PARAM_TREE_PATH_BYTES);
    memcpy(&signature[6 * RYDE_192S_SECURITY_BYTES + (RYDE_192S_PARAM_RESPONSE_BYTES * e) + RYDE_192S_PARAM_TREE_PATH_BYTES],
           &cmt_i[(RYDE_192S_PARAM_COMMITMENT_BYTES * e) + (2 * RYDE_192S_SECURITY_BYTES * challenge2[e])], 2 * RYDE_192S_SECURITY_BYTES);

    // Compute share [alpha]_ch2
    if(challenge2[e] != (uint8_t) (RYDE_192S_PARAM_N_MPC - 1)) {
      ryde_192s_reconstruct_share(tmp1, tmp2, tmp3, y, H, &theta_i[RYDE_192S_PARAM_TREE_LEAF_BYTES * e], challenge2[e], salt);
    }
    else {
      rbc_37_vec_set(tmp1, x1_N[e], RYDE_192S_PARAM_N - RYDE_192S_PARAM_K);
      rbc_37_vec_set(tmp2, x2_N[e], RYDE_192S_PARAM_K);
      rbc_37_vec_set(tmp3, u_N[e], RYDE_192S_PARAM_W - 1);
    }
    ryde_192s_reconstruct_alpha(&(response1[e]), tmp1, tmp2, tmp3, &(challenge1[e]));

    if(challenge2[e] == (uint8_t) (RYDE_192S_PARAM_N_MPC - 1)) {
        rbc_37_vec_set_zero(x2_N[e], RYDE_192S_PARAM_K);
        rbc_37_vec_set_zero(a_N[e], RYDE_192S_PARAM_W - 1);
        rbc_37_elt_set_zero(c_N[e]);
    }

    // Add [alpha]_ch2, [x2]_N, [a]_N and [c]_N to the signature
    uint32_t index = 6 * RYDE_192S_SECURITY_BYTES + (RYDE_192S_PARAM_RESPONSE_BYTES * e) + RYDE_192S_PARAM_TREE_PATH_BYTES + 2 * RYDE_192S_SECURITY_BYTES;
    rbc_37_vec_to_string(&signature[index], response1[e].alpha_, RYDE_192S_PARAM_W - 1);
    rbc_37_vec_to_string(&signature[index + RYDE_192S_VEC_W_MINUS_ONE_BYTES], x2_N[e], RYDE_192S_PARAM_K);
    rbc_37_vec_to_string(&signature[index + RYDE_192S_VEC_W_MINUS_ONE_BYTES + RYDE_192S_VEC_K_BYTES], a_N[e], RYDE_192S_PARAM_W - 1);
    memcpy(&signature[index + RYDE_192S_VEC_W_MINUS_ONE_BYTES + RYDE_192S_VEC_K_BYTES + RYDE_192S_VEC_W_MINUS_ONE_BYTES], c_N[e], RYDE_192S_PARAM_M_BYTES);
  }

  // -------------------------------------------------------------------------------------------------------- Clear Data

  #ifdef VERBOSE
    printf("\nchallenge1:");
    for(uint8_t e = 0; e < RYDE_192S_PARAM_TAU; e++) {
      printf("\n  Iteration %" PRIu8 ":", e);

      uint8_t challenge1_gamma_string[RYDE_192S_VEC_N_BYTES] = {0};
      rbc_37_vec_to_string(challenge1_gamma_string, challenge1[e].gamma, RYDE_192S_PARAM_N);
      printf("\n    - gamma   : "); for(size_t i = 0 ; i < RYDE_192S_VEC_N_BYTES ; i++) { printf("%02x", challenge1_gamma_string[i]); }
      uint8_t challenge1_epsilon_string[RYDE_192S_PARAM_M_BYTES] = {0};
      rbc_37_elt_to_string(challenge1_epsilon_string, challenge1[e].epsilon);
      printf("\n    - epsilon : "); for(size_t i = 0 ; i < RYDE_192S_PARAM_M_BYTES ; i++) { printf("%02x", challenge1_epsilon_string[i]); }
    }

    printf("\nchallenge2: "); for(size_t i = 0 ; i < RYDE_192S_PARAM_TAU ; i++) printf("%" PRIu8 " ", challenge2[i]);

    printf("\nsignature:"); size_t index = 0;
    printf("\n  salt :"); for(size_t i = 0 ; i < 2 * RYDE_192S_SECURITY_BYTES ; i++) { printf("%02x", signature[i]); } index+= 2 * RYDE_192S_SECURITY_BYTES;
    printf("\n  h1   :"); for(size_t i = 0 ; i < 2 * RYDE_192S_SECURITY_BYTES ; i++) { printf("%02x", signature[index + i]); } index+= 2 * RYDE_192S_SECURITY_BYTES;
    printf("\n  h2   :"); for(size_t i = 0 ; i < 2 * RYDE_192S_SECURITY_BYTES ; i++) { printf("%02x", signature[index + i]); } index+= 2 * RYDE_192S_SECURITY_BYTES;

    for(uint8_t e = 0; e < RYDE_192S_PARAM_TAU; e++) {
      printf("\n  Iteration %" PRIu8 ":", e);
      printf("\n    - tree      : "); for(size_t i = 0 ; i < RYDE_192S_PARAM_TREE_PATH_BYTES ; i++) { printf("%02x", signature[index + i]); } index+= RYDE_192S_PARAM_TREE_PATH_BYTES;
      printf("\n    - cmt       : "); for(size_t i = 0 ; i < 2 * RYDE_192S_SECURITY_BYTES ; i++) { printf("%02x", signature[index + i]); } index+= 2 * RYDE_192S_SECURITY_BYTES;
      printf("\n    - alpha     : "); for(size_t i = 0 ; i < RYDE_192S_VEC_W_MINUS_ONE_BYTES ; i++) { printf("%02x", signature[index + i]); } index+= RYDE_192S_VEC_W_MINUS_ONE_BYTES;
      printf("\n    - x2        : "); for(size_t i = 0 ; i < RYDE_192S_VEC_K_BYTES ; i++) { printf("%02x", signature[index + i]); } index+= RYDE_192S_VEC_K_BYTES;
      printf("\n    - a         : "); for(size_t i = 0 ; i < RYDE_192S_VEC_W_MINUS_ONE_BYTES ; i++) { printf("%02x", signature[index + i]); } index+= RYDE_192S_VEC_W_MINUS_ONE_BYTES;
      printf("\n    - c         : "); for(size_t i = 0 ; i < RYDE_192S_PARAM_M_BYTES ; i++) { printf("%02x", signature[index + i]); } index+= RYDE_192S_PARAM_M_BYTES;
    }
    printf("\n");
  #endif

  memset(salt_and_mseed, 0, 3 * RYDE_192S_SECURITY_BYTES);
  memset(theta, 0, RYDE_192S_PARAM_TAU * RYDE_192S_SECURITY_BYTES);
  memset(theta_i, 0, RYDE_192S_PARAM_TREE_LEAF_BYTES * RYDE_192S_PARAM_TAU);
  memset(cmt_i, 0, RYDE_192S_PARAM_COMMITMENT_BYTES * RYDE_192S_PARAM_TAU);
  memset(h1, 0, 2 * RYDE_192S_SECURITY_BYTES);
  memset(h2, 0, 2 * RYDE_192S_SECURITY_BYTES);
  memset(response1_str, 0, RYDE_192S_PARAM_W * RYDE_192S_PARAM_M_BYTES * RYDE_192S_PARAM_D);

  for(size_t e = 0; e < RYDE_192S_PARAM_TAU; e++) {
    rbc_37_vec_clear(u[e]);
    rbc_37_vec_clear(x1_N[e]);
    rbc_37_vec_clear(x2_N[e]);
    rbc_37_vec_clear(a_N[e]);
    rbc_37_vec_clear(u_N[e]);
    rbc_37_elt_set_zero(c_N[e]);
    ryde_192s_clear_challenge1(&(challenge1[e]));
    ryde_192s_clear_response1(&(response1[e]));
    ryde_192s_clear_main_shares(&(main_shares[e]));
  }

  rbc_37_vec_clear(tmp1);
  rbc_37_vec_clear(tmp2);
  rbc_37_vec_clear(tmp3);

  rbc_37_vec_clear(x1);
  rbc_37_vec_clear(x2);
  rbc_37_qpoly_clear(A);
  rbc_37_mat_clear(H);
  rbc_37_vec_clear(y);

  return EXIT_SUCCESS;
}
