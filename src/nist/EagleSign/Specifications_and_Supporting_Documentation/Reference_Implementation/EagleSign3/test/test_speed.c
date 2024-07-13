/*
 * Implementors: EagleSign Team
 * This implementation is highly inspired from Dilithium and
 * Falcon Signatures' implementations
 */

#include <stdint.h>
#include "../sign.h"
#include "../poly.h"
#include "../polyvec.h"
#include "../polymatrix.h"
#include "../params.h"
#include "cpucycles.h"
#include "speed_print.h"

#define NTESTS 1000

uint64_t t[NTESTS];

int main(void)
{
  unsigned int i;
  size_t smlen;
  uint8_t pk[CRYPTO_EAGLESIGN_PUBLICKEYBYTES];
  uint8_t sk[CRYPTO_EAGLESIGN_SECRETKEYBYTES];
  uint8_t sm[CRYPTO_EAGLESIGN_BYTES + CRHBYTES];
  uint8_t seed[CRHBYTES] = {0};
  uint8_t seed_prime[SEEDBYTES] = {0};
  polyvecl A[3];
  poly *a = &A[0].vec[0];
  poly *b = &A[1].vec[0];
  poly *c = &A[2].vec[0];

  for (i = 0; i < NTESTS; ++i)
  {
    t[i] = cpucycles();
    polyvec_matrix_expand(A, seed);
  }
  print_results("polyvec_matrix_expand:", t, NTESTS);

  for (i = 0; i < NTESTS; ++i)
  {
    t[i] = cpucycles();
    poly_uniform(a, seed_prime, 0);
  }
  print_results("poly_uniform:", t, NTESTS);

  for (i = 0; i < NTESTS; ++i)
  {
    t[i] = cpucycles();
    poly_challenge_y1_c(a, seed_prime, 0, 0);
  }
  print_results("poly_challenge_y1_c:", t, NTESTS);

  for (i = 0; i < NTESTS; ++i)
  {
    t[i] = cpucycles();
    poly_uniform_eta_y2(a, seed, 0);
  }
  print_results("poly_uniform_eta_y2:", t, NTESTS);

  for (i = 0; i < NTESTS; ++i)
  {
    t[i] = cpucycles();
    poly_uniform_eta_g(a, seed, 0);
  }
  print_results("poly_uniform_eta_g:", t, NTESTS);

  for (i = 0; i < NTESTS; ++i)
  {
    t[i] = cpucycles();
    poly_uniform_eta_d(a, seed, 0);
  }
  print_results("poly_uniform_eta_d:", t, NTESTS);

  for (i = 0; i < NTESTS; ++i)
  {
    t[i] = cpucycles();
    poly_ntt(a);
  }
  print_results("poly_ntt:", t, NTESTS);

  for (i = 0; i < NTESTS; ++i)
  {
    t[i] = cpucycles();
    poly_invntt_tomont(a);
  }
  print_results("poly_invntt_tomont:", t, NTESTS);

  for (i = 0; i < NTESTS; ++i)
  {
    t[i] = cpucycles();
    poly_pointwise_montgomery(c, a, b);
  }
  print_results("poly_pointwise_montgomery:", t, NTESTS);

  for (i = 0; i < NTESTS; ++i)
  {
    t[i] = cpucycles();
    crypto_sign_keypair(pk, sk);
  }
  print_results("Keypair:", t, NTESTS);

  for (i = 0; i < NTESTS; ++i)
  {
    t[i] = cpucycles();
    crypto_sign(sm, &smlen, sm, CRHBYTES, sk);
  }
  print_results("Sign:", t, NTESTS);

  for (i = 0; i < NTESTS; ++i)
  {
    t[i] = cpucycles();
    crypto_sign_verify(sm, CRYPTO_EAGLESIGN_BYTES, sm, CRHBYTES, pk);
  }
  print_results("Verify:", t, NTESTS);

  return 0;
}
