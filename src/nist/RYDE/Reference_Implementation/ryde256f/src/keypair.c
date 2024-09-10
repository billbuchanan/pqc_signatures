/** 
 * @file ryde_256f_keygen.c
 * @brief Implementation of keypair.h
 */

#include "string.h"
#include "randombytes.h"
#include "seedexpander_shake.h"
#include "rbc_43_vspace.h"
#include "parameters.h"
#include "parsing.h"
#include "keypair.h"



/**
 * \fn int ryde_256f_keygen(uint8_t* pk, uint8_t* sk)
 * \brief Keygen of the RQC_KEM IND-CCA2 scheme
 *
 * The public key is composed of the syndrome <b>s</b> as well as the seed used to generate vectors <b>g</b> and <b>h</b>.
 *
 * The secret key is composed of the seed used to generate the vectors <b>x</b> and <b>y</b>.
 * As a technicality, the public key is appended to the secret key in order to respect the NIST API.
 *
 * \param[out] pk String containing the public key
 * \param[out] sk String containing the secret key
 * \return EXIT_SUCCESS if verify is successful. Otherwise, it returns EXIT_FAILURE
 */
int ryde_256f_keygen(uint8_t* pk, uint8_t* sk) {
  #ifdef VERBOSE
    printf("\n\n\n### KEYGEN ###");
  #endif

  rbc_43_field_init();

  uint8_t sk_seed[RYDE_256F_SECURITY_BYTES] = {0};
  uint8_t pk_seed[RYDE_256F_SECURITY_BYTES] = {0};

  seedexpander_shake_t sk_seedexpander;
  seedexpander_shake_t pk_seedexpander;

  rbc_43_vspace support;
  rbc_43_mat H;
  rbc_43_vec x1, x2, x, y;

  rbc_43_vspace_init(&support, RYDE_256F_PARAM_W);
  rbc_43_mat_init(&H, RYDE_256F_PARAM_N - RYDE_256F_PARAM_K, RYDE_256F_PARAM_K);
  rbc_43_vec_init(&x1, RYDE_256F_PARAM_N - RYDE_256F_PARAM_K);
  rbc_43_vec_init(&x2, RYDE_256F_PARAM_K);
  rbc_43_vec_init(&x, RYDE_256F_PARAM_N);
  rbc_43_vec_init(&y, RYDE_256F_PARAM_N - RYDE_256F_PARAM_K);

  // Create seed expanders for public key and secret key
  if (randombytes(sk_seed, RYDE_256F_SECURITY_BYTES) != EXIT_SUCCESS) {
    memset(sk_seed, 0, RYDE_256F_SECURITY_BYTES);
    return EXIT_FAILURE;
  }
  if (randombytes(pk_seed, RYDE_256F_SECURITY_BYTES) != EXIT_SUCCESS) {
    memset(sk_seed, 0, RYDE_256F_SECURITY_BYTES);
    memset(pk_seed, 0, RYDE_256F_SECURITY_BYTES);
    return EXIT_FAILURE;
  }
  seedexpander_shake_init(&sk_seedexpander, sk_seed, RYDE_256F_SECURITY_BYTES, NULL, 0);
  seedexpander_shake_init(&pk_seedexpander, pk_seed, RYDE_256F_SECURITY_BYTES, NULL, 0);

  // Compute secret key
  rbc_43_vspace_set_random_full_rank_with_one(&sk_seedexpander, support, RYDE_256F_PARAM_W);
  rbc_43_vec_set_random_from_support(&sk_seedexpander, x, RYDE_256F_PARAM_N, support, RYDE_256F_PARAM_W, 1);

  for(size_t i = 0; i < RYDE_256F_PARAM_N - RYDE_256F_PARAM_K; i++) {
    rbc_43_elt_set(x1[i], x[i]);
  }

  for(size_t i = 0; i < RYDE_256F_PARAM_K; i++) {
    rbc_43_elt_set(x2[i], x[i + RYDE_256F_PARAM_N - RYDE_256F_PARAM_K]);
  }

  // Compute public key
  rbc_43_mat_set_random(&pk_seedexpander, H, RYDE_256F_PARAM_N - RYDE_256F_PARAM_K, RYDE_256F_PARAM_K);
  rbc_43_mat_vec_mul(y, H, x2, RYDE_256F_PARAM_N - RYDE_256F_PARAM_K, RYDE_256F_PARAM_K);
  rbc_43_vec_add(y, y, x1, RYDE_256F_PARAM_N - RYDE_256F_PARAM_K);

  // Parse keys to string
  ryde_256f_public_key_to_string(pk, pk_seed, y);
  ryde_256f_secret_key_to_string(sk, sk_seed, pk_seed);

  #ifdef VERBOSE
    printf("\n\nsk_seed: "); for(int i = 0 ; i < RYDE_256F_SECURITY_BYTES ; ++i) printf("%02x", sk_seed[i]);
    printf("\npk_seed: ");   for(int i = 0 ; i < RYDE_256F_SECURITY_BYTES ; ++i) printf("%02x", pk_seed[i]);
    printf("\nsk: "); for(int i = 0 ; i < RYDE_256F_SECRET_KEY_BYTES ; ++i) printf("%02x", sk[i]);
    printf("\npk: "); for(int i = 0 ; i < RYDE_256F_PUBLIC_KEY_BYTES ; ++i) printf("%02x", pk[i]);

    printf("\nx:");
    size_t length = ((RYDE_256F_PARAM_N - RYDE_256F_PARAM_K) * RYDE_256F_PARAM_M + 7 ) / 8;
    uint8_t x1_string[length];
    rbc_43_vec_to_string(x1_string, x1, RYDE_256F_PARAM_N - RYDE_256F_PARAM_K);
    printf("\n    - x1   : "); for(size_t i = 0 ; i < length ; i++) { printf("%02x", x1_string[i]); }
    memset(x1_string, 0, length);
    uint8_t x2_string[RYDE_256F_VEC_K_BYTES] = {0};
    rbc_43_vec_to_string(x2_string, x2, RYDE_256F_PARAM_K);
    printf("\n    - x2   : "); for(size_t i = 0 ; i < RYDE_256F_VEC_K_BYTES ; i++) { printf("%02x", x2_string[i]); }
    memset(x2_string, 0, RYDE_256F_VEC_K_BYTES);

    length = ((RYDE_256F_PARAM_N - RYDE_256F_PARAM_K) * RYDE_256F_PARAM_K * RYDE_256F_PARAM_M + 7 ) / 8;
    uint8_t H_string[length];
    rbc_43_mat_to_string(H_string, H, RYDE_256F_PARAM_N - RYDE_256F_PARAM_K, RYDE_256F_PARAM_K);
    printf("\nH: "); for(size_t i = 0 ; i < length ; i++) { printf("%02x", H_string[i]); }
    memset(H_string, 0, length);

    length = ((RYDE_256F_PARAM_N - RYDE_256F_PARAM_K) * RYDE_256F_PARAM_M + 7 ) / 8;
    uint8_t y_string[length];
    rbc_43_vec_to_string(y_string, y, RYDE_256F_PARAM_N - RYDE_256F_PARAM_K);
    printf("\ny: "); for(size_t i = 0 ; i < length ; i++) { printf("%02x", y_string[i]); }
    memset(y_string, 0, length);
  #endif

  rbc_43_vspace_clear(support);
  rbc_43_mat_clear(H);
  rbc_43_vec_clear(x1);
  rbc_43_vec_clear(x2);
  rbc_43_vec_clear(x);
  rbc_43_vec_clear(y);

  return EXIT_SUCCESS;
}
