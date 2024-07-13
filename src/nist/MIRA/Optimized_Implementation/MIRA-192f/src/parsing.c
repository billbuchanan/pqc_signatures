/** 
 * @file sign_mira_192_parsing.c
 * @brief Implementation of parsing.h
 */

#include <stdio.h>
#include "string.h"
#include "seedexpander_shake.h"
#include "parameters.h"
#include "parsing.h"
#include "tree.h"
#include "mpc.h"
#include "finite_fields.h"



/**
 * \fn void sign_mira_192_public_key_to_string(uint8_t* pk, const uint8_t* pk_seed, const rbc_31_vec y)
 * \brief This function parses a public key into a string
 *
 * The public key is composed of the vector <b>y</b> as well as the seed used to generate matrix <b>H</b>.
 *
 * \param[out] pk String containing the public key
 * \param[in] pk_seed Seed used to generate the public key
 * \param[in] y rbc_31_vec representation of vector y
 */
void sign_mira_192_public_key_to_string(uint8_t* pk, const uint8_t* pk_seed, const gf16_mat M0) {
  memcpy(pk, pk_seed, SIGN_MIRA_192_SECURITY_BYTES);
  gf16_mat_to_string(pk + SIGN_MIRA_192_SECURITY_BYTES, &(M0[SIGN_MIRA_192_PARAM_K]), 1, SIGN_MIRA_192_PARAM_M * SIGN_MIRA_192_PARAM_N - SIGN_MIRA_192_PARAM_K);
}



/**
 * \fn void sign_mira_192_public_key_from_string(rbc_31_mat H, rbc_31_vec y, const uint8_t* pk)
 * \brief This function parses a public key from a string
 *
 * The public key is composed of the vector <b>y</b> as well as the seed used to generate matrix <b>H</b>.
 *
 * \param[out] H rbc_31_mat representation of vector H
 * \param[out] y rbc_31_vec representation of vector y
 * \param[in] pk String containing the public key
 */
void sign_mira_192_public_key_from_string(gf16_mat M0, gf16_mat *Mi, const uint8_t* pk) {
  uint8_t pk_seed[SIGN_MIRA_192_SECURITY_BYTES] = {0};
  seedexpander_shake_t pk_seedexpander;
  
  memcpy(pk_seed, pk, SIGN_MIRA_192_SECURITY_BYTES);
  seedexpander_shake_init(&pk_seedexpander, pk_seed, SIGN_MIRA_192_SECURITY_BYTES, NULL, 0);

  //Sample M1, ..., M_k
  for(int i=0 ; i<SIGN_MIRA_192_PARAM_K ; i++) {
    //Identity
    for(int j=0 ; j<SIGN_MIRA_192_PARAM_K ; j++) {
      if(j==i) Mi[i][j] = 1;
      else Mi[i][j] = 0;
    }
    gf16_mat_set_random(&pk_seedexpander, &(Mi[i][SIGN_MIRA_192_PARAM_K]), 1, SIGN_MIRA_192_PARAM_N * SIGN_MIRA_192_PARAM_M - SIGN_MIRA_192_PARAM_K);
  }

  gf16_mat_set_zero(M0, 1, SIGN_MIRA_192_PARAM_K);
  gf16_mat_from_string(&(M0[SIGN_MIRA_192_PARAM_K]),  1, SIGN_MIRA_192_PARAM_M * SIGN_MIRA_192_PARAM_N - SIGN_MIRA_192_PARAM_K, pk + SIGN_MIRA_192_SECURITY_BYTES);
}



/**
 * \fn void sign_mira_192_secret_key_to_string(uint8_t* sk, const uint8_t* seed, const uint8_t* pk)
 * \brief This function parses a secret key into a string
 *
 * The secret key is composed of the seed used to generate vectors <b>x = (x1,x2)</b> and <b>y</b>.
 * As a technicality, the public key is appended to the secret key in order to respect the NIST API.
 *
 * \param[out] sk String containing the secret key
 * \param[in] seed Seed used to generate the vectors x and y
 * \param[in] pk String containing the public key
 */
void sign_mira_192_secret_key_to_string(uint8_t* sk, const uint8_t* sk_seed, const uint8_t* pk_seed) {
  memcpy(sk, sk_seed, SIGN_MIRA_192_SECURITY_BYTES);
  memcpy(sk + SIGN_MIRA_192_SECURITY_BYTES, pk_seed, SIGN_MIRA_192_SECURITY_BYTES);
}



