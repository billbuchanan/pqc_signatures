/** 
 * @file ryde_128f_parsing.c
 * @brief Implementation of parsing.h
 */

#include "string.h"
#include "rbc_31_vspace.h"
#include "seedexpander_shake.h"
#include "parameters.h"
#include "parsing.h"
#include "tree.h"
#include "mpc.h"



/**
 * \fn void ryde_128f_public_key_to_string(uint8_t* pk, const uint8_t* pk_seed, const rbc_31_vec y)
 * \brief This function parses a public key into a string
 *
 * The public key is composed of the vector <b>y</b> as well as the seed used to generate matrix <b>H</b>.
 *
 * \param[out] pk String containing the public key
 * \param[in] pk_seed Seed used to generate the public key
 * \param[in] y rbc_31_vec representation of vector y
 */
void ryde_128f_public_key_to_string(uint8_t* pk, const uint8_t* pk_seed, const rbc_31_vec y) {
  memcpy(pk, pk_seed, RYDE_128F_SECURITY_BYTES);
  rbc_31_vec_to_string(&pk[RYDE_128F_SECURITY_BYTES], y, RYDE_128F_PARAM_N - RYDE_128F_PARAM_K);
}



/**
 * \fn void ryde_128f_public_key_from_string(rbc_31_mat H, rbc_31_vec y, const uint8_t* pk)
 * \brief This function parses a public key from a string
 *
 * The public key is composed of the vector <b>y</b> as well as the seed used to generate matrix <b>H</b>.
 *
 * \param[out] H rbc_31_mat representation of vector H
 * \param[out] y rbc_31_vec representation of vector y
 * \param[in] pk String containing the public key
 */
void ryde_128f_public_key_from_string(rbc_31_mat H, rbc_31_vec y, const uint8_t* pk) {
  uint8_t pk_seed[RYDE_128F_SECURITY_BYTES] = {0};
  seedexpander_shake_t pk_seedexpander;

  rbc_31_vspace support;
  rbc_31_vspace_init(&support, RYDE_128F_PARAM_W);

  // Compute parity-check matrix
  memcpy(pk_seed, pk, RYDE_128F_SECURITY_BYTES);
  seedexpander_shake_init(&pk_seedexpander, pk_seed, RYDE_128F_SECURITY_BYTES, NULL, 0);

  rbc_31_mat_set_random(&pk_seedexpander, H, RYDE_128F_PARAM_N - RYDE_128F_PARAM_K, RYDE_128F_PARAM_K);

  // Compute syndrome
  rbc_31_vec_from_string(y, RYDE_128F_PARAM_N - RYDE_128F_PARAM_K, &pk[RYDE_128F_SECURITY_BYTES]);

  rbc_31_vspace_clear(support);
}



/**
 * \fn void ryde_128f_secret_key_to_string(uint8_t* sk, const uint8_t* seed, const uint8_t* pk)
 * \brief This function parses a secret key into a string
 *
 * The secret key is composed of the seed used to generate vectors <b>x = (x1,x2)</b> and <b>y</b>.
 * As a technicality, the public key is appended to the secret key in order to respect the NIST API.
 *
 * \param[out] sk String containing the secret key
 * \param[in] seed Seed used to generate the vectors x and y
 * \param[in] pk String containing the public key
 */
void ryde_128f_secret_key_to_string(uint8_t* sk, const uint8_t* sk_seed, const uint8_t* pk) {
  memcpy(sk, sk_seed, RYDE_128F_SECURITY_BYTES);
  memcpy(&sk[RYDE_128F_SECURITY_BYTES], pk, RYDE_128F_SECURITY_BYTES);
}



/**
* \fn void ryde_128f_secret_key_from_string(rbc_31_vec y, rbc_31_mat H, rbc_31_vec x1, rbc_31_vec x2, rbc_31_qpoly A, const uint8_t* sk)
* \brief This function parses a secret key from a string
*
* The secret key is composed of the seed used to generate vectors <b>x = (x1,x2)</b> and <b>y</b>.
* Additionally, it calculates the public matrix <b>H</b> and the annihilator polynomial <b>A</b>.
*
* As a technicality, the public key is appended to the secret key in order to respect the NIST API.
*
* \param[out] y rbc_31_vec representation of vector y
* \param[out] y rbc_31_mat representation of matrix H
* \param[out] x1 rbc_31_vec representation of vector x1
* \param[out] x2 rbc_31_vec representation of vector x2
* \param[out] A rbc_31_qpoly representation of polynomial A
* \param[in] sk String containing the secret key
*/
void ryde_128f_secret_key_from_string(rbc_31_vec y, rbc_31_mat H, rbc_31_vec x1, rbc_31_vec x2, rbc_31_qpoly A, const uint8_t* sk) {

  uint8_t sk_seed[RYDE_128F_SECURITY_BYTES] = {0};
  uint8_t pk_seed[RYDE_128F_SECURITY_BYTES] = {0};

  seedexpander_shake_t sk_seedexpander;
  seedexpander_shake_t pk_seedexpander;

  memcpy(sk_seed, sk, RYDE_128F_SECURITY_BYTES);
  seedexpander_shake_init(&sk_seedexpander, sk_seed, RYDE_128F_SECURITY_BYTES, NULL, 0);

  rbc_31_vspace support;
  rbc_31_vspace_init(&support, RYDE_128F_PARAM_W);

  rbc_31_vec x;
  rbc_31_vec_init(&x, RYDE_128F_PARAM_N);

  // Compute secret key
  rbc_31_vspace_set_random_full_rank_with_one(&sk_seedexpander, support, RYDE_128F_PARAM_W);
  rbc_31_vec_set_random_from_support(&sk_seedexpander, x, RYDE_128F_PARAM_N, support, RYDE_128F_PARAM_W, 1);

  for(size_t i = 0; i < RYDE_128F_PARAM_N - RYDE_128F_PARAM_K; i++) {
    rbc_31_elt_set(x1[i], x[i]);
  }

  for(size_t i = 0; i < RYDE_128F_PARAM_K; i++) {
    rbc_31_elt_set(x2[i], x[i + RYDE_128F_PARAM_N - RYDE_128F_PARAM_K]);
  }

  rbc_31_qpoly_annihilator(A, support, RYDE_128F_PARAM_W);

  // Compute public key
  memcpy(pk_seed, &sk[RYDE_128F_SECURITY_BYTES], RYDE_128F_SECURITY_BYTES);
  seedexpander_shake_init(&pk_seedexpander, pk_seed, RYDE_128F_SECURITY_BYTES, NULL, 0);

  rbc_31_mat_set_random(&pk_seedexpander, H, RYDE_128F_PARAM_N - RYDE_128F_PARAM_K, RYDE_128F_PARAM_K);
  rbc_31_mat_vec_mul(y, H, x2, RYDE_128F_PARAM_N - RYDE_128F_PARAM_K, RYDE_128F_PARAM_K);
  rbc_31_vec_add(y, y, x1, RYDE_128F_PARAM_N - RYDE_128F_PARAM_K);

  rbc_31_vspace_clear(support);
  rbc_31_vec_clear(x);
}

