/** 
 * @file sign_mira_192_keygen.c
 * @brief Implementation of keygen.h
 */

#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include "string.h"
#include "randombytes.h"
#include "seedexpander_shake.h"
#include "parameters.h"
#include "parsing.h"
#include "keygen.h"
#include "finite_fields.h"



/**
 * \fn int sign_mira_keygen(uint8_t* pk, uint8_t* sk)
 * \brief Keygen of the MIRA_additive signature scheme
 *
 * \param[out] pk String containing the public key
 * \param[out] sk String containing the secret key
 * \return EXIT_SUCCESS if verify is successful. Otherwise, it returns EXIT_FAILURE
 */
int sign_mira_192_keygen(uint8_t* pk, uint8_t* sk) {
  #ifdef VERBOSE
    printf("\n\n\n### KEYGEN ###");
  #endif

  uint8_t sk_seed[SIGN_MIRA_192_SECURITY_BYTES] = {0};
  uint8_t pk_seed[SIGN_MIRA_192_SECURITY_BYTES] = {0};

  seedexpander_shake_t sk_seedexpander;
  seedexpander_shake_t pk_seedexpander;

  // Create seed expanders for public key and secret key
  randombytes(sk_seed, SIGN_MIRA_192_SECURITY_BYTES);
  randombytes(pk_seed, SIGN_MIRA_192_SECURITY_BYTES);
  seedexpander_shake_init(&sk_seedexpander, sk_seed, SIGN_MIRA_192_SECURITY_BYTES, NULL, 0);
  seedexpander_shake_init(&pk_seedexpander, pk_seed, SIGN_MIRA_192_SECURITY_BYTES, NULL, 0);

  //Sample M1, ..., M_k
  gf16_mat Mi[SIGN_MIRA_192_PARAM_K];
  for(int i=0 ; i<SIGN_MIRA_192_PARAM_K ; i++) {
    gf16_mat_init(Mi + i, SIGN_MIRA_192_PARAM_M, SIGN_MIRA_192_PARAM_N);
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

  //Sample E
  gf16_mat coordinates, E;
  gf16_mat_init(&coordinates, SIGN_MIRA_192_PARAM_R, SIGN_MIRA_192_PARAM_N);
  gf16_mat_init(&E, SIGN_MIRA_192_PARAM_M, SIGN_MIRA_192_PARAM_N);

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
  gf16_mat M0;
  gf16_mat_init(&M0, SIGN_MIRA_192_PARAM_M, SIGN_MIRA_192_PARAM_N);

  gf16_mat_set(M0, F, SIGN_MIRA_192_PARAM_M, SIGN_MIRA_192_PARAM_N);
  for(int i=0 ; i<SIGN_MIRA_192_PARAM_K ; i++) {
    gf16_mat_scalar_mul(tmp_mat, Mi[i], f[i], SIGN_MIRA_192_PARAM_M, SIGN_MIRA_192_PARAM_N);
    gf16_mat_add(M0, M0, tmp_mat, SIGN_MIRA_192_PARAM_M, SIGN_MIRA_192_PARAM_N);
  }

  // Parse keys to string
  sign_mira_192_public_key_to_string(pk, pk_seed, M0);
  sign_mira_192_secret_key_to_string(sk, sk_seed, pk_seed);

  #ifdef VERBOSE
    printf("\n\nsk_seed: "); for(int i = 0 ; i < SIGN_MIRA_192_SECURITY_BYTES ; ++i) printf("%02x", sk_seed[i]);
    printf("\npk_seed: ");   for(int i = 0 ; i < SIGN_MIRA_192_SECURITY_BYTES ; ++i) printf("%02x", pk_seed[i]);
    printf("\nsk: "); for(int i = 0 ; i < SIGN_MIRA_192_SECRET_KEY_BYTES ; ++i) printf("%02x", sk[i]);
    printf("\npk: "); for(int i = 0 ; i < SIGN_MIRA_192_PUBLIC_KEY_BYTES ; ++i) printf("%02x", pk[i]);

    printf("\nE:");
    size_t length = (SIGN_MIRA_192_PARAM_N * SIGN_MIRA_192_PARAM_M + 1) / 2;
    uint8_t E_string[length];
    gf16_mat_to_string(E_string, E, SIGN_MIRA_192_PARAM_M, SIGN_MIRA_192_PARAM_N);
    printf("\n    - E   : "); for(size_t i = 0 ; i < length ; i++) { printf("%02x", E_string[i]); }
    memset(E_string, 0, length);

    length = (SIGN_MIRA_192_PARAM_N * SIGN_MIRA_192_PARAM_M + 1) / 2;
    uint8_t M0_string[length];
    gf16_mat_to_string(M0_string, M0, SIGN_MIRA_192_PARAM_M, SIGN_MIRA_192_PARAM_N);
    printf("\nM0: "); for(size_t i = 0 ; i < length ; i++) { printf("%02x", M0_string[i]); }
    memset(M0_string, 0, length);

    for(int mat=0 ; mat<SIGN_MIRA_192_PARAM_K ; mat++) {
      length = (SIGN_MIRA_192_PARAM_N * SIGN_MIRA_192_PARAM_M + 1) / 2;
      uint8_t Mi_string[length];
      gf16_mat_to_string(Mi_string, Mi[mat], SIGN_MIRA_192_PARAM_M, SIGN_MIRA_192_PARAM_N);
      printf("\nM%d: ", mat+1); for(size_t i = 0 ; i < length ; i++) { printf("%02x", Mi_string[i]); }
      memset(Mi_string, 0, length);
    }
    
  #endif

  for(int i=0 ; i<SIGN_MIRA_192_PARAM_K ; i++) gf16_mat_clear(Mi[i]);
  gf16_mat_clear(support);
  gf16_mat_clear(coordinates);
  gf16_mat_clear(E);
  gf16_mat_clear(F);
  gf16_mat_clear(f);
  gf16_mat_clear(B);
  gf16_mat_clear(M0);
  gf16_mat_clear(tmp_mat);

  return EXIT_SUCCESS;
}