/**
* \fn void sign_mira_192_secret_key_from_string(rbc_31_vec y, rbc_31_mat H, rbc_31_vec x1, rbc_31_vec x2, rbc_31_qpoly A, const uint8_t* sk)
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
void sign_mira_192_secret_key_from_string(gf16_mat M0, gf16_mat *Mi, gf16_mat x, gf16_mat E, gfqm *A, const uint8_t* sk) {
  uint8_t sk_seed[SIGN_MIRA_192_SECURITY_BYTES] = {0};
  uint8_t pk_seed[SIGN_MIRA_192_SECURITY_BYTES] = {0};

  seedexpander_shake_t sk_seedexpander;
  seedexpander_shake_t pk_seedexpander;

  memcpy(sk_seed, sk, SIGN_MIRA_192_SECURITY_BYTES);
  seedexpander_shake_init(&sk_seedexpander, sk_seed, SIGN_MIRA_192_SECURITY_BYTES, NULL, 0);

  memcpy(pk_seed, &sk[SIGN_MIRA_192_SECURITY_BYTES], SIGN_MIRA_192_SECURITY_BYTES);
  seedexpander_shake_init(&pk_seedexpander, pk_seed, SIGN_MIRA_192_SECURITY_BYTES, NULL, 0);

  //Sample M1, ..., M_k
  for(int i=0 ; i<SIGN_MIRA_192_PARAM_K ; i++) {
    //Identity
    for(int j=0 ; j<SIGN_MIRA_192_PARAM_K ; j++) {
      if(j==i) Mi[i][j] = 1;
      else Mi[i][j] = 0;
    }
    gf16_mat_set_random(&pk_seedexpander, &(Mi[i][SIGN_MIRA_192_PARAM_K]), 1, SIGN_MIRA_192_PARAM_N * SIGN_MIRA_192_PARAM_M - SIGN_MIRA_192_PARAM_K);
  }

  //Sample PARAM_R independant elements of GF(q^m)
  gf16_mat support;
  gf16_mat_init(&support, SIGN_MIRA_192_PARAM_M, SIGN_MIRA_192_PARAM_R);
  //Identity matrix "trick"
  for(int i=0 ; i<SIGN_MIRA_192_PARAM_R ; i++) {
    for(int j=0 ; j<SIGN_MIRA_192_PARAM_R ; j++) {
      if(i==j) support[i*SIGN_MIRA_192_PARAM_R + j] = 1;
      else support[i*SIGN_MIRA_192_PARAM_R + j] = 0;
    }
  }

  gf16_mat_set_random(&sk_seedexpander, support + SIGN_MIRA_192_PARAM_R * SIGN_MIRA_192_PARAM_R, SIGN_MIRA_192_PARAM_M - SIGN_MIRA_192_PARAM_R, SIGN_MIRA_192_PARAM_R);

  //Compute A
  gfqm support_vec[SIGN_MIRA_192_PARAM_R];
  for(int i=0 ; i<SIGN_MIRA_192_PARAM_R ; i++) {
    for(int j=0 ; j<SIGN_MIRA_192_PARAM_M ; j++) {
      support_vec[i][j] = support[j*SIGN_MIRA_192_PARAM_R + i];
    }
  }

  gfqm_qpoly_annihilator(A, support_vec, SIGN_MIRA_192_PARAM_R);

  //Sample E
  gf16_mat coordinates;
  gf16_mat_init(&coordinates, SIGN_MIRA_192_PARAM_R, SIGN_MIRA_192_PARAM_N);

  gf16_mat_set_random(&sk_seedexpander, coordinates, SIGN_MIRA_192_PARAM_R, SIGN_MIRA_192_PARAM_N);
  gf16_mat_mul(E, support, coordinates, SIGN_MIRA_192_PARAM_M, SIGN_MIRA_192_PARAM_R, SIGN_MIRA_192_PARAM_N);

  //Compute F
  gf16_mat F, tmp_mat;
  gf16_mat B, f;
  gf16_mat_init(&B, 1, SIGN_MIRA_192_PARAM_K);
  gf16_mat_init(&f, 1, SIGN_MIRA_192_PARAM_K);
  gf16_mat_init(&F, SIGN_MIRA_192_PARAM_M, SIGN_MIRA_192_PARAM_N);
  gf16_mat_init(&tmp_mat, SIGN_MIRA_192_PARAM_M, SIGN_MIRA_192_PARAM_N);

  gf16_mat_set_random(&sk_seedexpander, B, 1, SIGN_MIRA_192_PARAM_K);

  gf16_mat_set(F, E, SIGN_MIRA_192_PARAM_M, SIGN_MIRA_192_PARAM_N);
  for(int i=0 ; i<SIGN_MIRA_192_PARAM_K ; i++) {
    gf16_mat_scalar_mul(tmp_mat, Mi[i], B[i], SIGN_MIRA_192_PARAM_M, SIGN_MIRA_192_PARAM_N);
    gf16_mat_add(F, F, tmp_mat, SIGN_MIRA_192_PARAM_M, SIGN_MIRA_192_PARAM_N);
  }

  gf16_mat_set(f, F, 1, SIGN_MIRA_192_PARAM_K);

  //Compute M0
  gf16_mat_set(M0, F, SIGN_MIRA_192_PARAM_M, SIGN_MIRA_192_PARAM_N);
  for(int i=0 ; i<SIGN_MIRA_192_PARAM_K ; i++) {
    gf16_mat_scalar_mul(tmp_mat, Mi[i], f[i], SIGN_MIRA_192_PARAM_M, SIGN_MIRA_192_PARAM_N);
    gf16_mat_add(M0, M0, tmp_mat, SIGN_MIRA_192_PARAM_M, SIGN_MIRA_192_PARAM_N);
  }

  //Sample x
  gf16_mat_add(x, f, B, 1, SIGN_MIRA_192_PARAM_K);

  gf16_mat_clear(support);
  gf16_mat_clear(coordinates);
  gf16_mat_clear(tmp_mat);
  gf16_mat_clear(F);
  gf16_mat_clear(f);
  gf16_mat_clear(B);
}
